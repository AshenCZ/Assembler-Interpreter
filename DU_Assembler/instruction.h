#pragma once
#ifndef INSTR
#define INSTR

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

using boolVecPtr = std::unique_ptr<std::vector<bool> >;
using intVecPtr = std::unique_ptr<std::vector<int> >;
using floatVecPtr = std::unique_ptr<std::vector<float> >;
using navestiPtr = std::unique_ptr< std::unordered_map<std::string, int> >;

enum number_type { integer, floating };
enum ldst_type { load, store };
enum io_type { in, out };
enum cmp_type { eq, ne, gt, lt, ge, le};

class assembler_exception : std::exception
{
public:
	std::string what_failed;
	assembler_exception() : std::exception(), what_failed("nic") {};
	assembler_exception(std::string w) : std::exception(), what_failed(w) {};
};

class ProgramData
{
public:
	unsigned int prg_counter;
	navestiPtr navesti;
	boolVecPtr P_register;
	intVecPtr I_register;
	floatVecPtr F_register;

	ProgramData() = default;
	ProgramData(int a, int b, int c)
	{
		prg_counter = 0;
		navesti = std::make_unique<std::unordered_map<std::string, int> >();
		I_register = std::make_unique<std::vector<int> >(a);
		F_register = std::make_unique<std::vector<float> >(b);
		P_register = std::make_unique<std::vector<bool> >(c);
	}
	ProgramData(int a, int b, int c, navestiPtr p)
	{
		prg_counter = 0;
		navesti = std::move(p);
		I_register = std::make_unique<std::vector<int>>(a);
		F_register = std::make_unique<std::vector<float>>(b);
		P_register = std::make_unique<std::vector<bool>>(c);
	}
	void init()
	{
		P_register->at(1) = true;
	}
};

class instruction
{
	public:
		instruction() : condition_(1) {};
		instruction(int c) : condition_(c) {};
		virtual ~instruction() = default;

		virtual void execute(ProgramData& prg_data)
		{
			if ((*prg_data.P_register)[condition_])
				p_execute(prg_data);
		}
		int get_condition() { return condition_; }

	protected:
		int condition_; // do this is Px
		virtual void p_execute(ProgramData& prg_data) { std::cout << "huhueahehahe" << std::endl; };
};
class instr_Binary : public instruction
{
	protected:
		number_type num_type;
		int store_;
		int arg1_, arg2_;
	public:
		virtual ~instr_Binary() = default;
		instr_Binary() : instruction(), num_type(integer), store_(-1), arg1_(-1), arg2_(-1) {};
		instr_Binary(int cond, number_type t, int s, int a1, int a2) : instruction(cond), num_type(t), store_(s), arg1_(a1), arg2_(a2) {};
};

// not templated because of cross-reasons
class instr_CVRT : public instruction
{
private:
	int pos_;
	int store_;
	number_type num_type_;
	void p_execute(ProgramData& prg_data)
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
public:
	instr_CVRT() : instruction(), num_type_(integer), pos_(0), store_(0) {};
	instr_CVRT(int cond, number_type nt, int p, int s) : instruction(cond),num_type_(nt), pos_(p), store_(s) {}	
};

class instr_JMP : public instruction
{
private:
	std::string where_to_jump_;
	void p_execute(ProgramData& prg_data)
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
public:
	instr_JMP() : instruction() {};
	instr_JMP(std::string w, int cond) : instruction(cond), where_to_jump_(w) {};
	virtual ~instr_JMP() = default;
};

// Template policies
struct FL
{
	typedef float vtype;
	static void push(ProgramData& cntx, vtype a, int s)
	{
		cntx.F_register->at(s) = a;
	}
	static vtype get(ProgramData& cntx, int  a)
	{
		return cntx.F_register->at(a);
	}
	static std::string get_name()
	{
		return "F";
	}
};
struct INT
{
	typedef int vtype;
	static void push(ProgramData& cntx, vtype a, int s)
	{
		cntx.I_register->at(s) = a;
	}
	static vtype get(ProgramData& cntx, int a)
	{
		return cntx.I_register->at(a);
	}
	static std::string get_name()
	{
		return "R";
	}
};

template <typename OP> class instr_LDC : public instruction
{
private:
	int store_;
	typename OP::vtype constant;
	void p_execute(ProgramData& prg_data)
	{
		OP::push(prg_data, constant, store_);
	}

public:
	instr_LDC() : instruction(), store_(0), i_constant(0) {};
	instr_LDC(int cond, int s, typename OP::vtype c) : instruction(cond), store_(s), constant(c) {};
};

template<typename OP> class instr_ADD2 : public instr_Binary
{
public:
	instr_ADD2() : instr_Binary() {};
	instr_ADD2(int cond, number_type t, int s, int a1, int a2) : instr_Binary(cond, t, s, a1, a2) {};

protected:
	void p_execute(ProgramData& prg_data)
	{
		typename OP::vtype arg1 = OP::get(prg_data, arg1_);
		typename OP::vtype arg2 = OP::get(prg_data, arg2_);
		OP::push(prg_data, arg1 + arg2, store_);	
	}
};

template<typename OP> class instr_SUB2 : public instr_Binary
{
public:
	instr_SUB2() : instr_Binary() {};
	instr_SUB2(int cond, number_type t, int s, int a1, int a2) : instr_Binary(cond, t, s, a1, a2) {};

protected:
	void p_execute(ProgramData& prg_data)
	{
		typename OP::vtype arg1 = OP::get(prg_data, arg1_);
		typename OP::vtype arg2 = OP::get(prg_data, arg2_);
		OP::push(prg_data, arg1 - arg2, store_);
	}
};

