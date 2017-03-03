/**********************************************************************************************
*File Name: VirtualMachine.cpp
*Written by: Jay Huan Ryu, Donghyuk Bae
*Date: 4/18/2016
*Class: CSE460 Operating System
*Description: This codes will take in the integer that is converted from Assembler.h and
*	      implementing the job that codes supposed to do and return the result of fuction.
***********************************************************************************************/

#include <cstdlib>  // for exit()
#include <iostream> // for cout...
#include <vector>   // using vector for register and memory
#include <stack>    // using stack for storing values for call and return function
#include <fstream>  // for fstream::open()
#include <string>
#include "VirtualMachine.h"

using namespace std;

VirtualMachine::VirtualMachine()
{
	r = vector <int> (REG_FILE_SIZE); //constructor
	mem = vector <int> (MEM_SIZE);
	frames = vector<int>(32); // 32 represent for 32 frames
	
	this->pc = 0;
	this->sp = 256;
	this->base = 0;
	this->limit = 0;
	this->clo = 0;
	this->sr = 0;
	this->nextmemory = 0;
	this->pageNumberToGet = -1;
	
	accessRegisters = vector <int> (32);
	for(int i = 0; i < accessRegisters.size(); i++)
	{
		accessRegisters[i] = -1;
		inverted = vector <string> (32);
	}
	for(int i = 0; i <inverted.size(); i++)
	{
		inverted[i] = "NULL";
	}

}

int VirtualMachine::ReadMemory(string originalFileName, int pageNumber, int frameNumber, int *progBase, int *progLimit) {
	fstream readmemory;
	readmemory.open(originalFileName.c_str(), ios::in);
	if(!readmemory.is_open()) {
		cout << infilename << "Failed to open infilename." << endl;
		exit(1);
	}

	nextmemory = frameNumber * FRAME_SIZE;

	string line;
	
	for(int i = 0; i < pageNumber * FRAME_SIZE; i++)
	{
		getline(readmemory, line);
	}
	*progBase = nextmemory;
	
	for(int i = 0; i < FRAME_SIZE; i++)
	{
		if(readmemory.eof())
		{
			while(i < FRAME_SIZE)
			{
				mem[nextmemory++] = 0;
				i++;
			}
			break;
		}
		getline(readmemory, line);
		mem[nextmemory++] = atoi(line.c_str());
		int temp = atoi(line.c_str());
	}

	*progLimit = 8;

	readmemory.close();
	inverted[frameNumber] = originalFileName;
	if(!lru)
	{
		accessRegisters[frameNumber] = clo;
	}
	return 0;
}

void VirtualMachine::copyFromMemory(int frame, list<PCB*>* jobs)
{
	bool halt = false;
	
	string updateFileName;
	string process = inverted[frame];
	int page = 0;
	
	inverted[frame] = "NULL";
	
	if(!strcmp(process.c_str(), originalFileName.c_str()))
	{
		for(int i = 0; i < frames.size(); i++)
		{
			if(frames[i] == frame)
			{
				page = 1;
				break;
			}
		}
		updateFileName = originalFileName;
		
		if(modified[page] == 1)
		{
			halt = true;
			modified[page] = 0;
		}
		valid[page] = 0;
	}
	else
	{
		for(list<PCB *>::iterator it = jobs->begin(); it != jobs->end(); it++)
		{
			if(!strcmp((*it) ->originalfilename.c_str(), process.c_str()))
			{
				for(int i = 0; i < (*it) -> frames.size(); i++)
				{
					if((*it) -> frames[i] == frame)
					{
						page = i;
						break;
					}
				}
				if((*it) -> modified[page] == 1)
				{
					halt = true;
					(*it) -> modified[page] = 0;
				}
				(*it) -> valid[page] = 0;
			}
		}
	}
	if(halt)
	{
		fstream readProgs;
		readProgs.open(updateFileName.c_str(), ios::in);
		
		string newFileName = updateFileName + "2";
		
		fstream newFile;
		newFile.open(newFileName.c_str(), ios::out);
		
		if(!readProgs.is_open())
		{
			cout << "Porgs failed to open.";
		}
		if(!newFile.is_open())
		{
			cout << "Progs failed to open.";
		}
		string line;
		getline(readProgs,line);
		int i = 0;
		int count = FRAME_SIZE * page;
		int frameCount = FRAME_SIZE * frame;
		
		while(!readProgs.eof() && i < count)
		{
			newFile << line << endl;
			getline(readProgs, line);
			i++;
		}
		for(int j = 0; j < FRAME_SIZE && mem[frameCount] != 0; j++)
		{
			newFile << mem[frameCount] << endl;
			getline(readProgs, line);
			frameCount++;
		}
		while(!readProgs.eof())
		{
			newFile << line << endl;
			getline(readProgs,line);
		}
		readProgs.close();
		newFile.close();
		
		string removeFile = "rm " + updateFileName;
		system(removeFile.c_str());
		int result = rename(newFileName.c_str(), updateFileName.c_str()); 
		
	}
}

