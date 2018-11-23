//
//  FileInteractor.cpp
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#include "FileInteractor.h"
#include "TranslateException.h"
#include "UtilFunctions.h"
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
using std::unordered_map;
using std::unordered_set;
using std::stringstream;
using std::getline;

namespace rules_translator {
    
    namespace utils {
        
        using type = FileInteractor::ReadContentType;
        unordered_map<FileInteractor::ReadContentType, string> mapType2String = {
            { type::block, "block" }, { type::word, "word" }, { type::terminate, "terminate" },
            { type::equal, "equal" }, { type::productionEqual, "production equal" }, { type::semicolon, "semicolon" }, { type::orSymbol, "or symbol" }, { type::word_using, "keyword using" }, { type::word_get_type, "keyword get_type" }, { type::word_token_type, "keyword token_type" }, { type::word_class, "keyword class" }, { type::word_enum, "keyword enum" }, { type::word_terminate, "keyword terminate" },
        };
        
        namespace utils {
//            unordered_set<char> dividers = {
//                ' ', '\n', '\t', '\r'
//            };
            using t = FileInteractor::ReadContentType;
            unordered_map<string, t> keywordMap = {
                { "terminate", t::word_terminate }, { "enum", t::word_enum }, { "class", t::word_class }, { "token_type", t::word_token_type }, { "get_type", t::word_get_type }, { "using", t::word_using },
            };
        }
//
//        inline bool isDivider(const char c) {
//            return utils::dividers.find(c) != utils::dividers.end();
//        }
        inline bool isWordBegin(const char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
        }
        inline bool isNumber(const char c) {
            return c >= '0' && c <= '9';
        }
        inline bool canInWord(const char c) {
            return isWordBegin(c) || isNumber(c) || c == ':';
        }
        inline FileInteractor::ReadContentType getWordType(const string &s) {
            auto f = utils::keywordMap.find(s);
            return f == utils::keywordMap.end() ? FileInteractor::ReadContentType::word :  f->second;
        }
        
    }
    
    class FileInteractor_Impl {
        string buffer; // to buff if there is more than one symbol readed by a time
        size_t buffered_pos = 0; // if buffered_pos == buffer.length(), it means that another buffer should be read;
        
        std::ifstream origin;
        std::ofstream target;
        
        bool closed = false; // to signal if it's closed
        bool finished = false;
        
