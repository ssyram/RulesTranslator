//
//  UtilFunctions.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "UtilFunctions.h"
#include <unordered_set>
using std::unordered_set;

namespace rules_translator::utils {
    namespace utils {
        unordered_set<char> dividers = {
            ' ', '\n', '\t', '\r'
        };
    }

    bool isDivider(const char c) {
        return utils::dividers.find(c) != utils::dividers.end();
    }
//    bool isWordBegin(const char c) {
//        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
//    }
    bool isNumber(const char c) {
        return c >= '0' && c <= '9';
    }
//    bool canInWord(const char c) {
//        return isWordBegin(c) || isNumber(c) || c == ':';
//    }
    string &trimDivider(string &s) {
        for (string::iterator it = s.begin(); it != s.end(); ++it)
            if (isDivider(*it))
                s.erase(it--);
        
        return s;
    }
}
