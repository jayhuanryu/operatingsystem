#include <iostream> // for cout...
#include <vector>   // using vector for register and memory
#include <stack>    // using stack for storing values for call and return function
#include <fstream>
#include <string>
#include <list>
#include "PCB.h"

using namespace std;

class VirtualMachine 
{

private: 
	static const int REG_FILE_SIZE = 4;
	static const int MEM_SIZE = 256;
	static const int FRAME_SIZE = 8;

	vector<int> r;   // stands for register
	vector<int> mem; // stands for memory
	stack<int> s;// data stored in function call and return 
	
	vector<int> frames;
	vector<int> valid; //represent valid/invalid bit
	vector<int> modified; // represent modified bit
	vector<int> accessRegisters;
	vector<string> inverted;

	int pc;       // program counter
	int ir; 		  // instruction register
	int sr; 		  // status register
	int sp; 	  // stack pointer
	int base; 	  // memory base
	int limit;	  //memory length
	int clo; 		  //clock
	int nextmemory;		//next memory location
	int pagenumber;
	int pageNumberToGet;
	list<PCB *> * osJobs;

	string infilename;
	string outfilename;
	string readFileName;
	string writeFileName;
	string originalFileName;

	bool lru;
	bool execute;

public:
	VirtualMachine();
	int ReadMemory(string, int, int, int *, int *);
	void copyFromMemory(int, list<PCB *> * jobs);
	void Execute(string);
	
	bool overflow1(int);
	bool overflow2(int,int);
	bool overflow3(int,int,int);
	bool checkmemory(int);

	int pageNumber(int);
	int offsetNumber(int);
	int physicalToLogical(int);
	void returnCode(int);
	void readOrWrite(int);

	void Load(int, int);    // load
	void Loadi(int,int);	// loadi will assign the constant to the destination register
	void Store(int, int);   //store
	void Add(int, int);     //add without carry
	void Addi(int, int);	//addi without carry
	void Addc(int, int);	//add with carry
	void Addci(int, int);   //addi with carry
	void Sub(int, int);     //subtraction without carry
	void Subi(int, int);	//instance subtraction without carry
	void Subc(int, int);    //subtraction with carry
	void Subci(int, int);   //instance subtraction with carry
	void AND(int, int);    	//AND operation (Logical Arithmetic)
	void ANDi(int,int);		//AND operation with instance value
	void XOR(int, int);     //XOR operation (Logical Arithmetic)
	void XORi(int, int);	//XORi operation (Logical Arithmetic) with instance value	
	void Comple(int); 		// complement of the integer
	void Shl(int);    		//shift left not considering the sign bit
	void Shla(int);   		//shift left keeping the sign bit
	void Shr(int);    		//shift right not considering the sign bit
	void Shra(int);   		//shift right keeping the sign bit
	void Compr(int, int);   //compare the value and sets sr
	void Compri(int, int);  //compare the value with instance value and set sr
	void Getstat(int);//load SR to destination register
	void Putstat(int);//set SR the value of destination register
	void Jump(int);   //set pc to address
	void Jumpl(int);  //set pc to address if LESS ==1 else, do nothing
	void Jumpe(int);  //set pc to address if EQUAL == 1 else, do nothing
	void Jumpg(int);  //set pc to address if GREATER == 1 else, do nothing
	void Call(int);	 //push VM status, set pc to address
	void Return();   //pop and restore VM status
	void Read(int);	 // read new content of r[RD] from .in file
	void Write(int);  //write r[RD] into .out file
	void Halt();	 //halt execution
	void Noop();   //no operation
	


	friend class OS;
};
