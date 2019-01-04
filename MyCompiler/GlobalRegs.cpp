#include "pch.h"
#include "GlobalRegs.h"

// Operator: 0-26: classify the mid codes
const set<Operator> baseSet = {
	Operator::arrLd,
	Operator::arrSt,
	Operator::minus,
	Operator::plus,
	Operator::slash,
	Operator::times,
	Operator::neg,
	Operator::assignOp,
	Operator::read,
	Operator::print,
	Operator::save,
	Operator::stPara,
	Operator::call,
	Operator::stRetVal,
	Operator::restore,
// baseEndSet:
	Operator::eql,
	Operator::neq,
	Operator::geq,
	Operator::gtr,
	Operator::leq,
	Operator::les,
	Operator::returnOp,
	Operator::jmp,
	Operator::jez,
};// 15 + 9

const set<Operator> baseEndSet = {
	Operator::eql,
	Operator::neq,
	Operator::geq,
	Operator::gtr,
	Operator::leq,
	Operator::les,
	Operator::returnOp,
	Operator::jmp,
	Operator::jez,
};// 9

const set<Operator> sepSet = {
	Operator::notOp,
	Operator::st$ra,
	Operator::setLabel,
};// 3


GlobalRegs::GlobalRegs(SymbolTable &tab, vector<Quadruples> &midCodes) :tab(tab), midCodes(midCodes)
{
}

GlobalRegs::~GlobalRegs()
{
}


void GlobalRegs::allocateRegisters() {
	for (auto item = midCodes.begin(); item != midCodes.end(); item++) {
		// 先划分函数：
		while (item!=midCodes.end() && !isFunHead(&*item)) {
			item++;
		}
		if (item->dst->lab == labelType::fun_lb || item->dst->lab == labelType::main_lb) {
			funItem = item->dst->funItem;
		}
		if (item != midCodes.end()) { // 一定进入了函数体
			item++;
			if (item->op == Operator::st$ra) {	// 跳过函数内第一条存$ra指令
				item++;
			}
			auto funHead = item;
			while (item != midCodes.end() && !isFunHead(&*item)) {
				item++;
			}
			auto funTail = item; // prev(item, 1);
			if (funHead != midCodes.end() && funTail - funHead >= 0) {
				// 检查是否是有效的函数头尾。
				baseBlock(funHead, funTail);
			}
			item--;
		}
	}
}

