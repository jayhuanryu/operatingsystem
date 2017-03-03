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
	
	this->pc = 0;
	this->sp = 256;
	this->base = 0;
	this->limit = 0;
	this->clo = 0;
	this->sr = 0;
	nextmemory = 0;
	
	//this->infilename += infilename;
	//cout << "finished constructor" << endl;
	//ReadMemory();


}

void VirtualMachine::ReadMemory(string infilename, int *progBase, int *progLimit) {
	fstream ReadMemory
	ReadMemory.open(infilename.c_str(), ios::in);
	if(!ReadMemory.is_open()) {
		cout << infilename << " failed to open" << endl;
	}

	*progBase = nextmemory;

	string line;
	getline(ReadMemory, line);

	while(!ReadMemory.eof()) {
		mem[nextmemory++] = atoi(line.c_str());
		getline(ReadMemory, line);
	}

	*progLimit = nextmemory - *progBase;

	ReadMemory.close();
/*	this->infilename += infilename;
	int objectcode;
	int iterator = 0;

	string infilename = "prog.o";
	cout << "reading into memory" << endl;
	ifstream infile;
	infile.open(infilename.c_str());
	infile >> objectcode;

	while (infile.good()) {

		cout << "first objectcode is" << objectcode << endl;

		mem[iterator]=objectcode;
		infile >> objectcode;
		limit++;
		if (limit > MEM_SIZE) {
			cout << "instructions are more than memory capacity" << endl;
		}
		iterator++;
	} */
}

void VirtualMachine::Execute(){

	bool halt = false;
	ofstream outfile;
	outfile.open("prog.out", ios::out);


	while(!halt) {// to be changed into infinite loop
		ir = mem[pc]; // instuction fetch
		cout << "mem[pc]" << mem[pc] << " pc " << pc << endl; // testing purpose, to be deleted
		pc++;

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
			Write(RD, outfile);
		} else if ( op == 24) {
			halt = Halt();
			break;
		} else if ( op == 25) {
			Noop();
		} else  {
			cout << "invalid op" << op << endl;
			exit(1);
		}	
		cout << "instruction passed" << endl;
	}
	
		outfile << "Final value of clock: " << clo << endl;
		outfile.close();		
}	

void VirtualMachine::Load(int RD, int addr) {
	r[RD] = mem[addr];
	clo += 4;
	cout << "load" << r[RD] << " " << mem[addr] << endl;
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
	mem[addr] = r[RD];
	clo += 4;
	cout << "store" << r[RD] << " " << mem[addr] << endl;
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
	pc = addr;
	clo++;
	cout << "jump" << addr << endl;
}

void VirtualMachine::Jumpl(int addr) {
	if (sr & 8) {
		pc = addr;
		clo++;
	}
	cout << "jumpl" << pc << " " << addr << endl;
}

void VirtualMachine::Jumpe(int addr) {
	if (sr & 4) {
		pc = addr;
		clo++;
	}
	cout << "jumpe" << pc << " " << addr << endl;
}

void VirtualMachine::Jumpg(int addr) {
	if (sr & 2) {
		pc = addr;
		clo++;
	}
	cout << "jumpg" << pc << " " << addr << endl;
}            

void VirtualMachine::Call(int addr){
	if (sp <= 0) {
		cout << "Stack is Full" << endl;
		exit(20);
	}
	
	s.push(pc);
	s.push(r[0]);
	s.push(r[1]);
	s.push(r[2]);
	s.push(r[3]);
	s.push(sr);
	sp = sp - 6;
	pc = addr;
	clo += 4;

	cout << "call passed " << endl; // testing purpose
	cout << "call" << pc << " " << sp << endl;	
}

void VirtualMachine::Return(){
	sr = s.top();
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
	sr  = sr + 6;
	clo += 4;
	cout << "return" << sr << endl;
}

void VirtualMachine::Read(int RD) {
	
	int temp = 0;
	infilename += ".in";
	ifstream infile;
	infile.open(infilename.c_str());
	infile >> temp;
	r[RD] = temp;
	infile.close();
	cout << "read sucessful" << endl;
	cout << "r[RD] " << r[RD] << endl;
	clo += 28;
}

void VirtualMachine::Write(int RD, ofstream& outfile) {

	clo += 28;
	for(int i = 0 ; i < 4 ; i ++)
	{
		outfile << "Register value: " << r[i] << endl;
	}
}

bool VirtualMachine::Halt() {

	clo++;
	cout << "halt called exit..." << endl;
	return true;
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
