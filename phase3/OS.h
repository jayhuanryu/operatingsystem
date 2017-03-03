#ifndef OS_H
#define OS_H

#include "Assembler.h"
#include "VirtualMachine.h"
#include "PCB.h"
#include <iostream>
#include <list>
#include <string>
#include <cstring>
#include <queue>
#include <fstream>

using namespace std;

class OS {
	private:
		//Private variables
		Assembler as;
		VirtualMachine vm;
		list <PCB *>jobs;
		queue<PCB *>readyQ;
		queue<PCB *>waitQ;
		PCB * running;
		int os_clock;
		int context_switch_clock;
		int idle_clock;
		int multi_prog;
		int page_replace;
		list<int> unusedFrames;
		fstream readProgs;
		bool lessMultiProg;
		//int pcb_limit;

		//Private functions
		void getPCBfromVM();
		void givePCBtoVM();
		void outputErrorCode(string);
		bool done();
		void readIn(string);
		void replaceAlgorithm();
		

	public:
		//constructor
		OS();
		//function
		void start(int);
};

#endif