template<typename OP> class instr_MUL2 : public instr_Binary
{
public:
	instr_MUL2() : instr_Binary() {};
	instr_MUL2(int cond, number_type t, int s, int a1, int a2) : instr_Binary(cond, t, s, a1, a2) {};

protected:
	void p_execute(ProgramData& prg_data)
	{
		typename OP::vtype arg1 = OP::get(prg_data, arg1_);
		typename OP::vtype arg2 = OP::get(prg_data, arg2_);
		OP::push(prg_data, arg1 * arg2, store_);
	}
};

template<typename OP> class instr_DIV2 : public instr_Binary
{
	public:
		instr_DIV2() : instr_Binary() {};
		instr_DIV2(int cond, number_type t, int s, int a1, int a2) : instr_Binary(cond, t, s, a1, a2) {};

	protected:
		void p_execute(ProgramData& prg_data)
		{
			typename OP::vtype arg1 = OP::get(prg_data, arg1_);
			typename OP::vtype arg2 = OP::get(prg_data, arg2_);
			if (arg2 == 0)
			{
				std::cout << "DIV operation failed." << std::endl;
				throw(assembler_exception("DIV's arguments were " + std::to_string(arg1) + " / " + std::to_string(arg2)));
			}
			else
				OP::push(prg_data, arg1 / arg2, store_);
		}
};

template<typename OP> class instr_LDST2 : public instruction
{
public:
	instr_LDST2() : instruction(), num_type(integer), store_(-1), arg_(-1) {};
	instr_LDST2(int cond, ldst_type ls, number_type t, int s, int a) :
		instruction(cond), load_or_store(ls), num_type(t), store_(s), arg_(a) {};
protected:
	number_type num_type;
	ldst_type load_or_store;
	int store_;
	int arg_;
	void p_execute(ProgramData& prg_data)
	{
		if (load_or_store == load)
		{
			// load LD R15 = [R20]
			int from = (int)OP::get(prg_data, arg_); // R20 = 110
			if (from > 255)
			{
				std::cout << "Instruction LD/ST on line TODO has failed. Index is too high." << std::endl;
				throw(assembler_exception("Index was " + std::to_string(from)));
				return;
			}
			// R15 = [110] 
			OP::push(prg_data, OP::get(prg_data, from), store_);

		}
		else
		{
			// store ST [R33] = R55
			int to = (int)OP::get(prg_data, store_); // R20 = 110
			if (to > 255)
			{
				std::cout << "Instruction LD/ST on line TODO has failed. Index is too high." << std::endl;
				throw(assembler_exception("Index was " + std::to_string(to)));
				return;
			}
			// R15 = [110] 
			OP::push(prg_data, OP::get(prg_data, arg_), to);
		}
	}
};

template<typename OP> class instr_MOV : public instruction
{
public:
	instr_MOV() : instruction(), store_(-1), arg_(-1) {};
	instr_MOV(int cond, int s, int a) : instruction(cond), store_(s), arg_(a) {};
protected:
	int store_;
	int arg_;
	void p_execute(ProgramData& prg_data)
	{
		OP::vtype value = OP::get(prg_data,arg_);
		OP::push(prg_data, value, store_);
	}
};

template<typename OP> class instr_CMP : public instruction
{
protected:
	int pred1_;
	int pred2_;
	int arg1_;
	int arg2_;
	cmp_type relation_type;

	bool do_relation(typename OP::vtype a, typename  OP::vtype  b)
	{
		switch (relation_type)
		{
		case eq:
			return a == b;
			break;
		case ne:
			return a != b;
			break;
		case gt:
			return a > b;
			break;
		case lt:
			return a < b;
			break;
		case ge:
			return a >= b;
			break;
		case le:
			return a <= b;
			break;
		default:
			return false;
			break;
		}
	}

	void p_execute(ProgramData& prg_data)
	{
		bool pred1_value;
		typename OP::vtype a, b;
		a = OP::get(prg_data, arg1_);
		b = OP::get(prg_data, arg2_);

		pred1_value = do_relation(a, b);

		if (pred1_ != 1 && pred1_ != 0)
			prg_data.P_register->at(pred1_) = pred1_value;
		if (pred2_ != 1 && pred2_ != 0)
			prg_data.P_register->at(pred2_) = !pred1_value;
	}

public:

	instr_CMP() : instruction() {};
	instr_CMP(int cond, int p1, int p2, int a1, int a2, cmp_type rel) : instruction(cond)
	{
		pred1_ = p1;
		pred2_ = p2;
		arg1_ = a1;
		arg2_ = a2;
		relation_type = rel;
	}
};

template <typename OP> class instr_IO : public instruction
{
	private:
		void p_execute(ProgramData& prg_data)
		{
			if (op_type == in)
			{
				std::cout << "Insert value at " << OP::get_name() << "[" << pos_ << "]: ";
				typename OP::vtype input;
				std::cin >> input;

				OP::push(prg_data, input, pos_);
			}
			else if (op_type == out)
			{
				std::cout << "Value at " << OP::get_name() << "[" << pos_ << "] = " << OP::get(prg_data, pos_) << std::endl;
			}
		}

	protected:
		io_type op_type;
		int pos_;
	public:
		instr_IO() : instruction(), pos_(0), op_type(in) {};
		instr_IO(int cond, io_type op, int p) : instruction(cond), op_type(op), pos_(p) {};
};

#endif
