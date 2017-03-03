#include <iostream>
#include "Assembler.h"
#include "VirtualMachine.h"

using namespace std;

int main()
{
	string infilename;
	cin>> infilename;
	Assembler assembler;
	assembler.assemble(infilename);

	VirtualMachine vm(infilename);
	vm.Execute();
}
