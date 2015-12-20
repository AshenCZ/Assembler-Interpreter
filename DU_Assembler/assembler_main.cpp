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

// Two helper announcement methods, line_cannot_be_parsed and catch statement are the only places where fail is set.
void line_cannot_be_parsed(bool& fail, string message)
{
	fail = true;
	cout << message << " Exit." << endl;
}
void wrong_arguments(string instr, int num, int line_number, bool& fail)
{
	string a = "On line " + to_string(line_number);
	a += ", invalid number of arguments (" + to_string(num) + ") for " + instr + ".";

	line_cannot_be_parsed(fail, a);
}

// reads string and returns a valid condition, or sets fail on failiure
int return_condition(string condition, int line_number, bool fail)
{
	int cond = 1;
	if (condition != "")
	{
		if (condition[0] != 'P')
			line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Predicate must be of type P.");

		cond = stoi(condition.substr(1, condition.length() - 1));
		if (cond < 0 || cond > 255)
			line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Predicate out of range.");
	}
	return cond;
}

// Tokenizes one line (splits)
int tokenize(string line, string (&words)[200]) 
{
	int word_count = 0;
	string word = "";

	//Tokenizing = split into strings
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
		else if (line[i] == ';' && word.length() != 0) // no more spaces, we enter the next mode
		{
			words[word_count] = word;
			word = "";
			word_count++;
			break;
		}
		else if (line[i] == ';' && word.length() == 0) // no more spaces, we enter the next mode
		{
			break;
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
	}
	if (word != "")
	{
		words[word_count] = word;
		word_count++;
	}

	return word_count;
}

