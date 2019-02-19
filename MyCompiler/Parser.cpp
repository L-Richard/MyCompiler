#include "pch.h"
#include "Parser.h"


Parser::Parser(Lexer *lexer, Error& error_handler, MidCodeGen& codeGen, SymbolTable& symTab)
:error_handler(error_handler), codeGen(codeGen), symTab(symTab) {
	this->lexer = lexer;
}

Parser::~Parser()
{
}

void Parser::nextSym() {
	current_token = lexer->getSym();
}

void Parser::skip(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis_skip
	cout << "skip---------------------------" << endl;
#endif
	while (!fsys.count(current_token.getSymbol())) {
#ifdef DEBUG_Gramma_analysis_skip
		cout << "\t" << current_token.getlc() << "行:  symbol:"
			 << Enum2Str(current_token.getSymbol()) << " \"" 
			 << current_token.getIdentName() << "\" " << endl;
#endif
		nextSym();
	}
}

void Parser::test(SymSet s1, SymSet s2, int no) {
	if (!s1.count(current_token.getSymbol())) {
		//  error_handler;
		error_handler.reportErrorMsg(current_token, no);
		s1.insert(s2.begin(), s2.end());
		s1.insert(Symbol::eofsy);
		skip(s1);
	}
}

void Parser::testSemicolon() {
	if (current_token.getSymbol() == Symbol::semicolon) {
		nextSym();
	}
	else {
		// error
		error_handler.reportErrorMsg(current_token, 26);
		SymSet fsys = compoundBsys;
		fsys.insert(eofsy);
		fsys.insert(voidsy);
		fsys.insert(rbrace);
		skip(fsys);	
	}

}

void Parser::parse() {
	// parse the whole source file
	codeGen.emit(Operator::jmp, mainLabel, NULL, NULL);
	bool varDecDone = false;
	nextSym();
	// check type 
	test({ voidsy, intsy, charsy, constsy }, statementBsys, 1);
	while (current_token.getSymbol() == constsy) {
		constDefinition();
	}

	SymSet varFunDecBegSym = { intsy, charsy, voidsy };
	test(varFunDecBegSym, {}, 2);
	while (varFunDecBegSym.count(current_token.getSymbol())) {
		if (current_token.getSymbol() == Symbol::voidsy) {
			varDecDone = true;
		}
		SymSet fsys = compoundBsys;
		SymSet tmpSet = { voidsy, intsy, charsy };
		type_token = current_token;	// record type of var or func
		test(tmpSet, compoundBsys, 1);
		if (tmpSet.count(current_token.getSymbol())) {
			nextSym();
		}
		name_token = current_token;	// record name of var or func
		test({ ident }, compoundBsys, 7);
		if (current_token.getSymbol() == ident) {
			nextSym();
		}
		if (current_token.getSymbol() == Symbol::lparent) {
			varDecDone = true;
		}
		// main function
		if (name_token.getIdentName() == "main") {
			codeGen.setLabel(mainLabel);
			symTab.enterFunc(name_token, Type::voidtp, mainLabel);
			if (type_token.getSymbol() == Symbol::voidsy && current_token.getSymbol() == Symbol::lparent) {
				isMain = true;
				// main function no parameters, no return value;
				nextSym();
				test({ rparent }, compoundBsys, 4);
				if (current_token.getSymbol() == rparent) {
					nextSym();
				}
				test({ lbrace }, compoundBsys, 5);
				if (current_token.getSymbol() == lbrace) {
					hasRet = false;
					test(compoundBsys, {Symbol::rbrace}, 0);
					if (compoundBsys.count(current_token.getSymbol())) {
						nextSym();
						compoundstatement();
						codeGen.setLabel(endMainLabel);
					}
					test({ rbrace }, { eofsy }, 17);
					if (current_token.getSymbol() == rbrace) {
						nextSym();
					}
					symTab.funDone();
				}
			}
			else {
				error_handler.reportErrorMsg(name_token, 3);
			}
			break;
		}
		// function 
		if (varDecDone) {
			if (type_token.getSymbol() == Symbol::voidsy) {
				hasRet = false;
				function();
			}
			else {
				hasRet = true;
				function();
			}
		}
		// variable decleration
		else {
			varDeclaration();
		}
	}
	if (isMain == false) {
		// missing main function
		error_handler.reportErrorMsg(current_token, 16);
	}
	test({ Symbol::eofsy }, {}, 6);
}

Type Parser::tokenType() {
	Symbol tmpSy = type_token.getSymbol();
	Type typ = tmpSy == Symbol::intsy ? Type::inttp :
			   tmpSy == Symbol::charsy ? Type::chartp :
			   tmpSy == Symbol::voidsy ? Type::voidtp : Type::notp;
	return typ;
}

int Parser::signedInt(SymSet fsys) {
	/*
	begin with +, - or number, 
	end with something after the signed integer
	*/
	SymSet tmp = { Symbol::minus, Symbol::plus, Symbol::intcon };
	Symbol sy = current_token.getSymbol();
	int sign = 1;
	test(tmp, fsys, 11);
	if (tmp.count(sy)) {
		if (sy == Symbol::minus) {
			sign = -1;
			nextSym();
		}
		else if (sy == Symbol::plus) {
			nextSym();
		}
		if (current_token.getSymbol() == intcon) {
			int tmpR = current_token.getConstNum() * sign;
			nextSym();
			return tmpR;
		}
		else {
			// error
			error_handler.reportErrorMsg(current_token, 8);
			fsys.insert(tmp.begin(), tmp.end());
			skip(fsys);
			return 0;
		}
	}
	return 0;
}

