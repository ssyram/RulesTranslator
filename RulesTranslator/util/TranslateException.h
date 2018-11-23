//
//  TranslateException.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef TranslateException_h
#define TranslateException_h

#include <string>
namespace rules_translator {
    // this name is the one who is referred by others
    // but it's implementation can be varied, means it may be another class that accept a string
    // as its initializer
    using TranslateException = std::string;
}


#endif /* TranslateException_h */
