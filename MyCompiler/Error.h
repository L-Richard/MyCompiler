#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "Token.h"
using namespace std;
class Error {
private:
	struct ErrorMsg {
		string error_msg;	// ������Ϣ
		int cc;		// ���������к�
		int lc;		// �����к�
		int type;	// ��������
	};
	vector<ErrorMsg> errorMsgs;
	bool noError = true;
public:
	void reportErrorMsg(int cc, int lc, int type, string error_msg="");
	void reportErrorMsg(Token &t, int type, string error_msg="");
	Error();
	~Error();
};

enum ErrorType {

};
