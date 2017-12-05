//
// Created by Kike Piera Serra on 05/12/2017.
//
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.hpp>

#include <iostream>
//#include "huffmanNode.cpp"
#include "Parent.h"
#include "Leaf.h"

#include <queue>
#include <map>
#include <climits> // for CHAR_BIT
#include <iterator>
#include <algorithm>

using namespace std;
using namespace cv;
bool debug = false;

struct NodeCmp {
    bool operator()(const FNode* lhs, const FNode* rhs) const {
        return lhs->freq > rhs->freq;
    }
};


// take frequencies of each color and build a tree from the root with the values that appear at least once
FNode* buildTree(const map<int, int> &frequencies) {
    priority_queue<FNode*, vector<FNode*>, NodeCmp> trees;

    // get rid of values that do not appear, create leafs for those that do, and add leafs to tree
    for (auto i = frequencies.begin(); i != frequencies.end(); ++i) {
        int value = i->first;
        int freq = i->second;

        if (debug) {
            cout << "value: " << value << endl;
            cout << "freq: " << freq << endl;
        }

        if(freq != 0)
            trees.push(new Leaf(freq, value));
    }


    // go through the the leafs and build the tree from the root
    while (trees.size() > 1) {
        FNode* childR = trees.top();
        trees.pop();

        FNode* childL = trees.top();
        trees.pop();

        FNode* parent = new Parent(childR, childL);
        trees.push(parent);
    }


    return trees.top();
}



void generateCodes(const FNode* FNode, const vector<bool>& hCode, map<int, vector<bool>>& hMap) {
    // if the node is a leaf put the huffman code in the map for that color
    if (const Leaf* leaf = dynamic_cast<const Leaf*>(FNode)) {
        hMap[leaf->color] = hCode;
    }

    // if it is not a leaf (is a parent) do recursive call, from left to right
    else if (const Parent* parent = dynamic_cast<const Parent*>(FNode)) {
        vector<bool> leftPrefix = hCode;
        leftPrefix.push_back(false);
        GenerateCodes(parent->left, leftPrefix, hMap);

        vector<bool> rightPrefix = hCode;
        rightPrefix.push_back(true);
        GenerateCodes(parent->right, rightPrefix, hMap);
    }
}



void huffman(Mat imageMatrix) {
    // map of pixel values and how many times they appear
    map<int, int> frequencies;

// split in 3 planes RGB
    vector<Mat> planes;
    split(imageMatrix, planes);

    cout << imageMatrix.rows << endl;
    cout << imageMatrix.cols << endl;

    for (int i = 0; i < imageMatrix.rows; i++) {
        for (int j = 0; j < imageMatrix.cols; j++) {
            for (size_t k = 0; k < planes.size(); k++) {
                planes[k].convertTo(planes[k], CV_8UC1);

                int value = int(planes[k].at<uchar>(i, j));

                frequencies[value]++;
            }
        }
    }

    // build the tree of frequencies
    FNode* tree = buildTree(frequencies);


    map<int, vector<bool>> hMap;
    generateCodes(tree, vector<bool>(), hMap);
    delete tree;

    // go through the huffman map to print results
    if (debug) {
        for (map<int, vector<bool>>::const_iterator it = hMap.begin(); it != hMap.end(); ++it) {
            cout << it->first << " ";
            copy(it->second.begin(), it->second.end(),
                 ostream_iterator<bool>(cout));
            cout << endl;
        }
    }

    return hMap;
}