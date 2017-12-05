//
// Created by Kike Piera Serra on 05/12/2017.
//

#ifndef COMPRESSION_FNODE_H
#define COMPRESSION_FNODE_H


class FNode {
public:
    int freq;

    //virtual node needed in order to be a polymorphic class
    virtual ~FNode() {}

protected:
    explicit FNode(int f);
};


#endif //COMPRESSION_FNODE_H
