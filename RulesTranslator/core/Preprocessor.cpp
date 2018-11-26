//
//  Preprocessor.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "Preprocessor.h"
#include "../util/UtilFunctions.h"
#include "../util/TranslateException.h"
#include <unordered_set>

//#include <iostream>
//using std::cout;
//using std::endl;

using std::unordered_set;
using std::pair;

namespace rules_translator::utils {
    // this class is a wrapper of Production
    // it distinguish each of the Production by its content while class Production
    // distinguish itself by id
    //
    // this is needed
    // because while scanning, there is no such thing as id for each of
    // the Production, and in this time, it should have a means to distinguish each
    // of the Production to prevent the duplication of Productions.
    struct e_production {
        const Production p;
        e_production(const Production p): p(p) {}
        bool operator==(const e_production &ep) const {
            return p.left == ep.p.left && p.right == ep.p.right;
        }
    };
    size_t shift_change(size_t ori, uint8_t length, uint8_t pos) {
        return (ori << (64 - length)) >> (64 - pos);
    }
}
namespace std {
    template<>
    class hash<rules_translator::utils::e_production> {
    public:
        size_t operator()(const rules_translator::utils::e_production &ep) const {
            using namespace rules_translator::utils;
            const auto &rv = ep.p.right;
            auto size = rv.size();
            size_t r = 0;
            r |= shift_change(ep.p.left, 8, 0);
            r |= shift_change(size, 4, 8);
            if (!size) return r;
            for (uint8_t i = 0; i < 5 && i <= size / 2; ++i) {
                r |= shift_change(rv[i].type, 4, 12 + 10 * i);
                r |= rv[i].isTerminate ? 0 : (1 << (16 + 10 * i));
                r |= shift_change(rv[size - i - 1].type, 4, 17 + 10 *i);
                r |= rv[i].isTerminate ? 0 : (1 << (21 + 10 * i));
            }
            return r;
        }
    };
}

namespace rules_translator {
    
    class Preprocessor_Impl {
        using rct = FileInteractor::ReadContentType;
        using rc = FileInteractor::ReadContent;
        FileInteractor &fi;
        RulesInfo &info;
        unordered_map<string, symbol_type> terminate_typeMap;
        unordered_map<string, symbol_type> nonterminate_typeMap;
        
        unordered_set<size_t> with_process_set;
        // this try is to find out the info needed.
//        struct PathSet {
//            string *path;
//            size_t pathLength;
//        };
//        PathSet setOfTerminate = { { "terminate", "=", "enum", "class", }}
//        pair<string, string> getPathValue(string *path, size_t pathLength, size_t value1, size_t value2) {
//            pair<string, string> r;
//            optional<rc> buffer = fi.read();
//            for (size_t i = 0; buffer && i < pathLength; ++i)
//                if (buffer->content == path[i]) {
//                    if (i == value1)
//                        r.first = buffer->content;
//                    else if (i == value2)
//                        r.second = buffer->content;
//                }
//                else generateException(string("Expected content: ") + path[i]);
//
//            if (!buffer)
//                generateException("Unexpected end.");
//
//            return r;
//        }
        constexpr const static rct path_terminate[] = { rct::word_terminate, rct::equal, rct::word_enum, rct::word_class };
        constexpr const static rct path_token_type[] = { rct::semicolon, rct::word_token_type, rct::equal };
        constexpr const static rct path_get_type[] = { rct::semicolon, rct::word_get_type, rct::equal };
        void getAlongPath(const rct *path, size_t pathLength, optional<rc> &buffer) {
            for (size_t i = 0; i < pathLength && buffer; ++i, buffer = fi.read())
                if (buffer->type != path[i])
                    generateException(string("Expected content: ") + utils::mapType2String.find(path[i])->second);
            
            if (!buffer) generateException("Unexpected end.");
        }
        void generateException(const string &s) {
            throw TranslateException(s);
        }
        
