#pragma once
#include <string>
#include "mipsObjCode.h"

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
	mult,
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
public:
	Instruction();
	/*
	Instruction(MipsCode _op);
	Instruction(MipsCode _op, string _des);
	Instruction(MipsCode _op, string _des, string _src1);
	Instruction(MipsCode _op, string _des, string _src1, string _src2);

	*/
	string getOpName(MipsCode _opcode);

	string printInstr() {
		return op + des + src1 + src2;
	}

	~Instruction();
};