void GlobalRegs::baseBlock(vector<Quadruples>::iterator funHead, vector<Quadruples>::iterator funTail) {
	list<Base*> baseList;
	Base* entrance = new Base();
	Base* exitBase = new Base();
	Base* lastBase = entrance;	// 顺序前置基本块
	map<SymbolItem*, Base*> label_base;		// setlabel，
	map<SymbolItem*, Base*> gotolabel_base;	// gotolabel,
	for (auto item = funHead; item != funTail; item++) {
		Base* base = new Base();
		baseList.push_front(base);
		if (sepSet.count(item->op)) {
			base->isBase = false;
			while (item != funTail && sepSet.count(item->op)) {
				if (item->op == Operator::setLabel) {
					base->labels.insert(item->dst);
					label_base[item->dst] = base;
				}
				if (gotolabel_base.count(item->dst)) {
					// 有指令跳转到这个label
					base->lastBases.insert(gotolabel_base[item->dst]);
					gotolabel_base[item->dst]->nextBases.insert(base);
				}
				item++;
			}
			if (lastBase != NULL) {
				base->lastBases.insert(lastBase);
				lastBase->nextBases.insert(base);
			}
			lastBase = base;
			if (item == funTail)
				base->nextBases.insert(exitBase);
		} // end if (sepSet.count(item->op))
		else if (baseSet.count(item->op)) {
			base->isBase = true;
			while (item != funTail) {
				switch (item->op) {
					case Operator::arrSt: {
						base->inUse(item->src1);
						base->inUse(item->dst);
						break;
					}
					case Operator::stPara: {
						auto tmp = item->src1->paras.begin();
						while (tmp != item->src1->paras.end()) {
							base->inUse(*tmp);
							tmp++;
						}
						break;
					}
					case Operator::restore:
					case Operator::call:
					case Operator::save: break;
					default: {
						base->inUse(item->src1);
						base->inUse(item->src2);
						base->inDef(item->dst);
					} // end default
				} // end switch
				if (!sepSet.count(item->op)) {	// 基本块内记录非分隔类型四元式
					base->baseCodes.push_back(&*item);
				}

				if (baseEndSet.count(item->op)) {
					// base block end
					base->gotoLabel = item->dst;
					gotolabel_base[item->dst] = base;
					
					// 除了returnd都有gotolabel
					if (base->gotoLabel == NULL) { // returnOp, no goto label
						base->nextBases.insert(exitBase);
					}
					else if (label_base.count(base->gotoLabel)) {
						// gotolabel已经出现在之前的块中
						label_base[base->gotoLabel]->lastBases.insert(base);
						base->nextBases.insert(label_base[base->gotoLabel]);
					}
					// 前后基本块连接
					if (lastBase != NULL) {	// 如果顺序前置基本块存在，在当前块记录
						base->lastBases.insert(lastBase);
						lastBase->nextBases.insert(base);
					}
					// 更新lastBase
					if (item->op == Operator::returnOp || item->op == Operator::jmp) {
						// 对于无条件跳转指令，顺序前置基本块不存在
						lastBase = NULL;
					}
					else {// 对于setlabel，直接记录lastbase。
						lastBase = base;
					}
					item++;
					break;	// break while (item != funTail && baseSet.count(item->op))
				}// end if (baseEndSet.count(item->op))


				else if (sepSet.count(item->op)) {
					// 前后基本块连接
					if (lastBase != NULL) {	// 如果顺序前置基本块存在，在当前块记录
						base->lastBases.insert(lastBase);
						lastBase->nextBases.insert(base);
					}
					// 更新lastBase
					lastBase = base;
					break;
				}// end if (sepSet.count(item->op))
				item++;
			} // end while


			if (item == funTail) // 可能没有作用
				base->nextBases.insert(exitBase);
		} // end if (baseSet.count(item->op) || baseEndSet.count(item->op))
		// 前面两个循环退出时，item是下一次循环的第一个四元式
		item--;
	} // end for (auto item = funHead; item != funTail; item++)

	// 完成划分基本块，多遍扫描，完成数据流分析。
	dsa(baseList);

	// 构造冲突图
	map<SymbolItem*, ConflictNode*> hashTab;	// 冲突图的索引表
	conflictGraph(baseList, hashTab);

	// 分配寄存器：图着色算法
	graphColor(hashTab);

	// 基本块内寄存器分配
	baseAllocateReg(baseList);
	/*for (auto baseItem = baseList.begin(); baseItem != baseList.end(); baseItem++) {
		baseAllocateReg(*baseItem);
	}*/
}

void GlobalRegs::dsa(list<Base*> &baseList) {
	// 完成划分基本块，多遍扫描，完成数据流分析。
	bool changed = true;
	while (changed) {
		changed = false;
		for (auto item = baseList.begin(); item != baseList.end(); item++) {
			set<SymbolItem*> nextIn;
			set<SymbolItem*> *outVars = &(*item)->outVars;
			set<SymbolItem*> set_union;
			auto lastItem = (*item)->nextBases.begin();
			while (lastItem != (*item)->nextBases.end()) {
				nextIn.insert((*lastItem)->inVars.begin(), (*lastItem)->inVars.end());
				lastItem++;
			}
			set_union.insert(nextIn.begin(), nextIn.end());
			set_union.insert(outVars->begin(), outVars->end());
			if (set_union.size() != outVars->size()) {
				changed = true;
				outVars->insert(nextIn.begin(), nextIn.end());
			}
			set_union.clear();
			set<SymbolItem*> set_diff;
			set_difference(outVars->begin(), outVars->end(), (*item)->defVars.begin(), (*item)->defVars.end(), inserter(set_diff, set_diff.begin()));
			set_union.insert((*item)->useVars.begin(), (*item)->useVars.end());
			set_union.insert(set_diff.begin(), set_diff.end());
			set_union.insert((*item)->inVars.begin(), (*item)->inVars.end());
			if (set_union.size() != (*item)->inVars.size()) {
				changed = true;
				(*item)->inVars.insert(set_union.begin(), set_union.end());
			}// end if (...size() != ...size())
		} // end for (auto item = ...
	} // end while(changed)
}

