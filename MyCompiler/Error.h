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
public:
	void reportErrorMsg(int cc, int lc, int type, string error_msg="");
	void reportErrorMsg(Token &t, int type, string error_msg="");
	bool hasError() {
		return errorMsgs.size() > 0;
	}

	void print() {
		for (auto item = errorMsgs.begin(); item != errorMsgs.end(); item++) {
			cout << "Error: " << item->lc << "��, " << item->cc << "��: " << "error type: "
				<< item->type;
		}
	}
	Error();
	~Error();
};

enum ErrorType {

};
