//
// Created by Kike Piera Serra on 05/12/2017.
//

#ifndef COMPRESSION_LEAF_H
#define COMPRESSION_LEAF_H

#include "FNode.h"
#include <cstdint>


class Leaf : public FNode {
public:
    int color;

    Leaf(int freq, uint8_t c);
};


#endif //COMPRESSION_LEAF_H