/* Parses one line, by:
	- tokenizing the line into strings (basically split)
	- solving navesti and predicate statements
	- recognizing the command and reading and checking its parameters
*/
unique_ptr< instruction> parse_line(string line, int line_number, std::unordered_map<std::string, int>& navesti_given, std::vector<string>& navesti_needed, int instr_number, bool& fail)
{
	// convert all to uppercase
	for (size_t i = 0; i < line.length(); i++)
	{
		line[i] = toupper(line[i]);
	}

	string navesti = "";
	string cmd = "";

	string words[200];
	int word_count = tokenize(line, words);

	// skip empty line
	if (word_count == 0)
	{
		return make_unique<instruction>(-1);
	}

	// TOKENIZED at this point.

	// Begin to recognize the tokens!
	string command_conditionless = "";
	string condition = "";
	int index = 0;

	// navesti
	if (words[0].back() == ':')
	{
		navesti = words[0].substr(0, words[0].length() - 1);
		auto finding = navesti_given.find(navesti);
		if (finding == navesti_given.end())
		{
			navesti_given.insert({ navesti, instr_number });
		}
		else
		{
			line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Navesti " + navesti + " is a double!");
			return make_unique<instruction>(-1);
		}
		index = 1;
	}

	// predicate
	size_t dot = words[index].find('.');
	if (dot != string::npos)
	{
		condition = words[index].substr(dot + 1, words[index].length() - dot);
		command_conditionless = words[index].substr(0, dot);
	}
	else
	{
		command_conditionless = words[index];
	}

	// recognize command
	int number_arguments = word_count - index - 1;
	number_type nt;

	try { // catching for stoi() exception, for converting numbers from P151
		if (command_conditionless == "ADD" || command_conditionless == "SUB" ||
			command_conditionless == "MUL" || command_conditionless == "DIV")
		{
			if (number_arguments != 3)
			{
				wrong_arguments(command_conditionless, number_arguments, line_number, fail);
			}
			else
			{
				string arg1 = words[index + 1];
				string arg2 = words[index + 2];
				string arg3 = words[index + 3];
				if (arg1[0] == arg2[0] && arg1[0] == arg3[0])
				{
					if (arg1[0] == 'R')
					{
						nt = integer;
					}
					else if (arg1[0] == 'F')
						nt = floating;
					else
						line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of unkown type '" + arg1[0] + "' (not F or R).");

					int s = stoi(arg1.substr(1, arg1.length() - 1));
					int a1 = stoi(arg2.substr(1, arg2.length() - 1));
					int a2 = stoi(arg3.substr(1, arg3.length() - 1));

					if (s < 0 || s > 255 || a1 < 0 || a1 > 255 || a2 < 0 || a2 > 255)
					{
						line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Argument out of range.");
					}

					int cond = return_condition(condition, line_number, fail);

					//cout << "Returning: " << "Codnition: " << cond << " store: " << s << " arg1: " << a1 << " arg2: " << a2 << " type: " << nt << endl;

					if (command_conditionless == "ADD")
						return make_unique<instr_ADD>(cond, nt, s, a1, a2);
					else if (command_conditionless == "SUB")
						return make_unique<instr_SUB>(cond, nt, s, a1, a2);
					else if (command_conditionless == "MUL")
						return make_unique<instr_MUL>(cond, nt, s, a1, a2);
					else if (command_conditionless == "DIV")
						return make_unique<instr_DIV>(cond, nt, s, a1, a2);
				}
				else
				{
					line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of ADD/SUB/MUL/DIV must be of the same type!");
				}
			}
		}
		else if (command_conditionless == "OUT" || command_conditionless == "IN")
		{
			if (number_arguments != 1)
			{
				wrong_arguments(command_conditionless, number_arguments, line_number, fail);
			}

			string arg = words[index + 1];
			int iarg = stoi(arg.substr(1, arg.length() - 1));


			if (iarg < 0 || iarg > 255)
			{
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Argument out of range.");
			}

			int cond = return_condition(condition, line_number, fail);

			if (arg[0] == 'R')
			{
				nt = integer;
			}
			else if (arg[0] == 'F')
				nt = floating;
			else
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of unkown type '" + arg[0] + "' (not F or R).");

			//cout << "Returning: " << "Codnition: " << cond << " arg1: " << iarg << " type: " << nt << endl;

			if (command_conditionless == "IN")
				return make_unique<instr_IO>(cond, nt, in, iarg);
			else if (command_conditionless == "OUT")
				return make_unique<instr_IO>(cond, nt, out, iarg);
		}
		else if (command_conditionless == "LDC" || command_conditionless == "ST" ||
			command_conditionless == "LD")
		{
			if (number_arguments != 2)
			{
				wrong_arguments(command_conditionless, number_arguments, line_number, fail);
			}
			string arg1 = words[index + 1];
			string arg2 = words[index + 2];
			if (command_conditionless == "LDC")
			{
				arg2 = words[index + 1];
			}
			else if (command_conditionless == "LD")
			{
				arg2 = arg2.substr(1, arg2.length() - 2);
			}
			else if (command_conditionless == "ST")
			{
				arg1 = arg1.substr(1, arg1.length() - 2);
			}
			if (arg1[0] == arg2[0])
			{
				string check;
				if (command_conditionless == "LD" || command_conditionless == "LDC")
					check = arg1;
				else
					check = arg2;
				if (check[0] == 'R')
				{
					nt = integer;
				}
				else if (check[0] == 'F')
					nt = floating;
				else
					line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of unkown type '" + check[0] + "' (not F or R).");

				int s, a1;
				s = stoi(arg1.substr(1, arg1.length() - 1));
				a1 = stoi(arg2.substr(1, arg2.length() - 1));
				if (command_conditionless == "LDC")
				{
					s = stoi(arg1.substr(1, arg1.length() - 1));
					a1 = stoi(words[index + 2]);
				}

				if (s < 0 || s > 255 || a1 < 0 || a1 > 255)
				{
					line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Argument out of range.");
				}

				int cond = return_condition(condition, line_number, fail);

				//cout << "Returning: " << "Codnition: " << cond << " store: " << s << " arg1: " << a1 << " type: " << nt << endl;

				if (command_conditionless == "LD")
					return make_unique<instr_LDST>(cond, load, nt, s, a1);
				else if (command_conditionless == "ST")
					return make_unique<instr_LDST>(cond, store, nt, s, a1);
				else if (command_conditionless == "LDC")
					return make_unique<instr_LDC>(cond, nt, s, a1);
			}
			else
			{
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of LD/ST/LDC must be of the same type!");
			}
		}
		else if (command_conditionless == "CVRT")
		{
			if (number_arguments != 2)
			{
				wrong_arguments("CVRT", number_arguments, line_number, fail);
			}
			string arg1 = words[index + 1];
			string arg2 = words[index + 2];

			if (arg1[0] != arg2[0])
			{
				if (arg2[0] == 'F')
					nt = floating;
				else if (arg2[0] == 'R')
					nt = integer;
				else
					line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of CVRT is of unkown type!");

				int a1 = stoi(arg1.substr(1, arg1.length() - 1));
				int a2 = stoi(arg2.substr(1, arg2.length() - 1));

				if (a2 < 0 || a2 > 255 || a1 < 0 || a1 > 255)
				{
					line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Argument out of range.");
				}

				int cond = return_condition(condition, line_number, fail);

				//cout << "Returning: " << "Codnition: " << cond << " store: " << a1 << " arg1: " << a2 << " type: " << nt << endl;

				return make_unique<instr_CVRT>(cond, nt, a2, a1);
			}
			else
			{
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of CVRT must be of a different type!");
			}
		}
		else if (command_conditionless == "JMP")
		{
			if (number_arguments != 1)
			{
				wrong_arguments("JMP", number_arguments, line_number, fail);
			}

			int cond = return_condition(condition, line_number, fail);

			string nav = words[index + 1];
			//cout << "Returning: " << "Codnition: " << cond << " jumpto: " << nav << endl;

			// we remember the need for this jumpto to be valid
			navesti_needed.push_back(nav);

			return make_unique<instr_JMP>(nav, cond);

		}
		else if (command_conditionless.substr(0, 3) == "CMP")
		{
			if (number_arguments != 4)
			{
				wrong_arguments("CMP", number_arguments, line_number, fail);
			}

			int cond = return_condition(condition, line_number, fail);

			string pstr1 = words[index + 1];
			string pstr2 = words[index + 2];
			string arg1 = words[index + 3];
			string arg2 = words[index + 4];

			if (pstr1[0] != 'P' || pstr2[0] != 'P')
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". CMP has to have 2 predicates.");

			if (arg1[0] != arg2[0])
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". CMP has to have 2 args of the same type.");

			if (arg1[0] == 'F')
				nt = floating;
			else if (arg1[0] == 'R')
				nt = integer;
			else
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Argument of unknown type.");

			int p1 = stoi(pstr1.substr(1, pstr1.length() - 1));
			int p2 = stoi(pstr2.substr(1, pstr2.length() - 1));
			int a1 = stoi(arg1.substr(1, arg1.length() - 1));
			int a2 = stoi(arg2.substr(1, arg2.length() - 1));

			if (p1 < 0 || p1 > 255 || p2 < 0 || p2 > 255 || a1 < 0 || a1 > 255 || a2 < 0 || a2 > 255)
			{
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Argument out of range.");
			}

			string tp = command_conditionless.substr(3, 2);

			//	cout << "Returning: " << "Codnition: " << cond << " predicates: " << p1 << p2 << " arg1: " << a1 << " arg2: " << a2 << " type: " << nt << tp << endl;

			if (tp == "EQ")

				return make_unique<instr_CMP>(cond, p1, p2, a1, a2, nt, eq);
			if (tp == "NE")
				return make_unique<instr_CMP>(cond, p1, p2, a1, a2, nt, ne);
			if (tp == "GE")
				return make_unique<instr_CMP>(cond, p1, p2, a1, a2, nt, ge);
			if (tp == "GT")
				return make_unique<instr_CMP>(cond, p1, p2, a1, a2, nt, gt);
			if (tp == "LE")
				return make_unique<instr_CMP>(cond, p1, p2, a1, a2, nt, le);
			if (tp == "LT")
				return make_unique<instr_CMP>(cond, p1, p2, a1, a2, nt, lt);
		}
	}
	catch (invalid_argument e)
	{
		fail = true;
		cout << "On line " << line_number << " selected index is not a number. Exiting program." << endl;
		return make_unique<instruction>(-1);
	}
	
	// if it is not a valid instruction
	line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Invalid command found. Line skipped.");
	return make_unique<instruction>(-1);
}

