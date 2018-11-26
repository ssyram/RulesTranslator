//
//  FileInteractor.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef FileInteractor_h
#define FileInteractor_h

#include <string>
#include <optional>
#include <unordered_map>
using std::unordered_map;
using std::optional;
using std::string;

// this file holds the interfaces needed to interact with the target file
// including reading content from the file, writing content to the file.
//
// To be specific, this functionality is like the lexical analyzer of the
// file and output them as string. And the content of string will be
// corresponding to the rules/format of target part, which means
// the preprocessor will get what it wants to know every time it read().
//
// Writing means printing content to the next appropriate position.
//
namespace rules_translator {
    
    class FileInteractor_Impl;
    
    // an object of this class should not be destructed until nothing to write;
    class FileInteractor {
        friend class Preprocessor;
        friend class FileInteractor_Impl;
        
        FileInteractor_Impl *impl;
    public:
        // the return value of read
        enum class ReadContentType {
            word, block, terminate, equal, productionEqual, semicolon, orSymbol, word_terminate, word_enum, word_class, word_token_type, word_get_type, word_using
        };
        
        struct ReadContent {
            ReadContentType type;
            string content;
            ReadContent(ReadContentType t, const string &s): type(t), content(s) {}
            ReadContent() = default;
        };
        
        // target file is not existed, or it will be emptied
        FileInteractor(const string &originalFilePath, const string &targetFilePath);
        FileInteractor(const FileInteractor &) = delete;
        FileInteractor &operator=(const FileInteractor &) = delete;
        ~FileInteractor();
        
        FileInteractor &writeln(const string &);
        FileInteractor &write(const string &);
        FileInteractor &write(const long long &);
        void close();
        
        // rules of read:
        // content wrapped by {} is block, the whole content will be read as string
        // content wrapped by "" is terminate
        // content that is "=" is equal
        // content that is ":=" is productionEqual
        // other content is word
        optional<ReadContent> read();        
    };
    
    namespace utils {
        extern unordered_map<FileInteractor::ReadContentType, string> mapType2String;
    }
    
}

#endif /* FileInteractor_h */
