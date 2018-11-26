//
//  TestFileInteractor.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "TestFileInteractor.h"
#include "test.h"
#include "../util/UtilFunctions.h"
#include <unordered_map>
using std::unordered_map;

namespace rules_translator::test {
    
    const char *ORIGIN_FILE_NAME = "calculator.tsl";
    const char *TARGET_FILE_NAME = "calculator.cpp";
    
    void test_FileInteractor() {
        FileInteractor fi("calculator.tsl", "calculator.cpp");
        cout << "---------------start---------------" << endl;
        optional<FileInteractor::ReadContent> r;
        string s;
        using t = FileInteractor::ReadContentType;
        while ((r = fi.read())) {
            if (r->type == t::word || r->type == t::block || r->type == t::terminate)
                cout << "{ " << rules_translator::utils::mapType2String.find(r->type)->second << ": " << ::rules_translator::utils::trimDivider(r->content) << " }";
            else
                cout << "{ " << rules_translator::utils::mapType2String.find(r->type)->second << " }";
//            cout << endl;
            getline(cin, s);
        }
        cout << "---------------end---------------" << endl;
    }
}
