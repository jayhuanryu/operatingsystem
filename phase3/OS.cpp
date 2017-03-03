#include "OS.h"

OS::OS()
{
	//initial value of each clock
	//os_clock = 0;
	context_switch_clock = 0;
	idle_clock = 0;
	page_replace = 0;
	multi_prog = 5;
	//pcb_limit = 5;
	for(int i = 0; i < 32; i++)
	{
		unusedFrames.push_back(i);
	}
	lessMultiProg = false;
}

void OS::start(int lruValue)
{

	if (lruValue == 1)
		vm.lru = true;
	else
		vm.lru = false;

	//read in all files

	system("ls *.s > progs");

	//fstream readProgs;
	readProgs.open("progs", ios::in);

	if(!readProgs.is_open()) {
		cout << "Failed to open progs" << endl;
		exit(2);
	}

	string line;
	getline(readProgs, line);
	/*
	for(int i = 0; i < multi_prog && !readProgs.eof(); i++)
	{
		getline(readProgs, line);
		if(readProgs.eof())
		{
			lessMultiProg = true;
			break;
		}
		readIn(line);
	} */
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

		int base = 0;
		int limit = 0;
		if(unusedFrames.empty())
		{
			replaceAlgorithm();
		}
		int frameNumber = unusedFrames.front();
		int pageNum = vm.pageNumber(p -> pc);

		vm.ReadMemory(p->originalfilename, pageNum, frameNumber, &base, &limit);

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
		
		if(lessMultiProg)
		{
			getline(readProgs, line);
			if(!readProgs.eof())
			{
				lessMultiProg = false;
				readIn(line);
			}
		}
		
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
				case 0:  {//time slice
					int bit10 = vm.sr & 0x400;
					bit10 = bit10 >> 10;
					int frameNumber;	
					if(bit10 == 0)
					{
						getPCBfromVM();
						readyQ.push(running);
					}
					else
					{
						if(unusedFrames.empty())
						{
							replaceAlgorithm();
						}
						frameNumber = unusedFrames.front();
						unusedFrames.pop_front();
						
						int baseNew = 0;
						int limitNew = 0;
						
						if(vm.pageNumberToGet == -1)
						{
							exit(2);
						}
					
						vm.ReadMemory(running-> originalfilename, vm.pageNumberToGet, frameNumber, & baseNew, &limitNew);
						
						vm.frames[vm.pageNumberToGet] = frameNumber;
						vm.valid[vm.pageNumberToGet] = 1;
						vm.base = vm.frames[vm.pageNumber(vm.pc)] * vm.FRAME_SIZE;
						vm.pc= vm.frames[vm.pageNumber(vm.pc)] * vm.FRAME_SIZE + vm.offsetNumber(vm.pc);
						
						waitQ.push(running);
						running->state = "waiting";
						getPCBfromVM();
						running->interruptTime = vm.clo + 35;
						
					}
					break;
				}
				case 1: { // Halt Instruction
					running->state = "terminated";
					lessMultiProg = true;
					break;
				}
				case 2: {// Out of Bound
					running->state = "terminated";
					outputErrorCode("Out of Bound reference");
					lessMultiProg = true;
					break;
				}
				case 3: {// Stack Overflow
					running->state = "terminated";
					outputErrorCode("Stack Overflow");
					lessMultiProg = true;
					break;
				}
				case 4: {// Stack Underflow
					running->state = "terminated";
					outputErrorCode("Stack Underflow");
					lessMultiProg = true;
					break;
				}
				case 5: {// Invalid Opcode
					running->state = "terminated";
					outputErrorCode("Invalid Opcode");
					lessMultiProg = true;
					break;
				}
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
					
					int writeToReg = vm.sr;
					writeToReg &= 0x300;
					writeToReg >>= 8;
					int temp = vm.r[writeToReg] & 0x8000;
					if(temp)
					{
						temp = vm.r[writeToReg] | 0xffff0000;
					}
					else
					{
						temp = vm.r[writeToReg] & 0xffff;
					}
					running -> writeToRegister << temp << endl;
					
					vm.clo++;
					running->ioTime += 27;
					running->cpuTime++;

					waitQ.push(running);
					running->state = "waiting";
					getPCBfromVM();
					running->interruptTime = os_clock + 28;

					break;
				}
				default: {
					running->state = "terminated";
					outputErrorCode("Invalid Return Status");
					lessMultiProg = true;
					break;
				}
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
	
	for(list<PCB *>::iterator it = jobs.begin(); it != jobs.end(); it++)
	{
		(*it) -> turnaroundTime = (*it) -> cpuTime + (*it) -> waitingTime + (*it) -> ioTime + (*it) -> idleTime + (*it) -> contextSwitchTime;
		
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

	vm.infilename = running -> readfilename;
	vm.originalFileName = running -> originalfilename;
	
	//assigning values into register
	for(int i=0; i < 4; i++ ) {
		vm.r[i] = running->reg[i];
	}

	//retrieving stack values if there is any
	if(running->sp < 256) {
		
			int spTemp = running -> sp;
			
			while(spTemp < 256)
			{
				int frameNumberToOverwrite = vm.pageNumber(spTemp);
				vm.copyFromMemory(frameNumberToOverwrite, &jobs);
				spTemp += 6;
			}
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
	vm.frames = running -> frames;
	vm.valid = running -> valid;
	vm.modified = running -> modified;
	
	if(vm.lru)
	{
		vm.accessRegisters[vm.frames[vm.pageNumber(vm.physicalToLogical(vm.pc))]] = os_clock;
	}
}

void OS::getPCBfromVM() {
	//copy contents of VM into running PCB, includeing stack info
	running->pc = vm.pc;
	running->sr = vm.sr;
	running->sp = vm.sp;
	running->base = vm.base;
	running->limit = vm.limit;
	
	vm.osJobs = &jobs;

	//assigning registervalues to pcb
	for(int i = 0 ; i < 4; i++) {
		running->reg[i] = vm.r[i];
	}

	//retrieving the vm stack values to running pcb
	if(vm.sp < 256) {
		int spTemp = running ->sp;
		while(spTemp < 256)
		{
			vm.inverted[vm.pageNumber(spTemp)] = "NULL";
			spTemp += 6;
		}
	}
	if(running -> sp < 256)
	{
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
	running -> frames = vm.frames;
	running -> valid = vm.valid;
	running -> modified = vm.modified;
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

void OS::readIn(string line)
{
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
		if(unusedFrames.empty())
		{
			replaceAlgorithm();
		}
		int frameNumber = unusedFrames.front();
		int pageNum = vm.pageNumber(p -> pc);
		int temp = vm.ReadMemory(p->originalfilename, pageNum, frameNumber, &base, &limit);

		p->base = base;
		p->limit = limit;
		p->frames[pageNum] = frameNumber;
		p->valid[pageNum] = 1;
		p->pc = frameNumber * vm.FRAME_SIZE + vm.offsetNumber(p->pc);
	} //finish reading into memory	
}

void OS::replaceAlgorithm()
{
	int oldest = 0;
	
	if(!strcmp(vm.inverted[oldest].c_str(), "Stack"))
	{
		oldest = 1;
	}
	for(int i = oldest + 1; i < vm.accessRegisters.size(); i++)
	{
		if(vm.accessRegisters[i] < vm.accessRegisters[oldest])
		{
			if(strcmp(vm.inverted[i].c_str(), "Stack"))
			{
				oldest = i;
			}
		}
	}
	vm.copyFromMemory(oldest, &jobs);
	unusedFrames.push_back(oldest);
	page_replace++;
}

//main function
int main(int argc, char * argv[]) {
	OS os;
	if(argv[1] == NULL) {
		cout << "Invalid Switch used. type in fifo or lru only" << endl;
		exit(1);
	}

	string lruValue = argv[1];
	if(strcmp(lruValue.c_str(), "-lru") == 0)
	{
		os.start(1);
	} 
	else if(strcmp(lruValue.c_str(), "-fifo") == 0)
	{
		os.start(0);
	}
	else
	{
		cout << "Invalid switch used. -1ru or -fifo";
	}
}
