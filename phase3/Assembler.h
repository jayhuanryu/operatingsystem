#include <fstream>
#include <string>
#include <vector>

using namespace std;
class Assembler
{

private:
	string outfilename;
	string opcode;
	int rd;
	int operand;
	int instruction;
	vector<int> addrs; //storing address, to be compared with addr and number of line to check runtime error

public:
	Assembler();
	void assemble(string);
	int getinstruction(ifstream&);
	int format1(int, int, ifstream&);
	int format2(int, int, ifstream&);
	int format3(int, int, ifstream&); // this will take care of read or write which only takes in only 1 parameter or none.
	int format4(int);// read in no rs nor rd
	int format5(int, int, ifstream&);
	void checkruntimeerror(int);

};




