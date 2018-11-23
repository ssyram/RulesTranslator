//
//  RulesInfo.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/20.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef RulesInfo_h
#define RulesInfo_h

namespace rules_translator {
    
    struct RulesInfo {
        //        string terminate_class_name;
//        string token_type;
//        string get_type;
        size_t eof; // the ending terminate number, also at the same time the amount of other terminate
        size_t nonterminateType_amount;
        // same the map between nonterminate type and corresponding C++ class name
        unordered_map<symbol_type, string> nonterminateType_CppClassName_map;
        vector<Production> productions;
    };
    
}

#endif /* RulesInfo_h */
