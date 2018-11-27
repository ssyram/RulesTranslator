//
//  TableGenerator.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/19.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "TableGenerator.h"
#include "../object/ProductionWithDoc.h"
#include "../util/UtilFunctions.h"
#include "../object/CounselTable.h"
#include <unordered_set>
#include <sstream>
using std::stringstream;
using std::unordered_set;

#include <iostream>
#include <fstream>
using std::endl;

using ConditionPackage =
unordered_map<rules_translator::ProductionWithDoc,
	unordered_set<rules_translator::symbol_type>>;

std::ostream& console_out = std::cout;
#define cout out

namespace std {
	template <>
	class hash<rules_translator::symbol> {
	public:
		size_t operator()(const rules_translator::symbol &s) const {
			return s.type | (((size_t)s.isTerminate) << 63);
		}
	};

	template <>
	class hash<rules_translator::ProductionWithDoc> {
	public:
		size_t operator()(const rules_translator::ProductionWithDoc &p) const {
			//            using namespace rules_translator::utils;
			//            size_t r = p.p.productionId;
			//            r |= shift_change(p.docPos, 8, 56);
			return p.p.productionId | rules_translator::utils::shift_change(p.docPos, 8, 56);
		}
	};

	template <>
	class hash<ConditionPackage> {
	public:
		size_t operator()(const ConditionPackage &p) const {
			// 8 bit for size, 37 bit for multiple, 4 bit for sum of type_number and 15 bit for sum
			using namespace rules_translator::utils;
			size_t r = 0;
			r |= shift_change(p.size(), 8, 0);
			size_t w = 0, j = 0, vj = 0, temp = 0;
			for (const auto &it : p) {
				temp = it.first.p.productionId;
				w += temp;
				j *= temp;
				vj += it.second.size();
			}

			r |= shift_change(j, 37, 8);
			r |= shift_change(vj, 4, 45);
			r |= shift_change(w, 15, 49);

			return r;
		}
	};

	template <>
	class hash<unordered_map<ConditionPackage, size_t>> {
	public:
		size_t operator()(const unordered_map<ConditionPackage, size_t> &o) const {
			return 0;
		}
	};
}


namespace rules_translator {

	class TableGenerator_Impl {

		std::ofstream out; // for file output

		FileInteractor &fi;
		RulesInfo *info;

		// first generate support variables
		unordered_set<symbol_type> nullable;
		unordered_map<symbol_type, unordered_set<symbol_type>> first;
		unordered_map<symbol_type, vector<Production *>> left_map;

		unordered_map<ConditionPackage, size_t> package_condition_map;

		CounselTable actionTable;
		CounselTable gotoTable;

		size_t next_condition_id = 1; // start from 1, for 0 is invalid condition


		template <bool pre>
		void outputConditionPackage(const ConditionPackage& p,
			const size_t new_condition,
			const int pre_condition = -1)
		{
			if constexpr (pre)cout << "Condition [" << pre_condition << "->"
				<< new_condition << "]: {" << endl;
			else cout << "Condition [" << new_condition << "]: {" << endl;

			for (const auto &item : p) {
				const auto &pwd = item.first;
				const auto &symset = item.second;
				cout << "    id: " << pwd.p.productionId << ", " << info->nonterminate2StringMap[pwd.p.left] << ": { ";
				const auto &right = pwd.p.right;
				const size_t size = right.size();
				const size_t docPos = pwd.docPos;

				// output
				for (size_t i = 0; i < size; i++) {
					if (i == docPos) cout << "@ ";
					const auto &sym = right[i];
					if (sym.isTerminate)
						cout << "\"" << info->terminate2StringMap[sym.type] << "\" ";
					else
						cout << info->nonterminate2StringMap[sym.type] << " ";
				}
				cout << (docPos == size ? "@ };   { " : "};   { ");

				for (const auto &sym : symset)
					cout << info->terminate2StringMap[sym] << " ";
				cout << "}" << endl;
			}
			cout << "};" << endl;
		}


