//
//  UtilFunctions.hpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef UtilFunctions_hpp
#define UtilFunctions_hpp

#include <vector>
#include <string>
#include <unordered_map>
using std::unordered_map;
using std::vector;
using std::string;

namespace rules_translator::utils {
    
    bool isDivider(const char c);
    bool isNumber(const char c);
//    bool isWordBegin(const char c);
//    bool canInWord(const char c);
    
    size_t shift_change(size_t ori, uint8_t length, uint8_t pos);
    
    template <bool allowEmpty = true>
    vector<string> split(const string &s, const string &divider) {
        size_t b = 0, e = 0;
        vector<string> r;
        for (; (e = s.find(divider, b)) != string::npos; b = e + divider.length())
            if constexpr (allowEmpty)
                r.push_back(s.substr(b, e - b));
            else if (e - b > 0)
                r.push_back(s.substr(b, e - b));
        
        if constexpr (allowEmpty)
            r.push_back(s.substr(b, -1));
        else if (b < s.length())
            r.push_back(s.substr(b, -1));
        
        return r;
    }
    
    string &trimDivider(string &s);
    
}

#endif /* UtilFunctions_hpp */
