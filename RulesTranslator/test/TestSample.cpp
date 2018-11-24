//
//  TestSample.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/23.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "TestSample.h"
#include "TestTarget.h"
#include "../util/UtilFunctions.h"
#include <iostream>
#include <string>
using std::cin;

namespace rules_translator::test {
    void test_Sample() {
        std::string s;
        std::cout << "Input a formula" << std::endl;
        std::getline(cin, s);
        do {
            try {
                calculator_stuff::analyze(rules_translator::utils::trimDivider(s));
            }
            catch (string &e) {
                std::cout << e << ": (input content)" << s << std::endl;
            }
            std::getline(cin, s);
        } while (s != "q");
    }
}
