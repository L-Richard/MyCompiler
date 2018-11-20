#pragma once
#include <vector>
#include <string>
#include <iostream>
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
	Error();
	~Error();
};

enum ErrorType {

};