void Parser::constDefinition() {
	/* begin with const, end with token after semicolon, 
	type check, ident check*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "常量定义语句" << endl;
#endif
	nextSym();
	// type check
	test({intsy, charsy}, compoundBsys, 1);	
	if (current_token.getSymbol() == intsy || current_token.getSymbol() == charsy) {
		type_token = current_token;
		// error if not char or int
		do	{
			// check identifier name
			nextSym();
			SymSet fsys = compoundBsys;
			fsys.insert({ comma, semicolon });
			test({ ident }, fsys, 7);
			if (current_token.getSymbol() == ident) {
				name_token = current_token;
				nextSym();
			}
			// check assign symbol =
			if (current_token.getSymbol() != Symbol::becomes) {
				error_handler.reportErrorMsg(current_token, 10);
				SymSet fsys = {
					becomes, ident, comma, semicolon,
					Symbol::plus, Symbol::minus, intcon, charcon };
				skip(fsys);
			}
			else {
				nextSym();
			}
			// check const value
			if (type_token.getSymbol() == Symbol::intsy) {
				// 在signedInt中报错
				int tmpConstInt = signedInt({comma, semicolon, ident, intsy, charsy, voidsy, lbrace, eofsy});
				symTab.enterConst(name_token, Type::inttp, tmpConstInt);
				// enter symbol table
			}
			else if (type_token.getSymbol() == Symbol::charsy) {
				if (current_token.getSymbol() == charcon) {
					// enter symbol table 
					symTab.enterConst(name_token, Type::chartp, current_token.getIdentName()[0]);
					nextSym();
				}
				else {
					// error: type not coherent
					error_handler.reportErrorMsg(current_token, 12);
					skip({ comma, ident, semicolon, intsy, charsy, voidsy, lparent, lbrace });
				}
			}

		} while (current_token.getSymbol() == Symbol::comma);
		testSemicolon();
	}
}

void Parser::varDeclaration() {
	/* before call this function, we use type_token to record 
	 the type of var and we use name_token to record var's name.
	 And the current_token is not '(', it means not function 
	 declaration. If it is ',', we should declare multiple var.
	ident check, redefine check
	 */
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "变量声明语句" << endl;
#endif // DEBUG_Gramma_analysis
	bool firstLoop = true;
	do {
		if (!firstLoop) {
			// read an ident name
			nextSym();
			// check identifier name
			SymSet fsys = compoundBsys;
			fsys.insert({ comma, semicolon });
			test({ ident }, fsys, 7);
			// read a comma or lsquare or semicolon
			if (current_token.getSymbol() == ident) {
				name_token = current_token;
				nextSym();
			}
		}
		firstLoop = false;
		// var type
		Type tmp = tokenType();
		if (tmp == notp) {
			// illegal type
			error_handler.reportErrorMsg(type_token, 2);
		}
		// parse var
		if (current_token.getSymbol() == Symbol::lsquare) {
			// array type
			nextSym();
			Symbol sy = current_token.getSymbol();
			int tmpArrayMax = 0;
			if (sy == Symbol::plus || sy == Symbol::minus) {
				// error
				error_handler.reportErrorMsg(current_token, 13);
				nextSym();
			}
			// read a num
			SymSet fsys = compoundBsys;
			fsys.insert({ rbrace, comma });
			test({ intcon }, fsys, 8);
			if (current_token.getSymbol() == Symbol::intcon) {
				// enter table
				tmpArrayMax = current_token.getConstNum();
				symTab.enterVar(name_token, tmp, tmpArrayMax);
				nextSym();
				test({ rsquare }, fsys, 14);
				if (current_token.getSymbol() == Symbol::rsquare) {
					nextSym();
				}
				else {
#ifdef DEBUG_Gramma_analysis
					std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
					std::cout << "错误：缺少右中括号" << endl;
#endif // DEBUG_Gramma_analysis
				}
			}
			else {
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "错误：缺少无符号整数常量" << endl;
#endif // DEBUG_Gramma_analysis 
			}
		}
		else {
			// comman variable, enter symbol table
			symTab.enterVar(name_token, tmp, 0);
		}
	} while (current_token.getSymbol() == Symbol::comma);
	testSemicolon();
}

void Parser::paraList() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "paraList 参数列表语句" << endl;
#endif // DEBUG_Gramma_analysis
	/* begin with left parent, end with right parent*/
	SymSet fsys = compoundBsys;
	fsys.insert({ rparent, lbrace, ident });
	do {
		nextSym();
		// right parent, finish loop
		if (current_token.getSymbol() == Symbol::rparent) {
			break;
		}
		// type 
		else if (current_token.getSymbol() == Symbol::intsy || current_token.getSymbol() == Symbol::charsy) {
			type_token = current_token;
			nextSym();
			test({ ident }, fsys, 7);
			if (current_token.getSymbol() == Symbol::ident) {
				name_token = current_token;
				symTab.enterPara(name_token, tokenType());
				nextSym();
			}
		}
		else {
			// error: missing type define
			error_handler.reportErrorMsg(current_token, 1);
			skip(fsys);
		}
	} while (current_token.getSymbol() == Symbol::comma);
}

