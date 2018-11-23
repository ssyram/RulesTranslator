//
//  Production.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef Production_h
#define Production_h

#include <vector>
#include <string>
using std::string;
using std::vector;

namespace rules_translator {

    using symbol_type = size_t;
    struct symbol {
        bool isTerminate = false;
        symbol_type type;
        symbol() = default;
        symbol(bool a, symbol_type t): isTerminate(a), type(t) {}
        bool operator==(const symbol &s) const {
            return s.type == type && s.isTerminate == isTerminate;
        }
    };

    struct Production {
        symbol_type left;
        vector<symbol> right;
        size_t productionId;
//        string processName;
//        Production(const symbol_type &left, const vector<symbol> &right, const string &processName): left(left), right(right), processName(processName) {}
//        Production(const symbol_type &left, const vector<symbol> &&right, const string &processName): left(left), right(right), processName(processName) {}
        Production() = default;
        Production(Production &&p): left(p.left), right(std::move(p.right)), productionId(p.productionId) {}
        Production(const Production&) = default;
        Production &operator=(const Production &) = default;
        bool operator==(const Production &p) const {
            return productionId == p.productionId;
        }
    };
}


#endif /* Production_h */
