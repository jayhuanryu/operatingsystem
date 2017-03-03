#include "OS.h"

OS::OS()
{
	//initial value of each clock
	os_clock = 0;
	context_switch_clock = 0;
	idle_clock = 0;
}

void OS::start()
{
	//read in all files

	system("ls *.s > progs");

	fstream readProgs;
	readProgs.open("progs", ios::in);

	if(!readProgs.is_open()) {
		cout << "Failed to open progs" << endl;
		exit(2);
	}

	string line;
	getline(readProgs, line);
	int it = 0;
	while(!readProgs.eof()) {
		line = line.substr(0, line.size()-2);
		as.assemble(line); // call assembler to .s file to .o file

		//create PCB
		PCB * p = new PCB;
		//list all jobs into list
		jobs.push_back(p);
		//initially all the processes goes into readyQ
		readyQ.push(p); 
		p->state = "ready";

		
		//sets the file name
		string filename = "";
		filename = line;

		p->originalfilename = filename + ".o";
		p->readfilename = filename + ".in";
		p->writefilename = filename + ".out";
		p->stfilename = filename + ".st";
		p->filename = filename;

		//open input and outputfile streams in PCB
		p->openReadInFileStream();
		p->openWriteOutFileStream();

		//read into memory, modify PCB with values for base and limit
		int base = 0;
		int limit = 0;

		vm.ReadMemory(p->originalfilename, &base, &limit);

		p->base = base;
		p->limit = limit;
		p->pc = base;

		getline(readProgs, line);
	} //finish reading into memory

	string memorycheck = "mem";
	fstream printmem;
	printmem.open(memorycheck.c_str(), ios::out);
	for (int i = 0 ; i < vm.mem.size(); i++) {
		printmem << vm.mem[i] << endl;
	}
	//close file and delete the progs filename
	readProgs.close();
	cout << "\t\tremoving progs\n\n" << endl;
	system("rm progs");

	//copy first item in readyQ to running, load pcb data to vm

	running = readyQ.front();
	readyQ.pop();
	givePCBtoVM();

	int return_status;

	//main process
	while(!done()) {
		return_status = 0;

		vm.Execute(running->filename);

		//get return status from status register
		return_status = vm.sr;
		return_status = return_status >> 5;
		return_status = return_status & 7; 

		//manage clock for context switch
		os_clock += 5;
		context_switch_clock += 5;

			//update other processes
		for(list<PCB *>::iterator it = jobs.begin(); it != jobs.end(); it++) {
			if(!strcmp((*it)->state.c_str(), "terminated")) {
				(*it)->contextSwitchTime += 5;
			}	
		}

		//move item from waitQ to readyQ if interrupt time hit
		if(!waitQ.empty()) {
			while (waitQ.front()->interruptTime <= os_clock) {
				readyQ.push(waitQ.front());
				waitQ.pop();
				readyQ.back()->state = "ready";
				if(waitQ.empty()) {
					break;
				}
			}
		}

		//updating waiting time for all items in readyQ
		for(list<PCB *>::iterator it = jobs.begin(); it != jobs.end(); it++) {
			if(strcmp((*it)->state.c_str(), "ready"))
				(*it)->waitingTime += vm.clo;
		}

		if(!(running == NULL)) {

			//update os clock and cpu time
			os_clock += vm.clo;
			running->cpuTime += vm.clo;

			switch(return_status) {
				case 0: //time slice
					getPCBfromVM();
					readyQ.push(running);
					break;
				case 1: // Halt Instruction
					running->state = "terminated";
					break;
				case 2: // Out of Bound
					running->state = "terminated";
					outputErrorCode("Out of Bound reference");
					break;
				case 3: // Stack Overflow
					running->state = "terminated";
					outputErrorCode("Stack Overflow");
					break;
				case 4: // Stack Underflow
					running->state = "terminated";
					outputErrorCode("Stack Underflow");
					break;
				case 5: // Invalid Opcode
					running->state = "terminated";
					outputErrorCode("Invalid Opcode");
					break;
				case 6: {// I/O Operation(Read)
					int readIN = 0;
					running->readIntoRegister >> readIN;

					int readRegister = vm.sr;

					readRegister = readRegister >> 8;
					readRegister = readRegister & 3;

					if (readRegister < 0 || readRegister > 3) {
						outputErrorCode("Invalid IO Register");
						break;
					}

					vm.r[readRegister] = readIN;

					os_clock++;
					running->ioTime += 27;
					running->cpuTime++;
					waitQ.push(running);
					running->state = "waiting";
					getPCBfromVM();
					running->interruptTime = os_clock + 28;

					break;
				}
				case 7: {// I/O operation(Write)

					os_clock++;
					running->ioTime += 27;
					running->cpuTime++;

					waitQ.push(running);
					running->state = "waiting";
					getPCBfromVM();
					running->interruptTime = os_clock + 28;

					break;
				}
				default:
					running->state = "terminated";
					outputErrorCode("Invalid Return Status");
					break;
			}

			running = NULL;
		} // end if running == NULL

		//if all item in waitQ, update os_clock to ge tfirst item of waitQ
		if(readyQ.empty() && !waitQ.empty()) {

			int temp = waitQ.front()->interruptTime - os_clock;

			os_clock += temp;
			idle_clock += temp;
			readyQ.push(waitQ.front());
			waitQ.front()->state = "ready";
			waitQ.pop();

			//update idle time for non terminated processes in the joblist
			for(list<PCB *>::iterator it = jobs.begin(); it != jobs.end(); it++) {
				if(!strcmp((*it)->state.c_str(), "terminated")) {
					(*it)->idleTime += temp;
				}
			}
		}

		//move next process in the readyQ into running state
		if (!readyQ.empty()) {
			running = readyQ.front();
			running->state = "running";
			readyQ.pop();
			givePCBtoVM();
		}
	}


	
	//run program accounting data

	//run system accounting data

	int SystemTime = context_switch_clock + idle_clock;
	double floatingpointOs_clock = os_clock;	
	double cpuUtil = (floatingpointOs_clock - idle_clock) / floatingpointOs_clock;	
	int sum_cpu_time = 0;

	//calculate turnaround time for all processes, and all
	for(list<PCB *>::iterator it = jobs.begin(); it != jobs.end(); it++) {
		(*it)->turnaroundTime = (*it)->cpuTime + (*it)->waitingTime + (*it)->ioTime + (*it)->idleTime + (*it)->contextSwitchTime; 
		sum_cpu_time += (*it)->cpuTime;
	}

	double userCPUUtilization = sum_cpu_time / floatingpointOs_clock;
	double throughput = jobs.size() / (floatingpointOs_clock / 1000);

	//iterate through pcb list and output throughput and vm utilization to all files
	for (list<PCB *>::iterator it = jobs.begin(); it != jobs.end(); it++)  {
		
		fstream Accouting;
		Accouting.open((*it)->writefilename.c_str(), ios::ate | ios::app);

		if (!Accouting.is_open()) {
			cout << (*it)->writefilename << " Failed to Open." << endl;
			exit(1);
		}

		
		Accouting << "Process Accounting Data:" << endl;
		Accouting << "CPU Time = " << (*it)->cpuTime << " cycles"<< endl;
		Accouting << "Waiting Time = " << (*it)->waitingTime << " cycles" << endl;
		Accouting << "Turnaround Time = " << (*it)->turnaroundTime << " cycles" << endl;
		Accouting << "I/O Time = " << (*it)->ioTime << " cycles" << endl;
		Accouting << "Largest Stack Size = " << (*it)->largestStackSize << endl;

		Accouting << "System Accounting Data: " << endl;
		Accouting << "System Time = " << SystemTime << " cycles" << endl;
		Accouting << "System CPU Utilization(%) = " << cpuUtil * 100 << endl;
		Accouting << "User CPU Utilization(%) = " << userCPUUtilization * 100 << endl; 

		Accouting.close();

	}

	//delete .st file

	system("rm *.st");

}

