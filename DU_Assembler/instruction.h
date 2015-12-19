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

///TODO: no constructor and shit!
class ProgramData
{
public:
	int prg_counter;
	navestiPtr navesti;
	boolVecPtr P_register;
	intVecPtr I_register;
	floatVecPtr F_register;
};

class instruction
{
	public:
		instruction() : condition_(1) {};
		instruction(int c) : condition_(c) {};
		virtual ~instruction() = default;

		virtual void execute(ProgramData& prg_data) { std::cout << "huhueahehahe" << std::endl; }

	protected:
		int condition_; // do this is Px
};

class instr_Binary : public instruction
{
	protected:
		number_type num_type;
		int store_;
		int arg1_, arg2_;
	public:
		// does this matter?
		//	virtual ~instr_Binary() = default;
		instr_Binary() : instruction(), num_type(integer), store_(-1), arg1_(-1), arg2_(-1) {};
		instr_Binary(int cond, number_type t, int s, int a1, int a2) : instruction(), num_type(t), store_(s), arg1_(a1), arg2_(a2) {};
		virtual int do_operation(int a, int b, bool& s) { return 0; }
		virtual float do_operation(float a, float b, bool& s) { return 0;  }
		void execute(ProgramData& prg_data);
};

class instr_ADD : public instr_Binary
{
	public:
		instr_ADD() : instr_Binary() {};
		instr_ADD(int cond, number_type t, int s, int a1, int a2) : instr_Binary(cond, t, s, a1, a2) {};		
		 int do_operation(int a, int b, bool& s);
		float do_operation(float a, float b, bool& s);
};

class instr_SUB : public instr_Binary
{
	public:
		instr_SUB() : instr_Binary() {};
		instr_SUB(int cond, number_type t, int s, int a1, int a2) : instr_Binary(cond, t, s, a1, a2) {};
		virtual int do_operation(int a, int b, bool& s);
		virtual float do_operation(float a, float b, bool& s);
};

class instr_MUL : public instr_Binary
{
	public:
		instr_MUL() : instr_Binary() {};
		instr_MUL(int cond, number_type t, int s, int a1, int a2) : instr_Binary(cond, t, s, a1, a2) {};
		virtual int do_operation(int a, int b, bool& s);
		virtual float do_operation(float a, float b, bool& s);
};

class instr_DIV : public instr_Binary
{
	public:
		instr_DIV() : instr_Binary() {};
		instr_DIV(int cond, number_type t, int s, int a1, int a2) : instr_Binary(cond, t, s, a1, a2) {};
		virtual int do_operation(int a, int b, bool& s);
		virtual float do_operation(float a, float b, bool& s);
};

class instr_JMP : public instruction
{
	private:
		std::string where_to_jump_;
	public:
		instr_JMP() : instruction() {};
		instr_JMP(std::string w, int cond) : instruction(cond), where_to_jump_(w) {};
		virtual ~instr_JMP() = default;
		virtual void execute(ProgramData& prg_data);
};

class instr_LDST : public instruction
{
	protected:
		number_type num_type;
		ldst_type load_or_store;
		int store_;
		int arg_;
	public:
		instr_LDST() : instruction(), num_type(integer), store_(-1), arg_(-1) {};
		instr_LDST(int cond, ldst_type ls, number_type t, int s, int a) : instruction(cond), load_or_store(ls), num_type(t), store_(s), arg_(a) {};
		void execute(ProgramData& prg_data);
	private:
		void instr_LDST::store(std::vector<int>* vec);
		void instr_LDST::store(std::vector<float>*  vec);
};

class instr_LDC : public instruction
{
	private:
		int store_;
		number_type num_type;
		int i_constant;
		float f_constant;
	public:
		instr_LDC() : instruction(), store_(0), i_constant(0), num_type(integer) {};
		instr_LDC(int cond, number_type nt, int s, int c) : instruction(cond), f_constant(0), store_(s), i_constant(c), num_type(nt) {};
		instr_LDC(int cond, number_type nt, int s, float c) : instruction(cond), f_constant(c), store_(s), i_constant(0), num_type(nt) {};
		void execute(ProgramData& prg_data);
};

class instr_IO : public instruction
{
	private:
		void instr_IO::do_IO(std::vector<float> * vec);
		void instr_IO::do_IO(std::vector<int> * vec);

	protected:
		io_type op_type;
		number_type num_type;
		int pos_;
	public:
		instr_IO() : instruction(), pos_(0), op_type(in), num_type(integer) {};
		instr_IO(int cond, number_type nt, io_type op, int p) : instruction(cond), op_type(op), pos_(p), num_type(nt) {};
		void execute(ProgramData& prg_data);
};

class instr_CVRT : public instruction
{
private:
	int pos_;
	int store_;
	number_type num_type_;
public:
	instr_CVRT() : instruction(), num_type_(integer), pos_(0), store_(0) {};
	instr_CVRT(int cond, number_type nt, int p, int s) : instruction(cond),num_type_(nt), pos_(p), store_(s) {};
	void execute(ProgramData& prg_data);
};

class instr_CMP : public instruction
{
	protected:
		int pred1_;
		int pred2_;
		int arg1_;
		int arg2_;
		number_type num_type_;
		cmp_type relation_type;

		bool do_relation(float a, float b);
	public:
		void execute(ProgramData& prg_data);
		instr_CMP() : instruction() {};
		instr_CMP(int cond, int p1, int p2, int a1, int a2, number_type nt, cmp_type rel) : instruction(cond)
		{
			pred1_ = p1;
			pred2_ = p2;
			arg1_ = a1;
			arg2_ = a2;
			num_type_ = nt;
			relation_type = rel;
		}
};

#endif
