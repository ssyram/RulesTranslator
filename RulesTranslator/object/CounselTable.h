//
//  CounselTable.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef CounselTable_h
#define CounselTable_h

#include <vector>
using std::vector;

namespace rules_translator {
    
    using ll = long long;

    // this is the class of tables
    // that needs to be counseled
    // such as
    // action_table and goto_table
    class CounselTable {
        const size_t lineElementAmount;
        vector<ll*> table;
    public:
        CounselTable(size_t terminateTypeAmount): lineElementAmount(terminateTypeAmount) {}
        ~CounselTable() {
            for (auto &l: table) {
                if (l) delete[] l;
            }
        }
        ll *operator[](size_t lineNum) {
            for (size_t i = table.size(); i <= lineNum; ++i)
                table.push_back(new ll[lineElementAmount]()); // all elements will be initialize as 0
            return table[lineNum];
        }
        size_t lineAmount() const {
            return table.size();
        }
        size_t columnAmount() const {
            return lineElementAmount;
        }
    };

}
    
#endif /* CounselTable_h */
