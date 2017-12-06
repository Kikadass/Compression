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
#include <fstream>


using namespace std;
using namespace cv;
bool hDebug = false;

struct NodeCmp {
    bool operator()(const FNode* lhs, const FNode* rhs) const {
        return lhs->freq > rhs->freq;
    }
};


// take frequencies of each color and build a tree from the root with the values that appear at least once
FNode* buildTree(const map<uint8_t, int> &frequencies) {
    priority_queue<FNode*, vector<FNode*>, NodeCmp> trees;

    // get rid of values that do not appear, create leafs for those that do, and add leafs to tree
    for (auto i = frequencies.begin(); i != frequencies.end(); ++i) {
        uint8_t value = i->first;
        int freq = i->second;

        if (hDebug) {
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



void generateCodes(const FNode* FNode, const vector<bool>& hCode, map<uint8_t, vector<bool>>& hMap) {
    // if the node is a leaf put the huffman code in the map for that color
    if (const Leaf* leaf = dynamic_cast<const Leaf*>(FNode)) {
        hMap[leaf->color] = hCode;
    }

    // if it is not a leaf (is a parent) do recursive call, from left to right
    else if (const Parent* parent = dynamic_cast<const Parent*>(FNode)) {
        vector<bool> leftPrefix = hCode;
        leftPrefix.push_back(false);
        generateCodes(parent->left, leftPrefix, hMap);

        vector<bool> rightPrefix = hCode;
        rightPrefix.push_back(true);
        generateCodes(parent->right, rightPrefix, hMap);
    }
}



map<uint8_t, vector<bool>> huffman(Mat imageMatrix) {
    // map of pixel values and how many times they appear
    map<uint8_t, int> frequencies;

// split in 3 planes RGB
    vector<Mat> planes;
    split(imageMatrix, planes);

    for (int i = 0; i < imageMatrix.rows; i++) {
        for (int j = 0; j < imageMatrix.cols; j++) {
            for (size_t k = 0; k < planes.size(); k++) {
                planes[k].convertTo(planes[k], CV_8UC1);

                uint8_t value = uint8_t(planes[k].at<uchar>(i, j));

                frequencies[value]++;
            }
        }
    }

    // build the tree of frequencies
    FNode* tree = buildTree(frequencies);


    map<uint8_t, vector<bool>> hMap;
    generateCodes(tree, vector<bool>(), hMap);
    delete tree;

    // go through the huffman map to print results
    if (hDebug) {
        for (map<uint8_t, vector<bool>>::const_iterator it = hMap.begin(); it != hMap.end(); ++it) {
            cout << it->first << " ";
            copy(it->second.begin(), it->second.end(),
                 ostream_iterator<bool>(cout));
            cout << endl;
        }
    }

    return hMap;
}

void writeTo(string location, Mat image) {
    cout << "Huffman: " << endl;
    map<uint8_t, vector<bool>> hMap = huffman(image);


    cout << "Writing!" << endl;

    ofstream ofile(location, ios::binary);

    for (map<uint8_t, vector<bool>>::iterator i = hMap.begin(); i != hMap.end(); ++i) {
        ofile.write(reinterpret_cast<const char *>(&i->first), sizeof(i->first)); // null terminated string

        vector<bool> code = i->second;

        for (bool b : code) {
            ofile.write(reinterpret_cast<const char *>(&b), sizeof(b));
        }
    }

    // do the rest of the writing

    ofile.close();
}

void readFrom(string location) {
    cout << "Reading!" << endl;
    ifstream input(location, ios::binary);
    vector<int> buffer((istreambuf_iterator<char>(input)),
                       (istreambuf_iterator<char>()));

    input.close();



    map<vector<bool>, int> iHMap;

    int i = 0;
    for (int j = 0; j < buffer.size(); i++) {
        if (buffer[j] < 0) {
            buffer[j] = 256 + buffer[j];
        }

//cout << "value: " << buffer[j] << endl;
        int value = -1;
        vector<bool> code;

        if ((buffer[j] != 0 && buffer[j] != 1) || i == 0 || j == 0) {
            value = buffer[j];
//cout << "value: " << value << endl << "code: ";
            j++;
        }
        while ((buffer[j] == 0 || buffer[j] == 1) && j != 0 && j < buffer.size()) {

            code.push_back((bool) buffer[j]);

//cout << code.back();
            j++;
        }

        iHMap[code] = value;
//cout <<  endl;
    }


    vector<bool> code;
    code.push_back(1);
    cout << iHMap.find(code)->second << endl;

    // do the rest of the reading
}

