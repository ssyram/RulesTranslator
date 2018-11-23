//
//  TableGenerator.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef TableGenerator_h
#define TableGenerator_h

#include "Preprocessor.h"

namespace rules_translator {
    
    class TableGenerator_Impl;
    
    // Generate tables according to LR(1) rules
    // the table will be written to the target file
    class TableGenerator {
        TableGenerator_Impl *impl;
    public:
        TableGenerator(FileInteractor &, RulesInfo &);
        ~TableGenerator();
        void generate();
        void testGenerate();
    };

}

#endif /* TableGenerator_h */
