#ifndef PCB_H
#define PCB_H

#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

class PCB {

private:
	static const int REG_FILE_SIZE = 4;
	int pc;
	int sr;
	int sp;
	int base;
	int limit;
	vector<int> reg;
	vector<int> data;

	string originalfilename;
	string readfilename;
	string writefilename;
	string stfilename;
	string filename;

	string state;

	fstream readIntoRegister;
	// fstream writeToRegister;

	int cpuTime;
	int waitingTime;
	int turnaroundTime;
	int contextSwitchTime;
	int idleTime;
	int ioTime;
	int largestStackSize;
	int interruptTime;

public: 
	PCB();
	~PCB();
	void openReadInFileStream();
	void openWriteOutFileStream();
	void writedata(int);

	
	friend class OS;
};

#endif