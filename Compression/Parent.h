//
// Created by Kike Piera Serra on 05/12/2017.
//

#ifndef COMPRESSION_PARENT_H
#define COMPRESSION_PARENT_H
#include "FNode.h"


class Parent : public FNode {
public:
    FNode *left;
    FNode *right;

    Parent(FNode *c0, FNode *c1);

    ~Parent();
};


#endif //COMPRESSION_PARENT_H