void VirtualMachine::Execute(string filename){

	/*	
	//if there is any input file
	
	ifstream infile;
	infilename = filename + ".in";
	infile.open(infilename.c_str());
	
	//for output file
	ofstream outfile;
	outfilename = filename + ".out";
	// outfile.open(outfilename.c_str(), ios::out);
	*/
	clo = 0;
	execute = true;
	sr &= 0xffffff1f;
	sr = sr & 0xfffffbff;
	pageNumberToGet = -1;
	
	int logicalPc = physicalToLogical(pc);
	if(!valid[pageNumber(logicalPc)])
	{
		pageNumberToGet = pageNumber(logicalPc);
		pc = logicalPc;
		sr = sr | 0x400;
		execute = false;
		return;
	}
	
	while(execute) {// to be changed into infinite loop
		if(pc < base)
		{
			exit(2);
		}
		if(pc >= base + limit)
		{
			int pageNum = physicalToLogical(pc - 1);
			pageNum = pageNumber(pageNum);
			pageNum++;
			if(valid[pageNum] == 0)
			{
				pageNumberToGet = pageNum;
				pc = pageNum * FRAME_SIZE;
				sr = sr | 0x400;
				execute = false;
				continue;
			}
			else
			{
				pc = frames[pageNum] * FRAME_SIZE;
				base = pc;
				if(lru)
				{
					accessRegisters[frames[pageNumber(physicalToLogical(pc))]] = clo;
				}
			}
		}	
		ir = mem[pc]; // instuction fetch
		cout << " pc = " << pc << endl; // testing purpose, to be deleted
		pc++;
		if(clo >= 15)
		{
			execute = false;
		}

		int op = ir >> 11; //assigning first 5bits as opcode
		int RD = (ir >> 9) - (op << 2);
		int i = (ir >> 8) - (op << 3) - (RD<<1) ;
		int addr = ir - (op << 11) - (RD << 9) - (i << 8);
		int rs = (addr >> 6) ;

		
		cout << "object code is " << ir << endl;
		cout << "OP" << op << endl;
		cout << "RD" << RD << endl;
		cout << "I"  << i  << endl;
		cout <<"ADDR"<< addr<<endl;
		cout << "RS" << rs << endl;

		if (( op == 0 ) && ( i == 0)) {
			Load(RD, addr);
		} else if (( op == 0) && ( i == 1)) {
			Loadi(RD, addr);
		} else if ( op == 1) {
			Store(RD, addr); 
		} else if (( op == 2) && ( i == 0)) {
			Add(RD, rs);
		} else if (( op == 2) && ( i == 1)) {
			Addi(RD, addr);
		} else if (( op == 3) && ( i == 0)) {
			Addc(RD, rs);
		} else if (( op == 3) && ( i == 1)) {
			Addci(RD, addr);
		} else if (( op == 4) && ( i == 0)) {
			Sub(RD, rs);
		} else if (( op == 4) && ( i == 1)) {
			Subi(RD, addr);
		} else if (( op == 5) && ( i == 0)) {
			Subc(RD, rs);
		} else if (( op == 5) && ( i == 1)) {
			Subci(RD, addr);
		} else if (( op == 6) && ( i == 0)) {
			AND(RD, rs);
		} else if (( op == 6) && ( i == 1)) {
			ANDi(RD, addr);
		} else if (( op == 7) && ( i == 0)) {
			XOR(RD, rs);
		} else if (( op == 7) && ( i == 1)) {
			XORi(RD, addr);
		} else if ( op == 8) {
			Comple(RD);
		} else if ( op == 9) {
			Shl(RD);
		} else if ( op == 10) {
			Shla(RD);
		} else if ( op == 11) {
			Shr(RD);
		} else if ( op == 12) {
			Shra(RD);
		} else if (( op == 13) && ( i == 0)) {
			Compr(RD, rs);
		} else if (( op == 13) && ( i == 1)) {
			Compri(RD, addr);
		} else if ( op == 14) {
			Getstat(RD);
		} else if ( op == 15) {
			Putstat(RD);
		} else if ( op == 16) {
			Jump(addr);
		} else if ( op == 17) {
			Jumpl(addr);
		} else if ( op == 18) {
			Jumpe(addr);
		} else if ( op == 19) {
			Jumpg(addr);
		} else if ( op == 20) {
			Call(addr);
		} else if ( op == 21) {
			Return();
		} else if ( op == 22) {
			Read(RD);
		} else if ( op == 23) {
			Write(RD);
		} else if ( op == 24) {
			Halt();
			break;
		} else if ( op == 25) {
			Noop();
		} else  {
			cout << "invalid op" << op << endl;
			exit(1);
		}	
		cout << "instruction passed" << endl;
	}

		
		
}
bool VirtualMachine::checkmemory(int addr) {
	int temp = pageNumber(addr);

	if(valid[addr] == 1)
		return true;

	return false;
}