		void fillZeroProduction() {
			Production p;
			p.productionId = 0;
			p.left = info->nonterminateType_amount;
			p.right.emplace_back(false, info->productions[0].left);
			p.right.emplace_back(true, info->eof);
			info->terminate2StringMap[info->eof] = "$eof$";
			info->productions.emplace(info->productions.begin(), std::move(p));
		}


		void fillSupportVars() {
			//left_map
			{
				symbol_type temp_left = info->nonterminateType_amount + 1;
				vector<Production *> *temp = nullptr;
				for (auto &pro : info->productions) {
					if (temp_left != pro.left) {
						temp_left = pro.left;
						temp = &left_map[temp_left];
					}
					temp->push_back(&pro);
				}
			}

			// nullable
			int change = 1;
			while (change > 0) {
				change = 0;
				for (auto &pro : info->productions) {
					bool mark = true;
					for (auto it : pro.right)
						if (it.isTerminate || nullable.find(it.type) == nullable.end()) {
							mark = false;
							break;
						}
					if (mark) change += nullable.insert(pro.left).second;
				}
			}

			// first
			change = 1;
			while (change > 0) {
				change = 0;
				for (const auto &pro : info->productions) {
					const auto &left = pro.left;
					for (const auto &sym : pro.right)
						if (sym.isTerminate) {
							change += first[left].insert(sym.type).second;
							break;
						}
						else {
							// take out the first set of the contemporary sym type (nonterminate)
							auto &tt = first[sym.type];
							auto &fl = first[left];
							size_t os = fl.size();
							fl.insert(tt.begin(), tt.end());
							change += (fl.size() != os);
							// if it's not nullable, break loop
							if (nullable.find(sym.type) == nullable.end())
								break;
						}
				}
			}
		}


		template <bool test = false>
		void startCalculation() {
			ConditionPackage p;
			p[ProductionWithDoc(info->productions[0])];
			const auto &sset = first[info->nonterminateType_amount];
			symbol s(true, *sset.begin());
			for (auto si : sset)
				actionTable[0][si] = 1;
			if constexpr (test)calculateCondition<true>(p, 0, s);
			else calculateCondition(p, 0, s);
		}


