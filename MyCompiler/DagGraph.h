#pragma once
#include "MidCodeGen.h"
#include "c0_enum.h"
#include <map>
#include <set>
#include <list>
#include <vector>
#include <sstream>

using namespace std;


//
typedef struct Node {
	list<Quadruples> codes;	// 除了call以外都只含一个四元式

	list<SymbolItem*> vars;	// 记录利用过该节点的所有变量,(叶节点的vars中只有一个)
	list<SymbolItem*> current_vars;	// 记录当前对应该节点的所有变量
	list<Node*> fathers;
	Node *left = NULL;
	Node *right = NULL;
	Operator op = Operator::notOp;

	SymbolItem* repVar = NULL;	// 该节点的代表变量

	// used when generating opt codes:
	bool isGenerated = false;
	set<SymbolItem*> _vars;
	SymbolItem* _tmp = NULL;

	// 如果current_vars为空，我们需要在vars中找一个使用，需要反向记

	// function:
	bool isLeaf() { return left == NULL && right == NULL; }
	SymbolItem* getNodeItem() {	// 从节点选一个代表变量
		// 如果右非 tmp 类型，先current_vars中的非tmp类型全部生成
		// 如果只有tmp类型，再生成tmp的代码。
		return NULL;
	}
} Node ;



class DagGraph {
	vector<Quadruples> &midCodes;
	vector<Quadruples> optMidCodes;
	SymbolTable &tab;

	SymbolItem* fun = NULL;	// 添加临时变量
	int tmp_dag = 0;

	map<SymbolItem*, Node*> nodeTab;	// 节点表
	list<Node*> topNodes;				// top节点集

	// check Quadruples type
	// bool isCal(Quadruples* code);
	// bool isSeparate(Quadruples* code);

	// generate dag graph by insert new Node,
	// but parameter is Quardruples pointer
	void insertNode(Quadruples* midCode);	

	// 下面三个函数用于insertNode内
	void createFatherNode(Quadruples* code, Node* left, Node*right = NULL);	
	Node* createLeaf(SymbolItem* src, Quadruples* code = NULL);
	void changeNode(SymbolItem* src, Node* node);		// 更新变量对应的节点

	// generate optimized codes with the dag graph
	void genOptCode();
	void node2Code(Node* node);

	// 添加新的临时变量
	SymbolItem* addTemp() {
		stringstream ss;
		ss << "!Temp_Dag__" << tmp_dag++;
		SymbolItem* tmp = new SymbolItem();
		tmp->typ = Type::inttp;
		tmp->obj = ObjectiveType::tmp;
		tmp->ident_name = ss.str();
	
		auto ftab = this->fun->link;
		(*ftab)[tmp->ident_name] = tmp;
		tmp->level = 1;
		tmp->addr = this->fun->totalSize;
		this->fun->totalSize += INTSIZE;
		this->fun->tmpSize += INTSIZE;
		return tmp;
	}

public:
	DagGraph(SymbolTable &tab, vector<Quadruples>&codes);
	~DagGraph();
	// main optimize function
	void dagOpt();
	vector<Quadruples> &getOptMidCodes() { return this->optMidCodes; }
};







