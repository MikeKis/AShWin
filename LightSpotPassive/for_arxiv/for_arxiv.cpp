// for_arxiv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <math.h>
#include <sstream>
#include <boost/filesystem.hpp>

#include "../../AShWinCommon.h"

#include "../LightSpotPassive.h"

#define DECLARE_UNION_OPERATORS(type) inline type operator+(const type &s_1,const type &s_2){type s_ret;set_union(s_1.begin(),s_1.end(),s_2.begin(),s_2.end(),inserter(s_ret,s_ret.begin()),s_ret.key_comp());return(s_ret);}inline void operator+=(type &s_To,const type &s_From){for(type::const_iterator j=s_From.begin();j!=s_From.end();j++) s_To.insert(*j);}

const size_t EtalonHistoryLength = 100000000;
const size_t TestHistoryLength = 1000000;
const unsigned SamplingPeriod = 30;
const unsigned nTests = 30;

const int nStates = 2500;
const int nASSperColumn = 3;

using namespace std;

typedef pair<size_t, size_t> state_transition;

DECLARE_UNION_OPERATORS(set<state_transition>)

template<class T> void TiedRank(const VECTOR<T> &v_, VECTOR<double> &vd_TiedRank)
{
	auto v_copy = v_;
	sort(v_copy.begin(), v_copy.end());
	MAP<T, double> m_transrank;
	size_t i = 0;
	while (i < v_copy.size()) {
		auto j = i + 1;
		auto cur = v_copy[i];
		while (j < v_copy.size() && v_copy[j] == cur)
			++j;
		m_transrank[cur] = 1 + i + (j - i - 1) * 0.5;
		i = j;
	}
	vd_TiedRank.resize(v_.size());
	FORI(v_.size())
		vd_TiedRank[_i] = m_transrank[v_[_i]];

}

template<class T1, class T2> double dSpearman(const VECTOR<T1> &v_1, const VECTOR<T2> &v_2)
{

	if (v_1.size() != v_2.size())
		throw std::runtime_error("correlation of unequal size vectors");

	VECTOR<double> vd_Rank1, vd_Rank2;
	TiedRank(v_1, vd_Rank1);
	TiedRank(v_2, vd_Rank2);

	double dMeanRank = (v_1.size() + 1) * 0.5;
	double d = 0.;
	double d1 = 0.;
	double d2 = 0.;
	FORI(v_1.size()) {
		double d3 = vd_Rank1[_i] - dMeanRank;
		double d4 = vd_Rank2[_i] - dMeanRank;
		d += d3 * d4;
		d1 += d3 * d3;
		d2 += d4 * d4;
	}

	if (!d1 || !d2)
		return 0.;

	return d / sqrt(d1 * d2);

}

template<class STL_CONTAINER, class F> void mean_stddev(const STL_CONTAINER &a, F &mean, F &stddev)
{
	double dmean = 0.;
	for (auto i : a)
		dmean += i;
	dmean /= a.size();
	mean = (F)dmean;
	if (a.size() <= 1) {
		stddev = -1;
		return;
	}
	double dstddev = 0.;
	for (auto i: a)
		dstddev += (i - dmean) * (i - dmean);
	dstddev /= a.size() - 1;
	stddev = (F)sqrt(dstddev);
}

void GetTransitionCounts(LightSpot &ls, size_t TestLength, map<state_transition, size_t> &mstn_)
{
	size_t OldState;
	FORI(TestLength) {
		++ls;
		if (!(_i % SamplingPeriod)) {
			size_t NewState = ls.indGetCurrentState();
			if (_i && NewState != OldState)
				++mstn_[make_pair(OldState, NewState)];
			OldState = NewState;
		}
		if (!(_i % 1000000))
			cout << _i << endl;
	}
}

