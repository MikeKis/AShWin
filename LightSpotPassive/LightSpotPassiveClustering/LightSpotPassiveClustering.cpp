// LightSpotPassiveClustering.cpp : Определяет экспортированные функции для приложения DLL.
//


#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <sstream>
#include <array>

#include "../../AShWinCommon.h"

using namespace std;

size_t StartTime;
size_t OperationTime = 0xffffffff;
size_t MeasurementTime = 0xffffffff;
vector<array<double, 4> > PhaseSpacePoint;
int ExpId;
vector<array<double, 4> > NeuronCenter;
vector<size_t> vn_NeuronSpikes;
int TimeQuant = 30;

DYNAMIC_LIBRARY_ENTRY_POINT void SetParameters(int ExperimentId, size_t tactTermination, const pugi::xml_node &xn)
{
	auto starttime = xn.child("start_time");
	StartTime = 0;
	if (starttime)
		StartTime = atoi_s(xn.child_value("start_time"));
	OperationTime = atoi_s(xn.child_value("operation_time"));
	if (StartTime + OperationTime > tactTermination)
		OperationTime = tactTermination - StartTime;
	MeasurementTime = atoi_s(xn.child_value("measurement_time"));
	if (MeasurementTime >= OperationTime)
		throw std::runtime_error("invalid settings");
	TimeQuant = atoi_s(xn.child_value("time_quant"));
	size_t tact = 0;
	ifstream ifscsv("passive.csv");
	string str;
	while (getline(ifscsv, str).good()) {
		if (tact >= StartTime) {
			stringstream ss(str);
			char ch;
			array<double, 4> a;
			FORI(4) {
				ss >> a[_i];
				ss >> ch;
			}
			PhaseSpacePoint.push_back(a);
		}
		if (++tact == StartTime + OperationTime)
			break;
	}
	if (tact < StartTime + OperationTime)
		throw std::runtime_error("passive.csv - unexpected eof");
	ExpId = ExperimentId;
}

size_t tact = 0;

