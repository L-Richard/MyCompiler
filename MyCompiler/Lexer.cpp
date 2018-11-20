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
		// ��ĸ��ͷ���ж��Ƿ�ʱ�����֣�������ident
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
		// ���ֿ�ͷ����һ���������������ֲ�������ǰ����
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
					string msg = "\tError: ��������������ǰ����";
					error_handler.reportErrorMsg(char_counter, line_num, 0, msg);
				}
			}
		} while (isdigit(ch));
		if (numLength > ilmax) {
			inum = 0;
			string msg = "\tError: ���ֳ��ȳ���";
			error_handler.reportErrorMsg(char_counter, line_num, 0, msg);
		}
		// ��Ҫ����������������������
		while (isdigit(ch)) {
			getch();
		}
	}
	// �����ַ����ַ������ַ������ȵȡ���
	else if (ch == '"') {
		// ˫���ſ�ͷ���ַ���
		sym = Symbol::stringcon;
		getch();
		bool invalid_string_char = false;
		while (ch != '"' && cc != 1) {
			if (ch < 32 || ch > 126) {
				invalid_string_char = true;
			}
			ident_name.push_back(ch);	// ������ident_name��¼�ַ���
			getch();
		}
		if (ch != '"') {
			// correctly end, error handle
			string msg = "\tError: �ַ���ĩβȱ��˫����";
			error_handler.reportErrorMsg(char_counter, line_num, 3, msg);
		}
		else {
			getch();
		}
		if (invalid_string_char) {
			string msg = "\tError: �ַ�����ʹ���˷Ƿ��ַ�";
			error_handler.reportErrorMsg(char_counter, line_num, 3, msg);
		}
	}
	else if (ch == '\'') {
		// �ַ������������������м���뺬��һ���ַ������֣���ĸ���Ӽ��˳��ţ��»���
		// �������������β
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
			//error:!����Ҫ��=
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
				// error: �Ƿ��ַ�
				error_handler.reportErrorMsg(char_counter, line_num, 0, "�Ƿ��ַ�");
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
	// Ҳ���Դ�������ú������ڽ���ǰ������ű�
	reserved_words["printf"] = Symbol::printfsy;
	reserved_words["scanf"] = Symbol::scanfsy;

}

Lexer::~Lexer()
{
}
