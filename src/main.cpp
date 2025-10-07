#include <fstream>
#include <sstream>

#include "ir.hpp"

import vm;
import logger;

int main(int argc, char** argv)
{
	std::string stra = "";
	if (argc == 1)
		stra = "1.uir";
	else
		stra = argv[1];

	std::ifstream ifs(stra);

	std::stringstream in;
	in << ifs.rdbuf();
	std::string str(in.str());

	char* mem = new char[1024];

	int len = generate_ir(str, &mem);

	Log.outputFormat = "VM: INFO";
	Log.colorEnabled = false;
	VMInstance vm(reinterpret_cast<uint8_t*>(mem), len);

	vm.run();

	return 0;
}
