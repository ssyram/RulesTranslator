//
//  main.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/16.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include <fstream>
#include <variant>
using std::variant;
using std::fstream;
#include <iostream>
using std::cin;
using std::cout;
using std::endl;
#include <string>
using std::string;

variant<int, bool, double> k;

#include "test/TestFileInteractor.h"
#include "test/TestPreprocessor.h"
#include "test/TestTableGenerator.h"
#include "test/WholeTest.h"
#include "util/UtilFunctions.h"
#include <unordered_set>
#include "Runner.h"
#include "test/TestSample.h"
#include "extern/TargetFormat.h"
using std::unordered_set;

//namespace utils {
//    bool isNumber(const char c) {
//        return c >= '0' && c <= '9';
//    }
//}
//
//bool (*isnum)(const char c) = &::utils::isNumber;

int main(int argc, const char * argv[]) {
    
//    std::visit(overloaded {
//        [] () {
//            cout << "OK" << endl;
//        }
//    });
    
//    rules_translator::test::test_Sample();
    
//    rules_translator::test::test();

//    bool a = true;

//    cout << (a ? 1 : 2) <<endl;
    
//    if (argc != 3) {
//        cout << "Please input only two elements." << endl;
//        return 1;
//    }
//    
//    run(argv);
    
    // insert code here...
//    fstream fs("1.txt", std::ios::out | std::ios::in);
//
////    if (fs.is_open())
////        fs << "123" << endl;
////    else
////        cout << "no" << endl;
//    string s;
//    fs >> s;
//    cout << s << endl;
//    cout << "'" << (char)fs.get() << "'" << endl;
//    cout << "'" << (char)fs.get() << "'" << endl;
//    cout << "'" << (char)fs.get() << "'" << endl;
//    std::getline(fs, s);
//    if (s.empty())
//        cout << "yes!" << endl;
    
//    object_type c[];
    
//    rules_translator::test::test_Preprocessor();
//    rules_translator::test::test_TableGenerator();
    
//    int i = 3;
//    unsigned int k = i - 5;
//    cout << k << endl;
    
//    unordered_map<size_t, size_t> f;
//    for (int i = 0; i < 10000; ++i)
//        cout << f[i] << endl;
    
//    auto modifyBlock = [] (string &s) -> unordered_set<size_t> {
//        unordered_set<size_t> nums;
//        for (size_t i = 0; i < s.length(); ++i) {
//            if (s[i] == '$' && i + 1 < s.length()) {
//                char c = s[i + 1];
//                if (c == '$') {
//                    s.replace(s.begin() + i, s.begin() + i + 2, "__r");
//                    i += 2;
//                }
//                else if (utils::isNumber(c)) {
//                    s.replace(s.begin() + i, s.begin() + i + 1, "__c");
////                    cout << *it << it[1] << it[2] << it[3] << endl;
//                    i += 3;
//                    size_t temp = c - '0';
//                    while (i + 1 < s.length() && utils::isNumber(s[i + 1])) {
//                        ++i;
//                        temp = temp * 10 + s[i] - '0';
//                    }
//                    nums.insert(temp);
//                }
//            }
//        }
//        return nums;
//    };
//
//    string s = " $$.val = $1.val + $3.val; ";
//
//    auto set = modifyBlock(s);
//    cout << s << endl;
//    for (const auto &s: set)
//        cout << s << endl;
    
    rules_translator::test::test_FileInteractor();
    
//    cout << "abc" << "\a a" << endl;
//    string s = "abcdefghijklmnopq";
//    string::iterator it = s.begin();
//    it += 7;
//    s.replace(it, it + 2, "abcde");
//    cout << *it << endl;
//    ++it;
//    cout << *it << endl;
//    string s = "abc   d e f\rg \n  \rh i \n\r\n\tj";
//    rules_translator::utils::trimDivider(s);
//    cout << s << endl;
    
//    cout << at[0][1] << endl;
    
//    auto r = rules_translator::utils::split<false>("abc,, , a, ", ", ");
//    for (auto &it: r) {
//        cout << it << endl;
//    }
    
    return 0;
}
