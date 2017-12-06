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
bool hDebug = true;

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

void writeBitCodesHeader(vector<bool> code, ofstream& ofile){


    //add extra ones in order to make it multiple of 8 so it can be saved as bytes
    int extraOnes = code.size()%8;
    if (extraOnes != 0){
        code.insert(code.begin(), 8-extraOnes, 1);
    }

    cout << "code size" << code.size()/8 << endl;
    // write code size
    unsigned long l = code.size()/8;
    unsigned char c = static_cast<unsigned char>( l ); // simplest -- no checks for 8 bit bitsets
    ofile.write(reinterpret_cast<const char *>(&c), sizeof(c));



    int current_bit = 0;
    bitset< 8 > code1;
    for (bool bit : code) {

        code1.set(7-current_bit, bit);

        current_bit++;
        if (current_bit == 8) {
            l = code1.to_ulong();
            c = static_cast<unsigned char>( l ); // simplest -- no checks for 8 bit bitsets
            ofile.write(reinterpret_cast<const char *>(&c), sizeof(c));
            current_bit = 0;
        }
    }
}



void writeTo(string location, Mat image) {
    cout << "Huffman: " << endl;
    map<uint8_t, vector<bool>> hMap = huffman(image);


    cout << "Writing!" << endl;

    ofstream ofile(location, ios::binary);

    for (map<uint8_t, vector<bool>>::iterator i = hMap.begin(); i != hMap.end(); ++i) {
        ofile.write(reinterpret_cast<const char *>(&i->first), sizeof(i->first)); // null terminated string

        cout << "value: " <<  int(i->first) << endl;

        vector<bool> code = i->second;
        cout << "code: ";
        for (bool b : code){
            cout << b;
        }
        cout << endl;

        writeBitCodesHeader(code, ofile);
    }

    // write separator and its size size
    unsigned short separator = 255;
    unsigned char c = static_cast<unsigned char>( separator ); // simplest -- no checks for 8 bit bitsets
    ofile.write(reinterpret_cast<const char *>(&c), sizeof(c));
    ofile.write(reinterpret_cast<const char *>(&c), sizeof(c));
    cout << "separator: " << separator << " size: " << sizeof(c) << endl;


    //write the amount of rows and cols
    unsigned short rows = image.rows;
    unsigned short cols = image.cols;
    ofile.write(reinterpret_cast<const char *>(&rows), sizeof(rows)); // null terminated string
    ofile.write(reinterpret_cast<const char *>(&cols), sizeof(cols)); // null terminated string


    cout << "rows: " << rows << " size: " << sizeof(rows) << endl;
    cout << "cols: " << cols << " size: " << sizeof(cols) << endl;

    // split in 3 planes RGB
    vector<Mat> planes;
    split(image, planes);
    int current_bit = 0;
    bitset< 8 > code1;


    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            for (size_t k = 0; k < planes.size(); k++) {
                planes[k].convertTo(planes[k], CV_8UC1);

                uint8_t value = uint8_t(planes[k].at<uchar>(i, j));


                vector<bool> code = hMap.find(value)->second;

                for (bool bit : code) {

                    code1.set(7-current_bit, bit);

                    current_bit++;
                    if (current_bit == 8) {
                        unsigned long l = code1.to_ulong();
                        unsigned char c = static_cast<unsigned char>( l ); // simplest -- no checks for 8 bit bitsets
                        ofile.write(reinterpret_cast<const char *>(&c), sizeof(c));
                        current_bit = 0;
                    }
                }

            }
        }
    }

    ofile.close();
}


Mat readFrom(string location) {
    cout << "Reading!" << endl;
    ifstream input(location, ios::binary);
    vector<int> buffer((istreambuf_iterator<char>(input)),
                       (istreambuf_iterator<char>()));

    input.close();


    int x;

    map<vector<bool>, int> iHMap;

    int j = 0;
    bool header = true;
    int maxSize = 0;
    while (header) {
        if (buffer[j] < 0) {
            buffer[j] += 256;
        }

        int value = buffer[j];
        cout << "value: " << value << endl;
        j++;


        int size = buffer[j];
        if (size == -1) {
            header = false;
            continue;
        }
        if (size > maxSize) maxSize = size;

        j++;


        vector<bool> code;
        for (int k = 0; k < size; k++) {
            bitset<8> bitSetCode = bitset<8>(buffer[j]);
//                cout << "boolean " << bitSetCode << endl;

            // 1 is the only code that does not have 0s in the beggining
            // and 1s are added in the beggining of the codes in order to make 1byte
            if (buffer[j] == -1 && size == 1) {
                code.push_back(1);
            } else {
                bool extraOnes = true;
                if (k != 0) {
                    extraOnes = false;
                }

                for (int bit = 7; bit > -1; bit--) {
                    if ((bitSetCode[bit] == 0 && extraOnes) || !extraOnes) {
                        code.push_back(bitSetCode[bit]);
                        extraOnes = false;
                    }
                }
            }

            j++;

        }
        iHMap[code] = value;


        cout << "code: ";
        for (bool b : code) {
            cout << b;
        }
        cout << endl;
    }

    j++;
    int rows;
    int cols;
    for (int i = 0; i < 2; i++) {
        bitset<16> bitSetCode = bitset<16>(buffer[j]);
        j++;
        bitset<8> bitSetCode2 = bitset<8>(buffer[j]);
        j++;
        for (int i = 0; i < 8; i++) {
            bitSetCode[8 + i] = bitSetCode2[i];
        }

        if (i == 0) rows = bitSetCode.to_ulong();
        else cols = bitSetCode.to_ulong();
    }


    cout << "rows: " << rows << endl;
    cout << "cols: " << cols << endl;
    short planes[3][rows][cols];
    memset(planes, 0, 3 * rows * cols * sizeof(short));


    vector<bool> code;
    int i = 7;
    int p = 0;
    int r = 0;
    int c = 0;
    while (j < buffer.size()) {
        bitset<8> bitSetCode = bitset<8>(buffer[j]);

        code.push_back(bitSetCode[i]);
        i--;
        if (i < 0) {
            i = 7;
            j++;
        }

        int value = iHMap.find(code)->second;
        if (value < 255) {
            planes[p][r][c] = value;
            p++;

            if (p == 3) {
                p = 0;
                c++;
            }
            if (r == rows) {
                r = 0;
            }
            if (c == cols) {
                r++;
                c = 0;
            }

            code.erase(code.begin(), code.end());
        }
    }

    for (p = 0; p < 3; p++) {
        cout << "PLANE" << endl << endl;
        for (r = 0; r < 8; r++) {
            for (c = 0; c < 8; c++) {
                cout << planes[p][r][c] << " ";
            }
            cout << endl;
        }
    }

    vector<Mat> planesMat;
    planesMat.push_back(Mat(rows, cols, CV_16U, &planes[0]));
    cout << "PLANE" << endl << endl;

    planesMat.push_back(Mat(rows, cols, CV_16U, &planes[1]));
    cout << "PLANE" << endl << endl;

    planesMat.push_back(Mat(rows, cols, CV_16U, &planes[2]));
    cout << "PLANE" << endl << endl;

    Mat image;
    merge(planesMat, image);
    return image;
}