SymbolItem* Parser::function() {
	/* begin with left parent, 
	   end with symbol after right brace
	   eg: void foo(
	   */
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	if (hasRet)
		std::cout << "有返回值函数定义" << endl;
	else 
		std::cout << "无返回值函数定义" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* label = codeGen.genLabel(labelType::fun_lb);
	codeGen.setLabel(label);
	codeGen.emit(Operator::st$ra, symTab.getFunItem(), NULL, NULL);
	Type typ = tokenType();
	if (typ == Type::notp) {
		error_handler.reportErrorMsg(type_token, 2);
	}
	symTab.enterFunc(name_token, typ, label);
	paraList();
	test({ Symbol::rparent }, compoundBsys, 20);
	if (current_token.getSymbol() == Symbol::rparent) {
		nextSym();
	}
	test({ Symbol::lbrace }, compoundBsys, 5);
	if (current_token.getSymbol() == Symbol::lbrace) {
		nextSym();
		compoundstatement();
		if (returned == false && hasRet) {
			error_handler.reportErrorMsg(current_token, 42);
		}
		if (returned == false && !hasRet) {
			codeGen.emit(Operator::returnOp, symTab.getFunItem(), NULL, NULL);
		}
	}
	//?????????????????????????????????????????????????????????????????????????????????
	test({ Symbol::rbrace }, compoundBsys, 17);
	if (current_token.getSymbol() == Symbol::rbrace) {
		nextSym();
	}
	symTab.funDone();
	//?????????????????????????????????????????????????????????????????????????????????
	return NULL;
}

void Parser::compoundstatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "复合语句" << endl;
#endif // DEBUG_Gramma_analysis
	// 
	// while sy in statement begin symbol set
	while (current_token.getSymbol() == Symbol::constsy) {
		constDefinition();
	}
	while (current_token.getSymbol() == Symbol::intsy
		|| current_token.getSymbol() == Symbol::charsy) {
		type_token = current_token;
		nextSym();
		test({ ident }, compoundBsys, 7);
		if (current_token.getSymbol() == Symbol::ident) {
			name_token = current_token;
			nextSym();
			varDeclaration();
		}
		else {
#ifdef DEBUG_Gramma_analysis
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：缺少标识符" << endl;
#endif // DEBUG_Gramma_analysis
		}
	}
	if (statementBsys.count(current_token.getSymbol())) {
		multiStatement();
	}
}

void Parser::multiStatement() {
	/* begin with begining symbol of statement*/
	if (current_token.getSymbol() == rbrace) {
		// empty multi statements
#ifdef DEBUG_Gramma_analysis
		int lc = current_token.getlc();
		std::cout << lc << "行: ";
		std::cout << "空语句列" << endl;
#endif // DEBUG_Gramma_analysis
		return;
	}
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "语句列" << endl;
#endif // DEBUG_Gramma_analysis
	test({statementBsys}, {eofsy}, 16);
	while (statementBsys.count(current_token.getSymbol())) {
		statement();
	}
}

void Parser::statement() {
	Symbol sy = current_token.getSymbol();
	if (statementBsys.count(sy)) {
		switch (sy) {
		case Symbol::lbrace: {
			nextSym();
			multiStatement();
			test({ Symbol::rbrace }, statementBsys, 18);
			if (current_token.getSymbol() == Symbol::rbrace) {
				nextSym();
			}
			break;
		}
		case Symbol::ifsy:		ifStatement({});	break;
		case Symbol::switchsy:	switchStatement();	break;
		case Symbol::whilesy:	whileStatement();	break;
		case Symbol::scanfsy:	read();				returned = false;	break;
		case Symbol::printfsy:	write();			returned = false;	break;
		case Symbol::returnsy:	returnStatement();	break;
		case Symbol::semicolon: {
#ifdef DEBUG_Gramma_analysis
			int lc = current_token.getlc();
			std::cout << lc << "行: ";
			std::cout << "空语句" << endl;
#endif // DEBUG_Gramma_analysis
			nextSym();
			break;
		}
		case Symbol::ident: {
			// search symbol table
			name_token = current_token;
			nextSym();
			SymbolItem* tmp = symTab.search(name_token);
			if (tmp == NULL) {
				SymSet fsys = statementBsys;
				fsys.insert({ Symbol::eql });
				fsys.insert(expBsys.begin(), expBsys.end());
				skip(fsys);
				break;
			}
			if (tmp->obj == ObjectiveType::functy) {
				SymSet fsys = statementBsys;
				fsys.insert({ rbrace, eofsy });
				fsys.insert(statementBsys.begin(), statementBsys.end());
				test({ lparent }, fsys, 19);
				if (current_token.getSymbol() == lparent) {
					SymSet fsys = statementBsys;
					fsys.insert({ Symbol::semicolon, Symbol::rbrace });
					call(fsys, tmp);
					testSemicolon();
				}
			}
			else if (tmp->obj == ObjectiveType::varty || tmp->obj == ObjectiveType::arrayty){
				SymbolItem* arrIndex = NULL;
				if (tmp->obj == ObjectiveType::arrayty) {
					SymSet fsys = expBsys;
					fsys.insert(statementBsys.begin(), statementBsys.end());
					fsys.insert({ Symbol::rsquare, Symbol::eql });
					test({ lsquare }, fsys, 21);
					if (current_token.getSymbol() == Symbol::lsquare) {
						SymSet fsys = statementBsys;
						fsys.insert({ Symbol::rbrace, Symbol::becomes, Symbol::rsquare});
						arrIndex = selector(tmp, fsys);
					}
				}
				test({ becomes }, statementBsys, 10);
				if (current_token.getSymbol() == Symbol::becomes) {
					//////////////////////////////////////////////////////////////
					nextSym();
					Token errorToken = current_token;
					SymbolItem * assignRet = assignStatement();
					if (assignRet->typ > tmp->typ) {
						error_handler.reportErrorMsg(errorToken, 43);
					}
					if (tmp->obj == ObjectiveType::arrayty) {
						codeGen.emit(Operator::arrSt, assignRet, arrIndex, tmp);
					}
					else {
						/* optimization:
						   if assign statement is not array assig, we can
						   use the vat on the left, remove extra !Temp.
						*/
						codeGen.emit(Operator::assignOp, tmp, assignRet, NULL);
						codeGen.opt_assign(tmp, assignRet);
					}
				}
				else {
#ifdef DEBUG_Gramma_analysis
					std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
					std::cout << "出错：标识符后需要等号或小括号！" << endl;
#endif // DEBUG_Gramma_analysis
				}
			}
			else {
				error_handler.reportErrorMsg(name_token, 24);
				skip(statementBsys);
			}
			returned = false;
			break;
		}
		default: break;		// 默认是空白语句
		}
	}
}