void GlobalRegs::conflictGraph(list<Base*> &baseList, map<SymbolItem*, ConflictNode*> &hashTab) {
	auto item = baseList.begin();
	while (item != baseList.end()) {
		if (!(*item)->isBase) {
			if (item == baseList.begin()) {
				baseList.remove(*item);
				item = baseList.begin();
			} // end if 
			else {
				auto tmp = next(item, 1);
				baseList.remove(*item);
				item = tmp;
			} // end else 
		} // end if 
		else {
			set<SymbolItem*> in_out;
			in_out.insert((*item)->inVars.begin(), (*item)->inVars.end());
			in_out.insert((*item)->outVars.begin(), (*item)->outVars.end());
			for (auto var_1 = in_out.begin(); var_1 != in_out.end(); var_1++) {
				(*var_1)->isGlobal = true;
				ConflictNode* node_1 = NULL;
				if (hashTab.count(*var_1)) {
					node_1 = hashTab[*var_1];
				}
				else {
					node_1 = new ConflictNode(*var_1);
					hashTab[*var_1] = node_1;
				}

				for (auto var_2 = in_out.begin(); var_2 != in_out.end(); var_2++) {
					if (var_2 == var_1) continue;
					ConflictNode* node_2 = NULL;
					if (hashTab.count(*var_2)) {
						node_2 = hashTab[*var_2];
					}
					else {
						node_2 = new ConflictNode(*var_2);
						hashTab[*var_2] = node_2;
					}
					node_1->link.insert(node_2);
					node_2->link.insert(node_1);
				} // end for var_2
			} // end for var_1
		} // end else 
		if (item != baseList.end())
			item++;
	} // end while
}

void GlobalRegs::graphColor(map<SymbolItem*, ConflictNode*> &hashTab) {
	list<SymbolItem*> varList;	// 冲突图中变量的移除序列
	set<SymbolItem*> outVarSet;	// 冲突图中已移除变量集合
	set<SymbolItem*> allVarSet;	// 冲突图中所有变量集合
	for (auto var = hashTab.begin(); var != hashTab.end(); var++) {
		allVarSet.insert(var->first);
	}
	while (varList.size() != allVarSet.size()) {
		bool found = false;
		for (auto var = hashTab.begin(); var != hashTab.end(); var++) {
			if (!outVarSet.count(var->first) && var->second->link.size() < allRegs.size()) {
				varList.push_front(var->first);
				outVarSet.insert(var->first);
				found = true;
				break;
			}
		}
		if (!found) {
			set<SymbolItem*> diff;
			set_difference(allVarSet.begin(), allVarSet.end(), outVarSet.begin(), outVarSet.end(), inserter(diff, diff.begin()));
			SymbolItem* var = *diff.begin();
			varList.push_front(var);
			outVarSet.insert(var);
			hashTab[var]->isAllocatable = false;
		}
	}
	// all vars out of the conflict graph
	for (auto var = varList.begin(); var != varList.end(); var++) {
		allocateReg(hashTab[*var]);
	}
}

void GlobalRegs::allocateReg(ConflictNode* node) {
	if (node->isAllocatable == false) {
		return;
	}
	set<string> bannedRegs;
	set<string> diff;
	for (auto nodeItem = node->link.begin(); nodeItem != node->link.end(); nodeItem++) {
		if ((*nodeItem)->isAllocatable && (*nodeItem)->var->allocated_reg != "") {
			bannedRegs.insert((*nodeItem)->var->allocated_reg);
		}
	}
	set_difference(allRegs.begin(), allRegs.end(), bannedRegs.begin(), bannedRegs.end(), inserter(diff, diff.begin()));
	node->var->allocated_reg = *diff.begin();
	if (node->var->isPara) {
		node->var->isUsingReg = true;
		node->var->initial = false;
		node->var->saved = false;
	}
}

