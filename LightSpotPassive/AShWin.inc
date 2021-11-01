//#define USE_CPU
#define EMULATOR "ArNIGPU"

#define TESTING_PERIOD 1000000
#define MAXEMULATIONTIME 3000000
#define MEASUREMENT_PERIOD 400000
#define INPUT_SIGNAL_FILE "passive.rec"
#define NRECEPTORS 1200
#define RECOGNITION_MEASUREMENT_QUANTUM 40
#define NTESTS 3
#define MAXNNEU 100

#define SECOND_TEST_THRESHOLD 1000000
#define RECOGNIZER   // it should be retained.

CHROMOSOME_DEFINITION
{
	INITIALIZE_GENE(CHARTIME, rng.logUniform(3, 30)),
	INITIALIZE_GENE(RELWEIGHTDEC, rng.logUniform(0.001, 0.3)),
	INITIALIZE_GENE(ININON0WEIGHTPART, rng.logUniform(0.03, 0.3)), 
	INITIALIZE_GENE(MAXWEIGHT, rng.logUniform(1., 10.)),  
	INITIALIZE_GENE(INIWEIGHT, rng.logUniform(0.3, 10.)),
	INITIALIZE_GENE(WEIGHTINH, rng.logUniform(5., 100.)),
	INITIALIZE_GENE(NNEU, rng.logUniform(10, MAXNNEU)),
	INITIALIZE_GENE(INACTIVESTRONGSYNAPSESUPPRESSION, rng(1.3)),
	INITIALIZE_GENE(PLASTICITYCHARTIMERATIO, 0.3 + rng(2.7)),
	INITIALIZE_GENE(PAFFERENT, 0.1 + rng(0.7))
};

#define NO_GENE_DEPENDENCIES

#ifndef FOR_LINUX
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#endif

// Check that the given gene combination makes sense

bool Chromosome::bNoFatalDefect(void) const 
{
	if (GV(MAXWEIGHT) < GV(INIWEIGHT))
		return false;
	return true;
}

int StructuralCriterion(int){return 0;}   // not used

