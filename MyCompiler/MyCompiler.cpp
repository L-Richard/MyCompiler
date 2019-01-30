// MyCompiler.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "Token.h"
#include "Const.h"
#include "Lexer.h"
#include "Error.h"
#include "Parser.h"
#include "mipsObjCode.h"
#include "Instruction.h"
#include "DagGraph.h"
#include "GlobalRegs.h"

using namespace std;

string filename = "15071111_test_gramma.txt";
string outFileName = "15071111_result_obj.txt";
string midCodeFileName = "15071111_result_mid.txt";
string optMidCodeFileName = "15071111_result_optMid.txt";


int main(){
	//cout << "input absolute path of source file:" << endl;
	//cin >> filename;
 	Error error_handler = Error();
	Lexer *lexer = new Lexer(filename, error_handler);
	SymbolTable tab = SymbolTable(error_handler);
	MidCodeGen codeGen = MidCodeGen(tab);
	Parser parser = Parser(lexer, error_handler, codeGen, tab);
	DagGraph dag = DagGraph(tab, codeGen.getMidCodes());
	
	parser.parse();
	if (error_handler.hasError()) {
		error_handler.print();
	}
	// codeGen.print();
	else {
		MidCodeGen::print(midCodeFileName, codeGen.getMidCodes());
		dag.dagOpt();
		MidCodeGen::print(optMidCodeFileName, dag.getOptMidCodes());
		GlobalRegs regs = GlobalRegs(tab, dag.getOptMidCodes());
		regs.allocateRegisters();
		mipsObjCode obj = mipsObjCode(tab, dag.getOptMidCodes());
		obj.translate();
		obj.printObjFile(outFileName);
	}
}