SymbolItem* Parser::selector(SymbolItem* item, SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "selector 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* r = NULL;
	Token errorToken;
	if (current_token.getSymbol() == Symbol::lsquare) {
		nextSym();	// read a int con
		fsys.insert({ rsquare });
		errorToken = current_token;
		r = expression(fsys);
		test({ Symbol::rsquare }, expBsys, 14);
		if (current_token.getSymbol() == Symbol::rsquare) {
			nextSym();
		}
	}
	if (r->obj == ObjectiveType::constty && item->arrayNumMax <= r->addr) {
		error_handler.reportErrorMsg(errorToken, 44);
	}
	return r;
}

void Parser::returnStatement() {
	/* begin with return, end with the sym after semicolon
	return(expression());	// has return value function
	return;
	quadruples: return r; or return;
	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "return 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* r = NULL;

	SymSet fsys = statementBsys;
	fsys.insert({ rbrace });
	fsys.insert(expBsys.begin(), expBsys.end());
	nextSym();
	if (hasRet) {	// function with return 
		test({ lparent }, fsys, 19);
		if (current_token.getSymbol() == Symbol::lparent) {
			nextSym();
		}
		test(expBsys, fsys, 18);
		if (expBsys.count(current_token.getSymbol())) {
			fsys = statementBsys;
			fsys.insert({ Symbol::rbrace, Symbol::semicolon });
			r = expression(fsys);
			test({ rparent }, fsys, 20);
			if (current_token.getSymbol() == rparent) {
				nextSym();
				testSemicolon();
			}
			codeGen.emit(Operator::returnOp, symTab.getFunItem(), r, NULL);
			returned = true;
		}
	}
	else {		// function without return
		testSemicolon();
		if (isMain) 
			codeGen.emit(Operator::jmp, endMainLabel, NULL, NULL);
		else 
			codeGen.emit(Operator::returnOp, symTab.getFunItem(), NULL, NULL);
		returned = true;
	}
}

SymbolItem* Parser::assignStatement() {
	/* begin with first token after '=', end with sym after semicolon
		check '=' outside
	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "赋值语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::rbrace, Symbol::semicolon });
	fsys.insert(expBsys.begin(), expBsys.end());
	SymbolItem* r = NULL;
	r = expression(fsys);
	// do something
	testSemicolon();
	if (r == NULL) {
		r = new SymbolItem();
	}
	return r;
}

void Parser::ifStatement(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "if 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* endifLabel = codeGen.genLabel(labelType::end_if_lb);
	fsys.insert(statementBsys.begin(), statementBsys.end());
	fsys.insert({ Symbol::lbrace, Symbol::rbrace });
	nextSym();
	test({ Symbol::lparent }, statementBsys, 19);
	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		//codeGen.opt_if_record_start();	// optimize blank if statement
		condition(endifLabel);
		/////////////////////////////////////////////////////////////////
		test({ Symbol::rparent }, fsys, 20);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
		else {
			// error: missing rparent
#ifdef DEBUG_Gramma_analysis
			std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
			std::cout << "出错：不是语句开头符号!" << endl;
#endif // DEBUG_Gramma_analysis
		}
		statement();
		/*
		if (statementBsys.count(current_token.getSymbol())) {
			codeGen.opt_if_record_statement();
		}
		if (codeGen.getCon() == ConditionOptim::unknownCon) {
			codeGen.opt_if_record_end();
			codeGen.opt_if_optimize();
		}
		codeGen.restoreCon();
		*/
		codeGen.setLabel(endifLabel);
	}
}

