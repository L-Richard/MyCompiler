#include "pch.h"
#include "Error.h"

Error::Error()
{
}
void Error::reportErrorMsg(int cc, int lc, int type, string error_msg) {
	ErrorMsg e;
	e.cc = cc;
	e.lc = lc;
	e.type = type;
	e.error_msg = error_msg;
	string msg = "";
	errorMsgs.push_back(e);
#ifdef DEBUG
	cout << lc << " rows, " << cc << " columns, error type: " << type << " "
		 << msg << error_msg << endl;
#endif
}

Error::~Error()
{
}