int VirtualMachine::pageNumber(int addr) {
	int page = addr >> 3; //first 5 bits of address are the pageNumber
	page = addr & 31;

	return page;
}

int VirtualMachine::offsetNumber(int addr) {
	return addr & 7; //the last 3 bits of address are the offset number
}

int VirtualMachine::physicalToLogical(int addr) {

	//get frame number and offset
	int frame = pageNumber(addr);
	int offset = offsetNumber(addr);
	int page = 0;
	int i;
	//initializae page = 0(just for initial value)
	int iterator = 0;
	while (page == 0) {
		if (frames[i] == frame) {
			page = i;
		}
	}

	page = page << 3; //pages are first 5 bits
	page = page + offset; //concatenate with offset

	return page; //return the logical value
}

void VirtualMachine::returnCode(int i) {
	sr &= 0xff1f; // nullify the return code and reset. (return status = 000)
	i = i << 5; // move i to return status
	sr = sr | i; 
}

void VirtualMachine::readOrWrite(int i) {
	sr &= 0xffffcff; // nullify the io register and reset. (io register = 00)
	i = i << 8; // move i to io register
	sr = sr | i; 
}

void VirtualMachine::Load(int RD, int addr) {
	
	if (checkmemory(addr) == false) {
		sr = sr | 0x400;
		pc--;
		pc = physicalToLogical(pc);
		pageNumberToGet = pageNumber(addr);
		execute = false;
		return;
	}

	int temp1 = pageNumber(addr);
	int temp2 = offsetNumber(addr);
	int temp3 = frames[temp1] * FRAME_SIZE + temp2;
	r[RD] = mem[temp3];
	clo += 4;
}

void VirtualMachine::Loadi(int RD, int addr) {
	
	if (addr > 128) { //checking nagative
		addr = -(256-addr); // change into negative int
	}
	r[RD] = addr;
	cout << "rd " << RD << " addr " << addr << "value" << r[RD] << endl; 
	clo++;
	cout << "loadi" << r[RD] << " " << mem[addr] << endl;
}

void VirtualMachine::Store(int RD, int addr) {
	
	if (checkmemory(addr) == false) {
		sr = sr | 0x400;
		pc--;
		pc = physicalToLogical(pc);
		pageNumberToGet = pageNumber(addr);
		execute = false;
		return;
	}
	int temp1 = pageNumber(addr);
	int temp2 = offsetNumber(addr);
	int temp3 = frames[temp1] * FRAME_SIZE + temp2;

	mem[temp3] = r[RD];
	modified[temp1] = 1;
	clo += 4;
}	

