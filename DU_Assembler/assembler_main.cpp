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

enum parsing_state { first_word , command_load , arg_before_eq , args_after_eq , skip_space};

void line_cannot_be_parsed(string message)
{
	cout << message << " Exit." << endl;
}
void wrong_arguments(string instr, int num, int line_number)
{
	string a = "On line " + to_string(line_number);
	a += ", invalid number of arguments (" + to_string(num) + ") for " + instr + ".";

	line_cannot_be_parsed(a);
}

instruction parse_line(string line, int line_number)
{
	cout << "\nParseLine - Doing : " << line << endl;
	parsing_state state = first_word;
	//parsing_state future_state = first_word;
	string word = "";

	string navesti = "";
	string cmd = "";

	string words[20];
	int word_count = 0;
	
	//Tokenizing
	for (size_t i = 0; i < line.length(); i++)
	{
		if (line[i] == ' ' && word.length() == 0) // no more spaces, we enter the next mode
		{
			continue;
		}
		else if (line[i] == ' ' && word.length() != 0) // no more spaces, we enter the next mode
		{
			words[word_count] = word;
			word = "";
			word_count++;			
		}
		else if (line[i] == ',' && word.length() != 0) // no more spaces, we enter the next mode
		{
			words[word_count] = word;
			word = "";
			word_count++;
		}
		else if (line[i] == '=' && word.length() != 0) // no more spaces, we enter the next mode
		{
			words[word_count] = word;
			word = "";
			word_count++;
		}
		else if (line[i] == '=' && word.length() == 0) // no more spaces, we enter the next mode
		{
			continue;
		}
		else if (line[i] == ',' && word.length() == 0) // no more spaces, we enter the next mode
		{
			continue;
		}
		else
		{
			word += line[i];
		}

		/*
		if (line[i] == ':' && state == first_word) // mame navesti
		{
			navesti = word;
			word = "";
			state = skip_space;
			future_state = command_load;
		}
		else if (line[i] == ':' && state != first_word) // nasli sme syntax error, ":" je nekde jinde
		{
			line_cannot_be_parsed("Found : ne na miste navesti! " + line.substr(i-2,5) );
		}
		else if (line[i] == ' ' && state == first_word) // end of word
		{
			cmd = word;
			word = "";
			state = skip_space;
			future_state = arg_before_eq;
		}
		else if (line[i] == ' ' && state == skip_space) // we skip spaces
		{
			continue;
		}
		else if (line[i] != ' ' && state == skip_space) // no more spaces, we enter the next mode
		{
			word += line[i];
			state = future_state;
			future_state = skip_space;
		}
		else if (line[i] == '=' && state == skip_space) // no more spaces, we enter the next mode
		{
			word += line[i];
			state = future_state;
			future_state = skip_space;
		}*/
	}
	if (word != "")
	{
		words[word_count] = word;
		word_count++;
	}

	// TOKENIZED at this point.

	// test
	for (size_t i = 0; i < word_count; i++)
	{
		cout << "'" << words[i] << "'" << endl;
	}
	
	// Begin to recognize the tokens!
	string command_conditionless = "";
	string condition = "";
	int index = 0;

	if (words[0].back() == ':') // navesti
	{
		navesti = words[0].substr(0,words[0].length()-1);
		cout << "Navesti: '" << navesti << "'" << endl;
		index = 1;
	}

	size_t dot = words[index].find('.');
	if (dot != string::npos)
	{
		condition = words[index].substr(dot+1, words[index].length() - dot);
		command_conditionless = words[index].substr(0, dot);

		cout <<"Command: '" <<command_conditionless << "' . '" << condition << "'" << endl;
	}
	else
	{
		command_conditionless = words[index];
		cout << "Command: '" << command_conditionless << "'" << endl;
	}
			
	// recognize command

	int number_arguments = word_count - index - 1;

	if (command_conditionless == "ADD")
	{
		cout << "mame add " << endl;
		if (number_arguments != 3)
		{
			wrong_arguments("ADD", number_arguments, line_number);
		}
	}
	else if (command_conditionless == "OUT")
	{
		cout << "mame out " << endl;
		if (number_arguments != 1)
		{
			wrong_arguments("OUT", number_arguments, line_number);
		}
	}
	else if (command_conditionless == "JMP")
	{
		cout << "mame jmp " << endl;
		if (number_arguments != 1)
		{
			wrong_arguments("JMP", number_arguments, line_number);
		}
	}
	else if (command_conditionless.substr(0,3) == "CMP")
	{
		cout << "mame nejakej CMP" << endl;
		if (number_arguments != 4)
		{
			wrong_arguments("CMP", number_arguments, line_number);
		}
	}
	









	// actually parsing
	// ...

	// no returning by forcing std::move(), we are letting the compiler do what he thinks is best
	return instr_JMP("TU",1);
}

void read_input(file_resource& in, vector<instruction>& out_vec)
{
	string line;
	int line_num = 1;

	// some kind of while cycle
	while (getline(in.ifs, line))
	{
		out_vec.push_back(parse_line(line, line_num));
		line_num++;
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

	cout << "Parsing is complete. ---------- " << endl;
	// precompile
	// ...

	// run and simulate processor
	// ...


	// TEST AREA
	/*
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

	*/

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
