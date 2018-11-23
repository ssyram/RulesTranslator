//
//  TestSample.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/23.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "TestSample.h"

#include <stdio.h>
#include <iostream>
#include "../extern/rules_translator.h"

// tmn只是一个名称，这个名称可以任意自定义，也就是这个类名可以变换为任意字符串。
enum class tmn {
    add, sub, mul, div, lbkt, rbkt, num
};
// 必须要给我一个类型获取函数，用以映射从token到终结符类型
tmn getType(const char &c) {
    switch (c) {
        case '+':
            return tmn::add;
        case '-':
            return tmn::sub;
        case '*':
            return tmn::mul;
        case '/':
            return tmn::div;
        case '(':
            return tmn::lbkt;
        case ')':
            return tmn::rbkt;
        default:
            return tmn::num;
    }
}
// 如果$<num>指代的内容是一个终结符，则会将token_t本身替换它。

// 这里必须要声明下面会用到的getValue函数，但是可以不实现它。
double getValue(const char &c);

struct expr {
    double val;
};

// 最好放在一个明明空间中，就不会发生命名冲突
namespace calculator_stuff {
    using token_type = char;
    tmn (*get_type)(const token_type &) = &getType;
    class default_object_type {};
    using object_type = std::variant<expr, default_object_type, token_type>;template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
    object_type __process_1(object_type content[]) {
        object_type r = default_object_type{};
        std::visit(overloaded {
            [] (expr &&__c1, default_object_type &__r) {
                printf("%lf\n", __c1.val);
            }
            ,[] (auto &&, auto &) { assert(false); },
        }, std::move(content[0]), r);
        return r;
    }
    object_type __process_2(object_type content[]) {
        object_type r = expr{};
        std::visit(overloaded {
            [] (expr &&__c3, expr &&__c1, expr &__r) {
                __r.val = __c1.val + __c3.val;
            }
            ,[] (auto &&, auto &&, auto &) { assert(false); },
        }, std::move(content[2]), std::move(content[0]), r);
        return r;
    }
    object_type __process_3(object_type content[]) {
        object_type r = expr{};
        std::visit(overloaded {
            [] (expr &&__c3, expr &&__c1, expr &__r) {
                __r.val = __c1.val - __c3.val;
            }
            ,[] (auto &&, auto &&, auto &) { assert(false); },
        }, std::move(content[2]), std::move(content[0]), r);
        return r;
    }
    object_type __process_4(object_type content[]) {
        object_type r = expr{};
        std::visit(overloaded {
            [] (expr &&__c1, expr &__r) {
                __r.val = __c1.val;
            }
            ,[] (auto &&, auto &) { assert(false); },
        }, std::move(content[0]), r);
        return r;
    }
    object_type __process_5(object_type content[]) {
        object_type r = expr{};
        std::visit(overloaded {
            [] (expr &&__c3, expr &&__c1, expr &__r) {
                __r.val = __c1.val * __c3.val;
            }
            ,[] (auto &&, auto &&, auto &) { assert(false); },
        }, std::move(content[2]), std::move(content[0]), r);
        return r;
    }
    object_type __process_6(object_type content[]) {
        object_type r = expr{};
        std::visit(overloaded {
            [] (expr &&__c3, expr &&__c1, expr &__r) {
                __r.val = __c1.val / __c3.val;
            }
            ,[] (auto &&, auto &&, auto &) { assert(false); },
        }, std::move(content[2]), std::move(content[0]), r);
        return r;
    }
    object_type __process_7(object_type content[]) {
        object_type r = expr{};
        std::visit(overloaded {
            [] (expr &&__c1, expr &__r) {
                __r.val = __c1.val;
            }
            ,[] (auto &&, auto &) { assert(false); },
        }, std::move(content[0]), r);
        return r;
    }
    object_type __process_8(object_type content[]) {
        object_type r = expr{};
        std::visit(overloaded {
            [] (expr &&__c2, expr &__r) {
                __r.val = __c2.val;
            }
            ,[] (auto &&, auto &) { assert(false); },
        }, std::move(content[1]), r);
        return r;
    }
    object_type __process_9(object_type content[]) {
        object_type r = expr{};
        std::visit(overloaded {
            [] (token_type &&__c1, expr &__r) {
                __r.val = getValue(__c1);
            }
            ,[] (auto &&, auto &) { assert(false); },
        }, std::move(content[0]), r);
        return r;
    }
    const std::unordered_map<size_t, std::function<object_type (object_type[])>> pf_map = {
        { 9, &__process_9 },{ 8, &__process_8 },{ 7, &__process_7 },{ 6, &__process_6 },{ 5, &__process_5 },{ 4, &__process_4 },{ 3, &__process_3 },{ 2, &__process_2 },{ 1, &__process_1 },
    };
    using ll = long long;constexpr const ll action_table[33][8] = {
        { 0, 0, 0, 0, 1, 0, 1, 0,  },
        { 0, 0, 0, 0, 3, 0, 2, 0,  },
        { -9, -9, -9, -9, 0, 0, 0, -9,  },
        { 0, 0, 0, 0, 5, 0, 4, 0,  },
        { -9, -9, -9, -9, 0, -9, 0, 0,  },
        { 0, 0, 0, 0, 5, 0, 4, 0,  },
        { -7, -7, -7, -7, 0, -7, 0, 0,  },
        { -4, -4, 8, 10, 0, -4, 0, 0,  },
        { 0, 0, 0, 0, 5, 0, 4, 0,  },
        { -5, -5, -5, -5, 0, -5, 0, 0,  },
        { 0, 0, 0, 0, 5, 0, 4, 0,  },
        { -6, -6, -6, -6, 0, -6, 0, 0,  },
        { 14, 16, 0, 0, 0, 13, 0, 0,  },
        { -8, -8, -8, -8, 0, -8, 0, 0,  },
        { 0, 0, 0, 0, 5, 0, 4, 0,  },
        { -2, -2, 8, 10, 0, -2, 0, 0,  },
        { 0, 0, 0, 0, 5, 0, 4, 0,  },
        { -3, -3, 8, 10, 0, -3, 0, 0,  },
        { 14, 16, 0, 0, 0, 19, 0, 0,  },
        { -8, -8, -8, -8, 0, 0, 0, -8,  },
        { -7, -7, -7, -7, 0, 0, 0, -7,  },
        { -4, -4, 22, 24, 0, 0, 0, -4,  },
        { 0, 0, 0, 0, 3, 0, 2, 0,  },
        { -5, -5, -5, -5, 0, 0, 0, -5,  },
        { 0, 0, 0, 0, 3, 0, 2, 0,  },
        { -6, -6, -6, -6, 0, 0, 0, -6,  },
        { 27, 29, 0, 0, 0, 0, 0, -1,  },
        { 0, 0, 0, 0, 3, 0, 2, 0,  },
        { -2, -2, 22, 24, 0, 0, 0, -2,  },
        { 0, 0, 0, 0, 3, 0, 2, 0,  },
        { -3, -3, 22, 24, 0, 0, 0, -3,  },
        { 0, 0, 0, 0, 0, 0, 0, 32,  },
        { 0, 0, 0, 0, 0, 0, 0, 0,  },
    };
    constexpr const size_t production_elementAmount_table[] = {
        2,
        1,
        3,
        3,
        1,
        3,
        3,
        1,
        3,
        1,
    };
    constexpr const size_t production_left_table[] = {
        4,
        3,
        0,
        0,
        0,
        1,
        1,
        1,
        2,
        2,
    };
    constexpr const ll goto_table[30][4] = {
        { 0, 0, 0, 0,  },
        { 26, 21, 20, 31,  },
        { 0, 0, 0, 0,  },
        { 18, 7, 6, 0,  },
        { 0, 0, 0, 0,  },
        { 12, 7, 6, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 9, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 11, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 15, 6, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 17, 6, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 23, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 25, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 28, 20, 0,  },
        { 0, 0, 0, 0,  },
        { 0, 30, 20, 0,  },
    };
    constexpr const size_t eof = 7;
    
