#include "pch.h"
#include "DagGraph.h"

// unary operator
set<Operator> one = { Operator::assignOp, Operator::neg };

// Operator: 0-26: classify the mid codes
set<Operator> calOpSet = {
	Operator::minus,
	Operator::plus,
	Operator::slash,
	Operator::times,
	Operator::neg,
	Operator::assignOp,
};// 6: arrLd, arrSt ???

set<Operator> sepOpSet = {
	Operator::notOp,
	Operator::st$ra,
	Operator::arrLd,
	Operator::arrSt,
	Operator::eql,
	Operator::neq,
	Operator::geq,
	Operator::gtr,
	Operator::leq,
	Operator::les,
	Operator::setLabel,
	Operator::returnOp,
	Operator::save,
	Operator::stPara,
	Operator::call,
	Operator::stRetVal,
	Operator::restore,
	Operator::jmp,
	Operator::jez,
	Operator::read,
	Operator::print,
};// 21



DagGraph::DagGraph(SymbolTable &tab, vector<Quadruples>&codes): midCodes(codes), tab(tab){
}

DagGraph::~DagGraph()
{
}


void DagGraph::dagOpt() {
	for (auto item = midCodes.begin(); item != midCodes.end(); ) {
		if (item->op == Operator::setLabel 
			&& (item->dst->lab == labelType::fun_lb || item->dst->lab == labelType::main_lb)) {

			this->fun = item->dst->funItem;
		}
		if (sepOpSet.count(item->op)) {
			optMidCodes.push_back(*item);
			item++;
		}
		else if(calOpSet.count(item->op)) {
			while (item != midCodes.end() && sepOpSet.count(item->op) == 0) {
				insertNode(&*item);
				item++;
			}
			genOptCode();
/*
			if (sepOpSet.count(item->op)) {
				optMidCodes.push_back(*item);
			}
			if (item == midCodes.end())
				break;
*/
		}
	}
}

void DagGraph::createFatherNode(Quadruples* code, Node* node_left, Node* node_right) {
	// ���ڵ�û��ͬ�����㣬�����µĸ��ڵ㣬
	// ��¼op�����б�������ǰ�������ӽڵ�ָ�롣
	Node* node_father = new Node();
	node_father->left = node_left;
	node_father->right = node_right;
	node_father->op = code->op;
	// �ı�dst�����Ľڵ�Ϊ�½���father�ڵ㡣
	changeNode(code->dst, node_father);
	// ���ڵ����top�ڵ㼯
	topNodes.push_back(node_father);
	// �ӽڵ㲻�˳�top�ڵ㼯�����ڵ����˳�����ɴ���
	// topNodes.remove(node_left);
	// topNodes.remove(node_right);

	// �ӽڵ��¼���ڵ�ָ��
	if (node_left != NULL) {
		node_left->fathers.push_back(node_father);
	}
	if (node_right != NULL) {
		node_right->fathers.push_back(node_father);
	}
}

Node* DagGraph::createLeaf(SymbolItem* src, Quadruples* code) {
	// Ҷ�ڵ㲻����topNodes
	Node* node = new Node();
	node->vars.push_back(src);
	node->current_vars.push_back(src);
	node->repVar = src;
	nodeTab[src] = node;
	node->isGenerated = true;
	return node;
}

void DagGraph::changeNode(SymbolItem* dst, Node* node) {
	node->vars.push_back(dst);
	node->current_vars.push_back(dst);
	if (nodeTab.count(dst) && !nodeTab[dst]->isLeaf()) {	// Ҷ�ڵ�Ҫ��������
		nodeTab[dst]->current_vars.remove(dst);
		//if (nodeTab[dst]->current_vars.empty()) {
		//	topNodes.remove(nodeTab[dst]);	// ����ڵ�û�б��������ô�����ʼ����
		//}
	}
	nodeTab[dst] = node;
}

void DagGraph::insertNode(Quadruples* code) {
	SymbolItem* left = code->src1;
	SymbolItem* right = code->src2;
	Node* node_left  = NULL;
	Node* node_right = NULL;
	int flag_num = 0;

	// ��ڵ�
	if (!nodeTab.count(left))
		node_left = createLeaf(left);
	else
		node_left = nodeTab[left];

	// �ҽڵ㣺���ж��Ƿ��Ƕ�Ԫ����
	if (!one.count(code->op))
		if (!nodeTab.count(right))
			node_right = createLeaf(right);
		else
			node_right = nodeTab[right];

	// �����ڵ�
	auto item = node_left->fathers.begin();
	while (item != node_left->fathers.end()) {
		if ((*item)->op == code->op) {
			if (one.count(code->op)) {
				changeNode(code->dst, *item);
				break;
			}
			else {
				if ((*item)->right == node_right) {
					changeNode(code->dst, *item);
					break;
				}
			}
		}
		item++;
	}
	if (item == node_left->fathers.end()) {
		// ����end()��ʾû���ҵ���Ӧ����
		createFatherNode(code, node_left, node_right);
	}
}

void DagGraph::genOptCode() {
	for (auto item = topNodes.begin(); item != topNodes.end(); item++) {
		node2Code(*item);
	}
}


void DagGraph::node2Code(Node* node) {
	if (node->isLeaf()) {
		return;
	}
	vector<SymbolItem*> _vars;
	vector<SymbolItem*> _tmps;
	vector<SymbolItem*> _const;
	for (auto item = node->current_vars.begin(); item != node->current_vars.end(); item++) {
		if ((*item)->obj == ObjectiveType::tmp)
			_tmps.push_back(*item);
		else if ((*item)->obj == ObjectiveType::constty)
			_const.push_back(*item);
		else if ((*item)->obj == ObjectiveType::varty)
			_vars.push_back(*item);
		else {
			cout << "fatal: DagGraph::node2Code: wrong obj: " << (*item)->obj << endl;
			exit(1);
		}
	}
	// set representative var of node
	if (node->repVar != NULL)
		;
	else if (!_vars.empty()) {
		node->repVar = _vars[0];
		// ��һ��ָ����㣬����ֱ��assign
		auto var = _vars.begin();
		Quadruples code;
		code.dst = *var;
		code.op = node->op;
		code.src1 = node->left->repVar;
		if (!one.count(node->op))
			code.src2 = node->right->repVar;
		optMidCodes.push_back(code);
		auto var_2 = var;
		for (var_2++; var_2 != _vars.end(); var_2++) {
			Quadruples code;
			code.dst = *var_2;
			code.op = Operator::assignOp;
			code.src1 = *var;
			optMidCodes.push_back(code);
		}
		/*
		for (auto var = _vars.begin(); var != _vars.end(); var++) {
			Quadruples code;
			code.dst = *var;
			code.op = node->op;
			code.src1 = node->left->repVar;
			if (!one.count(node->op))
				code.src2 = node->right->repVar;
			optMidCodes.push_back(code);
		}
		*/
	}
	else {
		if (!_tmps.empty()) {
			node->repVar = _tmps[0];
		}
		else { // current_vars empty, addTemp
			node->repVar = addTemp();
		}
		Quadruples code;
		code.dst = node->repVar;
		code.op = node->op;
		code.src1 = node->left->repVar;
		if (!one.count(node->op))
			code.src2 = node->right->repVar;
		optMidCodes.push_back(code);
	}
}