double devaluate(int ind, const VECTOR<PAIR<float,VECTOR<PAIR<int,bool> > > > &vprvpindb_, double dConnectivityFactor = 1.)
{
	int a, SelectedDevice;

// Create network configuration file and storing its contents in log file

    SETNNP(".." FOLDER_SEPARATOR "Experiments" FOLDER_SEPARATOR "WTAabsNoauxSpot.nnp");
	SUBMACRO(INPUT_SIGNAL_FILE);
	SUBMACRO(NRECEPTORS);
	SUBMACRO(TESTING_PERIOD);
	SUBMACRO(MEASUREMENT_PERIOD);
	SUBVALUE("LEARNING_PERIOD", MAXEMULATIONTIME - TESTING_PERIOD);
	SUBGENEINT(NNEU);
	SUBGENE(RELWEIGHTDEC);
	SUBGENE(WEIGHTINH);
	SUBGENE(ININON0WEIGHTPART);
	float rAfferentIniWeight, rAfferentMaxWeight = 0;
	double dAfferentEffWeight = GV(INIWEIGHT);
	rAfferentMaxWeight = GV(MAXWEIGHT);
	rAfferentIniWeight = dAfferentEffWeight * rAfferentMaxWeight / (rAfferentMaxWeight - dAfferentEffWeight);
	SUBVALUE("INIWEIGHT", rAfferentIniWeight);
	SUBGENE(MAXWEIGHT);
	SUBGENEINT(CHARTIME);
	SUBGENE(INACTIVESTRONGSYNAPSESUPPRESSION);
	SUBGENE(PLASTICITYCHARTIMERATIO);
    char buf[3000];
    sprintf(buf, ".." FOLDER_SEPARATOR "Experiments" FOLDER_SEPARATOR "%d.nnc", 1000 + ind);
	ofstream ofs(buf);
	SAVENNC(ofs);
	ofs.close();
	mutLog.lock();
	ofsTaskLog << "-------------------------------\n";
	SAVENNC(ofsTaskLog);
	ofsTaskLog << "-------------------------------\n";
	mutLog.unlock();
	stringstream ssNNC;
	SAVENNC(ssNNC);
	NNCDONE;

	// Run simulation

	sprintf(buf, "monitoring.%d.csv", 1000 + ind);
	string strMonitoringFile(buf);
    sprintf(buf, "-e%d", 1000 + ind);
    int ntrials = 0;
	boost::this_thread::sleep(boost::posix_time::milliseconds((ind % 101) * 300));
	int sum;
	vector<string> vstr_argsEmulator;
	do {
		do {
			mutsoc.lock();
			SelectedDevice = min_element(vpn_ComputingDeviceStatus.begin(), vpn_ComputingDeviceStatus.end()) - vpn_ComputingDeviceStatus.begin();
			++vpn_ComputingDeviceStatus[SelectedDevice].first;
			mutsoc.unlock();
			mutLog.lock();
			ofsTaskLog << "ind" << ind << " - " << EMULATOR << " started at " << boost::posix_time::second_clock::local_time() << std::endl;
			mutLog.unlock();
			sprintf(buf + 300, "-T%d", MAXEMULATIONTIME);
			vstr_argsEmulator.resize(9);
			vstr_argsEmulator[0] = ".." FOLDER_SEPARATOR "Experiments";
			vstr_argsEmulator[1] = buf;
			vstr_argsEmulator[2] = buf + 300;
			vstr_argsEmulator[3] = "-S100000";
			vstr_argsEmulator[4] = "-R";
			vstr_argsEmulator[5] = "-dS30000";
			vstr_argsEmulator[6] = "-D100000";
			vstr_argsEmulator[7] = "-n3";
			vstr_argsEmulator[8] = "-t0-" + str(MAXEMULATIONTIME);
#ifndef USE_CPU
			sprintf(buf + 100, "-C%d", SelectedDevice);
			vstr_argsEmulator.push_back(buf + 100);
#endif

			sum = 0;
			FORI(NTESTS) {
				a = ExecuteProcess(EMULATOR, vstr_argsEmulator, ofsTaskLog);
				if (a < 0) {
					printf("\a\a\a Abnormal " EMULATOR " termination -- trying to restart few minutes later!!!\n");
					mutLog.lock();
					ofsTaskLog << "ind" << ind << " abnormal " EMULATOR " termination -- trying to restart (" << boost::posix_time::second_clock::local_time() << ")\n";
					mutLog.unlock();
					break;
				}
				if (a >= SHOULD_BE_LAUNCHED_ON_ANOTHER_NODE)
					break;
				sum += a;
			}
			mutsoc.lock();
			--vpn_ComputingDeviceStatus[SelectedDevice].first;
			mutsoc.unlock();
			mutLog.lock();
			ofsTaskLog << "ind" << ind << " - " << EMULATOR << " finished at " << boost::posix_time::second_clock::local_time() <<  " returned  " << a << std::endl;
			mutLog.unlock();
			if (a >= 0)
				break;
			boost::this_thread::sleep(boost::posix_time::seconds(200));
			ntrials++;
		} while (ntrials < 30);
		if (ntrials == 30) 
			do {
				printf("CANNOT SPAWN!\n");
				boost::this_thread::sleep(boost::posix_time::seconds(10));
			} while (true);
		if (a < SHOULD_BE_LAUNCHED_ON_ANOTHER_NODE) {
			remove(strMonitoringFile.c_str());
			sprintf(buf, "spikes.%d.bin", 1000 + ind);
			string strProtocolFile(buf);
			remove(strProtocolFile.c_str());
			sprintf(buf, "%d.finsave.nns", 1000 + ind);
			remove(buf);
			pnTasksDoneonCard[SelectedDevice]++;
			return(sum / NTESTS);
		}
		mutsoc.lock();
		++vpn_ComputingDeviceStatus[SelectedDevice].first;
		mutsoc.unlock();
		string strLog;
		time_t tim = time(NULL);
		ctime_s(buf + 200, 100, &tim);
		mutLog.lock();
		ofsTaskLog << "ind" << ind << " - task sent to another cluster node at " << boost::posix_time::second_clock::local_time() << " (device " << SelectedDevice << ")" << endl;
		mutLog.unlock();
		int ret = CoordinateCluster(a - SHOULD_BE_LAUNCHED_ON_ANOTHER_NODE, SelectedDevice, ind, vstr_argsEmulator, ssNNC.str(), strLog, pnTasksDoneonCard);
		if (strLog.length()) {
			mutsoc.lock();
			--vpn_ComputingDeviceStatus[SelectedDevice].first;
			mutsoc.unlock();
			mutLog.lock();
			ofsTaskLog << "ind " << ind << " result: " << ret << endl;
			ofsTaskLog << strLog << "\n-------------------\n";
			pnTasksDoneonCard[SelectedDevice]++;
			mutLog.unlock();
			return ret;
		}
		mutLog.lock();
		ofsTaskLog << "ind " << ind << " -- too long waiting for result -- restarting on another device\n";
		mutLog.unlock();
	} while (true);
}

double Chromosome::operator()(int ind) const {return devaluate(ind, vprvpindb_);}

void Chromosome::ApplyAdditionalCriteria(VECTOR<double> &vd_AdditionalCriterionValues) const 
{
}
