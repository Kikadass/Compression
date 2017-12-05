//
// Created by Kike Piera Serra on 05/12/2017.
//

#include "Parent.h"

Parent::Parent(FNode *c0, FNode *c1) : FNode(c0->freq + c1->freq){
    left = c0;
    right = c1;
}

Parent::~Parent() {
    delete left;
    delete right;
}