		// firstly, trace the package
		// then, if it's already existed, lock the condition and return
		// otherwise, lock the condition and continue to dispatch the condition
		template <bool test = false>
		void calculateCondition(ConditionPackage &p, const size_t pre_condition, const symbol &s)
		{
			// find out the package
			auto tracePackage = [&p, this]() {
				size_t change = 1;

				// add all the production with the specified left and the content in the set
				auto putSet = [&p, this, &change](const symbol_type left, const unordered_set<symbol_type> &s) {
					//                    const auto &vp = left_map.find(left)->second;
					for (auto &pro : left_map.find(left)->second) {
						auto &nset = p[ProductionWithDoc(*pro)]; // this phrase will also create the target

						size_t os = nset.size();
						nset.insert(s.begin(), s.end());
						change += (nset.size() != os);
					}
				};

				while (change > 0) {
					change = 0;
					for (auto &it : p) {
						const ProductionWithDoc &pwd = it.first;
						// if needs reduce
						if (pwd.end())
							continue;

						else {
							symbol s = pwd.getNext();
							if (s.isTerminate)
								continue;

							if (pwd.last())
								putSet(s.type, it.second);
							else {
								const auto &followStringSymbol = pwd.getFollowString();
								auto &fs = followStringSymbol[0]; // first symbol in following symbol string
								unordered_set<size_t> ts;
								if (fs.isTerminate)
									ts.insert(fs.type);
								else { // fs is nonterminate
									const auto &temp = first.find(fs.type)->second;
									ts.insert(temp.begin(), temp.end());

									size_t i = 1;
									auto size = followStringSymbol.size();
									for (; !followStringSymbol[i - 1].isTerminate &&
										nullable.find(followStringSymbol[i - 1].type) != nullable.end() && i < size; ++i)
									{
										const auto &f = followStringSymbol[i];
										if (f.isTerminate) {
											ts.insert(f.type);
											break;
										}
										const auto &fir = first[f.type];
										ts.insert(fir.begin(), fir.end());

									}
									// for if this is true, it also means i == size
									if (!followStringSymbol[i - 1].isTerminate &&
										nullable.find(followStringSymbol[i - 1].type) != nullable.end())
										ts.insert(it.second.begin(), it.second.end());
									// the same as:
									//  for (auto st: it.second)
									//     ts.insert(st);
								}

								putSet(s.type, ts);
							}
						}
					}
				}
			};
			tracePackage();
			
			size_t &condition = package_condition_map[p];
			// a new condition
			bool is_condition_already = true;
			if (condition == 0)
			{
				condition = next_condition_id++;
				is_condition_already = false;
			}


			auto fillShiftGotoAction = [this, condition, pre_condition, &s]() {
				ll &v = s.isTerminate ? actionTable[pre_condition][s.type] : gotoTable[pre_condition][s.type];
				// in fact, no collision for GOTO, just for assignment below.
				// just shift-reduce or reduce-reduce collision.
				if (v != 0 && v != condition)
					generateCollisionException(pre_condition, s, v, condition);
				v = condition;

				// shift
				if (s.isTerminate)
					cout << "SHIFT: \"" << info->terminate2StringMap[s.type] << "\" "
					<< pre_condition << "->" << condition << endl;
				// goto
				else
					cout << "GOTO: <" << info->nonterminate2StringMap[s.type] << "> "
					<< pre_condition << "->" << condition << endl;
			};


			cout << "\n-------------------------------------------------------\n" << std::endl;
			// output pre condition
			if constexpr (test)outputConditionPackage<true>(p, condition, pre_condition);
			// output new condition
			if (!is_condition_already) // only output 1 time of new condition
				if constexpr (test)outputConditionPackage<false>(p, condition);
			fillShiftGotoAction();


			// if already existed
			if (is_condition_already)
				return;


			// find where can it get to or reduce
			unordered_set<symbol> count;

			// output REDUCE
			for (auto const&[pro, symbol_type_set] : p)
				if (pro.end())
					cout << "Reduce: " << condition << ", id: " << pro.p.productionId << endl;

			for (auto const&[pro, symbol_type_set] : p)
			{
				// reduce
				if (pro.end())
				{
					ll *line = actionTable[condition];
					const ll targetValue = -((ll)pro.p.productionId);
					for (auto n : symbol_type_set)
					{
						ll &v = line[n];
						if (v && v != targetValue)
						{
							generateCollisionException(condition, symbol(true, n), v, targetValue);
						}
						v = targetValue;
					}
					continue;
				}


				// shift
				const symbol &sym = pro.getNext();
				if (!count.insert(sym).second) // test if it's already gone that way.
					continue;
				ConditionPackage np;
				for (auto const&[pro, symbol_type_set] : p)
					if (!pro.end() && pro.getNext() == sym)
						np[pro.next()] = symbol_type_set;
				if constexpr (test)calculateCondition<true>(np, condition, sym);
				else calculateCondition(np, condition, sym);

			} // end DFS

		} // end function void calculateCondition();


		// REDUCE: condition 
		void generateCollisionException(
			ll condition,
			symbol sym,
			ll shift_cond,
			ll newCome)
		{

			cout << "\n-------------------------------------------------------\n" << std::endl;

			cout << "Collision occurs:" << endl;
			if (sym.isTerminate)
				cout << "SHIFT: \"" << info->terminate2StringMap[sym.type] << "\" "
				<< condition << "->" << shift_cond << endl;
			else
				cout << info->nonterminate2StringMap[sym.type] << endl;

			auto outputProduction = [this, condition](ll id) {
				const auto &p = info->productions[id];
				cout << "Reduce: " << condition << ", id: " << p.productionId << ", "
					<< info->nonterminate2StringMap[p.left] << ": { ";
				for (const auto &sym : p.right)
					if (sym.isTerminate)
						cout << "\"" << info->terminate2StringMap[sym.type] << "\" ";
					else
						cout << info->nonterminate2StringMap[sym.type] << " ";
				cout << "};" << endl;
			};

			if (shift_cond <= 0)
				outputProduction(-shift_cond);

			if (newCome > 0)
				cout << "newly come Condition <" << newCome << ">." << endl;
			else
				outputProduction(-newCome);

			throw TranslateException("Collision occurs!");
		}


