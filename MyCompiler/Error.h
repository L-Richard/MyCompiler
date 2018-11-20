#pragma once
#include <vector>
#include <string>
#include <iostream>
using namespace std;
class Error {
private:
	struct ErrorMsg {
		string error_msg;	// 补充信息
		int cc;		// 出错行内列号
		int lc;		// 出错行号
		int type;	// 出错类型
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
