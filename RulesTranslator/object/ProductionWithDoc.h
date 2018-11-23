//
//  ProductionWithDoc.h
//  RulesTranslator
//
//  Created by 潇湘夜雨 on 2018/11/17.
//  Copyright © 2018 ssyram. All rights reserved.
//

#ifndef ProductionWithDoc_h
#define ProductionWithDoc_h

#include "Production.h"
#include "../util/TranslateException.h"

namespace rules_translator {
    
    // this is a wrapper of Production
    // used by the TableGenerator
    struct ProductionWithDoc {
        const size_t docPos;
        const Production p;
        ProductionWithDoc(ProductionWithDoc &&pwd): p(std::move(pwd.p)), docPos(pwd.docPos) {}
        ProductionWithDoc(const ProductionWithDoc &) = default;
        ProductionWithDoc(Production &&p): p(p), docPos(0) {}
        ProductionWithDoc(const Production &p): p(p), docPos(0) {}
        ProductionWithDoc(const Production &p, size_t docPos): p(p), docPos(docPos) {}
        ProductionWithDoc next() const {
            if (end()) throw TranslateException("Trying to get a not exist next.");
            return ProductionWithDoc(p, docPos + 1);
        }
        bool last() const { // the next will be the end
            return docPos + 1 == p.right.size();
        }
        bool end() const { // reached end
            return docPos == p.right.size();
        }
        symbol getNext() const {
            if (end()) throw TranslateException("Trying to get a not exist next.");
            return p.right[docPos];
        }
        vector<symbol> getFollowString() const {
            if (last()) throw TranslateException("Trying to get a not exist follow.");
            vector<symbol> r;
            for (size_t i = docPos + 1; i < p.right.size(); ++i)
                r.push_back(p.right[i]);
            return r;
        }
        bool operator==(const ProductionWithDoc &pwd) const {
            return pwd.p == p && pwd.docPos == docPos;
        }
    };
    
}

#endif /* ProductionWithDoc_h */