		template <bool consoleOutput = false>
		void outputResult() {
			stringstream ss;
			auto outputCounselTable = [&ss](const string &name, CounselTable &table) {
				ss << "constexpr const ll " << name << "[" << table.lineAmount() << "][" << table.columnAmount() << "] = {" << std::endl;
				for (size_t i = 0; i < table.lineAmount(); ++i) {
					ss << "{ ";
					for (size_t j = 0; j < table.columnAmount(); ++j)
						ss << table[i][j] << ", ";
					ss << " }, " << std::endl;
				}
				ss << "};" << std::endl;
			};
			ss << "using ll = long long;";

			// action_table
			outputCounselTable("action_table", actionTable);

			// production_elementAmount_table
			ss << "constexpr const size_t production_elementAmount_table[] = {" << std::endl;

			for (auto &pro : info->productions)
				ss << pro.right.size() << "," << std::endl;
			ss << "};" << std::endl;

			// production_left_table
			ss << "constexpr const size_t production_left_table[] = {" << std::endl;
			for (auto &pro : info->productions)
				ss << pro.left << "," << std::endl;
			ss << "};" << std::endl;

			// goto_table
			outputCounselTable("goto_table", gotoTable);

			// eof
			ss << "constexpr const size_t eof = " << info->eof << ";" << std::endl;

			if constexpr (consoleOutput)cout << ss.str() << endl;
			else fi.writeln(ss.str());
		}


	public:

		TableGenerator_Impl(FileInteractor& fi, RulesInfo* info)
			:
			fi(fi),
			info(info),
			actionTable(info->eof + 1),
			gotoTable(info->nonterminateType_amount)
		{
			const char* out_path = "C:/Users/lenovo/Source/Repos/___RulsTSL/x64/Release/rule_V1.lr1";
			out.open(out_path, std::ios::out | std::ios::trunc);
#undef cout
			if (!out.is_open())
			{
				std::cout << "C:/Users/lenovo/Source/Repos/___RulsTSL/x64/Release/rule_V1.lr1" << std::endl;
				exit(0);
			}

#define cout out
		}


		void generate()
		{
			fillZeroProduction();

			cout << "productions: " << endl;
			for (const auto &p : info->productions) {
				cout << "{ " << p.left << " : { ";
				for (const auto &r : p.right)
					cout << (r.isTerminate ? "(t)" : "(n)") << r.type << ", ";
				cout << "}, ";
				cout << "id: " << p.productionId << " }" << endl;
			}

			fillSupportVars();
			cout << "nullable:\n{ ";
			for (auto &sym : nullable)
				cout << sym << ", ";
			cout << "}" << endl;

			cout << "first: {" << endl;
			for (auto &nt : first) {
				cout << nt.first << ": { ";
				for (auto &t : nt.second)
					cout << t << ", ";
				cout << "}," << endl;
			}
			cout << "};" << endl;

			cout << "left_map: {" << endl;
			for (auto &left : left_map) {
				cout << left.first << ": { ";
				for (auto &t : left.second)
					cout << t->productionId << ", ";
				cout << "}," << endl;
			}
			cout << "};" << endl;

			startCalculation<true>();

			outputResult();
		}


		~TableGenerator_Impl()
		{
			delete info;
			out.close();
		}

	};


	TableGenerator::TableGenerator(FileInteractor &fi, RulesInfo *info) {
		impl = new TableGenerator_Impl(fi, info);
	}
	TableGenerator::~TableGenerator() {
		delete impl;
	}
	void TableGenerator::generate() {
		impl->generate();
	}

}