void VirtualMachine::Add(int RD, int rs) {
	r[RD] += r[rs];
	if (overflow2(r[RD],r[rs])) {
		sr = (sr | 16);
	} else {
		sr = (sr & 0xffef); 
	}
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	cout << "add" << r[RD] << " " << r[rs] << endl;
	clo++;
}

void VirtualMachine::Addi(int RD, int addr) {
	r[RD] += addr;
	if (overflow2(r[RD],addr)) {
		sr = (sr | 16);
	} else {
		sr = (sr & 0xffef);
	}
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	cout << "addi" << r[RD] << " " << mem[addr] << endl;
	clo++;
}

void VirtualMachine::Addc(int RD, int rs) {
	if (sr&16) { 
		r[RD] = r[RD] + r[rs] + 1;
	} else {
		r[RD] = r[RD] + r[rs];
	}
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
 	cout << "addc" << r[RD] << " " << r[rs] << endl;
	clo++;
}

void VirtualMachine::Addci(int RD, int addr) {
	if (sr&16) {
		r[RD] = r[RD] + addr + 1;
	} else {
		r[RD] = r[RD] + addr;
	}
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	cout << "addci" << r[RD] << " " << mem[addr] << endl;
	clo++;
}

void VirtualMachine::Sub(int RD, int rs) {
	r[RD] -= r[rs];
	if (overflow2(r[RD],r[rs])) {
		sr = (sr | 16);
	} else {
		sr = (sr & 0xffef);
	} 
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	cout << "sub" << r[RD] << " " << r[rs] << endl;
	clo++;

}

void VirtualMachine::Subi(int RD, int addr) {
	r[RD] -= addr;
	if (overflow2(r[RD],addr)) {
		sr = (sr | 16);
	} else {
		sr = (sr & 0xffef);
	}
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	cout << "subi" << r[RD] << " " << mem[addr] << endl;
	clo++;
	
}

void VirtualMachine::Subc(int RD, int rs) {
	if (sr&16) {
		r[RD] = r[RD] - r[rs] - 1;
	} else {
		r[RD] = r[RD] - r[rs];
	}
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	cout << "subc" << r[RD] << " " << r[rs] << endl;
	clo++;
	
}

void VirtualMachine::Subci(int RD, int addr) {
	if (sr&16) {
		r[RD] = r[RD] - addr - 1;
	} else {
		r[RD] = r[RD] - addr;
	}
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	cout << "subci" << r[RD] << " " << mem[addr] << endl;
	clo++;

}

void VirtualMachine::AND(int RD, int rs) {
	r[RD] = r[RD] & r[rs];
	clo++;
	cout << "and" << r[RD] << " " << r[rs] << endl;
}

void VirtualMachine::ANDi(int RD, int addr) {  
	r[RD] = r[RD] & addr;
	clo++;
	cout << "andi" << r[RD] << " " << addr << endl;
}

void VirtualMachine::XOR(int RD, int rs) {
	r[RD] = r[RD] ^ r[rs];
	clo++;
	cout << "xor" << r[RD] << " " << r[rs] << endl;
}

void VirtualMachine::XORi(int RD, int addr) {
	r[RD] = r[RD] ^ addr;
	clo++;
	cout << "xori" << r[RD] << " " << addr << endl;
}

void VirtualMachine::Comple(int RD) {
	r[RD] = ~r[RD];
	clo++;
	cout << "comple" << r[RD] << endl;
}

void VirtualMachine::Shl(int RD) {	
	r[RD] = r[RD] << 1;
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	clo++;
	cout << "shl" << r[RD] << endl;
}

void VirtualMachine::Shla(int RD) {
	int signbit = r[RD] >> 16;
	r[RD] = (r[RD] - (signbit << 16)) << 1;
	if((r[RD] >> 16) != signbit) {
		r[RD] = (signbit == 1) ? (r[RD] + 16) : (r[RD] - 16);
	}
	if(r[RD] & 16) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	cout << "shla" << r[RD] << endl;
	clo++;	
}