DYNAMIC_LIBRARY_ENTRY_POINT bool ObtainOutputSpikes(const vector<int> &v_Firing, int nEquilibriumPeriods)
{
	int j;
	if (tact >= StartTime) {
		if (tact < StartTime + OperationTime - MeasurementTime) 
			for (auto i: v_Firing) {
				if (i >= vn_NeuronSpikes.size()) {
					vn_NeuronSpikes.resize(i + 1, 0);
					NeuronCenter.resize(i + 1);
				}
				FORI(4) {
					NeuronCenter[i][_i] = (NeuronCenter[i][_i] * vn_NeuronSpikes[i] + PhaseSpacePoint[tact - StartTime][_i]) / (vn_NeuronSpikes[i] + 1);
					++vn_NeuronSpikes[i];
				}
			}
		else {
			static vector<int> vn_inQuant(vn_NeuronSpikes.size(), 0);
			static int ninQuantTotal = 0;
			for (auto i: v_Firing) {
				if (i < vn_inQuant.size())
					++vn_inQuant[i];
				++ninQuantTotal;
			}
			if ((tact - (StartTime + OperationTime - MeasurementTime)) % TimeQuant == TimeQuant - 1) {
				double ad[4] = {0., 0., 0., 0.};
				if (ninQuantTotal) {
					FOR_(j, 4) {
						ad[j] = 0.;
						FORI(vn_inQuant.size())
							ad[j] += NeuronCenter[_i][j] * vn_inQuant[_i];
					}
					FORI(4)
						ad[j] /= ninQuantTotal;
				}
				fill(vn_inQuant.begin(), vn_inQuant.end(), 0);
				ninQuantTotal = 0;
			}
		}
		auto &curtar = vvb_Target[tact];
		if (bExclusiveClassification) {
			if (CurrentTarget < 0) {
				auto j = find(curtar.begin(), curtar.end(), true);
				if (j != curtar.end()) {
					CurrentTarget = j - curtar.begin();
					vvpsmvs_FiringNeurons[CurrentTarget].push_back(pair<set<int>, map<vector<int>, set<int> > >());
				}
			}
			else if (!curtar[CurrentTarget]) {
				auto j = find(curtar.begin(), curtar.end(), true);
				if (j != curtar.end()) {
					CurrentTarget = j - curtar.begin();
					vvpsmvs_FiringNeurons[CurrentTarget].push_back(pair<set<int>, map<vector<int>, set<int> > >());
				}
				else {
					CurrentTarget = NO_PATTERN;
					vs_FiringNeuronsoutsidePatterns.push_back(set<int>());
				}
			}
			if (CurrentTarget >= 0)
				for (auto i : v_Firing)
					vvpsmvs_FiringNeurons[CurrentTarget].back().first.insert(i);
			else for (auto i : v_Firing)
				vs_FiringNeuronsoutsidePatterns.back().insert(i);
		}
		else {
			vector<int> v_curtar;
			FORI(nPatterns)
				if (curtar[_i]) {
					if (tact == MeasurementOnset || !vvb_Target[tact - 1][_i])
						vvpsmvs_FiringNeurons[_i].push_back(pair<set<int>, map<vector<int>, set<int> > >());
					++v_TotalPatternTime[_i];
					v_curtar.push_back(_i);
				}
			for (auto i : v_Firing) {
				if (vpnvn_Neuron.size() <= i)
					vpnvn_Neuron.resize(i + 1, pair<int, vector<size_t> >(0, vector<size_t>(nPatterns, 0)));
				++vpnvn_Neuron[i].first;
				if (v_curtar.size()) {
					for (auto l : v_curtar)
						++vpnvn_Neuron[i].second[l];
					if (v_curtar.size() == 1)
						vvpsmvs_FiringNeurons[v_curtar.front()].back().first.insert(i);
					else for (auto l : v_curtar)
						vvpsmvs_FiringNeurons[l].back().second[v_curtar].insert(i);
				}
			}
		}
		if (OperationTime != 0x7fffffff)
			--OperationTime;
	}
	++tact;
	return OperationTime > 0;
}

struct neuron_recognizer
{
	int Neuron;
	int Pattern;
	double dsig;
	neuron_recognizer(int neuron, int pattern, double ds) : Neuron(neuron), Pattern(pattern), dsig(ds) {}
};

inline bool operator<(const neuron_recognizer &nr1, const neuron_recognizer &nr2) { return nr1.dsig != nr2.dsig ? nr1.dsig < nr2.dsig : make_pair(nr1.Neuron, nr1.Pattern) < make_pair(nr2.Neuron, nr2.Pattern); }

