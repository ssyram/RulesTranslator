//
//  TargetFormat.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/18.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef TargetFormat_h
#define TargetFormat_h

// here is prepare
// include lines are not printed automatically
#include <variant>
#include <optional>
#include <unordered_map>
#include <functional>
#include <string>
#include <stack>

using type1 = int;
using type2 = double;
using type3 = unsigned long long;
using token_type = unsigned int;
enum class tmn {
    a, b
};
tmn get_type(const token_type &t) {
    return (tmn)t;
}

// this file describes how the final final part will look like


// Preprocessor
class default_object_type {};
using object_type = std::variant<type1, type2, type3, default_object_type, token_type>;

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

// the part of process wrapper

// assume that the target formula is:
// a := "1" b "2" { $$ = $1 + $2 + $3 }
// the corresponding type is:
// type1 := token_type type2 token_type { $$ = $1 + $2 + $3 }
object_type __process_1(object_type content[]) {
    object_type r = type1{};
    
    std::visit(overloaded {
        [] (token_type &&__c1, type2 &&__c2, token_type &&__c3, type1 &__r) {
            __r = __c1 + __c2 + __c3;
        },
        [] (auto &&, auto &&, auto &&, auto &) { assert(false); },
    }, std::move(content[0]), std::move(content[1]), std::move(content[2]), r);
    return r;
}

// ... and the following will be like above

// the function table, map: production_num-corresponding_function_pointer map
// start from 1, 0 means default
const std::unordered_map<size_t, std::function<object_type (object_type[])>> pf_map = {
    { 1, &__process_1},
};
// the table of action, action type(minus or not) and action number(the pure value)
// position: condition position
// not minus: SHIFT
// minus: REDUCE

//class action_table {
//    static constexpr const ll raw_actiontable[] = {
//        1, /* this means shift to condition 1 */
//        -1, /* this is reduce by production 1 */
//    };
//public:
//    class line_object {
//        size_t line;
//    public:
//        line_object(size_t line): line(line) {}
//        line_object(const line_object &object): line(object.line) {}
//        line_object(line_object&& object): line(object.line) {}
//        ll operator[](size_t column) {
//            return raw_actiontable[line * 0 + column];
//        }
//    };
//    line_object operator[](size_t line) {
//        return line_object(line);
//    }
//};
//constexpr const ll **generateActionTable(ll **r) {
//    for (size_t i = 0; i < 2; ++i)
//        r[i] = raw_actiontable + i;
//    return r;
//}
//constexpr const ll **action_table = generateActionTable(action_table);
//class goto_table {
//    static constexpr const ll raw_goto_table[] = {
//        1, 2
//    };
//public:
//    class line_object {
//        size_t line;
//    public:
//        line_object(size_t line): line(line) {}
//        line_object(const line_object &object): line(object.line) {}
//        line_object(line_object&& object): line(object.line) {}
//        ll operator[](size_t column) {
//            return raw_goto_table[line * 0 + column];
//        }
//    };
//    line_object operator[](size_t line) {
//        return line_object(line);
//    }
//};

// TableGenerator
// tables
using ll = long long;
// action table, record the target type and information
// action_table[condition][new_terminate_type] = info
// if (info > 0) info = new_condition
// else if (info < 0) info = target_production_num
// else throw("not valid type.");
constexpr const ll action_table[2][1] = {
    { 1, /* this means shift to condition 1 */ },
    { -1, /* this is reduce by production 1 */ },
};
// record how many elements are there in a specific production
// production_elementAmount_table[production_num] = element amount in given production_num
constexpr const size_t production_elementAmount_table[] = {
    0, // for production dose not exist, this can be anything
    3, // the first production has 3 elements
};
// record the left in each of the production
// production_left_table[production_num] = nonterminate_type
constexpr const size_t production_left_table[] = {
    0,
    2
};
// record the target condition
// goto_table[condition][new_nonterminate_type] = new_condition
constexpr const ll goto_table[2][1] = {
    {1},
    {2}
};
constexpr const size_t eof = 10; // this shows the value of eof

// AnalyzerGenerator
// the target syntactic analyzer
class SyntacticAnalyzer {
//    using symbol_type = long long; // minus is terminate while the opposite is nonterminate
    using condition_of_analysis = size_t;
    struct analysis_symbol {
        condition_of_analysis condition;
        object_type object;
        analysis_symbol() = default;
        analysis_symbol(condition_of_analysis c, object_type &o): condition(c), object(o) {}
        analysis_symbol(condition_of_analysis c, object_type &&o): condition(c), object(o) {}
//        analysis_symbol(object_type &&o): object(o) {}
    };
    std::stack<analysis_symbol> astack; // stands for analysis stack
    template <bool finish>
    void reduce(ll na, const token_type &t) {
        size_t length = production_elementAmount_table[na];
        object_type content[length];
//        content[length - 1] = t;
        for (size_t i = 0; i < length; ++i) {
            content[length - 1 - i] = std::move(astack.top().object);
            astack.pop();
        }
//        symbol_type type = (symbol_type)production_left_table[na];
        size_t condition = goto_table[astack.top().condition][production_left_table[na]];
        if (!condition)
            throw std::make_pair(t, "condition error");
        const auto &pf = pf_map.find(na);
        if (pf == pf_map.end())
            astack.emplace(condition, default_object_type());
        else
            astack.emplace(condition, pf->second(content));
        analyze<finish>(t);
    }
public:
    SyntacticAnalyzer() { astack.emplace(0, default_object_type()); }
    template <bool finish = false>
    void analyze(const token_type &t) {
        ll symtype;
        if constexpr (finish)
            symtype = eof; // this should be determined by eof type
        else symtype = (ll)get_type(t);
        ll nextAction = action_table[astack.top().condition][symtype];
        if (nextAction > 0)
            // shift
            astack.emplace((condition_of_analysis)nextAction, t);
        else if (nextAction < 0)
            reduce<finish>(-nextAction, t);
        else {
            if constexpr (finish) throw std::make_pair(t, "Final Position Error");
            else throw std::make_pair(t, "abc");
        }
    }
//    void finish(const token_type &t) {
//        ll na = action_table[astack.top().condition][10]; // the last number is generated as the eof type
//        if (na == 0)
//            throw std::make_pair(t, "abc");
//    }
};

template <typename T>
std::optional<std::pair<token_type, std::string>> analyze(const T &token_stream) {
    if (!token_stream.length())
        return std::nullopt;
    SyntacticAnalyzer analyzer;
    try {
        for (size_t i = 0; i < token_stream.length(); ++i)
            analyzer.analyze(token_stream[i]);
        analyzer.analyze<true>(token_stream[token_stream.length() - 1]);
    }
    catch (std::pair<token_type, std::string> &e) {
        return e;
    }
    return std::nullopt;
}



#endif /* TargetFormat_h */