void Parser::switchStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "switch语句" << endl;
#endif // DEBUG_Gramma_analysis
	map<int, Type> caseTab;
	SymbolItem* tmpExp = NULL;
	nextSym();
	SymSet fsys = statementBsys;
	fsys.insert(expBsys.begin(), expBsys.end());
	fsys.insert({ Symbol::lbrace, Symbol::rparent });
	test({ Symbol::lparent }, fsys, 19);
	if (current_token.getSymbol() == lparent) {
		nextSym();
		SymSet fsys = statementBsys;
		fsys.insert({ Symbol::rparent, Symbol::lbrace });
		tmpExp = expression(fsys);	// exp value
		
		fsys = statementBsys;
		fsys.insert({ Symbol::rbrace, Symbol::casesy });
		test({ Symbol::rparent }, fsys, 20);
		if (current_token.getSymbol() == rparent) {
			nextSym();
		}
		SymbolItem* end_switch = codeGen.genLabel(labelType::end_switch_lb);
		test({ lbrace }, fsys, 5);
		if (current_token.getSymbol() == lbrace) {
			nextSym();
			fsys.erase({ Symbol::rparent });
			test({ casesy }, fsys, 25);
			while (current_token.getSymbol() == casesy) {
				caseStatement(tmpExp, end_switch, &caseTab);
			}
			if (current_token.getSymbol() == defaultsy) {
				defaultStatement();
			}
		}
		/*if (tmpExp->obj != ObjectiveType::constty) {
			codeGen.setLabel(end_switch);
		}
		*/
		codeGen.setLabel(end_switch);
		//codeGen.restoreCon();
		test({ rbrace }, statementBsys, 17);
		if (current_token.getSymbol() == rbrace) {
			nextSym();
		}
	}
}

void Parser::caseStatement(SymbolItem* expItem, SymbolItem* end_switch, map<int, Type> *caseTab) {
	/*	begin with case, 
	end with some words after statement
	convert to if, else
	don't check case const type, only compare the value
	optimization of const expItem:
		when calling this function, codeGen.con is either unknown or false
		false means there has been some case branch matches, and skip all
			other case branch and default.
		unknown means there is no case matches, and keep the initial state 
			which is unknown. 
	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "case 语句" << endl;
#endif // DEBUG_Gramma_analysis
	bool skip_case = false;
	nextSym();
	// read a const value
	SymbolItem* constValue = NULL;
	SymbolItem* end_case = codeGen.genLabel(labelType::end_case_lb);

	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::casesy, Symbol::colon, Symbol::rbrace });
	Symbol sy = current_token.getSymbol();
	test(conValueBsys, fsys, 40);
	if (conValueBsys.count(sy)) { // check duplication
		Type ty;
		int conVal = 0;
		if (sy == Symbol::charcon) {
			ty = Type::chartp;
			conVal = current_token.getConstChar();
			constValue = codeGen.genCon(ty, conVal);
			nextSym();
		}
		else {
			ty = Type::inttp;
			conVal = this->signedInt(fsys);
			constValue = codeGen.genCon(ty, conVal);
		}
		//if (caseTab->count(conVal) && (*caseTab)[conVal] == ty) {
		// so we don't care the type of case const value
		if (caseTab->count(conVal)) {
			error_handler.reportErrorMsg(current_token, 41);
		}
		else {
			(*caseTab)[conVal] = ty;
		}
	} 
	else {
		/* make sure constValue != NULL when error occurs,
		 and continue compiling.
		*/
		constValue = new SymbolItem();
	}
	/* ??????????????????????????????????????????????????????????
	if (expItem->obj == ObjectiveType::constty && codeGen.getCon() != ConditionOptim::falseCon) {
		 if expItem is const, optimize.
			if there is true case, delete false case and default
			if no true case, only keep default
		if (expItem->addr == constValue->addr) {
			codeGen.setTrueCon();
		}
		else {
			skip_case = true;
			codeGen.setFalseCon();
		}
	}*/

	/*  note: if expItem is const, then 
		false condition won't emit code inside the codeGen
		true neither don't need the jmp or setlabel code
	*/
	if (expItem->obj != ObjectiveType::constty) {
		codeGen.emit(Operator::eql, end_case, expItem, constValue);
	}
	// test colon
	test({ Symbol::colon }, fsys, 38);
	if (current_token.getSymbol() == Symbol::colon) {
		nextSym();
	}
	statement();

	codeGen.emit(Operator::jmp, end_switch, NULL, NULL);
	codeGen.emit(Operator::setLabel, end_case, NULL, NULL);
/*
	if (expItem->obj != ObjectiveType::constty) {
	}
	else {
		 if expItem is const,
		if condition is true, set condition false to skip all cases.
		if condition is false, restore condition value, 

		if (codeGen.getCon() == ConditionOptim::trueCon) { //match
			codeGen.setFalseCon();
		}
		else if (skip_case) {	// not match
			codeGen.restoreCon();
		}
	}
*/
}

