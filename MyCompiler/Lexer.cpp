#include "pch.h"
#include "Lexer.h"
using namespace std;
Lexer::Lexer(string filename, Error& error_handler) : error_handler(error_handler) {
	// 1.error handle 2.
	source_file.open(filename);
	source_file.getline(line, llng);
	initial_reserved_word();
	ll = strlen(line);
	//cout << line << endl;
	cc = 0;
	lc = 1;
	getch();
}

void Lexer::getch() {
	while (cc == ll) {
		cc = 0;
		if (source_file.eof()) {
#ifdef DEBUG_Lexer_analysis
			cout << "end of source file" << endl;
#endif // DEBUG_Lexer_analysis
			ch = EOF;
			return;
		}
		source_file.getline(line, llng);
		//cout << line << endl;
		ll = strlen(line);
		lc++;
	}
	ch = line[cc];
	cc++;
	return;
}

Token Lexer::getSym() {
	int line_num, char_counter;		// record line number and char counter
	while (ch == ' ' || ch == '\t' || ch == '\0')
		getch();
	// initialize the return values
	line_num = lc;
	char_counter = cc - 1;
	ident_name = "";
	inum = 0;
	sym = notsy;
	if (isalpha(ch) || ch == '_') {
		// 字母开头，判断是否时保留字，否则是ident
		do {
			ident_name.push_back(ch);
			getch();
		} while (isalnum(ch) || ch == '_');
		if (reserved_words.count(ident_name)) {
			sym = reserved_words[ident_name];
		}
		else {
			sym = Symbol::ident;
		}
	}
	else if (isdigit(ch)) {
		// 数字开头，读一个整数常量，数字不可以有前导零
		sym = Symbol::intcon;	// const of integer
		bool isZero = ch == '0';	// if first number is zero
		int numLength = 0;
		do {
			inum = inum * 10 + ch - '0';
			numLength++;
			getch();
			if (isZero && isdigit(ch)) {
				numLength--;
				isZero = ch == '0';
				if (!isZero) {
					// error 
					string msg = "\tError: 整数常量不能有前导零";
					error_handler.reportErrorMsg(char_counter, line_num, 0, msg);
				}
			}
		} while (isdigit(ch));
		if (numLength > ilmax) {
			inum = 0;
			string msg = "\tError: 数字长度超界";
			error_handler.reportErrorMsg(char_counter, line_num, 0, msg);
		}
		// 需要跳读，跳过后续所有数字
		while (isdigit(ch)) {
			getch();
		}
	}
	// 特殊字符，字符串，字符常量等等。。
	else if (ch == '"') {
		// 双引号开头，字符串
		sym = Symbol::stringcon;
		getch();
		bool invalid_string_char = false;
		while (ch != '"' && cc != 1) {
			if (ch < 32 || ch > 126) {
				invalid_string_char = true;
			}
			ident_name.push_back(ch);	// 可以用ident_name记录字符串
			getch();
		}
		if (ch != '"') {
			// correctly end, error handle
			string msg = "\tError: 字符串末尾缺少双引号";
			error_handler.reportErrorMsg(char_counter, line_num, 3, msg);
		}
		else {
			getch();
		}
		if (invalid_string_char) {
			string msg = "\tError: 字符串中使用了非法字符";
			error_handler.reportErrorMsg(char_counter, line_num, 3, msg);
		}
	}
	else if (ch == '\'') {
		// 字符常量，两个单引号中间必须含有一个字符，数字，字母，加减乘除号，下划线
		// 出错后跳读到行尾
		sym = Symbol::charcon;
		getch();
		while (ch != '\'' && cc != 1) {
			ident_name.push_back(ch);	// record charactors between quote
			getch();
		}
		if (ch != '\'') {
			// error: missing quote
			error_handler.reportErrorMsg(char_counter, line_num, 1, "missing quote");
		}
		else {
			getch();
			if (ident_name.size() > 1) {
				// error: too many charactors
				error_handler.reportErrorMsg(char_counter, line_num, 1, "too many chars");
			}
			else if (ident_name.size() == 0) {
				// error: must has a charactor
				error_handler.reportErrorMsg(char_counter, line_num, 1, "must has a charactor");
			} 
			else { // ident_name.size() == 1 and end with '
				char tmp = ident_name[0];
				if (!(isalnum(tmp) || tmp == '+' || tmp == '-' || tmp == '*' || tmp == '/' || tmp == '_')) {
					// error: illegal const char
					error_handler.reportErrorMsg(char_counter, line_num, 1, "illegal const char");
				}
			}
		}
	}
	else if (ch == '!') {
		getch();
		if (ch == '=') {
			sym = Symbol::neq;
			getch();
		}
		else {
			//error:!后面要有=
			error_handler.reportErrorMsg(char_counter, line_num, 0, "missing '=' after '!'");
		}
	}
	else if (ch == '>') {
		getch();
		if (ch == '=') {
			sym = Symbol::geq;
			getch();
		}
		else {
			sym = Symbol::gtr;
		}
	}
	else if (ch == '<') {
		getch();
		if (ch == '=') {
			sym = Symbol::leq;
			getch();
		}
		else {
			sym = Symbol::les;
		}
	}
	else if (ch == '=') {
		getch();
		if (ch == '=') {
			sym = Symbol::eql;
			getch();
		}
		else {
			sym = Symbol::becomes;
		}
	}
	else {
		string tmp = "";
		tmp.push_back(ch);
		if (reserved_words.count(tmp)) {
			sym = reserved_words[tmp];
		}
		else  {
			sym = Symbol::notsy;
			if (ch == EOF) {
				sym = Symbol::eofsy;
			}
			else {
				// error: 非法字符
				error_handler.reportErrorMsg(char_counter, line_num, 0, "非法字符");
				cout << ch << endl;
			}
		}
		getch();
	}
	Token t = Token(sym, line_num, char_counter, ident_name, inum);
	return t;
}


