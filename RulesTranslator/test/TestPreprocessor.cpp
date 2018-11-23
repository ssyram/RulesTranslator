//
//  TestPreprocessor.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/18.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "TestPreprocessor.h"
#include "test.h"

namespace rules_translator::test {
    
    void test_Preprocessor() {
        FileInteractor fi(ORIGIN_FILE_NAME, TARGET_FILE_NAME);
        Preprocessor pp(fi);
        RulesInfo &info = *(pp.generateInfo());
        cout << "eof: " << info.eof << endl;
//        cout << "get_type: " << info.get_type << endl;
//        cout << "token_type: " << info.token_type << endl;
        cout << "nonterminate type amount: " << info.nonterminateType_amount << endl;
        cout << "nonterminate type - cpp class name map: " << endl;
        for (const auto &p: info.nonterminateType_CppClassName_map)
            cout << "{ " << p.first << " : " << p.second << " }" << endl;
        cout << endl;
        cout << "productions: " << endl;
        for (const auto &p: info.productions) {
            cout << "{ " << p.left << " : { ";
            for (const auto &r: p.right)
                cout << (r.isTerminate ? "(t)" : "(n)") << r.type << ", ";
            cout << "}, ";
            cout << "__process_" << p.productionId << " }" << endl;
        }
    }
    
}
