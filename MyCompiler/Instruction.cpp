#include "pch.h"
#include "Instruction.h"


Instruction::Instruction() {
	string op = "";
	string des = "";
	string src1 = "";
	string src2 = "";
}

Instruction::~Instruction()
{
}
/*

Instruction::Instruction(MipsCode _op) {
	op = getOpName(_op);
}

Instruction::Instruction(MipsCode _op, string _des) {
	op = getOpName(_op);
	des = _des;
}
Instruction::Instruction(MipsCode _op, string _des, string _src1) {
	op = getOpName(_op);
	des = _des;
	src1 = _src1;
}
Instruction::Instruction(MipsCode _op, string _des, string _src1, string _src2) {
	op = getOpName(_op);
	des = _des;
	src1 = _src1;
	src2 = _src2;
}
*/


string Instruction::getOpName(MipsCode _opcode) {
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
	case MipsCode::add:	return "add"; 
	case MipsCode::addi:	return "addi"; 
	case MipsCode::sub:		return "sub"; 
	case MipsCode::subi:	return "subi"; 
	case MipsCode::mult:	return "mul"; 
	case MipsCode::ddiv:	return "div"; 
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