        // append type is the last one, to avoid being set in the table // 0 is the appended type
        symbol_type nt_next_id = 0;
        symbol_type getNonterminateType(const string &s) {
            const auto &it = nonterminate_typeMap.find(s);
            if (it != nonterminate_typeMap.end())
                return it->second;
            nonterminate_typeMap[s] = nt_next_id;
            info.nonterminateType_CppClassName_map[nt_next_id] = "default_object_type";
            return nt_next_id++;
        }
        // main processes
        void fillNameMetaData() {
            // get terminate info
            optional<rc> buffer = fi.read();
            getAlongPath(path_terminate, 4, buffer);
            if (buffer.value().type != rct::word)
                generateException("Expected a class name of enum class.");
            string enum_class_name = buffer->content;
            buffer = fi.read();
//            optional<rc> buffer = fi.read();
//            if (buffer.value().type != rct::word) generateException("Expected a word to specify terminate class name");
//            info.terminate_class_name = std::move(buffer->content);
//            buffer = fi.read();
            if (buffer.value().type != rct::block)
                generateException("Expected a class body to specify all the terminate symbols");
            string &s = buffer->content;
            // remove comment from
            {
                // remove line comment
                for (size_t b = 0; (b = s.find("//", b)) != string::npos; ++b) {
                    size_t del = b;
                    while (del < s.length() && s[del] != '\n' && s[del] != '\r') ++del;
                    s.erase(s.begin() + b, s.begin() + del + 1); // for that del will not be string::npos which is the largest size_t, unlike the following one it will be ok to exceed the boundery
                }
                for (size_t b = 0; (b = s.find("/*", b)) != string::npos; ++b) {
                    size_t del = s.find("*/");
                    if (del == string::npos)
                        s.erase(s.begin() + b, s.end());
                    else
                        s.erase(s.begin() + b, s.begin() + del + 2);
                }
            }
            auto tempv = utils::split<false>(utils::trimDivider(s), ","); // split and not allow empty
            for (size_t i = 0; i < tempv.size(); ++i) {
                if (terminate_typeMap.find(tempv[i]) != terminate_typeMap.end())
                    generateException("No duplicated type");
                terminate_typeMap[tempv[i]] = i;
            }
            info.eof = tempv.size();
            
            // get token_type
            buffer = fi.read();
            getAlongPath(path_token_type, 3, buffer);
            if (buffer.value().type != rct::word) generateException("Expected a word to specify the type of token");
            fi.write("using token_type = ").write(buffer->content).writeln(";");
//            info.token_type = std::move(buffer->content);
            
            // get get_type
            buffer = fi.read();
            getAlongPath(path_get_type, 3, buffer);
            if (buffer.value().type != rct::word)
                generateException("Expected a word to specify the name of the function used to get type");
            fi.write(enum_class_name).write(" (*get_type)(const token_type &) = &").write(buffer->content).writeln(";");
//            info.get_type = std::move(buffer->content);
            fi.read();
        }
        // the return type is the first nonterminate
        optional<rc> fillBindingList() {
            optional<rc> buffer = fi.read(); // used to generate object type
            unordered_set<string> objectTypes;
            while (buffer.value().type == rct::word_using) {
                buffer = fi.read();
                if (buffer.value().type != rct::word)
                    generateException("Expected a word after keyword using to specify name of a nonterminate symbol");
                string &s = buffer->content;
                if (nonterminate_typeMap.find(s) != nonterminate_typeMap.end())
                    generateException(string("This nonterminate type of name '") + s + "' has been signed.");
                nonterminate_typeMap[std::move(s)] = nt_next_id;
                buffer = fi.read();
                if (buffer.value().type != rct::equal) generateException("Expected symbol '='");
                buffer = fi.read();
                if (buffer.value().type != rct::word)
                    generateException("Expected a word represented a class name in C++ language");
                objectTypes.insert(buffer->content);
                info.nonterminateType_CppClassName_map[nt_next_id++] = std::move(buffer->content);
                buffer = fi.read();
                if (buffer.value().type != rct::semicolon)
                    generateException("Expected a semicolon after a using statement");
                buffer = fi.read();
            }
//            if (buffer.value().type != rct::word || terminate_typeMap.find(buffer->content) != terminate_typeMap.end())
//                generateException("Expected a kind of nonterminate symbol");
//
//            s.type = getNonterminateType(buffer->content);
            
            // generate codes
            fi.writeln("class default_object_type {};");
            fi.write("using object_type = std::variant<");
            for (auto &s: objectTypes)
                fi.write(s).write(", ");
            fi.write("default_object_type, token_type>;");
//            fi.write(info.token_type).writeln(">;");
            fi.writeln("template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };");
            fi.writeln("template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;");
            
            return buffer;
        }
        void generateProductions(optional<rc> &&buffer) {
            size_t next_production_id = 1; // start from 1, because 0 is the default production
            size_t temp_left = -1;
            unordered_set<utils::e_production> eps; // to distinguish each of the production
            while (buffer) {
                Production p;
                try {
                    while (buffer.value().type == rct::semicolon)
                        buffer = fi.read();
                }
                catch (std::bad_optional_access &) {
                    break;
                }
                if (buffer.value().type == rct::orSymbol) {
                    if (temp_left == -1) generateException("Not a valid Or Symbol.");
                    p.left = temp_left;
                }
                else {
                    if (buffer.value().type != rct::word ||
                        terminate_typeMap.find(buffer->content) != terminate_typeMap.end())
                        generateException("Expected a kind of nonterminate symbol");
                    p.left = getNonterminateType(buffer->content);
                    temp_left = p.left;
                    buffer = fi.read();
                    if (buffer.value().type != rct::productionEqual)
                        generateException("Expected a ':='");
                }
                buffer = fi.read();
                auto temp_f = terminate_typeMap.begin();
                // tackle production right
                while (buffer && (buffer->type == rct::terminate || buffer->type == rct::word)) {
                    if (buffer->type == rct::terminate)
                        if ((temp_f = terminate_typeMap.find(buffer->content)) == terminate_typeMap.end())
                            generateException("No such terminate type!");
                        else
                            p.right.emplace_back(true, temp_f->second);
                    else {
                        if (terminate_typeMap.find(buffer->content) != terminate_typeMap.end())
                            p.right.emplace_back(true, terminate_typeMap.find(buffer->content)->second);
                        else
                            p.right.emplace_back(false, getNonterminateType(buffer->content));
                    }
                    buffer = fi.read();
                }
                p.productionId = next_production_id;
                // block tackle
                if (buffer && buffer.value().type == rct::block) {
                    do { // give a break point
                        // change all $<num> and $$ to formal name and return what the <num>s are
                        // $<num> will be changed to __c<num>
                        // $$ will be changed to __r
                        
                        // an error in using iterator
    //                    auto modifyBlock = [] (string &s) -> unordered_set<size_t> {
    //                        unordered_set<size_t> nums;
    //                        for (auto it = s.begin(); it != s.end(); ++it) {
    //                            if (*it == '$' && it + 1 != s.end()) {
    //                                if (it[1] == '$') {
    //                                    s.replace(it, it + 2, "__r");
    //                                }
    //                                else if (utils::isNumber(it[1])) {
    //                                    s.replace(it, it + 1, "__c");
    //                                    cout << *it << it[1] << it[2] << it[3] << endl;
    //                                    it += 3;
    //                                    size_t temp = *it - '0';
    //                                    while (utils::isNumber(it[1])) {
    //                                        ++it;
    //                                        temp = temp * 10 + *it - '0';
    //                                    }
    //                                    nums.insert(temp);
    //                                }
    //                            }
    //                        }
    //                        return nums;
    //                    };
                        if (utils::trimMeaninglessDividers(buffer->content).empty()) break;
                        auto modifyBlock = [] (string &s) -> unordered_set<size_t> {
                            unordered_set<size_t> nums;
                            for (size_t i = 0; i < s.length(); ++i) {
                                if (s[i] == '$' && i + 1 < s.length()) {
                                    char c = s[i + 1];
                                    if (c == '$') {
                                        s.replace(s.begin() + i, s.begin() + i + 2, "__r");
                                        i += 2;
                                    }
                                    else if (c >= '1' && c <= '9') {
                                        s.replace(s.begin() + i, s.begin() + i + 1, "__c");
                                        // cout << *it << it[1] << it[2] << it[3] << endl;
                                        i += 3;
                                        size_t temp = c - '0';
                                        while (i + 1 < s.length() && utils::isNumber(s[i + 1])) {
                                            ++i;
                                            temp = temp * 10 + s[i] - '0';
                                        }
                                        nums.insert(temp);
                                    }
                                }
                            }
                            return nums;
                        };
                        auto numset = modifyBlock(buffer->content);
                        char title[128] = {0};
                        p.productionId = next_production_id;
                        sprintf(title, "object_type __process_%lu(object_type content[]) {", next_production_id);
                        
                        fi.writeln(title);
    //                    string &ts = info.nonterminateType_CppClassName_map.find(p.left)->second;
                        const auto &it = info.nonterminateType_CppClassName_map.find(p.left);
                        const string &ts = it == info.nonterminateType_CppClassName_map.end() ? "default_object_type" : it->second;
                        fi.write("object_type r = ").write(ts).writeln("{};");
                        fi.writeln("std::visit(overloaded {");
                        fi.write("[] (");
                        // output all parameter list corresponding to the specified position
                        sprintf(title, "__c");
                        for (auto i: numset) {
                            if (i > p.right.size())
                                generateException("Block content out of range.");
                            symbol &s = p.right[i - 1];
                            if (s.isTerminate)
    //                            fi.write(info.token_type).write(" &&");
                                fi.write("token_type &");
                            else {
                                const auto &it = info.nonterminateType_CppClassName_map.find(s.type);
                                if (it == info.nonterminateType_CppClassName_map.end())
                                    fi.write("default_object_type &");
                                else
                                    fi.write(it->second).write(" &");
                            }
                            sprintf(title + 3, "%lu", i);
                            fi.write(title).write(", ");
                        }
                        
                        fi.write(ts).writeln(" &__r) {");
                        fi.writeln(buffer->content);
                        fi.write("}\n,[] (");
                        for (size_t i = 0; i < numset.size(); ++i)
                            fi.write("auto &, ");
                        fi.write("auto &) { assert(false); },\n}, ");
                        sprintf(title, "content[");
                        for (auto i: numset) {
                            sprintf(title + 8, "%lu], ", i - 1);
                            fi.write(title);
                        }
                        fi.writeln("r);\n return r;");
                        fi.writeln("}");
                        with_process_set.emplace(next_production_id);
                    } while (false);
                    buffer = fi.read();
                }
                info.productions.emplace_back(std::move(p));
                // hold reference in the set, almost zero construct cost
                if (!eps.emplace(info.productions[next_production_id - 1]).second)
                    generateException("This production has already existed.");
                ++next_production_id;
            }
            fi.writeln("const std::unordered_map<size_t, std::function<object_type (object_type[])>> pf_map = {");
            char item[50];
            for (auto &pss: with_process_set) {
                sprintf(item, "{ %ld, &__process_%ld },", pss, pss);
                fi.write(item);
            }
            fi.writeln("\n};");
        }
    public:
        Preprocessor_Impl(FileInteractor &fi): fi(fi), info(*(new RulesInfo)) {}
        RulesInfo *generateInfo() {
            fillNameMetaData();
            generateProductions(fillBindingList());
            info.nonterminateType_amount = nt_next_id;
            for (auto p: terminate_typeMap)
                info.terminate2StringMap.insert(std::make_pair(p.second, p.first));
            for (auto p: nonterminate_typeMap)
                info.nonterminate2StringMap.insert(std::make_pair(p.second, p.first));
            
            return &info;
        }
    };
    
    Preprocessor::Preprocessor(FileInteractor &fi) {
        impl = new Preprocessor_Impl(fi);
    }
    Preprocessor::~Preprocessor() {
        delete impl;
    }
    RulesInfo *Preprocessor::generateInfo() {
        return impl->generateInfo();
    }
}
