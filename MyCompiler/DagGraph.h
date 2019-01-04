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
	list<Quadruples> codes;	// ����call���ⶼֻ��һ����Ԫʽ

	list<SymbolItem*> vars;	// ��¼���ù��ýڵ�����б���,(Ҷ�ڵ��vars��ֻ��һ��)
	list<SymbolItem*> current_vars;	// ��¼��ǰ��Ӧ�ýڵ�����б���
	list<Node*> fathers;
	Node *left = NULL;
	Node *right = NULL;
	Operator op = Operator::notOp;

	SymbolItem* repVar = NULL;	// �ýڵ�Ĵ������

	// used when generating opt codes:
	bool isGenerated = false;
	set<SymbolItem*> _vars;
	SymbolItem* _tmp = NULL;

	// ���current_varsΪ�գ�������Ҫ��vars����һ��ʹ�ã���Ҫ�����

	// function:
	bool isLeaf() { return left == NULL && right == NULL; }
	SymbolItem* getNodeItem() {	// �ӽڵ�ѡһ���������
		// ����ҷ� tmp ���ͣ���current_vars�еķ�tmp����ȫ������
		// ���ֻ��tmp���ͣ�������tmp�Ĵ��롣
		return NULL;
	}
} Node ;



class DagGraph {
	vector<Quadruples> &midCodes;
	vector<Quadruples> optMidCodes;
	SymbolTable &tab;

	SymbolItem* fun = NULL;	// �����ʱ����
	int tmp_dag = 0;

	map<SymbolItem*, Node*> nodeTab;	// �ڵ��
	list<Node*> topNodes;				// top�ڵ㼯

	// check Quadruples type
	// bool isCal(Quadruples* code);
	// bool isSeparate(Quadruples* code);

	// generate dag graph by insert new Node,
	// but parameter is Quardruples pointer
	void insertNode(Quadruples* midCode);	

	// ����������������insertNode��
	void createFatherNode(Quadruples* code, Node* left, Node*right = NULL);	
	Node* createLeaf(SymbolItem* src, Quadruples* code = NULL);
	void changeNode(SymbolItem* src, Node* node);		// ���±�����Ӧ�Ľڵ�

	// generate optimized codes with the dag graph
	void genOptCode();
	void node2Code(Node* node);

	// ����µ���ʱ����
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







