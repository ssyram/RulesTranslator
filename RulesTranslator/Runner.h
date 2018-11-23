//
//  Runner.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef Runner_h
#define Runner_h

#include "core/Preprocessor.h"
#include "core/TableGenerator.h"
#include "core/AnalyzerGenerator.h"
#include <iostream>
using std::cout;
using std::endl;

using namespace rules_translator;

// this function is the entry of the whole project
int run(int argc, const char *argv[]) {
    if (argc != 3) {
        cout << "Please input exactly only 2 elements." << endl;
        return 1;
    }
    FileInteractor fi(argv[1], argv[2]);
    Preprocessor pss(fi);
    RulesInfo *info = pss.generateInfo();
    TableGenerator generator(fi, *info);
    generator.testGenerate();
    AnalyzerGenerator analyzer(fi);
    analyzer.generate();
    return 0;
}

#endif /* Runner_h */
