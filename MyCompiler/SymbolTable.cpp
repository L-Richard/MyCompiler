#include "pch.h"
#include "SymbolTable.h"


SymbolTable::SymbolTable(Error &error_handler) : error_handler(error_handler)
{
}

SymbolTable::~SymbolTable() {

}

void SymbolTable::enterFunc(Token name_token, enum Type typ) {
	string name = name_token.getIdentName();
	if (level != 0) {
		// error: define function in functon
		error_handler.reportErrorMsg(name_token, 23);
	}
	SymbolItem *item = new SymbolItem();
	if (symTab.count(name)) {
		// error: redefine identifier
		error_handler.reportErrorMsg(name_token, 9);
	}
	else {
		symTab[name] = item;
		item->obj = functy;
		item->link = new map<string, SymbolItem*>;
		item->ident_name = name;
		item->level = 0;
		item->typ = typ;
		if (typ == Type::inttp || typ == Type::chartp) {
			item->hasRet = true;
		}
		// change Symbol state
		level = 1;
		lastFunItem = item;
		ftab = lastFunItem->link;
	}
}

void SymbolTable::enterVar(Token name_token, enum Type typ, int arraySize) {
	string name = name_token.getIdentName();
	SymbolItem *item = new SymbolItem();
	if (level == 0) {
		if (symTab.count(name)) {
			// error: redefine
			error_handler.reportErrorMsg(name_token, 9);
		}
		else {
			symTab[name] = item;
			
		}
	}
	else {
		if (ftab->count(name)) {
			// error: redefine
			error_handler.reportErrorMsg(name_token, 9);
		}
		else {
			(*ftab)[name] = item;
		}
	}
	item->ident_name = name;
	item->typ = typ;
	item->addr = stack;
	item->level = this->level;
	int tmpSize = 0;
	switch (typ) {
	case Type::inttp:	tmpSize = INTSIZE;	break;
	case Type::chartp:	tmpSize = CHARSIZE;	break;
	}
	if (arraySize == 0) {
		item->obj = ObjectiveType::varty;
		stack += tmpSize;
	}
	else {
		item->obj = ObjectiveType::arrayty;
		stack += tmpSize * arraySize;
	}
}

void SymbolTable::enterPara(Token name_token, enum Type typ) {
	string name = name_token.getIdentName();
	enterVar(name_token, typ, 0);
	lastFunItem->paraList.push_back(typ);
}

void SymbolTable::enterConst(Token name_token, enum Type typ, int constValue) {
	string name = name_token.getIdentName();
	SymbolItem *item = new SymbolItem();
	if (level == 0) {
		if (symTab.count(name)) {
			// error:
			error_handler.reportErrorMsg(name_token, 9);
		}
		else {
			symTab[name] = item;
		}
	}
	else {
		if (ftab->count(name)) {
			// error:
			error_handler.reportErrorMsg(name_token, 9);
		}
		else {
			(*ftab)[name] = item;
		}
	}
	item->obj = ObjectiveType::constty;
	item->typ = typ;
	item->addr = constValue;
}

SymbolItem* SymbolTable::search(Token name_token) {
//	SymbolItem *tmpR;
	string name = name_token.getIdentName();
	if (level == 1) {
		if (ftab->count(name)) {
			return (*ftab)[name];
		}
	}
	if (symTab.count(name)) {
		return symTab[name];
	}
	error_handler.reportErrorMsg(name_token, 22);
	return NULL;
}

void SymbolTable::funDone() {
	// delete table in function, and reset pointers to NULL
	return;
	for (auto i = ftab->begin(); i != ftab->end(); i++) {
		delete (i->second);
		i->second = NULL;
	}
	delete ftab;
	ftab = NULL;
	lastFunItem->link = NULL;
	level = 0;
}