void OS::givePCBtoVM() {
	//copy contents of PCB running into VM, including stack info

	vm.pc = running->pc;
	vm.sr = running->sr;
	vm.sp = running->sp;
	vm.base = running->base;
	vm.limit = running->limit;

	//assigning values into register
	for(int i=0; i < 4; i++ ) {
		vm.r[i] = running->reg[i];
	}

	//retrieving stack values if there is any
	if(running->sp < 256) {
			fstream readProgs;
			readProgs.open(running->stfilename.c_str(), ios::in);

			if(!readProgs.is_open()) {
				cout << running->stfilename << "failed to open." << endl;
				exit(1);
			}

			int stackPointer = running->sp;

			string line;
			getline(readProgs, line);

			while (!readProgs.eof()) {
				vm.mem[stackPointer++] = atoi(line.c_str()); // change string values into integer
				getline(readProgs, line);
			}
	}
}

void OS::getPCBfromVM() {
	//copy contents of VM into running PCB, includeing stack info
	running->pc = vm.pc;
	running->sr = vm.sr;
	running->sp = vm.sp;
	running->base = vm.base;
	running->limit = vm.limit;

	//assigning registervalues to pcb
	for(int i = 0 ; i < 4; i++) {
		running->reg[i] = vm.r[i];
	}

	//retrieving the vm stack values to running pcb
	if(vm.sp < 256) {
		fstream readProgs;
		readProgs.open(running->stfilename.c_str(), ios::out);

		if(!readProgs.is_open()) {
			cout << running->stfilename << "failed to open" << endl;
			exit(2);
		}

		int stackPointer = running->sp;

		int temp = 256 - stackPointer;

		if(running->largestStackSize < temp)
			running->largestStackSize = temp;

		while (stackPointer < 256) {
			readProgs<<vm.mem[stackPointer++] << endl;
		}
	}
}

void OS::outputErrorCode(string code){
	fstream errorcodeOut;
	errorcodeOut.open(running->writefilename.c_str(), ios::in | ios::out);

	errorcodeOut << "Error Code = " << code << endl;
	errorcodeOut.close();
}

bool OS::done() {
	if(readyQ.empty() && waitQ.empty() && !running) {
		return true;
	}
	return false;
}



//main function
int main() {
	OS os;
	os.start();
}
