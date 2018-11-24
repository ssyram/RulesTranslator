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
using std::cout;
using std::endl;

using ConditionPackage =
    unordered_map<rules_translator::ProductionWithDoc, unordered_set<rules_translator::symbol_type>>;


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
            for (const auto &it: p) {
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
    
//    bool operator==(const ConditionPackage &pl, const ConditionPackage &pr) {
//        if (pl.size() != pr.size())
//            return false;
//
//    }
    
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
        FileInteractor &fi;
        RulesInfo &info;
        
        // first generate support variables
        unordered_set<symbol_type> nullable;
        unordered_map<symbol_type, unordered_set<symbol_type>> first;
//        unordered_map<symbol_type, unordered_set<symbol_type>> follow;
        unordered_map<symbol_type, vector<Production *>> left_map;
        
        unordered_map<ConditionPackage, size_t> package_condition_map;
//        unordered_map<size_t, ConditionPackage *> searchPackageByConditionMap;
        
        
        CounselTable actionTable;
        CounselTable gotoTable;
        
        size_t next_condition_id = 1; // start from 1, for 0 is invalid condition
        
//        size_t getPackageCondition(const ConditionPackage &p) {
//            auto &v = package_condition_map[p];
//            if (v == 0)
//                v = next_condition_id++;
//            return v;
//        }
        
        template <bool pre>
        void outputConditionPackage (const ConditionPackage &p, const size_t condition) {
            if constexpr (pre) cout << "Condition <pre>: {" << endl;
            else cout << "Condition <" << condition << ">: {" <<endl;
            for (const auto &item: p) {
                const auto &pwd = item.first;
                const auto &symset = item.second;
                cout << "\t id: " << pwd.p.productionId << ", " << info.nonterminate2StringMap[pwd.p.left] << ": { ";
                const auto &right = pwd.p.right;
                size_t size = right.size();
                size_t docPos = pwd.docPos;
                for (size_t i = 0; i < size; ++i) {
                    if (i == docPos) cout << "^.";
                    const auto &sym = right[i];
                    if (sym.isTerminate)
                        cout << "\"" << info.terminate2StringMap[sym.type] << "\", ";
                    else
                        cout << info.nonterminate2StringMap[sym.type] << ", ";
                }
                
                cout << (docPos == size ? "^. }; { " : "}; { ");
                for (const auto &sym: symset)
                    cout << info.terminate2StringMap[sym] << ", ";
                cout << "}" << endl;
            }
            cout << "};" << endl;
        }

        void fillZeroProduction() {
            Production p;
            p.productionId = 0;
            p.left = info.nonterminateType_amount;
            p.right.emplace_back(false, info.productions[0].left);
            p.right.emplace_back(true, info.eof);
            info.terminate2StringMap[info.eof] = "$eof$";
            info.productions.emplace(info.productions.begin(), std::move(p));
        }
        void fillSupportVars() {
            //left_map
            {
                symbol_type temp_left = info.nonterminateType_amount + 1;
                vector<Production *> *temp = nullptr;
                for (auto &pro: info.productions) {
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
                for (auto &pro: info.productions) {
                    bool mark = true;
                    for (auto it: pro.right)
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
                for (const auto &pro: info.productions) {
                    const auto &left = pro.left;
                    for (const auto &sym: pro.right)
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
            p[ProductionWithDoc(info.productions[0])];
            const auto &sset = first[info.nonterminateType_amount];
            symbol s(true, *sset.begin());
            for (auto si: sset)
                actionTable[0][si] = 1;
            if constexpr (test) calculateCondition<true>(p, 0, s);
            else calculateCondition(p, 0, s);
        }
        // firstly, trace the package
        // then, if it's already existed, lock the condition and return
        // otherwise, lock the condition and continue to dispatch the condition
        template <bool test = false>
        void calculateCondition(ConditionPackage &p, const size_t pre_condition, const symbol &s) {
            // find out the package
            auto tracePackage = [&p, this] () {
                size_t change = 1;
                // add all the production with the specified left and the content in the set
                auto putSet = [&p, this, &change] (const symbol_type left, const unordered_set<symbol_type> &s) {
//                    const auto &vp = left_map.find(left)->second;
                    for (auto &pro: left_map.find(left)->second) {
                        auto &nset = p[ProductionWithDoc(*pro)]; // this phrase will also create the target
//                        for (auto st: s)
//                            change += nset.insert(st).second;
                        size_t os = nset.size();
                        nset.insert(s.begin(), s.end());
                        change += (nset.size() != os);
                    }
                };
                while (change > 0) {
                    change = 0;
                    for (auto &it: p) {
                        const ProductionWithDoc &pwd = it.first;
                        // if needs reduce
                        if (pwd.end())
                            continue;
//                            for (auto n: it.second) { // n is a number, means the terminate type
//                                if (actionTable[pre_condition][n] != 0)
//                                    throw TranslateException("Reduce Problem.");
//                                actionTable[pre_condition][n] = -(ll)(pwd.p.productionId);
//                            }
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
                                    
    //                                    ts.insert(.begin(),)
    //                                    for (auto st: first[followStringSymbol[i].type])
    //                                        ts.insert(st);
                                    }
                                    // for if this is true, it also means i == size
                                    if (!followStringSymbol[i - 1].isTerminate &&
                                        nullable.find(followStringSymbol[i - 1].type) != nullable.end())
                                        ts.insert(it.second.begin(), it.second.end());
                                    // the same as:
    //                                    for (auto st: it.second)
    //                                        ts.insert(st);
                                }
                                
                                putSet(s.type, ts);
                            }
                        }
                    }
                }
            };
            if constexpr (test) outputConditionPackage<true>(p, 0); // output pre condition
            tracePackage();
//            size_t condition = getPackageCondition(p);
            size_t &condition = package_condition_map[p];
            auto fillShiftGotoAction = [this, &condition, &pre_condition, &s] () {
                ll &v = s.isTerminate ? actionTable[pre_condition][s.type] : gotoTable[pre_condition][s.type];
                if (v && v != condition)
                    generateCollisionException(pre_condition, s, v, condition);
                cout << "From Condition [" << pre_condition << "], shift to Condition [" << condition << "]" << endl;
                v = condition;
            };
            // if already existed
            if (condition) {
                fillShiftGotoAction();
                if constexpr (test) cout << "Get to condition: " << condition << endl;
                return;
            }
            // a new condition
            condition = next_condition_id++;
            if constexpr (test) outputConditionPackage<false>(p, condition);
            fillShiftGotoAction();
            // find where can it get to or reduce
            unordered_set<symbol> count;
            for (const auto &it: p) {
                const auto &pro = it.first;
                // reduce
                if (pro.end()) {
                    ll *line = actionTable[condition];
                    ll targetValue = -((ll)pro.p.productionId);
                    cout << "Reduce in Condition [" << condition << "], id: " << pro.p.productionId << endl;
                    for (auto n: it.second) {
                        ll &v = line[n];
                        if (v && v != targetValue) {
                            generateCollisionException(pre_condition, s, v, targetValue);
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
                for (const auto &it: p)
                    if (!it.first.end() && it.first.getNext() == sym)
                        np[it.first.next()] = it.second;
                if constexpr (test) calculateCondition<true>(np, condition, sym);
                else calculateCondition(np, condition, sym);
            }
        }
        void generateCollisionException(const ll &pre_condition, const symbol &sym, const ll &before, const ll &newCome) {
            cout << "Collision occurs, previous condition: " << pre_condition << endl;
            cout << "Accept symbol: ";
            if (sym.isTerminate)
                cout << "\"" << info.terminate2StringMap[sym.type] << "\"" << endl;
            else
                cout << info.nonterminate2StringMap[sym.type] << endl;
            auto outputProduction = [this] (ll id) {
                const auto &p = info.productions[id];
                cout << "id: " << p.productionId << ", " << info.nonterminate2StringMap[p.left] << ": { ";
                for (const auto &sym: p.right)
                    if (sym.isTerminate)
                        cout << "\"" << info.terminate2StringMap[sym.type] << "\", ";
                    else
                        cout << info.nonterminate2StringMap[sym.type] << ", ";
                cout << "};" << endl;
            };
            if (before > 0)
                cout << "existed Condition <" << before << ">." << endl;
            else
                outputProduction(-before);
            if (newCome > 0)
                cout << "newly come Condition <" << newCome << ">." << endl;
            else
                outputProduction(-newCome);
            throw TranslateException("Collision occurs!");
        }
        template <bool consoleOutput = false>
        void outputResult() {
            stringstream ss;
            auto outputCounselTable = [&ss] (const string &name, CounselTable &table) {
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
//            size_t size = info.productions.size();
//            for (size_t i = 1; i < size; ++i) {
//
//            }
            for (auto &pro: info.productions)
                ss << pro.right.size() << "," << std::endl;
            ss << "};" << std::endl;
            
            // production_left_table
            ss << "constexpr const size_t production_left_table[] = {" << std::endl;
            for (auto &pro: info.productions)
                ss << pro.left << "," << std::endl;
            ss << "};" << std::endl;
            
            // goto_table
            outputCounselTable("goto_table", gotoTable);
            
            // eof
            ss << "constexpr const size_t eof = " << info.eof << ";" << std::endl;
            
            if constexpr (consoleOutput) cout << ss.str() << endl;
            else fi.writeln(ss.str());
        }
    public:
        TableGenerator_Impl(FileInteractor &fi, RulesInfo &info): fi(fi), info(info), actionTable(info.eof + 1), gotoTable(info.nonterminateType_amount) {}
        void generate() {
            fillZeroProduction();
            fillSupportVars();
            startCalculation();
            outputResult();
        }
        void test() {
            fillZeroProduction();
            
            cout << "productions: " << endl;
            for (const auto &p: info.productions) {
                cout << "{ " << p.left << " : { ";
                for (const auto &r: p.right)
                    cout << (r.isTerminate ? "(t)" : "(n)") << r.type << ", ";
                cout << "}, ";
                cout << "id: " << p.productionId << " }" << endl;
            }
            
            fillSupportVars();
            cout << "nullable:\n{ ";
            for (auto &sym: nullable)
                cout << sym << ", ";
            cout << "}" << endl;
            
            cout << "first: {" << endl;
            for (auto &nt: first) {
                cout << nt.first << ": { ";
                for (auto &t: nt.second)
                    cout << t << ", ";
                cout << "}," << endl;
            }
            cout << "};" << endl;
            
            cout << "left_map: {" << endl;
            for (auto &left: left_map) {
                cout << left.first << ": { ";
                for (auto &t: left.second)
                    cout << t->productionId << ", ";
                cout << "}," << endl;
            }
            cout << "};" << endl;
            
            startCalculation<true>();
            
            outputResult();
        }
    };
    
    TableGenerator::TableGenerator(FileInteractor &fi, RulesInfo &info) {
        impl = new TableGenerator_Impl(fi, info);
    }
    TableGenerator::~TableGenerator() {
        delete impl;
    }
    void TableGenerator::generate() {
        impl->generate();
    }
    void TableGenerator::testGenerate() {
        impl->test();
    }
    
}
