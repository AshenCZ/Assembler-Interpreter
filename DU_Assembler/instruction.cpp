#include "instruction.h"

int instr_ADD::do_operation(int a, int b,bool& s)
{
	s = true;
	return a + b;
}
float instr_ADD::do_operation(float a, float b, bool& s)
{
	s = true;
	return a + b;
}

int instr_SUB::do_operation(int a, int b, bool& s)
{
	s = true;
	return a - b;
}
float instr_SUB::do_operation(float a, float b, bool& s)
{
	s = true;
	return a - b;
}

int instr_MUL::do_operation(int a, int b, bool& s)
{
	s = true;
	return a * b;
}
float instr_MUL::do_operation(float a, float b, bool& s)
{
	s = true;
	return a * b;
}

int instr_DIV::do_operation(int a, int b, bool& s)
{
	if (b == 0)
	{
		s = false;
		return -1;
	}

	s = true;
	return a / b;
}
float instr_DIV::do_operation(float a, float b, bool& s)
{
	if (b == 0)
	{
		s = false;
		return -1;
	}

	s = true;
	return a / b;
}

void instr_Binary::execute(ProgramData& prg_data)
{
	std::cout << "BInary\n";
	if ((*prg_data.P_register)[condition_])
	{

		if (num_type == integer)
		{
			int a = prg_data.I_register->at(arg1_);
			int b = prg_data.I_register->at(arg2_);

			bool success;
			int result = do_operation(a, b, success);
			
			if(success)
				prg_data.I_register->at(store_) = result;			
			else
			{
				std::cout << "Operation failed. Exit." << std::endl; ///TDOD WHERE FAILED??
				throw(std::exception()); ///TODO Add exception
			}
		}
		else if (num_type == floating)
		{
			float a = prg_data.F_register->at(arg1_);
			float b = prg_data.F_register->at(arg2_);
			
			bool success;
			float result = do_operation(a, b,success);
			
			if(success)
				prg_data.F_register->at(store_) = result;
			else
			{
				std::cout << "Operation failed. Exit." << std::endl;///TDOD WHERE FAILED??
				throw(std::exception()); ///TODO Add exception
			}
		}
		else
		{
			std::cout << "Unknown number type in instruction ADD/SUB/MUL/DIV, exit." << std::endl;
			throw(std::exception()); ///TODO Add exception
		}
	}
}

void instr_JMP::execute(ProgramData& prg_data)
{
	std::cout << "Jumping to " << where_to_jump_ << std::endl;
	// find where to jump and if the condition applies, jump
	auto ret = prg_data.navesti->find(where_to_jump_);

	if (ret != prg_data.navesti->end())
	{
		if ((*prg_data.P_register)[condition_])
			prg_data.prg_counter = ret->second;
	}
}

void instr_LDST::store(std::vector<int>*  vec)
{
	if (load_or_store == load)
	{
		// load LD R15 = [R20]
		int from = vec->at(arg_); // R20 = 110
		vec->at(store_) = vec->at(from); // R15 = [110] 
		
	}
	else
	{
		// store ST [R33] = R55
		int to = vec->at(store_); // R33 = 77
		vec->at(to) = vec->at(arg_); // R15 = [110] 
	}
}
void instr_LDST::store(std::vector<float>*  vec)
{
	if (load_or_store == load)
	{
		// load LD R15 = [R20]
		int from = vec->at(arg_); // R20 = 110
		vec->at(store_) = vec->at(from); // R15 = [110] 

	}
	else
	{
		// store ST [R33] = R55
		int to = vec->at(store_); // R33 = 77
		vec->at(to) = vec->at(arg_); // R15 = [110] 
	}
}
void instr_LDST::execute(ProgramData& prg_data)
{
	if (num_type == integer)
	{
		store(prg_data.I_register.get());
	}
	else
	{
		store(prg_data.F_register.get());
	}	
}

void instr_LDC::execute(ProgramData &prg_data)
{
	if (num_type == integer)
	{
		prg_data.I_register->at(store_) = i_constant;
	}
	else if (num_type == floating)
	{
		prg_data.F_register->at(store_) = f_constant;
	}
}

void instr_IO::do_IO(std::vector<float> * vec)
{
	if (op_type == in)
	{
		std::cout << "Insert value at F[" << pos_ << "]: ";
		std::cin >> vec->at(pos_);
	}
	else if (op_type == out)
	{
		std::cout << "Value at F[" << pos_ << "] = " << vec->at(pos_) << std::endl;
	}
}
void instr_IO::do_IO(std::vector<int> * vec)
{
	if (op_type == in)
	{
		std::cout << "Insert value at R[" << pos_ << "]: ";
		std::cin >> vec->at(pos_);
	}
	else if (op_type == out)
	{
		std::cout << "Value at R[" << pos_ << "] = " << vec->at(pos_) << std::endl;
	}
}
void instr_IO::execute(ProgramData& prg_data)
{
	if (num_type == integer)
	{
		do_IO(prg_data.I_register.get());
	}
	else if (num_type == floating)
	{
		do_IO(prg_data.F_register.get());
	}
}

void instr_CVRT::execute(ProgramData& prg_data)
{
	if (num_type_ == integer)
	{
		prg_data.F_register->at(store_) = (float)(prg_data.I_register->at(pos_));
	}
	else if (num_type_ == floating)
	{
		prg_data.I_register->at(store_) = (int)(prg_data.F_register->at(pos_));
	}
}