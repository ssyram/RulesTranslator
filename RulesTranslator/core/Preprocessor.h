//
//  Preprocessor.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef Preprocessor_h
#define Preprocessor_h

#include "../util/FileInteractor.h"
#include <unordered_map>
#include "../object/Production.h"
#include "../object/RulesInfo.h"
using std::unordered_map;

namespace rules_translator {
    
    class Preprocessor_Impl;
    
    // read from the file interactor
    // generate all semantic functions
    //
    // all symbols will be saved as symbol(bool isFalse, size_t num)
    // so a C++ binding map for non-terminate and C++ class name is needed.
    // and all of the Production is needed
    class Preprocessor {
        Preprocessor_Impl *impl;
    public:
        Preprocessor(FileInteractor &);
        Preprocessor(const Preprocessor &) = delete;
        Preprocessor &operator=(const Preprocessor &) = delete;
        ~Preprocessor();
        RulesInfo *generateInfo();
    };
    
}

#endif /* Preprocessor_h */
