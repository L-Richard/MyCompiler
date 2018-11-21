#pragma once
#include <string>
enum ObjectiveType {
	constty,
	varty,
	functy,
	arrayty
};
enum Type {
	inttp,
	chartp,
	nottp
};

struct bTabItem {
	
};

struct SymbolItem {
	std::string ident_name;
	
	enum ObjectiveType obj;

	enum Symbol typ;
	int addr;		// offset from base of this block
	bTabItem *link;
	bool isGlobal;
};






class SymbolTable {
private:
	
public:
	SymbolTable();
	~SymbolTable();
};