void Lexer::initial_reserved_word() {
	reserved_words["+"] = Symbol::plus;
	reserved_words["-"] = Symbol::minus;
	reserved_words["*"] = Symbol::times;
	reserved_words["/"] = Symbol::slash;

	reserved_words["("] = Symbol::lparent;
	reserved_words["["] = Symbol::lsquare;
	reserved_words["]"] = Symbol::rsquare;
	reserved_words[")"] = Symbol::rparent;
	reserved_words["{"] = Symbol::lbrace;
	reserved_words["}"] = Symbol::rbrace;

	reserved_words[","] = Symbol::comma;
	reserved_words["."] = Symbol::period;
	reserved_words[";"] = Symbol::semicolon;
	reserved_words["'"] = Symbol::quote;
	reserved_words[":"] = Symbol::colon;
	reserved_words["="] = Symbol::eql;

//  not used
//	reserved_words["<"] = Symbol::lss;
//	reserved_words[">"] = Symbol::gtr;
//	reserved_words[">="] = Symbol::geq;
//	reserved_words["<="] = Symbol::leq;
//	reserved_words["!="] = Symbol::neq;

	reserved_words["char"] = Symbol::charsy;
	reserved_words["int"] = Symbol::intsy;
	reserved_words["const"] = Symbol::constsy;
	reserved_words["void"] = Symbol::voidsy;

	reserved_words["if"] = Symbol::ifsy;
	reserved_words["while"] = Symbol::whilesy;
	reserved_words["switch"] = Symbol::switchsy;
	reserved_words["case"] = Symbol::casesy;
	// 也可以处理成内置函数，在解析前填入符号表。
	reserved_words["printf"] = Symbol::printfsy;
	reserved_words["scanf"] = Symbol::scanfsy;

}

Lexer::~Lexer()
{
}
