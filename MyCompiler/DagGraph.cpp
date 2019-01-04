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
	// 父节点没有同样运算，建立新的父节点，
	// 记录op，所有变量，当前变量，子节点指针。
	Node* node_father = new Node();
	node_father->left = node_left;
	node_father->right = node_right;
	node_father->op = code->op;
	// 改变dst变量的节点为新建的father节点。
	changeNode(code->dst, node_father);
	// 父节点进入top节点集
	topNodes.push_back(node_father);
	// 子节点不退出top节点集，按节点产生顺序生成代码
	// topNodes.remove(node_left);
	// topNodes.remove(node_right);

	// 子节点记录父节点指针
	if (node_left != NULL) {
		node_left->fathers.push_back(node_father);
	}
	if (node_right != NULL) {
		node_right->fathers.push_back(node_father);
	}
}

Node* DagGraph::createLeaf(SymbolItem* src, Quadruples* code) {
	// 叶节点不加入topNodes
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
	if (nodeTab.count(dst) && !nodeTab[dst]->isLeaf()) {	// 叶节点要保留变量
		nodeTab[dst]->current_vars.remove(dst);
		//if (nodeTab[dst]->current_vars.empty()) {
		//	topNodes.remove(nodeTab[dst]);	// 如果节点没有变量，不用从它开始生成
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

	// 左节点
	if (!nodeTab.count(left))
		node_left = createLeaf(left);
	else
		node_left = nodeTab[left];

	// 右节点：先判断是否是二元运算
	if (!one.count(code->op))
		if (!nodeTab.count(right))
			node_right = createLeaf(right);
		else
			node_right = nodeTab[right];

	// 处理父节点
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
		// 到达end()表示没有找到相应运算
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
		// 第一条指令计算，其余直接assign
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