        // no other functions except close() can access this function
        // in order to keep no more than one close operation
        // all close operation should only access close() but not this function
        void real_close() {
            if (!finished) throw TranslateException("The part translation has not finished.");
            
            while (getline(origin, buffer))
                target << buffer << std::endl;
            
            origin.close();
            target.close();
        }
    public:
        FileInteractor_Impl(const string &originalFilePath, const string &targetFilePath): origin(originalFilePath, std::ios::in), target(targetFilePath, std::ios::trunc | std::ios::out) {
            if (originalFilePath.size() < 5 || originalFilePath.substr(originalFilePath.length() - 4, -1) != ".tsl")
                throw TranslateException("The postfix of original file should be tsl");
            if (!origin.is_open() || !target.is_open())
                throw TranslateException("Can't open files.");
            auto locate_origin_position = [this] () {
                while (1) {
                    while (getline(origin, buffer) && !(!buffer.empty() && buffer[0] == '`')) {
                        target << buffer << std::endl;
                    }
                    if (origin.eof())
                        throw TranslateException("No translation part found.");
                    if (buffer.substr(0, 6) == "```tsl") {
                        origin >> buffer;
                        break;
                    }
                }
            };
            locate_origin_position();
        }
        ~FileInteractor_Impl() {
            close();
        }
        optional<FileInteractor::ReadContent> read() {
            // rules of read:
            // content wrapped by {} is block, the whole content will be read as string
            // content wrapped by "" is terminate
            // content that is "=" is equal
            // content that is ":=" is productionEqual
            // other content is word
            if (finished) return std::nullopt;
            
//            // here use while to avoid empty line
//            while (buffered_pos == buffer.length()) {
//                getline(origin, buffer);
//                buffered_pos = 0;
//            }
            if (buffer.length() == buffered_pos) {
                origin >> buffer;
                buffered_pos = 0;
            }
            
            FileInteractor::ReadContentType t = FileInteractor::ReadContentType::word;
            stringstream ss;
            
            auto err = [this, &ss] (const string &s, size_t pos = -1) {
                ss.str("");
                ss << s;
                if (pos == -1)
                    ss << ", at position: ";
                ss << (pos == -1 ? (size_t)origin.tellg() : pos);
                throw TranslateException(ss.str());
            };
            
            // jump out of comments
            while (buffer[buffered_pos] == '/') {
                if (++buffered_pos >= buffer.length() || buffer[buffered_pos] != '/')
                    err("Not a valid slash here.");
                getline(origin, buffer);
                origin >> buffer;
                buffered_pos = 0;
            }
            
            size_t originalPos = origin.tellg();
            switch (buffer[buffered_pos]) {
                case '{':
                    // must not delete the dividers, for some divider in block may be very important in dividing inside words
                    while (++buffered_pos < buffer.length() && buffer[buffered_pos] != '}')
//                        if (!utils::isDivider(buffer[buffered_pos]))
                        ss << buffer[buffered_pos];
                    if (buffer[buffered_pos] == '}') {
                        ++buffered_pos;
                        break;
                    }
                    getline(origin, buffer, '}');
                    if (origin.eof())
                        err("lack corresponding '}' with position: ", originalPos);
                    t = FileInteractor::ReadContentType::block;
//                    for (auto it = buffer.begin(); it != buffer.end(); ++it)
//                        if (!utils::isDivider(*it))
//                            ss << *it;
                    ss << buffer;
                    origin >> buffer;
                    buffered_pos = 0;
                    break;
                    
                case '\"':
                    while (++buffered_pos < buffer.length() && buffer[buffered_pos] != '"')
                        if (utils::isDivider(buffer[buffered_pos]))
                            err("trying to divide a terminate");
                        else
                            ss << buffer[buffered_pos];
                    if (buffer[buffered_pos] != '"')
                        err("trying to divide a terminate");
                    t = FileInteractor::ReadContentType::terminate;
                    ++buffered_pos;
                    break;
                    
                case ';':
                    t = FileInteractor::ReadContentType::semicolon;
                    ++buffered_pos;
                    break;
                    
                case '=':
                    t = FileInteractor::ReadContentType::equal;
                    ++buffered_pos;
                    break;
                    
                case ':':
                    if (!(++buffered_pos < buffer.length() && buffer[buffered_pos] == '='))
                        err("Not a valid colon");
                    t = FileInteractor::ReadContentType::productionEqual;
                    ++buffered_pos;
                    break;
                    
                case '|':
                    t = FileInteractor::ReadContentType::orSymbol;
                    ++buffered_pos;
                    break;
                    
                case '`':
                    if (buffered_pos + 3 != buffer.length() || buffer[buffered_pos + 1] != '`' || buffer[buffered_pos + 2] != '`')
                        err("Not a valid end, a valid end must be \"```\" in the tail of a line.");
                    finished = true;
                    return std::nullopt;
                    break;
                    
                default:
                    break;
            }
            // word analysis
            if (t == FileInteractor::ReadContentType::word) {
            
                if (!utils::isWordBegin(buffer[buffered_pos]))
                    err("Unexpected char");
                ss << buffer[buffered_pos];
                while (++buffered_pos < buffer.length() && utils::canInWord(buffer[buffered_pos]))
                    if (buffer[buffered_pos] == ':') {
                        if (++buffered_pos < buffer.length() && buffer[buffered_pos] == ':') ss << "::";
                        else err("Not a valid colon");
                    }
                    else ss << buffer[buffered_pos];
                
                t = utils::getWordType(ss.str());
                if (t != FileInteractor::ReadContentType::word)
                    ss.str("");
            }
            return FileInteractor::ReadContent(t, ss.str());
        }
        void write(const string &content) {
            if (closed) throw TranslateException("The process is already finished, no write permitted.");
            target << content;
        }
        void writeln(const string &content) {
            if (closed) throw TranslateException("The process is already finished, no write permitted.");
            target << content << std::endl;
        }
        void write(const long long &ll) {
            if (closed) throw TranslateException("The process is already finished, no write permitted.");
            target << ll;
        }
        void close() {
            if (!closed) real_close();
            closed = true;
        }
    };
    
    
    
    
    FileInteractor::FileInteractor(const string &originalFilePath, const string &targetFilePath) {
        impl = new FileInteractor_Impl(originalFilePath, targetFilePath);
    }
    FileInteractor::~FileInteractor() {
        delete impl;
    }
    optional<FileInteractor::ReadContent> FileInteractor::read() {
        return impl->read();
    }
    FileInteractor &FileInteractor::write(const string &content) {
        impl->write(content);
        return *this;
    }
    FileInteractor &FileInteractor::writeln(const string &content) {
        impl->writeln(content);
        return *this;
    }
    void FileInteractor::close() {
        impl->close();
    }
    
}
