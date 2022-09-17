// for_arxiv.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <math.h>

#include "../../AShWinCommon.h"

#include "../LightSpotPassive.h"

const size_t EtalonHistoryLength = 100000000;
const size_t TestHistoryLength = 1000000;
const unsigned SamplingPeriod = 7;
const unsigned nTests = 30;

using namespace std;

void GetTransitionCounts(LightSpot &ls, size_t TestLength, map<pair<size_t, size_t>, size_t> &mpindindn_)
{
	size_t OldState;
	FORI(TestLength) {
		++ls;
		if (!(_i % SamplingPeriod)) {
			size_t NewState = ls.indGetCurrentState();
			if (_i && NewState != OldState)
				++mpindindn_[make_pair(OldState, NewState)];
			OldState = NewState;
		}
		if (!(_i % 1000000))
			cout << _i << endl;
	}
}

int main()
{
	LightSpot ls;
	map<pair<size_t, size_t>, size_t> mpindindn_Etalon;
	GetTransitionCounts(ls, EtalonHistoryLength, mpindindn_Etalon);
	ofstream ofs("for_arxiv.csv");
	for (const auto &j: mpindindn_Etalon)
		ofs << "-1," << j.first.first << ',' << j.first.second << ',' << j.second << endl;
	FORI(nTests) {
		cout << "test " << _i << endl;
		map<pair<size_t, size_t>, size_t> mpindindn_;
		GetTransitionCounts(ls, TestHistoryLength, mpindindn_);
		for (const auto &j : mpindindn_)
			ofs << _i << "," << j.first.first << ',' << j.first.second << ',' << j.second << endl;
	}
}