int main(int ARGC, char *ARGV[])
{
	string str;
	map<state_transition, size_t> mstn_Etalon;
	char ch;
	LightSpot ls;
	ofstream ofs;
	ifstream ifstheor;
	vector<map<state_transition, size_t> > vmstn_theor;
	vector<map<state_transition, double> > vmstd_exp;
	ifstream ifs;
	vector<vector<pair<double, size_t> > > vvpdind_(nStates);

	namespace fs = boost::filesystem;

	vector<fs::path> vpat_;
	string strpath("." FOLDER_SEPARATOR);

	vector<size_t> vn_Etalon;
	vector<vector<size_t> > vvn_theor;
	vector<vector<double> > vvd_exp;

	vector<double> vd_theorSpearman, vd_expSpearman;

	double dtheorSpearmanMean, dtheorSpearmanStddev, dexpSpearmanMean, dexpSpearmanStddev;

	vector<map<size_t, size_t> > vmindn_EtalonTransitions_to(nStates);

	unsigned o;

	switch (ARGC) {

		case 1: GetTransitionCounts(ls, EtalonHistoryLength, mstn_Etalon);
				ofs.open("for_arxiv.csv");
				for (const auto &j: mstn_Etalon) {
					ofs << j.first.first << ',' << j.first.second << ',' << j.second << endl;
					vmindn_EtalonTransitions_to[j.first.second][j.first.first] = j.second;
				}
				FOR_(o, nTests) {
					cout << "test " << o << endl;
					map<state_transition, size_t> mstn_;
					GetTransitionCounts(ls, TestHistoryLength, mstn_);
					vector<map<size_t, size_t> > vmindn_(nStates);
					for (const auto &j: mstn_) 
						vmindn_[j.first.second][j.first.first] = j.second;
					FORI(nStates) {
						set<size_t> sind_fromStates;
						vector<size_t> vn_Etalon, vn_;
						for (const auto &p: vmindn_EtalonTransitions_to[_i])
							if (p.second)
								sind_fromStates.insert(p.first);
						for (const auto &p: vmindn_[_i])
							sind_fromStates.insert(p.first);
						for (auto q: sind_fromStates) {
							vn_Etalon.push_back(vmindn_EtalonTransitions_to[_i][q]);
							vn_.push_back(vmindn_[_i][q]);
						}
						vd_theorSpearman.push_back(dSpearman(vn_Etalon, vn_));
					}
				}

				mean_stddev(vd_theorSpearman, dtheorSpearmanMean, dtheorSpearmanStddev);

				cout << "theor:\tmean\t" << dtheorSpearmanMean << "\tstddev\t" << dtheorSpearmanStddev << "\n";

				break;

		case 2: ifstheor.open("for_arxiv.csv");

				while (getline(ifstheor, str).good()) {
					stringstream ss(str);
					pair<state_transition, size_t> pstn_;
					ss >> pstn_.first.first >> ch >> pstn_.first.second >> ch >> pstn_.second;
					mstn_Etalon.insert(pstn_);
				}

				for (const auto &j: mstn_Etalon)
					vmindn_EtalonTransitions_to[j.first.second][j.first.first] = j.second;

				vpat_ = vector<fs::path>(fs::recursive_directory_iterator(fs::path((strpath + ARGV[1]).c_str())), fs::recursive_directory_iterator());

				for (const auto &path: vpat_) 
					if (!fs::is_directory(path)) {
						ifstream ifsexp(path.c_str());
						vector<map<size_t, double> > vmindd_(nStates);
						while (getline(ifsexp, str).good()) {
							stringstream ss(str);
							size_t fromState, toState;
							double dW;
							ss >> toState >> ch >> fromState >> ch >> dW;
							vmindd_[toState][fromState] = dW;
						}
						FORI(nStates) {
							set<size_t> sind_fromStates;
							vector<size_t> vn_Etalon;
							vector<double> vd_;
							for (const auto &p: vmindn_EtalonTransitions_to[_i])
								if (p.second)
									sind_fromStates.insert(p.first);
							for (const auto &v: vmindd_[_i])
								sind_fromStates.insert(v.first);
							for (auto q: sind_fromStates) {
								vn_Etalon.push_back(vmindn_EtalonTransitions_to[_i][q]);
								vd_.push_back(vmindd_[_i][q]);
							}
							vd_expSpearman.push_back(dSpearman(vn_Etalon, vd_));
						}
					}

				mean_stddev(vd_expSpearman, dexpSpearmanMean, dexpSpearmanStddev);

				cout << "exp:\tmean\t" << dexpSpearmanMean << "\tstddev\t" << dexpSpearmanStddev << "\n";

				break;

		case 3: ifs.open(ARGV[1]);
				while (getline(ifs, str).good()) {
					if (str.substr(0, 12) == "lin," + tostr(TestHistoryLength) + ",") {
						stringstream ss(str.substr(12));
						int indpostneuron;
						ss >> indpostneuron;
						if (2 * nStates <= indpostneuron && indpostneuron < (2 + nASSperColumn) * nStates) {
							int LinkType;
							ss >> ch >> LinkType;
							if (!LinkType) {
								float r;
								double dW;
								int indpreneuron;
								ss >> ch >> LinkType >> ch >> r >> ch >> r >> ch >> LinkType >> ch >> LinkType >> ch >> indpreneuron >> ch >> r >> ch >> r >> ch >> dW;
								int StateTo = (indpostneuron - 2 * nStates) / nASSperColumn;
								int StateFrom = -1 - indpreneuron - nStates;
								vvpdind_[StateTo].push_back(make_pair(dW, StateFrom));
							}
						}
					}
				}

				ofs.open(ARGV[2]);
				for (size_t t = 0; t < nStates; ++t)
					for(const auto &w: vvpdind_[t])
						if (w.first > 1.71759)
							ofs << t << ',' << w.second << ',' << w.first << endl;

				break;

	}

	return 0;
}
