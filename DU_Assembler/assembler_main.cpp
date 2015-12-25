#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <memory>
#include <regex>

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
					{
						if (nt == floating)
							return make_unique<instr_ADD2<FL>>(cond, nt, s, a1, a2);
						else
							return make_unique<instr_ADD2<INT>>(cond, nt, s, a1, a2);
					}
					else if (command_conditionless == "SUB")
					{
						if (nt == floating)
							return make_unique<instr_SUB2<FL>>(cond, nt, s, a1, a2);
						else
							return make_unique<instr_SUB2<INT>>(cond, nt, s, a1, a2);
					}
					else if (command_conditionless == "MUL")
					{
						if (nt == floating)
							return make_unique<instr_MUL2<FL>>(cond, nt, s, a1, a2);
						else
							return make_unique<instr_MUL2<INT>>(cond, nt, s, a1, a2);
					}
					else if (command_conditionless == "DIV")
					{
						if (nt == floating)
							return make_unique<instr_DIV2<FL>>(cond, nt, s, a1, a2);
						else
							return make_unique<instr_DIV2<INT>>(cond, nt, s, a1, a2);
					}
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

			io_type cmd_type;
			if (command_conditionless == "IN")
				cmd_type = in;
			else if (command_conditionless == "OUT")
				cmd_type = out;

			if (nt == floating)
				return make_unique<instr_IO<FL>>(cond, cmd_type, iarg);
			else
				return make_unique<instr_IO<INT>>(cond, cmd_type, iarg);
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

				int s = stoi(arg1.substr(1, arg1.length() - 1));
				float  a1 = stof(arg2.substr(1, arg2.length() - 1));

				if (command_conditionless == "LDC")
				{
					s = stoi(arg1.substr(1, arg1.length() - 1));
					a1 = stof(words[index + 2]);
				}

				if (s < 0 || s > 255 || a1 < 0 || a1 > 255)
				{
					line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Argument out of range.");
				}

				int cond = return_condition(condition, line_number, fail);

				//cout << "Returning: " << "Codnition: " << cond << " store: " << s << " arg1: " << a1 << " type: " << nt << endl;

				if (command_conditionless == "LD")
					if (nt == floating)
						return make_unique<instr_LDST2<FL>>(cond, load, nt, s, (int)a1);
					else
						return make_unique<instr_LDST2<INT>>(cond, load, nt, s, (int)a1);
				else if (command_conditionless == "ST")
					if (nt == floating)
						return make_unique<instr_LDST2<FL>>(cond, store, nt, s, (int)a1);
					else
						return make_unique<instr_LDST2<INT>>(cond, store, nt, s, (int)a1);
				else if (command_conditionless == "LDC")
				{
					if (nt == floating)
						return make_unique<instr_LDC<FL>>(cond, s, a1);
					else
					{
						int b = (int)a1;
						return make_unique<instr_LDC<INT>>(cond, s, b);
					}
				}


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
		else if (command_conditionless == "MOV")
		{
			if (number_arguments != 2)
			{
				wrong_arguments("MOV", number_arguments, line_number, fail);
			}
			string arg1 = words[index + 1];
			string arg2 = words[index + 2];

			if (arg1[0] == arg2[0])
			{
				if (arg2[0] == 'F')
					nt = floating;
				else if (arg2[0] == 'R')
					nt = integer;
				else
					line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of MOV is of unkown type!");

				int a1 = stoi(arg1.substr(1, arg1.length() - 1));
				int a2 = stoi(arg2.substr(1, arg2.length() - 1));

				if (a2 < 0 || a2 > 255 || a1 < 0 || a1 > 255)
				{
					line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Argument out of range.");
				}

				int cond = return_condition(condition, line_number, fail);

				if (nt == floating)
					return make_unique<instr_MOV<FL>>(cond, a1, a2);
				else
					return make_unique<instr_MOV<INT>>(cond, a1, a2);
			}
			else
			{
				line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Arguments of MOV must be of the same type!");
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

			string cmp_relation = command_conditionless.substr(3, 2);

			//	cout << "Returning: " << "Codnition: " << cond << " predicates: " << p1 << p2 << " arg1: " << a1 << " arg2: " << a2 << " type: " << nt << tp << endl;

			cmp_type e_relation;

			if (cmp_relation == "EQ")
				e_relation = eq;
			if (cmp_relation == "NE")
				e_relation = ne;
			if (cmp_relation == "GE")
				e_relation = ge;
			if (cmp_relation == "GT")
				e_relation = gt;
			if (cmp_relation == "LE")
				e_relation = le;
			if (cmp_relation == "LT")
				e_relation = lt;

			if (nt == floating)
				return make_unique<instr_CMP<FL>>(cond, p1, p2, a1, a2, e_relation);
			if (nt == integer)
				return make_unique<instr_CMP<INT>>(cond, p1, p2, a1, a2, e_relation);

		}
	}
	catch (invalid_argument e)
	{
		fail = true;
		cout << "On line " << line_number << " selected index is not a number. Exiting program." << endl;
		return make_unique<instruction>(-1);
	}
	
	// if it is not a valid instruction
	if (fail == false)
		line_cannot_be_parsed(fail, "On line " + to_string(line_number) + ". Invalid command found.");

	return make_unique<instruction>(-1);
}

