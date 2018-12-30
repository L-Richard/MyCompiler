#include "pch.h"
#include "SymbolTable.h"


SymbolTable::SymbolTable(Error &error_handler) : error_handler(error_handler)
{
}
SymbolTable::~SymbolTable() {

}

void SymbolTable::enterFunc(Token name_token, enum Type typ, SymbolItem* label) {
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
		item->fun_label = label;
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
	item->ident_name = name;
	item->typ = typ;
	item->level = this->level;
	item->arrayNumMax = arraySize;
	// compute variable size
	int tmpSize = typeSize(typ);
	if (arraySize == 0) {
		item->obj = ObjectiveType::varty;
	}
	else {
		item->obj = ObjectiveType::arrayty;
		tmpSize = tmpSize * arraySize;
	}
// check which table to insert
	if (level == 0) {
		if (symTab.count(name)) 
			error_handler.reportErrorMsg(name_token, 9);
		else {
			symTab[name] = item;
			item->varSize = tmpSize;
		}
	}
	else {
		if (ftab->count(name)) 
			error_handler.reportErrorMsg(name_token, 9);
		else {
			(*ftab)[name] = item;
			item->addr = stack;
			stack += tmpSize;
		}
	}
	align();
}

void SymbolTable::enterTemp(SymbolItem* tmp) {
	if (level == 0) {
#ifdef DEBUG
		cout << "fatal: SymbolTable::enterTemp(): level != 1" << endl;
#endif // DEBUG
	}
	(*ftab)[tmp->ident_name] = tmp;
	lastFunItem->tmpSize += INTSIZE;
	tmp->level = 1;
	tmp->addr = stack;
	stack += INTSIZE;
	align();
}

void SymbolTable::enterPara(Token name_token, enum Type typ) {
	string name = name_token.getIdentName();
	enterVar(name_token, typ, 0);
	lastFunItem->paraSize += typeSize(typ);
	lastFunItem->paraList.push_back(typ);
}

void SymbolTable::enterConst(Token name_token, enum Type typ, int constValue) {
	string name = name_token.getIdentName();
	SymbolItem *item = new SymbolItem();
	if (level == 0) {
		if (symTab.count(name)) {
			error_handler.reportErrorMsg(name_token, 9);
		}
		else {
			symTab[name] = item;
		}
	}
	else {
		if (ftab->count(name)) {
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
	/*
	if (ftab != NULL) {
		for (auto i = ftab->begin(); i != ftab->end(); i++) {
			delete (i->second);
			i->second = NULL;
		}
	}
	lastFunItem->link = NULL;
	delete ftab;
	*/
	lastFunItem->totalSize = stack;
	ftab = NULL;
	lastFunItem = NULL;
	
	level = 0;
	stack = 4;
}


/************************* Optimization  ****************************/
void SymbolTable::removeLastTemp(SymbolItem* Temp) {
	ftab->erase(Temp->ident_name);
	stack -= INTSIZE;
	lastFunItem->tmpSize -= INTSIZE;
}
void SymbolTable::killVar(SymbolItem* var) {

}

