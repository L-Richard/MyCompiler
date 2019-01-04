#pragma once
#include <string>

using namespace std;

enum class MipsCode {
	//load to the register
	lw,
	lb,
	//store to the memory
	sw,
	sb,
	//load the immdiate number.
	li,
	//load address
	la,
	//jump and link
	jal,
	//
	jr,
	//jumpd
	j,
	//add the ident
	add,
	addi,
	//sub
	sub,
	subi,
	//mult
	mul,
	sll,
	//dive
	ddiv,
	//assign
	move,
	//branch
	ble, blt,  //	b <=, b <, b <=0
	blez, bltz,
	bge, bgt,  //	b >=, b >, b >=0
	bgez, bgtz,
	beq, bne,
	beqz, bnez,		
	syscall,
	label,
	mflo,
	note
};

class Instruction {
private:
	string op = "";
	string des = "";
	string src1 = "";
	string src2 = "";

	string getOpName(MipsCode _opcode) {
	switch (_opcode) 
		{
		case MipsCode::lw:	return "lw";
		case MipsCode::lb:	return "lb"; 
		case MipsCode::sw:	return "sw"; 
		case MipsCode::sb:	return "sb"; 
		case MipsCode::li:	return "li"; 
		case MipsCode::la:	return "la"; 
		case MipsCode::jal:	return "jal"; 
		case MipsCode::jr:	return "jr"; 
		
		case MipsCode::add:		return "addu"; 
		case MipsCode::addi:	return "addi"; 
		case MipsCode::sub:		return "sub"; 
		case MipsCode::subi:	return "subi"; 
		case MipsCode::mul:		return "mul"; 
		case MipsCode::ddiv:	return "div"; 
		case MipsCode::sll:		return "sll"; 

		case MipsCode::move:	return "move"; 
		case MipsCode::mflo:	return "mflo"; 

		case MipsCode::bgez:	return "bgez"; 
		case MipsCode::blez:	return "blez"; 
		case MipsCode::bgtz:	return "bgtz"; 
		case MipsCode::bltz:	return "bltz"; 
		case MipsCode::beqz:	return "beqz"; 
		case MipsCode::bnez:	return "bnez"; 
		case MipsCode::beq:		return "beq"; 
		case MipsCode::bne:		return "bne"; 
		case MipsCode::ble:		return "ble"; 
		case MipsCode::blt:		return "blt"; 
		case MipsCode::bge:		return "bge"; 
		case MipsCode::bgt:		return "bgt"; 
		
		case MipsCode::syscall:	return "syscall"; 
		case MipsCode::label:	return ""; 
		case MipsCode::j:		return "j"; 
		case MipsCode::note:	return "#"; 
		default:	return "#error op";
		}
	}

public:
	Instruction(MipsCode _op) {
		op = getOpName(_op);
	}

	Instruction(MipsCode _op, string _des) {
		op = getOpName(_op);
		des = _des;
	}

	Instruction(MipsCode _op, string _des, string _src1) {
		op = getOpName(_op);
		des = _des;
		src1 = _src1;
	}

	Instruction(MipsCode _op, string _des, string _src1, string _src2) {
		op = getOpName(_op);
		des = _des;
		src1 = _src1;
		src2 = _src2;
	}

	string printInstr() {
		string r = "";
		if (op != "") {
			r = "\t";
		}
		r = r + op + des + src1 + src2;
		return r;
	}

	string getOp() {
		return this->op;
	}
	string getDst() {
		return this->des;
	}
	string getS1() {
		return this->src1;
	}
	string getS2() {
		return this->src2;
	}

	void setOp(string s) {
		this->op = s;
	}
	void setDst(string s) {
		this->des = s;
	}
	void setS1(string s) {
		this->src1 = s;
	}
	void setS2(string s) {
		this->src2 = s;
	}
	~Instruction();
};

