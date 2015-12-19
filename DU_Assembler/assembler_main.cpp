#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <memory>

#include "instruction.h"

using namespace std;

class file_resource
{
	private:
		bool state_;

	public:
		ifstream ifs;
	
		int getState() { return state_; }

		file_resource()
		{
			state_ = false;
		}

		file_resource(string filename)
		{
			ifs.open(filename);	
			if (ifs.is_open()) { state_ = true; }
			else { state_ = false; }
		}
		~file_resource()
		{
			if(ifs.is_open())
				ifs.close();
		}
};

instruction parse_line(string line)
{
	cout << "ParseLine - Doing : " << line << endl;
	instr_JMP parsed("nav1",1);
	
	// actually parsing
	// ...

	return move(parsed);
}

void read_input(file_resource& in, vector<instruction>& out_vec)
{
	string line;

	// some kind of while cycle
	while (getline(in.ifs, line))
	{
		out_vec.push_back(parse_line(line));
	}
}

int main(int argc, char**argv)
{
	// We check if parametrs are correct
	if (argc != 2)
	{
		cout << "Wrong number of arguments (" << argc - 1 << "). Pass one assembler file, please. Exiting." << endl;
		return 0;
	}

	file_resource inFile((string)((argv[1])));

	// Read input into instructions
	vector<instruction> asmb_instructions(100);
	read_input(inFile, asmb_instructions);

	// precompile
	// ...

	// run and simulate processor
	// ...


	// TEST AREA
	vector< unique_ptr<instruction> > vec;

	vec.push_back(move(make_unique<instr_DIV>(1, floating, 10, 5, 6)));
	vec.push_back(move(make_unique<instr_SUB>(1, floating, 10, 5, 6)));
	vec.push_back(move(make_unique<instr_MUL>(1, floating, 10, 5, 6)));
	vec.push_back(move(make_unique<instr_ADD>(1, integer, 10, 5, 6)));
	vec.push_back(move(make_unique<instr_DIV>(1, integer, 10, 5, 6)));
	vec.push_back(move(make_unique<instr_MUL>(1, integer, 10, 5, 6)));
	vec.push_back(move(make_unique<instr_JMP>("TU",1)));
	vec.push_back(move(make_unique<instr_JMP>("TAM", 1)));
	vec.push_back( move( make_unique<instr_LDST>(1, load, integer, 100, 6) ) );
	vec.push_back(move(make_unique<instr_LDST>(1, store, integer, 120, 121)));
	vec.push_back(move(make_unique<instr_LDST>(1, load, floating, 100, 6)));
	vec.push_back(move(make_unique<instr_LDST>(1, store, floating, 120, 121)));
	vec.push_back(move(make_unique<instr_LDC>(1,integer,254,987)));
	vec.push_back(move(make_unique<instr_LDC>(1, floating, 254, (float)987.5)));
	vec.push_back(move(make_unique<instr_IO>(1, integer, out, 20)));
	vec.push_back(move(make_unique<instr_IO>(1, integer, in, 20)));
	vec.push_back(move(make_unique<instr_IO>(1, integer, out, 20)));

	vec.push_back(move(make_unique<instr_IO>(1, integer, in, 110)));
	vec.push_back(move(make_unique<instr_IO>(1, floating, in, 210)));

	vec.push_back(move(make_unique<instr_CVRT>(1, floating, 210, 111)));
	vec.push_back(move(make_unique<instr_CVRT>(1, integer, 110, 211)));

	vec.push_back(move(make_unique<instr_IO>(1, integer, out, 111)));
	vec.push_back(move(make_unique<instr_IO>(1, floating, out, 211)));
	
	vec.push_back(move(make_unique<instr_IO>(1, floating, in, 220)));
	vec.push_back(move(make_unique<instr_IO>(1, floating, in, 221)));
	vec.push_back(move(make_unique<instr_CMP>(1, 100,101, 220,221,floating, eq)));
	vec.push_back(move(make_unique<instr_CMP>(1, 102, 103, 220, 221, floating, ne)));
	vec.push_back(move(make_unique<instr_CMP>(1, 104, 105, 220, 221, floating, ge)));
	vec.push_back(move(make_unique<instr_CMP>(1, 106, 107, 220, 221, floating, gt)));
	vec.push_back(move(make_unique<instr_CMP>(1, 108, 109, 220, 221, floating, le)));
	vec.push_back(move(make_unique<instr_CMP>(1, 110, 111, 220, 221, floating, lt)));

	ProgramData prg;
	prg.I_register = std::make_unique<std::vector<int> >(256);
	prg.F_register = std::make_unique<std::vector<float> >(256);

	prg.I_register->at(5) = 10;
	prg.I_register->at(6) = 20;
	prg.I_register->at(20) = 1000;
	prg.I_register->at(120) = 122;
	prg.I_register->at(121) = 666;


	prg.F_register->at(5) = 10;
	prg.F_register->at(6) = 20;
	prg.F_register->at(20) = 1000;
	prg.F_register->at(120) = 122;
	prg.F_register->at(121) = 666;


	prg.P_register = std::make_unique<std::vector<bool> >(256);
	prg.P_register->at(1) = true;

	prg.navesti = std::make_unique< std::unordered_map<std::string, int> >();
	prg.prg_counter = 0;

	(*prg.navesti)["TU"] = 2;
	(*prg.navesti)["TAM"] = 5;

	int count = 0;
	while (prg.prg_counter < vec.size())
	{		
		vec[prg.prg_counter]->execute(prg);		
		prg.prg_counter++;
		count++;
		if (count > 20 && prg.P_register->at(1) == true)
		{
			cout << "terminate\n";
			prg.P_register->at(1) = false;
		}
	}
	
	cout << prg.I_register->at(100);
	cout << prg.I_register->at(122);

	cout << prg.F_register->at(100);
	cout << prg.F_register->at(122);

	cout << endl << prg.I_register->at(254);
	cout << endl << prg.F_register->at(254);

	cout << endl << prg.F_register->at(211) << endl;
	cout << endl << endl << "EQ " << (bool)prg.P_register->at(100) << " " << (bool)prg.P_register->at(101) << endl;
	cout << "NE " << (bool)prg.P_register->at(102) << " " << (bool)prg.P_register->at(103) << endl;
	cout << "GE " << (bool)prg.P_register->at(104) << " " << (bool)prg.P_register->at(105) << endl;
	cout << "GT " << (bool)prg.P_register->at(106) << " " << (bool)prg.P_register->at(107) << endl;
	cout << "LE " << (bool)prg.P_register->at(108) << " " << (bool)prg.P_register->at(109) << endl;
	cout << "LT " << (bool)prg.P_register->at(110) << " " << (bool)prg.P_register->at(111) << endl;
	return 0;
}

/*

rozdil mezi:

file_resource inFile2(filename);
cout << inFile2.getState();

file_resource inFile3( (string)((argv[1])));
cout << inFile3.getState();

file_resource inFile3( string((argv[1])) );
cout << inFile3.getState();

???????

Prvni faka, druhe take, treti vubec
*/
