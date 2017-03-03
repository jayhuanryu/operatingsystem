#include "PCB.h"

using namespace std;

PCB::PCB() {
	reg = vector<int> (REG_FILE_SIZE);
	frames = vector<int> (FRAME_COUNT);
	valid = vector<int> (FRAME_COUNT);
	modified = vector <int> (FRAME_COUNT);

	vector<int> data;
	pc = 0;
	sr = 0;
	sp = 256;
	base = 0;
	limit = 0;
	
	state = "created";

	for (int i = 0; i < REG_FILE_SIZE; i++) {
		reg[i] = 0;
	}

	for(int i = 0; i < FRAME_COUNT; i++) {
		frames[i] = 0;
		valid[i] = 0;
		modified[i] = 0;
	}

	cpuTime = 0;
	waitingTime = 0;
	turnaroundTime = 0;
	contextSwitchTime = 0;
	idleTime = 0;
	ioTime = 0;
	largestStackSize = 0;

}

PCB::~PCB() {
	readIntoRegister.close();
	writeToRegister.close();
}

void PCB::openReadInFileStream() {
	readIntoRegister.open(readfilename.c_str(), ios::in);

	if(!readIntoRegister.is_open()) {
		cout << readfilename << " failed to open" << endl;
		exit(2);
	}
}

void PCB::openWriteOutFileStream() {
	writeToRegister.open(writefilename.c_str(), ios::out);

	if(!writeToRegister.is_open()) {
		cout << writefilename << "failed to open" << endl;
		exit(2);
	}
}
/*
void PCB::writedata(int input) {
	data.push_back(input);
}
*/