// Calls parse_line on each line, exits if failure happened, pushes the instructions into the proper vector
bool read_input(file_resource& in, vector< unique_ptr< instruction > >& out_vec, std::unordered_map<std::string, int>& navesti_given, std::vector<string>& navesti_needed)
{
	bool fail = false;
	string line;
	int line_num = 1;

	while (getline(in.ifs, line))
	{
		unique_ptr<instruction> tmp = move(parse_line(line, line_num, navesti_given, navesti_needed, out_vec.size(), fail ));
		if (fail) // fail state occured, we are exiting the program
			break;
		if (tmp->get_condition() != -1) // if it's a valid instruction, we add it
			out_vec.push_back(move(tmp));
		line_num++;
	}

	return fail;
}

int main(int argc, char**argv)
{
	// We check if parametrs are correct
	if (argc != 2)
	{
		cout << "Wrong number of arguments (" << argc - 1 << "). Pass one assembler file, please. Exiting." << endl;
		return 0;
	}

	// We allocate the opened file to an object, so we close it automatically
	file_resource inFile((string)((argv[1])));

	// Read input into instructions, already checking parameters and validity of parameters.
	std::vector< unique_ptr< instruction > > asmb_instructions;
	std::unordered_map<std::string, int> navesti_given;
	std::vector<string> navesti_needed;
	bool fail_state = read_input(inFile, asmb_instructions, navesti_given, navesti_needed);
	if (fail_state)
	{
		return 0;
	}

	cout << endl << ">> Parsing is complete." << endl << endl;

	// We do a check of navesti needed and navesti provided
	bool navesti_fail = false;
	for (auto navesti : navesti_needed)
	{
		auto finding = navesti_given.find(navesti);
		if (finding == navesti_given.end())
		{
			cout << " Navesti " << navesti << " not found. Please add this navesti if you want to jump to it. Exiting." << endl;
			navesti_fail = true;
			break;
		}
	}
	if (navesti_fail)
	{
		return 0;
	}

	cout  << ">> Validating is complete. Running simulation... " << endl << endl;

	// run and simulate processor
	navestiPtr navesti = make_unique< std::unordered_map<std::string, int> >(move(navesti_given));
	ProgramData program_data(256, 256, 256, move(navesti));
	program_data.init();
	
	while (program_data.prg_counter < asmb_instructions.size())
	{
		int tmp = program_data.prg_counter;
		asmb_instructions[program_data.prg_counter]->execute(program_data);

		if(tmp == program_data.prg_counter)
			program_data.prg_counter++;

		///HACK , ///TODO for P0 a P1 being constant
		if (program_data.P_register->at(0) != false)
			program_data.P_register->at(0) = false;
		if (program_data.P_register->at(1) != true)
			program_data.P_register->at(1) = true;
	}
	
	cout << "\n>> The end.\n\n";
	return 0;
}

/*

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
