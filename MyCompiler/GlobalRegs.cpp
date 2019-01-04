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
		// �Ȼ��ֺ�����
		while (item!=midCodes.end() && !isFunHead(&*item)) {
			item++;
		}
		if (item->dst->lab == labelType::fun_lb || item->dst->lab == labelType::main_lb) {
			funItem = item->dst->funItem;
		}
		if (item != midCodes.end()) { // һ�������˺�����
			item++;
			if (item->op == Operator::st$ra) {	// ���������ڵ�һ����$raָ��
				item++;
			}
			auto funHead = item;
			while (item != midCodes.end() && !isFunHead(&*item)) {
				item++;
			}
			auto funTail = item; // prev(item, 1);
			if (funHead != midCodes.end() && funTail - funHead >= 0) {
				// ����Ƿ�����Ч�ĺ���ͷβ��
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
	Base* lastBase = entrance;	// ˳��ǰ�û�����
	map<SymbolItem*, Base*> label_base;		// setlabel��
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
					// ��ָ����ת�����label
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
				if (!sepSet.count(item->op)) {	// �������ڼ�¼�Ƿָ�������Ԫʽ
					base->baseCodes.push_back(&*item);
				}

				if (baseEndSet.count(item->op)) {
					// base block end
					base->gotoLabel = item->dst;
					gotolabel_base[item->dst] = base;
					
					// ����returnd����gotolabel
					if (base->gotoLabel == NULL) { // returnOp, no goto label
						base->nextBases.insert(exitBase);
					}
					else if (label_base.count(base->gotoLabel)) {
						// gotolabel�Ѿ�������֮ǰ�Ŀ���
						label_base[base->gotoLabel]->lastBases.insert(base);
						base->nextBases.insert(label_base[base->gotoLabel]);
					}
					// ǰ�����������
					if (lastBase != NULL) {	// ���˳��ǰ�û�������ڣ��ڵ�ǰ���¼
						base->lastBases.insert(lastBase);
						lastBase->nextBases.insert(base);
					}
					// ����lastBase
					if (item->op == Operator::returnOp || item->op == Operator::jmp) {
						// ������������תָ�˳��ǰ�û����鲻����
						lastBase = NULL;
					}
					else {// ����setlabel��ֱ�Ӽ�¼lastbase��
						lastBase = base;
					}
					item++;
					break;	// break while (item != funTail && baseSet.count(item->op))
				}// end if (baseEndSet.count(item->op))


				else if (sepSet.count(item->op)) {
					// ǰ�����������
					if (lastBase != NULL) {	// ���˳��ǰ�û�������ڣ��ڵ�ǰ���¼
						base->lastBases.insert(lastBase);
						lastBase->nextBases.insert(base);
					}
					// ����lastBase
					lastBase = base;
					break;
				}// end if (sepSet.count(item->op))
				item++;
			} // end while


			if (item == funTail) // ����û������
				base->nextBases.insert(exitBase);
		} // end if (baseSet.count(item->op) || baseEndSet.count(item->op))
		// ǰ������ѭ���˳�ʱ��item����һ��ѭ���ĵ�һ����Ԫʽ
		item--;
	} // end for (auto item = funHead; item != funTail; item++)

	// ��ɻ��ֻ����飬���ɨ�裬���������������
	dsa(baseList);

	// �����ͻͼ
	map<SymbolItem*, ConflictNode*> hashTab;	// ��ͻͼ��������
	conflictGraph(baseList, hashTab);

	// ����Ĵ�����ͼ��ɫ�㷨
	graphColor(hashTab);

	// �������ڼĴ�������
	baseAllocateReg(baseList);
	/*for (auto baseItem = baseList.begin(); baseItem != baseList.end(); baseItem++) {
		baseAllocateReg(*baseItem);
	}*/
}

void GlobalRegs::dsa(list<Base*> &baseList) {
	// ��ɻ��ֻ����飬���ɨ�裬���������������
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
	list<SymbolItem*> varList;	// ��ͻͼ�б������Ƴ�����
	set<SymbolItem*> outVarSet;	// ��ͻͼ�����Ƴ���������
	set<SymbolItem*> allVarSet;	// ��ͻͼ�����б�������
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
	set<SymbolItem*> baseVars;	// ֻ��Ծ�ڻ������ڵı���
	set<SymbolItem*> def_use;	// �ڻ�������ʹ�ö����
	set<SymbolItem*> in_out;	// ��������Ծ�ı���
	vector<Quadruples*> codes;
	map<SymbolItem*, Quadruples*> var_code;	// ��¼��������λ��

	// ���ȫ�ֱ���û�еõ��Ĵ������ڻ�������Ҳ������Ĵ���
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
	

	// ������мĴ���
	set<string> bannedRegs;		// ȫ�ֱ����õ��ļĴ���
	set<string> freeRegs;		// �ڵ�ǰ��������еļĴ���
	for (auto gVar : in_out) {
		if (gVar->allocated_reg != "") {
			bannedRegs.insert(gVar->allocated_reg);
		}
	}
	set_difference(allRegs.begin(), allRegs.end(), bannedRegs.begin(), bannedRegs.end(), inserter(freeRegs, freeRegs.begin()));

	// ���м����˳����䣬���һ��ʹ�ú�����ͷţ����ü��������ͷżĴ���
	for (auto codeItem : codes) {
		set<SymbolItem*> var_3;	// һ���м�����еı���
		// �������Ƿ��ǻ�����ֲ�����
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
		if (!releaseRegs.empty()) {	// �ͷżĴ���
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
/* �����ٱȽ��Ż�Ч����ѡ��һ�ַ���
	// ���ü�������
	vector<pair<SymbolItem*, int>> orderedBaseVars(ref_count.begin(), ref_count.end());
	sort(orderedBaseVars.begin(), orderedBaseVars.end(), 
		// lambda begin
		[](pair<SymbolItem*, int> a, pair<SymbolItem*, int> b) {
		return a.second < b.second;
		// end of lambda expression
	});

	// local vars ����Ĵ���
	for (auto localVar : orderedBaseVars) {
		if (freeRegs.empty())
			break;
		localVar.first->allocated_reg = *(freeRegs.begin());
		freeRegs.erase(freeRegs.begin());
	}
*/
}