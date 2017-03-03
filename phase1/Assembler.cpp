/********************************************************************************************
*  File Name : Assembler.cpp
*  Name : Jay Huan Ryu, Donghyuk Bae
*  Description : This code will read in the assembly line and outputs its corresponding
*				 object program. This code ill catch any out-of-range error for ADDR and
*				 CONST and stop producing object codes. Also any value other than 0,1,2,3
*			     for RD or RS is illegal; and any opcode other than the ones listed in the
*			 	 above VM instruction table is illegal.
********************************************************************************************/ 

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>	
#include "Assembler.h"

using namespace std;


Assembler::Assembler()
{
	//constructor
	this->outfilename = "prog.o";
	this->instruction = 0;
	this->rd = 0;
	this->operand = 0;
	
}

void Assembler::assemble(string infilename) {

	int counter = 1; //to check runtime error, counting instructions
	infilename +=".s";
	ifstream infile;
	infile.open(infilename.c_str());
	ofstream outfile;
	outfile.open(outfilename.c_str());
	infile >> opcode; 
	while(infile.good()) {
		int instruction = getinstruction(infile);
		outfile << instruction << endl;
		counter++;
		infile >> opcode;
	}

	checkruntimeerror(counter);
	infile.close();
	outfile.close();

}

int Assembler::getinstruction(ifstream& infile)
{
	int tempinstruction = 0;
	if (opcode == "load") {
		return tempinstruction = format2(0,0, infile);
	} else if (opcode == "loadi") {
		return tempinstruction = format2(0,1, infile);
	} else if (opcode == "store") {
		return tempinstruction = format2(1,1, infile);
	} else if (opcode == "add") {
		return tempinstruction = format1(2,0, infile);
	} else if (opcode == "addi") {
		return tempinstruction = format2(2,1, infile);
	} else if (opcode == "addc") {
		return tempinstruction = format1(3,0, infile);
	} else if (opcode == "addci") {
		return tempinstruction = format2(3,1, infile);
	} else if (opcode == "sub") {
		return tempinstruction = format1(4,0, infile);
	} else if (opcode == "subi") {
		return tempinstruction = format2(4,1, infile);
	} else if (opcode == "subc") { //set carry
		return tempinstruction = format1(5,0, infile);
	} else if (opcode == "subci") { //set carry
		return tempinstruction = format2(5,1, infile);
	} else if (opcode == "and") {
		return tempinstruction = format1(6,0, infile);
	} else if (opcode == "andi") {
		return tempinstruction = format2(6,1, infile);
	} else if (opcode == "xor") {
		return tempinstruction = format1(7,0, infile);
	} else if (opcode == "xori") {
		return tempinstruction = format2(7,1, infile);
	} else if (opcode == "compl") {
		return tempinstruction = format1(8,0, infile); // consider "don't care" into 0
	} else if (opcode == "shl") {
		return tempinstruction = format3(9,0, infile);
	} else if (opcode == "shla") {
		return tempinstruction = format3(10,0, infile);
	} else if (opcode == "shr") {
		return tempinstruction = format3(11,0, infile);
	} else if (opcode == "shra") {
		return tempinstruction = format3(12,0, infile);
	} else if (opcode == "compr") {
		return tempinstruction = format1(13,0, infile);
	} else if (opcode == "compri") {
		return tempinstruction = format2(13,1, infile);
	} else if (opcode == "getstat") {
		return tempinstruction = format3(14,0, infile);
	} else if (opcode == "putstat") {
		return tempinstruction = format1(15,0, infile);
	} else if (opcode == "jump") {
		return tempinstruction = format5(16,1, infile);
	} else if (opcode == "jumpl") {
		return tempinstruction = format5(17,1, infile);
	} else if (opcode == "jumpe") {
		return tempinstruction = format5(18,1, infile);
	} else if (opcode == "jumpg") {
		return tempinstruction = format5(19,1, infile);
	} else if (opcode == "call") {
		return tempinstruction = format5(20,1, infile);
	} else if (opcode == "return") {
		return tempinstruction = format4(21);
	} else if (opcode == "read") {
		return tempinstruction = format3(22,0, infile);
	} else if (opcode == "write") {
		return tempinstruction = format3(23,0, infile);
	} else if (opcode == "halt") {
		return tempinstruction = format4(24);
	} else if (opcode == "noop") {
		return tempinstruction = format4(25);
	} else {
		cout << "error in getinstruction" << endl;
		cout << opcode << endl;
		exit(0);
	}
	

}

int Assembler::format1(int op, int i, ifstream& infile) {
	infile >> rd >> operand;
	if (op != 1)
	{
		if (rd > 3 || rd <0) {
			cout << "invalid rd in format1 for op " << op << endl;
			exit(1);
		} else if( operand < 0 || operand > 3) {
			cout << "invalid rs in format1 for op " << op << endl;
			exit(1);
		}
	}
	
	int newop = op << 11;
	int newrd = rd << 9;
	int newi = i << 8;
	int newrs = operand << 6;
	addrs.push_back(0);
	return newop + newrd + newi + newrs;

}


int Assembler::format2(int op,int i, ifstream& infile) {
	infile >> rd >> operand;
	if (rd < 0 || rd > 3) {
		cout << "invalid rd in format2 for op" << op << endl;
		return 0;
	} 
	if (operand >= 128 || operand < -128) {
			cout << "invalid constant in format2 for op " << op << endl;
			exit(2);
	}
	if (operand < 0 )
	{
		operand &= 255;
	}
	int newop = op << 11;
	int newrd = rd << 9;
	int newi = i << 8;
		
	addrs.push_back(operand);
	cout << newop << endl;
	cout << newrd << endl;
	cout << newi  << endl;
	cout << operand<<endl;

	return newop + newrd + newi + operand; //c++ will automatically change the constant into binary
}

int Assembler::format3(int op, int i, ifstream& infile){ //takes in only rd
	infile >> rd ;
	if (rd > 3 || rd < 0) {
		cout <<"invalid rd in format3 for op " << op <<endl;
		exit(3);
	} else {
		int newop = op << 11;
		int newrd = rd << 9;
		int newi = i << 8;
		addrs.push_back(0);
		return newop + newrd + newi;
	}
	

	
}

int Assembler::format5(int op, int i, ifstream& infile)
{
	infile >> operand;
	int newop = op << 11;
	return newop+operand;
}

int Assembler::format4(int op){//not read in rd nor rs
	addrs.push_back(0);
	return op << 11;
}
void Assembler::checkruntimeerror(int counter) {
	/*for (int i = 0 ; i < addrs.size(); i++) {
		if (addrs[i] > counter) {
			cout << "Runtime error in line" << i+1;
		}
	}
	exit(100);*/
}


