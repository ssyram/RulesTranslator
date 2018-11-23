//
//  TestTableGenerator.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/20.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "TestTableGenerator.h"
#include "test.h"

namespace rules_translator::test {
    
    void test_TableGenerator() {
        FileInteractor fi(ORIGIN_FILE_NAME, TARGET_FILE_NAME);
        Preprocessor pss(fi);
        RulesInfo *info = pss.generateInfo();
        TableGenerator generator(fi, *info);
        generator.testGenerate();
    }
    
}