void Parser::defaultStatement() {
	/*	begin with default, 
	end with some word after statement	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "default 语句" << endl;
#endif // DEBUG_Gramma_analysis
	nextSym();
	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::rbrace });
	test({ Symbol::colon }, fsys, 38);
	if (current_token.getSymbol() == Symbol::colon) {
		nextSym();
	}
	else {
#ifdef DEBUG_Gramma_analysis
		std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
		std::cout << "出错：缺少冒号" << endl;
#endif // DEBUG_Gramma_analysis
	}
	statement();
}

void Parser::whileStatement() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "while 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* tmpCondition = NULL;
	SymbolItem* end_while = codeGen.genLabel(labelType::end_while_lb);
	SymbolItem* start_while = codeGen.genLabel(labelType::while_lb);
	Token error_token;
	nextSym();
	test({ Symbol::lparent }, {}, 0);
	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		error_token = current_token;
		codeGen.setLabel(start_while);
		condition(end_while);	// jump if condition is false
		test({ Symbol::rparent }, statementBsys, 20);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
			statement();
		}
		codeGen.emit(Operator::jmp, start_while, NULL, NULL);

		codeGen.setLabel(end_while);
		/*if (codeGen.getCon() == ConditionOptim::unknownCon) {
		}
		else if (codeGen.getCon() == ConditionOptim::trueCon) {
			// true condition: warning: dead loop
			codeGen.setLabel(end_while);
			cout << "Warning: ";
			cout << error_token.getlc() << "行: " << error_token.getcc() << "列: ";
			cout << "存在死循环，while条件永真" << endl;
		}
		else if (codeGen.getCon() == ConditionOptim::falseCon) {
			// erase start_while label
			codeGen.eraseLabel(start_while);
		}
		codeGen.restoreCon();
		*/
	}
}

SymbolItem* Parser::call(SymSet fsys, SymbolItem* funItem) {
	/*	begin with left parent, end with semicolon. */
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "call 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymbolItem* para = NULL;
	list<Type> paraTyps = funItem->paraList;
	if (current_token.getSymbol() == lparent) {
		nextSym();
		SymbolItem* paras = new SymbolItem();
		paras->obj = ObjectiveType::paras;
		auto typ = paraTyps.begin();
		while (typ != paraTyps.end()) {
			// 参数写入数据栈
			// record the paras in this while 
			Token error_token = current_token;
			para = expression(fsys);
			if (*typ == Type::chartp && para->typ == Type::inttp) {
				error_handler.reportErrorMsg(error_token, 39);
			}
			if (*typ == Type::inttp) {
				para->typ = Type::inttp;
			}
			if (para->typ != Type::notp) {
				// use a SymbolItem* record parameters, store them together in the end
				paras->paras.push_back(para);
				// codeGen.emit(Operator::stPara, NULL, para, NULL);
			}
			if (current_token.getSymbol() == Symbol::comma) {
				nextSym();
			}
			typ++;
			if (current_token.getSymbol() == Symbol::rparent && typ != paraTyps.end()) {
				error_handler.reportErrorMsg(current_token, 36);
				break;
			}
		}
		if (typ == paraTyps.end() && current_token.getSymbol() == Symbol::comma) {
			nextSym();
			error_handler.reportErrorMsg(current_token, 37);
			fsys.insert({ Symbol::rparent });
			skip(fsys);
		}
		// 保存现场
		codeGen.emit(Operator::save, NULL, symTab.getFunItem(), funItem);
		// store the paras into the stack
		codeGen.emit(Operator::stPara, NULL, paras, funItem);
		test({ Symbol::rparent }, fsys, 20);
		if (current_token.getSymbol() == rparent) {
			nextSym();
		}
	}
	codeGen.emit(Operator::call, funItem, NULL, NULL);
	if (funItem->hasRet) {
		SymbolItem* returnValue = codeGen.genTemp(funItem->typ);
		codeGen.emit(Operator::stRetVal, returnValue, NULL, NULL);
		// no need to restore, just load when use that var
		codeGen.emit(Operator::restore, NULL, symTab.getFunItem(), NULL);
		return returnValue;
	}
	else {
		return new SymbolItem();
	}
}

void Parser::read() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "scanf 语句" << endl;
#endif // DEBUG_Gramma_analysis
	/* begin with left parent, end with semicolon*/
	nextSym();
	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::rparent, Symbol::rbrace });
	test({ lparent }, fsys, 19);
	if (current_token.getSymbol() == Symbol::lparent) {
		do {
			nextSym();
			test({ Symbol::ident }, fsys, 7);
			if (current_token.getSymbol() == Symbol::ident) {
				SymbolItem* tmpItem = symTab.search(current_token);
				codeGen.emit(Operator::read, tmpItem, NULL, NULL);
				nextSym();
			}
			if (current_token.getSymbol() == Symbol::rparent) {
				break;
			}
		} while (current_token.getSymbol() == Symbol::comma);
		test({ Symbol::rparent }, fsys, 0);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
		testSemicolon();
	}
}