int condition(string cond)
{
	int ret = 1;
	if (cond != "")
		ret = stoi(cond);
	return ret;
}

unique_ptr< instruction> regextests(string line, std::unordered_map<std::string, int>& navesti_given, std::vector<string>& navesti_needed, int instr_number, int line_number, bool& fail)
{
	// convert all to uppercase
	for (size_t i = 0; i < line.length(); i++)
	{
		line[i] = toupper(line[i]);
	}
	cout << ".";
	
	string regexes[27];
	regexes[0] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(ADD)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*=[[:space:]]*R([0-9]+)[[:space:]]*,[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[13] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(ADD)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*=[[:space:]]*F([0-9]+)[[:space:]]*,[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	regexes[10] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(SUB)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*=[[:space:]]*R([0-9]+)[[:space:]]*,[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[23] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(SUB)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*=[[:space:]]*F([0-9]+)[[:space:]]*,[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	regexes[11] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(MUL)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*=[[:space:]]*R([0-9]+)[[:space:]]*,[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[24] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(MUL)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*=[[:space:]]*F([0-9]+)[[:space:]]*,[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	regexes[12] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(DIV)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*=[[:space:]]*R([0-9]+)[[:space:]]*,[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[25] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(DIV)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*=[[:space:]]*F([0-9]+)[[:space:]]*,[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	regexes[1] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(JMP)(\\.P([0-9]+))?[[:space:]]*([a-z0-9A-Z]+)[[:space:]]*(;.*)?";

	regexes[2] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(CMP)([A-Z]{2})(\\.P([0-9]+))?[[:space:]]*P([0-9]+)[[:space:]]*,[[:space:]]*P([0-9]+)[[:space:]]*=[[:space:]]*R([0-9]+)[[:space:]]*,[[:space:]]*R([0-9]*)[[:space:]]*(;.*)?";
	regexes[15] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(CMP)([A-Z]{2})(\\.P([0-9]+))?[[:space:]]*P([0-9]+)[[:space:]]*,[[:space:]]*P([0-9]+)[[:space:]]*=[[:space:]]*F([0-9]+)[[:space:]]*,[[:space:]]*F([0-9]*)[[:space:]]*(;.*)?";

	regexes[3] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(CVRT)(\\.P([0-9]*))?[[:space:]]*F([0-9]+)[[:space:]]*=[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[16] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(CVRT)(\\.P([0-9]*))?[[:space:]]*R([0-9]+)[[:space:]]*=[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	regexes[4] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(MOV)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*=[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[17] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(MOV)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*=[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	regexes[5] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(LDC)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*=[[:space:]]*([0-9]+)[[:space:]]*(;.*)?";
	regexes[18] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(LDC)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*=[[:space:]]*([.0-9]+)[[:space:]]*(;.*)?";

	regexes[6] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(ST)(\\.P([0-9]+))?[[:space:]]*\\[R([0-9]+)\\][[:space:]]*=[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[19] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(ST)(\\.P([0-9]+))?[[:space:]]*\\[R([0-9]+)\\][[:space:]]*=[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	regexes[7] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(LD)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*=[[:space:]]*\\[R([0-9]+)\\][[:space:]]*(;.*)?";
	regexes[20] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(LD)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*=[[:space:]]*\\[R([0-9]+)\\][[:space:]]*(;.*)?";

	regexes[8] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(OUT)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[21] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(OUT)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	regexes[9] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(IN)(\\.P([0-9]+))?[[:space:]]*R([0-9]+)[[:space:]]*(;.*)?";
	regexes[22] = "^[[:space:]]*(([A-Za-z0-9]+):)?[[:space:]]*(IN)(\\.P([0-9]+))?[[:space:]]*F([0-9]+)[[:space:]]*(;.*)?";

	std::smatch mac;
	unique_ptr< instruction> return_ptr;
	
	// ADD
	if (regex_match(line, mac, std::regex(regexes[0], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_ADD2<INT>>(condition(mac.str(5)), integer, stoi(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)));
	}
	else if (regex_match(line, mac, std::regex(regexes[13], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_ADD2<FL >>(condition(mac.str(5)), floating, stoi(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)));
	}

	// SUB
	else if (regex_match(line, mac, std::regex(regexes[10], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_SUB2<INT>>(condition(mac.str(5)), integer, stoi(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)));
	}
	else if (regex_match(line, mac, std::regex(regexes[23], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_SUB2<FL>>(condition(mac.str(5)), floating, stoi(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)));
	}

	// MUL
	else if (regex_match(line, mac, std::regex(regexes[11], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_MUL2<INT>>(condition(mac.str(5)), integer, stoi(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)));
	}
	else if (regex_match(line, mac, std::regex(regexes[24], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_MUL2<FL>>(condition(mac.str(5)), floating, stoi(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)));
	}

	// DIV
	else if (regex_match(line, mac, std::regex(regexes[12], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_DIV2<INT>>(condition(mac.str(5)), integer, stoi(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)));
	}
	else if (regex_match(line, mac, std::regex(regexes[25], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_DIV2<FL>>(condition(mac.str(5)), floating, stoi(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)));
	}

	// JMP
	else if (regex_match(line, mac, std::regex(regexes[1], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_JMP>(mac.str(6), condition(mac.str(5)));
	}

	// CMP
	else if (regex_match(line, mac, std::regex(regexes[2], std::regex_constants::icase)))
	{
		string cmp_relation = mac[4].str();
		cmp_type e_relation;
		if (cmp_relation == "EQ")
			e_relation = eq;
		if (cmp_relation == "NE")
			e_relation = ne;
		if (cmp_relation == "GE")
			e_relation = ge;
		if (cmp_relation == "GT")
			e_relation = gt;
		if (cmp_relation == "LE")
			e_relation = le;
		if (cmp_relation == "LT")
			e_relation = lt;
	
		return_ptr = make_unique<instr_CMP<INT>>(condition(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)), stoi(mac.str(9)), stoi(mac.str(10)), e_relation);
	}
	else if (regex_match(line, mac, std::regex(regexes[15], std::regex_constants::icase)))
	{
		string cmp_relation = mac[4].str();
		cmp_type e_relation;
		if (cmp_relation == "EQ")
			e_relation = eq;
		if (cmp_relation == "NE")
			e_relation = ne;
		if (cmp_relation == "GE")
			e_relation = ge;
		if (cmp_relation == "GT")
			e_relation = gt;
		if (cmp_relation == "LE")
			e_relation = le;
		if (cmp_relation == "LT")
			e_relation = lt;
		return_ptr = make_unique<instr_CMP<FL>>(condition(mac.str(6)), stoi(mac.str(7)), stoi(mac.str(8)), stoi(mac.str(9)), stoi(mac.str(10)), e_relation);
	}

	// CVRT
	else if (regex_match(line, mac, std::regex(regexes[3], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_CVRT>(condition(mac.str(5)), integer, stoi(mac.str(6)), stoi(mac.str(7)));
	}
	else if (regex_match(line, mac, std::regex(regexes[16], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_CVRT>(condition(mac.str(5)), floating , stoi(mac.str(6)), stoi(mac.str(7)));
	}

	// MOV
	else if (regex_match(line, mac, std::regex(regexes[17], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_MOV<FL>>(condition(mac.str(5)), stoi(mac.str(6)), stoi(mac.str(7)));
	}
	else if (regex_match(line, mac, std::regex(regexes[4], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_MOV<INT>>(condition(mac.str(5)), stoi(mac.str(6)), stoi(mac.str(7)));
	}

	// LDC
	else if (regex_match(line, mac, std::regex(regexes[5], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_LDC<INT>>(condition(mac.str(5)), stoi(mac.str(6)), stoi(mac.str(7)));
	}
	else if (regex_match(line, mac, std::regex(regexes[18], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_LDC<FL>>(condition(mac.str(5)), stoi(mac.str(6)), stof(mac.str(7)));
	}

	// ST
	else if (regex_match(line, mac, std::regex(regexes[6], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_LDST2<INT>>(condition(mac.str(5)), store, integer, stoi(mac.str(6)), stoi(mac.str(7)));
	}
	else if (regex_match(line, mac, std::regex(regexes[19], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_LDST2<FL>>(condition(mac.str(5)), store, floating, stoi(mac.str(6)), stoi(mac.str(7)));
	}

	// LD
	else if (regex_match(line, mac, std::regex(regexes[7], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_LDST2<INT>>(condition(mac.str(5)), load, integer, stoi(mac.str(6)), stoi(mac.str(7)));
	}
	else if (regex_match(line, mac, std::regex(regexes[20], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_LDST2<FL>>(condition(mac.str(5)), load, floating, stoi(mac.str(6)), stoi(mac.str(7)));
	}

	// OUT
	else if (regex_match(line, mac, std::regex(regexes[8], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_IO<INT>>(condition(mac.str(5)), out, stoi(mac.str(6)));
	}
	else if (regex_match(line, mac, std::regex(regexes[21], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_IO<FL>>(condition(mac.str(5)), out, stoi(mac.str(6)));
	}

	// IN
	else if (regex_match(line, mac, std::regex(regexes[9], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_IO<INT>>(condition(mac.str(5)), in, stoi(mac.str(6)));
	}
	else if (regex_match(line, mac, std::regex(regexes[22], std::regex_constants::icase)))
	{
		return_ptr = make_unique<instr_IO<FL>>(condition(mac.str(5)), in, stoi(mac.str(6)));
	}
	else
	{
		size_t pos = line.find(';');

		if( !( line == "" || pos != string::npos ))
			line_cannot_be_parsed(fail, "\n\nOn line " + to_string(line_number) + ". Unknown instruction found.");
			
		return_ptr = make_unique<instruction>(-1);
	}
	
	if (mac[2] != "")
	{
		string navesti = mac[2].str();
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
	}

	return return_ptr;
}

// Calls parse_line on each line, exits if failure happened, pushes the instructions into the proper vector
bool read_input(file_resource& in, vector< unique_ptr< instruction > >& out_vec, std::unordered_map<std::string, int>& navesti_given, std::vector<string>& navesti_needed)
{
	bool fail = false;
	string line;
	int line_num = 1;

	while (getline(in.ifs, line))
	{
		unique_ptr<instruction> tmp = regextests(line, navesti_given, navesti_needed, out_vec.size(), line_num, fail);

		//unique_ptr<instruction> tmp = move(parse_line(line, line_num, navesti_given, navesti_needed, out_vec.size(), fail ));

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

	std::vector< unique_ptr< instruction > > asmb_instructions;
	std::unordered_map<std::string, int> navesti_given;
	std::vector<string> navesti_needed;
	
	// Read input into instructions, already checking parameters and validity of parameters.
	cout << ">> Parsing:\n   ";
	bool fail_state = read_input(inFile, asmb_instructions, navesti_given, navesti_needed);
	if (fail_state) return 0;
	cout << endl << "\n>> Parsing is complete." << endl << endl;

	// We do a check of navesti needed and navesti provided
	for (auto navesti : navesti_needed)
	{
		auto finding = navesti_given.find(navesti);
		if (finding == navesti_given.end())
		{
			cout << "Navesti " << navesti << " not found. Please add this navesti if you want to jump to it. Exiting." << endl;
			return 0;
			break;
		}
	}
	cout  << ">> Validating is complete. Running simulation... " << endl << endl;

	// run and simulate processor
	navestiPtr navesti = make_unique< std::unordered_map<std::string, int> >(move(navesti_given));
	ProgramData program_data(256, 256, 256, move(navesti));
	program_data.init();
	
	while (program_data.prg_counter < asmb_instructions.size())
	{
		int tmp = program_data.prg_counter;
		try
		{
			asmb_instructions[program_data.prg_counter]->execute(program_data);
		}
		catch (assembler_exception e)
		{
			cout << e.what_failed << " Exit." << endl;
			return 0;
		}
		if(tmp == program_data.prg_counter)
			program_data.prg_counter++;
	}
	
	cout << "\n>> The end.\n\n";

	return 0;
}