void VirtualMachine::Shr(int RD) {	
	if(r[RD] & 1) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xfffe);
	}
	r[RD] = (r[RD] >> 1);
	clo++;
	cout << "shr" << r[RD] << endl;
}

void VirtualMachine::Shra(int RD) {
	if(r[RD] & 1) {
		sr = (sr | 1);
	} else {
		sr = (sr & 0xffef);
	}	
	int signbit = (r[RD] >> 16);
	if (signbit)
		r[RD] = (r[RD]>>1) + 16;
	else
		r[RD] = (r[RD] >> 1);
	cout << "shra" << r[RD] << endl;
	clo++;
}

void VirtualMachine::Compr(int RD, int rs) {
	if (r[RD] < r[rs] ) { //LESS
		sr = (sr | 8);
		sr = (sr & 25); //reset & remaining overflow bit
		clo++;
	} else if ( r[RD] == r[rs] ) { //E
		sr = (sr | 4);
		sr = (sr & 21); 
		clo++;
	} else { //G
		sr = (sr | 2);
		sr = (sr & 19);
		clo++;
	}
	cout << "compr" << r[RD] << " " << r[rs] << endl;	
}

void VirtualMachine::Compri(int RD, int addr) {
	if (r[RD] < addr ) { //L
		sr = (sr | 8);
		sr = (sr & 25);
		clo++;
	} else if ( r[RD] == addr ) { //E
		sr = (sr | 4);
		sr = (sr & 21);
		clo++;
	} else { // G
		sr = (sr | 2);
		sr = (sr & 19);
		clo++;
	}	
	cout << "compri" << r[RD] << " " << sr << endl;
}

void VirtualMachine::Getstat(int RD) {
	r[RD] = sr;
	clo++;
	cout << "getstat" << r[RD] << " " << sr << endl;
}

void VirtualMachine::Putstat(int RD) {
	sr = r[RD];
	clo++;
	cout << "putstat" << r[RD] << " " << sr << endl;
}

void VirtualMachine::Jump(int addr) {
	if (checkmemory(addr) == false) {
		sr = sr | 0x400;
		pc--;
		pc = physicalToLogical(pc);
		pageNumberToGet = pageNumber(addr);
		execute = false;
		return;
	}
	int temp1 = pageNumber(addr);
	int temp2 = offsetNumber(addr);
	int temp3 = frames[temp1] * FRAME_SIZE + temp2;
	pc = temp3;
	base = frames[temp1] * FRAME_SIZE;
	clo++;
	cout << "jump" << addr << endl;
}

void VirtualMachine::Jumpl(int addr) {
	if (checkmemory(addr) == false) {
		sr = sr | 0x400;
		pc--;
		pc = physicalToLogical(pc);
		pageNumberToGet = pageNumber(addr);
		execute = false;
		return;
	}
	int temp1 = pageNumber(addr);
	int temp2 = offsetNumber(addr);
	int temp3 = frames[temp1] * FRAME_SIZE + temp2;
	
	if (sr & 8) {
		pc = temp3;
		base = frames[temp1] * FRAME_SIZE;
		clo++;
	}
	cout << "jumpl" << pc << " " << addr << endl;
}

void VirtualMachine::Jumpe(int addr) {
	if (checkmemory(addr) == false) {
		sr = sr | 0x400;
		pc--;
		pc = physicalToLogical(pc);
		pageNumberToGet = pageNumber(addr);
		execute = false;
		return;
	}
	int temp1 = pageNumber(addr);
	int temp2 = offsetNumber(addr);
	int temp3 = frames[temp1] * FRAME_SIZE + temp2;
	if (sr & 4) {
		pc = temp3;
		base = frames[temp1] * FRAME_SIZE;
		clo++;
	}
	cout << "jumpe" << pc << " " << addr << endl;
}

void VirtualMachine::Jumpg(int addr) {
	if (checkmemory(addr) == false) {
		sr = sr | 0x400;
		pc--;
		pc = physicalToLogical(pc);
		pageNumberToGet = pageNumber(addr);
		execute = false;
		return;
	}
	int temp1 = pageNumber(addr);
	int temp2 = offsetNumber(addr);
	int temp3 = frames[temp1] * FRAME_SIZE + temp2;
	if (sr & 2) {
		pc = temp3;
		base = frames[temp1] * FRAME_SIZE;
		clo++;
	}
	cout << "jumpg" << pc << " " << addr << endl;
}            

