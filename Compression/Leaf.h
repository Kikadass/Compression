//
// Created by Kike Piera Serra on 05/12/2017.
//

#ifndef COMPRESSION_LEAF_H
#define COMPRESSION_LEAF_H

#include "FNode.h"


class Leaf : public FNode {
public:
    int color;

    Leaf(int freq, int c);
};


#endif //COMPRESSION_LEAF_H
