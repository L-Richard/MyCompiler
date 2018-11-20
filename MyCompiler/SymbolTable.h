#pragma once
#include <string>
enum ObjectiveType {
	constty,
	varty,
	functy,
	arrayty,
};
struct SymbolItem {
	std::string ident_name;
	enum ObjectiveType obj;
	enum Symbol typ;
	int addr;		// offset from base of this block
};
class SymbolTable {
private:
	
public:
	SymbolTable();
	~SymbolTable();
};

