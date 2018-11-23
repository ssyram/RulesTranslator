//
//  AnalyzerGenerator.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef AnalyzerGenerator_h
#define AnalyzerGenerator_h

#include "../util/FileInteractor.h"

// To read the code from file ../util/AnalyzerPrototype.h to get the
// code of analyzer and printed it to the target file

namespace rules_translator {
    
    class AnalyzerGnerator_Impl;
    
    // Generate the analyzer using the target table
    // and the interface function
    // which accept a container that implements
    // size_t length()
    // token_type operator[](size_t pos)
    class AnalyzerGenerator {
        AnalyzerGnerator_Impl *impl;
    public:
        AnalyzerGenerator(FileInteractor &fi);
        ~AnalyzerGenerator();
        void generate();
    };
    
}

#endif /* AnalyzerGenerator_h */