void GlobalRegs::baseAllocateReg(list<Base*> &baseList) {
	set<SymbolItem*> baseVars;	// 只活跃于基本块内的变量
	set<SymbolItem*> def_use;	// 在基本块内使用定义的
	set<SymbolItem*> in_out;	// 跨基本块活跃的变量
	vector<Quadruples*> codes;
	map<SymbolItem*, Quadruples*> var_code;	// 记录变量出现位置

	// 如果全局变量没有得到寄存器，在基本块内也不分配寄存器
	for (auto base : baseList) {
		in_out.insert(base->inVars.begin(), base->inVars.end());
		in_out.insert(base->outVars.begin(), base->outVars.end());
		
		def_use.insert(base->defVars.begin(), base->defVars.end());
		def_use.insert(base->useVars.begin(), base->useVars.end());
		
		for (auto code : base->baseCodes) {
			codes.push_back(code);
			varCodeRecord(code->dst, code, var_code);
			varCodeRecord(code->src1, code, var_code);
			varCodeRecord(code->src2, code, var_code);
		}
	}
	if (funItem != NULL) {
		for (auto para : funItem->paraItems)
			def_use.insert(para);
	}
	set_difference(def_use.begin(), def_use.end(), in_out.begin(), in_out.end(), inserter(baseVars, baseVars.begin()));
	

	// 计算空闲寄存器
	set<string> bannedRegs;		// 全局变量得到的寄存器
	set<string> freeRegs;		// 在当前基本块空闲的寄存器
	for (auto gVar : in_out) {
		if (gVar->allocated_reg != "") {
			bannedRegs.insert(gVar->allocated_reg);
		}
	}
	set_difference(allRegs.begin(), allRegs.end(), bannedRegs.begin(), bannedRegs.end(), inserter(freeRegs, freeRegs.begin()));

	// 按中间代码顺序分配，最后一次使用后可以释放，引用计数不易释放寄存器
	for (auto codeItem : codes) {
		set<SymbolItem*> var_3;	// 一条中间代码中的变量
		// 必须检查是否是基本块局部变量
		if (baseVars.count(codeItem->dst)) 
			var_3.insert(codeItem->dst);
		if (baseVars.count(codeItem->src1)) 
			var_3.insert(codeItem->src1);
		if (codeItem->src2 != NULL && baseVars.count(codeItem->src2)) 
			var_3.insert(codeItem->src2);
		set<string> releaseRegs;
		for (auto var : var_3) {
			if (var->allocated_reg != "") {
				if (var_code[var] == codeItem)
					releaseRegs.insert(var->allocated_reg);
			}
			else if (!freeRegs.empty()){
				var->allocated_reg = *(freeRegs.begin());
				if (var->isPara) {
					var->isUsingReg = true;
					var->initial = false;
					var->saved = false;
				}
				freeRegs.erase(freeRegs.begin());
			}
		}
		if (!releaseRegs.empty()) {	// 释放寄存器
			freeRegs.insert(releaseRegs.begin(), releaseRegs.end());
		}
	}
	/*for (auto var : funItem->paraItems) {
		if (!freeRegs.empty() && var->allocated_reg == "") {
			var->allocated_reg = *freeRegs.begin();
			freeRegs.erase(freeRegs.begin());
			var->isUsingReg = true;
			var->initial = false;
			var->saved = false;
		}
	}
	*/
/* 可以再比较优化效果，选择一种方法
	// 引用计数排序
	vector<pair<SymbolItem*, int>> orderedBaseVars(ref_count.begin(), ref_count.end());
	sort(orderedBaseVars.begin(), orderedBaseVars.end(), 
		// lambda begin
		[](pair<SymbolItem*, int> a, pair<SymbolItem*, int> b) {
		return a.second < b.second;
		// end of lambda expression
	});

	// local vars 分配寄存器
	for (auto localVar : orderedBaseVars) {
		if (freeRegs.empty())
			break;
		localVar.first->allocated_reg = *(freeRegs.begin());
		freeRegs.erase(freeRegs.begin());
	}
*/
}