void Parser::write() {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "printf 语句" << endl;
#endif // DEBUG_Gramma_analysis
	/* begin with printf, end with semicolon*/
	bool wHasExp = false;
	bool wHasStr = false;
	bool wHasComma = false;
	Token str, com, exp;
	nextSym();
	SymSet fsys = statementBsys;
	fsys.insert({ Symbol::rbrace });
	test({ lparent }, fsys, 19);

	if (current_token.getSymbol() == Symbol::lparent) {
		nextSym();
		SymbolItem* expItem = NULL;
		SymbolItem* strItem = NULL;
		str = current_token;
		if (current_token.getSymbol() == stringcon) {
			// stab
			wHasStr = true;
			nextSym();
			strItem = new SymbolItem();
			strItem->obj = ObjectiveType::constty;
			strItem->typ = Type::stringtp;
			strItem->ident_name = str.getString();
		}
		com = current_token;
		if (current_token.getSymbol() == Symbol::comma) {
			wHasComma = true;
			nextSym();
		}
		exp = current_token;
		if (expBsys.count(current_token.getSymbol())) {
			wHasExp = true;
			expItem = expression(fsys);
		}
		codeGen.emit(Operator::print, NULL, strItem, expItem);
		/*
		if (wHasStr) {
			// if string exists, then add '\n' if exp doesnot exist or exp is const
			if (!wHasExp) {
				SymbolItem *r = new SymbolItem();
				r->ident_name.push_back('\n');
			}
			else if (expItem->obj == ObjectiveType::constty) {
				if (expItem->typ == Type::chartp)
					r->ident_name.push_back((char)expItem->addr);
				else if (expItem->typ == Type::inttp)
					r->ident_name += int2str(expItem->addr);
				r->ident_name.push_back('\n');
			}
			codeGen.emit(Operator::print, NULL, r, NULL);
		}
		else if (wHasExp) {
			// if exp exist, check if it is const 
			if (expItem->obj == ObjectiveType::constty) {
				SymbolItem* r = new SymbolItem();
				if (expItem->typ == Type::chartp)
					r->ident_name.push_back((char)expItem->addr);
				else if (expItem->typ == Type::inttp)
					r->ident_name += int2str(expItem->addr);
				r->ident_name.push_back('\n');
				r->typ = Type::stringtp;
				r->obj = ObjectiveType::constty;
				codeGen.emit(Operator::print, NULL, r, NULL);
			}
			else {
			}
		}
		*/
		// error report
		if (wHasComma) {
			if (!wHasStr) {
				error_handler.reportErrorMsg(str, 31);
#ifdef DEBUG_Gramma_analysis
				std::cout << str.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少字符串" << endl;
#endif // DEBUG_Gramma_analysis
			}
			if (!wHasExp) {
				error_handler.reportErrorMsg(exp, 33);
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少表达式" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}
		else {
			if (wHasStr && wHasExp) {
				error_handler.reportErrorMsg(com, 32);
#ifdef DEBUG_Gramma_analysis
				std::cout << current_token.getlc() << "行， " << current_token.getcc() << "列， ";
				std::cout << "出错：缺少逗号" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}
		test({ rparent }, fsys, 20);
		if (current_token.getSymbol() == Symbol::rparent) {
			nextSym();
		}
		testSemicolon();
	}
}

void Parser::condition(SymbolItem* label) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "condition 语句" << endl;
#endif // DEBUG_Gramma_analysis
	SymSet fsys = statementBsys;
	fsys.insert({ rparent, rbrace });

	SymbolItem* exp1 = expression(fsys);
	SymbolItem* exp2 = NULL;
	Symbol sy = current_token.getSymbol();
	Token syToken = current_token;
	if (logicalOperatorSys.count(sy)) {
		nextSym();
		exp2 = expression(fsys);
	}
	if (exp2 != NULL) {
		// check type
		set<Type> tpSet = { Type::inttp, Type::chartp };
		// if (tpSet.count(exp1->typ) &&  tpSet.count(exp2->typ) && exp1->typ != exp2->typ) {
		// 	error_handler.reportErrorMsg(current_token, 30);
		// }
		Operator op = codeGen.symbol2Operator(sy);
		if (exp1->obj == ObjectiveType::constty) {
			// if exp1 is const, then exchange exp1 and exp2.
			SymbolItem* tmpExp = exp1;
			exp1 = exp2;
			exp2 = tmpExp;
			op = codeGen.op_lr(op);
		}
		codeGen.emit(op, label, exp1, exp2);
		/*
		if (exp1->obj == ObjectiveType::constty && exp2->obj == ObjectiveType::constty) {
			int con = computeConditionVal(exp1->addr, exp2->addr, sy);
			if (con == 0)
				codeGen.setFalseCon();
			else if (con > 0)
				codeGen.setTrueCon();
			else {
#ifdef DEBUG_Gramma_analysis
				std::cout << syToken.getlc() << "行， " << syToken.getcc() << "列， ";
				std::cout << "出错：非法逻辑运算符" << endl;
#endif // DEBUG_Gramma_analysis
			}
		}
		else {	// only need jump when exp is not const
			Operator op = codeGen.symbol2Operator(sy);
			if (exp1->obj == ObjectiveType::constty) {
				// if exp1 is const, then exchange exp1 and exp2.
				SymbolItem* tmpExp = exp1;
				exp1 = exp2;
				exp2 = tmpExp;
				op = codeGen.op_lr(op);
			}
			codeGen.emit(op, label, exp1, exp2);
		}*/
	}
	else {
		codeGen.emit(Operator::jez, label, exp1, NULL);
		/*
		if (exp1->obj == ObjectiveType::constty) {
			if (exp1->addr == 0)
				codeGen.setFalseCon();
			else
				codeGen.setTrueCon();
		}
		else {	// only need jump when exp is not const
			codeGen.emit(Operator::jez, label, exp1, NULL);
		}*/
	}
}

SymbolItem* Parser::expression(SymSet fsys) {
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "expression 语句" << endl;
#endif // DEBUG_Gramma_analysis
	// need an item record result of an expression
	bool firstItem = true;
	SymbolItem* item1 = NULL;
	SymbolItem* item2 = NULL;
	SymbolItem* r = NULL;
	test(expBsys, fsys, 33);
	SymSet itemFsys = { Symbol::plus, Symbol::minus, Symbol::semicolon };
	if (expBsys.count(current_token.getSymbol())) {
		Symbol sy = current_token.getSymbol();
		if (sy == Symbol::plus || sy == Symbol::minus) {
			nextSym();
		}
		item1 = item(itemFsys);
		if (sy == Symbol::minus) {
			if (item1->obj == ObjectiveType::constty) {
				item1 = codeGen.genCon(Type::inttp, -item1->addr);
			}
			else {
				r = codeGen.genTemp();
				codeGen.emit(Operator::neg, r, item1, NULL);
				item1 = r;
				r = NULL;
			}
		}
		sy = current_token.getSymbol();
		while (sy == Symbol::plus || sy == Symbol::minus) {
			nextSym();
			item2 = item(itemFsys);
			if (item1->obj == ObjectiveType::constty && item2->obj == ObjectiveType::constty) {
				if (firstItem) {
					firstItem = false;
					item1 = codeGen.genCon(item1->typ, item1->addr);
				}
				item1->addr = sy == Symbol::plus ? item1->addr + item2->addr : item1->addr - item2->addr;
				item1->ident_name = int2str(item1->addr);
				item1->typ = Type::inttp;
				sy = current_token.getSymbol();
				continue;
			}
			r = codeGen.genTemp();
			codeGen.emit(codeGen.symbol2Operator(sy), r, item1, item2);
			item1 = r;
			item2 = r = NULL;
			sy = current_token.getSymbol();
		}
		return item1;
	}
	else {
		return new SymbolItem();
	}
}

SymbolItem* Parser::item(SymSet fsys) {
	// factor is connected with * or /
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "item 语句" << endl;
#endif // DEBUG_Gramma_analysis
	bool firstItem = true;
	SymbolItem* r = NULL;
	SymbolItem* factor1 = NULL;
	SymbolItem* factor2 = NULL;
	fsys.insert({Symbol::times, Symbol::slash});
	test(expBsys, fsys, 34);
	if (expBsys.count(current_token.getSymbol())) {
		factor1 = factor(fsys);
		Symbol sy = current_token.getSymbol();
		while (sy == Symbol::times || sy == Symbol::slash) {
			nextSym();
			factor2 = factor(fsys);
			if (factor1->obj == ObjectiveType::constty && factor2->obj == ObjectiveType::constty) {
				if (firstItem) {
					firstItem = false;
					factor1 = codeGen.genCon(factor1->typ, factor1->addr);
				}
				factor1->addr = sy ==  Symbol::times ? factor1->addr * factor2->addr : factor1->addr / factor2->addr; 
				factor1->ident_name = int2str(factor1->addr);
				factor1->typ = Type::inttp;
				sy = current_token.getSymbol();
				continue;
			}
			r = codeGen.genTemp();
			codeGen.emit(codeGen.symbol2Operator(sy), r, factor1, factor2);
			factor1 = r;
			factor2 = r = NULL;
			sy = current_token.getSymbol();
		}
		return factor1;
	}
	return new SymbolItem();
}

SymbolItem* Parser::factor(SymSet fsys) {
	/* begin symbol set:
	plus, minus, intcon, charcon, ident
	*/
#ifdef DEBUG_Gramma_analysis
	int lc = current_token.getlc();
	std::cout << lc << "行: ";
	std::cout << "factor 语句" << endl;
#endif // DEBUG_Gramma_analysis
	
	test(expBsys, fsys, 35);
	if (expBsys.count(current_token.getSymbol())) {
		switch (current_token.getSymbol()) {
		case Symbol::ident: {
			// search symbol table
			name_token = current_token;
			SymbolItem *tmpItem = symTab.search(name_token);
			nextSym();
			if (tmpItem) {
				switch (tmpItem->obj) {
				case ObjectiveType::arrayty: {
					SymbolItem* offset = selector(tmpItem, fsys);
					SymbolItem* r = codeGen.genTemp();
					codeGen.emit(Operator::arrLd, r, offset, tmpItem);
					return r;
					break;
				}
				case ObjectiveType::functy: {
					if (tmpItem->hasRet == false) {
						error_handler.reportErrorMsg(name_token, 28);
					}
					return call(fsys, tmpItem);
					break;
				}
				case ObjectiveType::varty: {
					// load var
					return tmpItem;
					break;
				}
				case ObjectiveType::constty: {
					// displace
					return tmpItem;
					break;
				}
				default: {
					error_handler.reportErrorMsg(name_token, 29);
				}
				}
			}
			else {
				error_handler.reportErrorMsg(name_token, 22);
			}
			break;
		}
		case Symbol::plus:
		case Symbol::minus:
		case Symbol::intcon: {
			// do something
			int tmp = this->signedInt(fsys);
			SymbolItem* r = codeGen.genCon(Type::inttp, tmp);
			return r;
			break;
		}
		case Symbol::charcon: {
			// do something
			SymbolItem* r = codeGen.genCon(Type::chartp, current_token.getConstChar());
			nextSym();
			return r;
			break;
		}
		case Symbol::lparent: {
			nextSym();
			SymbolItem * r = expression(fsys);
			if (r->obj == ObjectiveType::constty) {
				/* if (const_ident), create a new const SymbolItem, otherwise */ 
				r = codeGen.genCon(Type::inttp, r->addr);
				// r->obj
			}
			test({ rparent }, fsys, 0);
			if (current_token.getSymbol() == rparent) {
				nextSym();
			}
			return r;
			break;
		}
		}
	}
	return new SymbolItem();
}