DYNAMIC_LIBRARY_ENTRY_POINT int Finalize(int OriginalTerminationCode)
{
	if (OriginalTerminationCode < 0 || !OriginalTerminationCode && OperationTime)
		return OriginalTerminationCode;
	if (OperationTime > 0 && OperationTime != 0x7fffffff)
		return 0;
	vector<int> vn_Silent(nPatterns, 0);
	vector<int> vn_Ambiguous(nPatterns, 0);   // now it is when reaction to a pattern contains right as well
															 // as wrong neurons
	vector<float> vr_Recognizing(nPatterns);
	size_t nRecognizing = 0;
	size_t ntotPatterns = 0;
	vector<int> vn_Wrong(nPatterns);
	vector<double> vd_TotalEntropy(nPatterns, 0.);
	double d = 0.;
	int nTotal = 0;
	if (bExclusiveClassification) {
		int nSilent = 0;
		int nAmbiguous = 0;
		vector<int> vn_Total(nPatterns, 0);
		map<int, vector<int> > mvn_;
		map<int, int> mn_OutsideFirings;
		int target = 0;
		int n;
		for (const auto &i : vvpsmvs_FiringNeurons) {
			nTotal += i.size();
			vn_Total[target] += i.size();
			n = count_if(i.begin(), i.end(), [](const pair<set<int>, map<vector<int>, set<int> > > &psmvs_) {return psmvs_.first.empty(); });
			nSilent += n;
			vn_Silent[target] += n;
			for (const auto &j : i)
				for (auto k : j.first) {
					auto p_ = mvn_.insert(make_pair(k, vector<int>(vvpsmvs_FiringNeurons.size(), 0)));
					++p_.first->second[target];
				}
			++target;
		}
		for (const auto &j : vs_FiringNeuronsoutsidePatterns)
			for (auto k : j) {
				auto p_ = mn_OutsideFirings.insert(make_pair(k, 0));
				++p_.first->second;
			}
		map<int, int> m_NeuronTarget;
		for (const auto l : mvn_) {
			auto MostRecognizedPattern = max_element(l.second.begin(), l.second.end());
			m_NeuronTarget[l.first] = *MostRecognizedPattern > mn_OutsideFirings[l.first] ? MostRecognizedPattern - l.second.begin() : -1;
		}
		int nWrong = 0;
		vector<int> vn_OutsideFirings(nPatterns);
		set<int> s_AllRecognizingNeurons;
		FORI(nPatterns) {   // _i is pattern id
			size_t nRecognizingNeurons = 0;
			set<int> s_RecognizingNeurons;
			n = count_if(
				vvpsmvs_FiringNeurons[_i].begin(),
				vvpsmvs_FiringNeurons[_i].end(),
				[&](const pair<set<int>, map<vector<int>, set<int> > > &psmvs_) {
				nRecognizingNeurons += psmvs_.first.size();
				if (any_of(psmvs_.first.begin(), psmvs_.first.end(), [&](int neuron) {return m_NeuronTarget[neuron] == _i; })) {
					if (any_of(psmvs_.first.begin(), psmvs_.first.end(), [&](int neuron) {return m_NeuronTarget[neuron] != _i; }))
						return true;
					s_RecognizingNeurons += psmvs_.first;
				}
				return false;
			}
			);
			nAmbiguous += n;
			vn_Ambiguous[_i] = n;
			vr_Recognizing[_i] = ddiv_s(nRecognizingNeurons, vvpsmvs_FiringNeurons[_i].size());
			nRecognizing += nRecognizingNeurons;
			ntotPatterns += vvpsmvs_FiringNeurons[_i].size();
			n = count_if(
				vvpsmvs_FiringNeurons[_i].begin(),
				vvpsmvs_FiringNeurons[_i].end(),
				[&](const pair<set<int>, map<vector<int>, set<int> > > &psmvs_) {return psmvs_.first.size() && none_of(psmvs_.first.begin(), psmvs_.first.end(), [&](int neuron) {return m_NeuronTarget[neuron] == _i; }); }
			);
			nWrong += n;
			vn_Wrong[_i] = n;
			vn_OutsideFirings[_i] = count_if(vs_FiringNeuronsoutsidePatterns.begin(), vs_FiringNeuronsoutsidePatterns.end(), [&](const set<int> &s_) {return !(s_ * s_RecognizingNeurons).empty(); });
			s_AllRecognizingNeurons += s_RecognizingNeurons;
			for (auto m : s_RecognizingNeurons) {
				double dp = count_if(vvpsmvs_FiringNeurons[_i].begin(), vvpsmvs_FiringNeurons[_i].end(), [=](const pair<set<int>, map<vector<int>, set<int> > > &psmvs_) {return psmvs_.first.find(m) != psmvs_.first.end(); }) /
					(double)vvpsmvs_FiringNeurons[_i].size();
				if (dp && dp < 1.)
					vd_TotalEntropy[_i] -= dp * log(dp);
			}
			d += vd_TotalEntropy[_i] * vvpsmvs_FiringNeurons[_i].size();
		}
		n = count_if(vs_FiringNeuronsoutsidePatterns.begin(), vs_FiringNeuronsoutsidePatterns.end(), [&](const set<int> &s_) {return !(s_ * s_AllRecognizingNeurons).empty(); });
		cout <<
			"TOTAL: Silent " <<
			ddiv_s(nSilent * 100., nTotal) <<
			"% Ambiguous " <<
			ddiv_s(nAmbiguous * 100., nTotal) <<
			"% Wrong " <<
			ddiv_s(nWrong * 100., nTotal) <<
			"% mean Nrecognizing " <<
			ddiv_s(nRecognizing, ntotPatterns) <<
			" mean recognition entropy " <<
			ddiv_s(d, ntotPatterns) <<
			" False positives: " <<
			(vs_FiringNeuronsoutsidePatterns.size() ? n * 100. / vs_FiringNeuronsoutsidePatterns.size() : 0.) <<
			"%\nFor targets:\n";
		FORI(nPatterns)
			cout <<
			"TARGET " <<
			_i <<
			": Silent " <<
			ddiv_s(vn_Silent[_i] * 100., vn_Total[_i]) <<
			"% Ambiguous " <<
			ddiv_s(vn_Ambiguous[_i] * 100., vn_Total[_i]) <<
			"% Wrong " <<
			ddiv_s(vn_Wrong[_i] * 100., vn_Total[_i]) <<
			"% Nrecognizing " <<
			vr_Recognizing[_i] <<
			" recognition entropy " <<
			vd_TotalEntropy[_i] <<
			" Outside " <<
			(vs_FiringNeuronsoutsidePatterns.size() ? vn_OutsideFirings[_i] * 100. / vs_FiringNeuronsoutsidePatterns.size() : 0.) <<
			"%\n";
		int nGoodCases = nTotal - nSilent - nAmbiguous - nWrong - n;
		return nGoodCases >= 0 ? ddiv_s(nGoodCases * 10000., nTotal) : 0;
	}
	int pattern;
	vector<double> vd_ppattern(nPatterns);
	vector<set<int> > vs_RecognizingNeurons(nPatterns);
	FOR_(pattern, nPatterns) {
		vd_ppattern[pattern] = ddiv_s(v_TotalPatternTime[pattern], tact - MeasurementOnset);
		nTotal += vvpsmvs_FiringNeurons[pattern].size();
	}
	set<neuron_recognizer> snr_;
	FOR_(pattern, nPatterns)
		FORI(vpnvn_Neuron.size())
		if (vpnvn_Neuron[_i].second[pattern] > (size_t)(vpnvn_Neuron[_i].first * vd_ppattern[pattern]) + 1)
			snr_.insert(neuron_recognizer(_i, pattern, dlnBinCum(vpnvn_Neuron[_i].first - vpnvn_Neuron[_i].second[pattern], vpnvn_Neuron[_i].first, 1 - vd_ppattern[pattern])));
	struct cumulative_recognizer
	{
		size_t ntot;
		size_t ncor;
		double dsig;
		cumulative_recognizer() : dsig(0) {}
	};
	vector<cumulative_recognizer> vcr_(nPatterns);
	vector<float> vr_OutsideFiring(nPatterns, 0);
	for (const auto &n : snr_)
		if (vcr_[n.Pattern].dsig <= 0 && none_of(vs_RecognizingNeurons.begin(), vs_RecognizingNeurons.end(), [&](const set<int> &s_) {return s_.find(n.Neuron) != s_.end(); })) {
			if (!vcr_[n.Pattern].dsig) {
				vs_RecognizingNeurons[n.Pattern].insert(n.Neuron);
				vcr_[n.Pattern].dsig = n.dsig;
				vcr_[n.Pattern].ntot = vpnvn_Neuron[n.Neuron].first;
				vcr_[n.Pattern].ncor = vpnvn_Neuron[n.Neuron].second[n.Pattern];
				vr_OutsideFiring[n.Pattern] = ddiv_s(vcr_[n.Pattern].ntot - vcr_[n.Pattern].ncor, vcr_[n.Pattern].ntot);
			}
			else {
				vcr_[n.Pattern].ntot += vpnvn_Neuron[n.Neuron].first;
				vcr_[n.Pattern].ncor += vpnvn_Neuron[n.Neuron].second[n.Pattern];
				double dsignew = dlnBinCum(vcr_[n.Pattern].ntot - vcr_[n.Pattern].ncor, vcr_[n.Pattern].ntot, 1 - vd_ppattern[n.Pattern]);
				if (dsignew >= vcr_[n.Pattern].dsig)
					vcr_[n.Pattern].dsig = 1;   // stop adding recognizing neurons for this pattern
				else {
					vcr_[n.Pattern].dsig = dsignew;
					vs_RecognizingNeurons[n.Pattern].insert(n.Neuron);
					vr_OutsideFiring[n.Pattern] = ddiv_s(vcr_[n.Pattern].ntot - vcr_[n.Pattern].ncor, vcr_[n.Pattern].ntot);
				}
			}
		}
	auto bfnRecognizingNeuron = [&](int neu) {return any_of(vs_RecognizingNeurons.begin(), vs_RecognizingNeurons.end(), [=](const set<int> &s_) {return s_.find(neu) != s_.end(); }); };
	size_t nNeutralSpikes = 0;
	size_t nTotalSpikes = 0;
	FORI(vpnvn_Neuron.size()) {
		if (!bfnRecognizingNeuron(_i))
			nNeutralSpikes += vpnvn_Neuron[_i].first;
		nTotalSpikes += vpnvn_Neuron[_i].first;
	}
	size_t nRecognizingSpikes = 0;
	FOR_(pattern, nPatterns)
		for (auto k : vs_RecognizingNeurons[pattern])
			nRecognizingSpikes += vpnvn_Neuron[k].second[pattern];   // Это число не может быть завышено, так как один нейрон распознает только один паттерн
	size_t nBadSpikes = nTotalSpikes - nNeutralSpikes - nRecognizingSpikes;
	int nSilentTot = 0;
	int nAmbiguousTot = 0;
	int nWrongTot = 0;
	FOR_(pattern, nPatterns) {
		int nAmbiguous = 0;
		int nWrong = 0;
		int nRecognized = 0;
		size_t nRecognizingNeurons = 0;
		map<int, int> mn_nRecognizedPatternsbyNeuron;
		for (const auto &m : vvpsmvs_FiringNeurons[pattern]) {
			set<int> s_AllActiveNeurons = m.first;
			for (const auto &p : m.second)
				s_AllActiveNeurons += p.second;
			nRecognizingNeurons += s_AllActiveNeurons.size();
			auto s_GoodRecognizingNeurons = s_AllActiveNeurons * vs_RecognizingNeurons[pattern];
			for (auto t : s_GoodRecognizingNeurons)
				++mn_nRecognizedPatternsbyNeuron[t];
			if (s_GoodRecognizingNeurons.size()) {   // recognized by a correct neuron
				++nRecognized;
				if (any_of(m.first.begin(), m.first.end(), [&](int neu) {return s_GoodRecognizingNeurons.find(neu) == s_GoodRecognizingNeurons.end() && bfnRecognizingNeuron(neu); })) {
					++nAmbiguous;
					continue;
				}
				for (const auto &p : m.second)
					if (any_of(
						p.second.begin(),
						p.second.end(),
						[&](int neu) {
						if (s_GoodRecognizingNeurons.find(neu) == s_GoodRecognizingNeurons.end()) {
							auto s = find_if(vs_RecognizingNeurons.begin(), vs_RecognizingNeurons.end(), [=](const set<int> &s_) {return s_.find(neu) != s_.end(); });
							if (s != vs_RecognizingNeurons.end()) {
								auto p_ = equal_range(p.first.begin(), p.first.end(), s - vs_RecognizingNeurons.begin());
								if (p_.first == p_.second)
									return true;
							}
						}
						return false;
					}
					)) {
						++nAmbiguous;
						break;
					}
			}
			else {
				if (any_of(m.first.begin(), m.first.end(), [&](int neu) {return bfnRecognizingNeuron(neu); })) {
					++nWrong;
					continue;
				}
				for (const auto &p : m.second)
					if (any_of(
						p.second.begin(),
						p.second.end(),
						[&](int neu) {
						auto s = find_if(vs_RecognizingNeurons.begin(), vs_RecognizingNeurons.end(), [=](const set<int> &s_) {return s_.find(neu) != s_.end(); });
						if (s != vs_RecognizingNeurons.end()) {
							auto p_ = equal_range(p.first.begin(), p.first.end(), s - vs_RecognizingNeurons.begin());
							if (p_.first == p_.second)
								return true;
						}
						return false;
					}
					)) {
						++nWrong;
						break;
					}
			}
		}
		int nRecognizedCorrectly = nRecognized - nAmbiguous;
		int nSilent = (int)vvpsmvs_FiringNeurons[pattern].size() - nRecognized - nWrong;
		vn_Silent[pattern] = nSilent;
		nSilentTot += nSilent;
		vn_Ambiguous[pattern] = nAmbiguous;
		nAmbiguousTot += nAmbiguous;
		vr_Recognizing[pattern] = ddiv_s(nRecognizingNeurons, vvpsmvs_FiringNeurons[pattern].size());
		nRecognizing += nRecognizingNeurons;
		ntotPatterns += vvpsmvs_FiringNeurons[pattern].size();
		vn_Wrong[pattern] = nWrong;
		nWrongTot += nWrong;
		for (auto m : mn_nRecognizedPatternsbyNeuron) {
			double dp = ddiv_s(m.second, vvpsmvs_FiringNeurons[pattern].size());
			if (dp && dp < 1.)
				vd_TotalEntropy[pattern] -= dp * log(dp);
		}
		d += vd_TotalEntropy[pattern] * vvpsmvs_FiringNeurons[pattern].size();
	}
	if (!nTotal || nTotalSpikes == nNeutralSpikes)
		return 0;
	cout <<
		"TOTAL: Silent " <<
		ddiv_s(nSilentTot * 100., nTotal) <<
		"% Ambiguous " <<
		ddiv_s(nAmbiguousTot * 100., nTotal) <<
		"% Wrong " <<
		ddiv_s(nWrongTot * 100., nTotal) <<
		"% mean Nrecognizing " <<
		ddiv_s(nRecognizing, ntotPatterns) <<
		" mean recognition entropy " <<
		ddiv_s(d, ntotPatterns) <<
		" Neutral spikes: " <<
		ddiv_s(nNeutralSpikes * 100., nTotalSpikes) <<
		"% Bad spikes: " <<
		ddiv_s(nBadSpikes * 100., nTotalSpikes) <<
		"%\nFor targets:\n";
	FORI(nPatterns)
		cout <<
		"TARGET " <<
		_i <<
		": Silent " <<
		ddiv_s(vn_Silent[_i] * 100., vvpsmvs_FiringNeurons[_i].size()) <<
		"% Ambiguous " <<
		ddiv_s(vn_Ambiguous[_i] * 100., vvpsmvs_FiringNeurons[_i].size()) <<
		"% Wrong " <<
		ddiv_s(vn_Wrong[_i] * 100., vvpsmvs_FiringNeurons[_i].size()) <<
		"% Nrecognizing " <<
		vr_Recognizing[_i] <<
		" recognition entropy " <<
		vd_TotalEntropy[_i] <<
		"\n";
	float rGoodCasesPart = ddiv_s(nTotal - nSilentTot - nAmbiguousTot - nWrongTot, nTotal);
	float rGoodSpikesPart = ddiv_s(nRecognizingSpikes, nTotalSpikes - nNeutralSpikes);
	return rGoodCasesPart && rGoodSpikesPart ? 20000. / (1. / rGoodCasesPart + 1. / rGoodSpikesPart) : 0.;
}