void VirtualMachine::Call(int addr){
	if (checkmemory(addr) == false) {
		sr = sr | 0x400;
		pc--;
		pc = physicalToLogical(pc);
		pageNumberToGet = pageNumber(addr);
		execute = false;
		return;
	}
	int oldPc = pc;
	int temp1 = pageNumber(addr);
	int temp2 = offsetNumber(addr);
	int temp3 = frames[temp1] * FRAME_SIZE + temp2;
	
	pc = temp3;
	base = frames[temp1] * FRAME_SIZE;

	int newSp = sp - 6;
	int frameNumberToOverwrite = pageNumber(newSp);
	if(strcmp(inverted[frameNumberToOverwrite].c_str(), "NULL"))
	{
		copyFromMemory(frameNumberToOverwrite, osJobs);
	}

	inverted[frameNumberToOverwrite] = "Stack";
	
	s.push(oldPc);
	s.push(r[0]);
	s.push(r[1]);
	s.push(r[2]);
	s.push(r[3]);
	s.push(sr);
	clo += 4;

	cout << "call passed " << endl; // testing purpose
	cout << "call" << pc << " " << sp << endl;	
}

void VirtualMachine::Return(){
	if(sp == 256)
	{
		returnCode(4);
		execute = false;
		return;
	}

	inverted[pageNumber(sp)] = "NULL";

	sr = s.top();
	cout << "pass2" << endl;
	s.pop();
	r[3] = s.top();
	s.pop();
	r[2] = s.top();
	s.pop();
	r[1] = s.top();
	s.pop();
	r[0] = s.top();
	s.pop();
	pc = s.top();
	s.pop();
	sp  = sp + 6;
	clo += 4;

	int temp1 = pageNumber(pc);
	base = temp1 * FRAME_SIZE;
	cout << "return" << sr << endl;
}

void VirtualMachine::Read(int RD) {
	returnCode(6);
	readOrWrite(RD);
	execute = false;
	cout << "read sucessful" << endl;
	cout << "r[RD] " << r[RD] << endl;
	clo += 28;

	/*	
	//VM Return-Status

	sr = sr & 31; //clear up the sr except for VLEGC
		
	// assigning return status
	temp = 6;
	temp = temp << 5;
	sr = sr | temp; 

	// cout << "after return status(READ)" << endl;
	
	//I/O register
	temp = RD;
	temp = temp << 8;
	sr = sr | temp; //assigning the register #


	// cout << "after io reg(READ)" << endl;
	*/
}

void VirtualMachine::Write(int RD) {

	returnCode(7);
	readOrWrite(RD);
	execute = false;
	clo += 28;
	/*
	fstream outfile;
	outfile.open(outfilename.c_str(), ios::out);

		outfile << "Register value: " << r[RD] << endl;

	//clear up the sr except or VLEGC
	sr = sr & 31;

	//VM Return-Status
	int temp = 7;
	temp = temp << 5;
	cout << "sr = " << sr << endl;
	cout << "temp = " << temp << endl;
	sr = sr | temp;

	//I/O register;
	temp = RD;
	temp = temp << 8;
	sr = sr | temp;
 	outfile << "passed" << endl;
 	*/
}

void VirtualMachine::Halt() {
	int temp1 = pageNumber(pc - 1);
	clo++;
	returnCode(1);
	execute = false;
	cout << "halt called exit..." << endl;
}

void VirtualMachine::Noop() {
	
	clo++;
}

bool VirtualMachine::overflow1(int a) {
	if (a>=65536)
		return 1;
	else 
		return 0;
}

bool VirtualMachine::overflow2(int a, int b) {
	if ((a+b)>=65536)
		return 1;
	else
		return 0;
}

bool VirtualMachine::overflow3(int a, int b, int c) {
	if ((a+b+c)>=65536)
		return 1;
	else
		return 0;
}
