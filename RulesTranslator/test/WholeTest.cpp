//
//  WholeTest.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/20.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "WholeTest.h"
#include "test.h"
#include "../core/Preprocessor.h"
#include "../core/TableGenerator.h"
#include "../core/AnalyzerGenerator.h"

namespace rules_translator::test {
    
    void test() {
        FileInteractor fi(ORIGIN_FILE_NAME, TARGET_FILE_NAME);
        Preprocessor pss(fi);
        RulesInfo *info = pss.generateInfo();
        TableGenerator generator(fi, *info);
        generator.testGenerate();
        AnalyzerGenerator analyzer(fi);
        analyzer.generate();
    }
    
}