    class SyntacticAnalyzer {
        using symbol_type = long long; // minus is terminate while the opposite is nonterminate
        using condition_of_analysis = size_t;
        struct analysis_symbol {
            condition_of_analysis condition;
            object_type object;
            analysis_symbol() = default;
            analysis_symbol(condition_of_analysis c, object_type &o): condition(c), object(o) {}
            analysis_symbol(condition_of_analysis c, object_type &&o): condition(c), object(o) {}
        };
        std::stack<analysis_symbol> astack; // stands for analysis stack
        template <bool finish>
        void reduce(ll na, const token_type &t) {
            size_t length = production_elementAmount_table[na];
            object_type content[length];
            for (size_t i = 0; i < length; ++i) {
                content[length - 1 - i] = std::move(astack.top().object);
                astack.pop();
            }
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
        SyntacticAnalyzer() { astack.emplace(1 /* start from condition 1 */, default_object_type()); }
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
    };
    
    template <typename T>
    std::optional<std::pair<token_type, std::string>> analyze(const T &token_stream) {
        if (!token_stream.size())
            return std::nullopt;
        SyntacticAnalyzer analyzer;
        try {
            for (size_t i = 0; i < token_stream.size(); ++i)
                analyzer.analyze(token_stream[i]);
            analyzer.analyze<true>(token_stream[token_stream.size() - 1]);
        }
        catch (std::pair<token_type, std::string> &e) {
            return e;
        }
        return std::nullopt;
    }
    
}

// 在这里可以假想已经生成了一个翻译后的对外接口函数，用来做语法分析，其函数原型如下：
// 因为token_t是规定的token的类型，所以自动生成token_t。
// stream_content是一个生成的trait类，能够有length和允许下标访问就OK。

// 这里实现getValue()也是OK的。
double getValue(const char &c) {
    return c - '0';
}

namespace rules_translator::test {
    void test_Sample() {
        const std::string &s = "1+2*(3+4*5-1)";
        calculator_stuff::analyze(s);
    }
}
