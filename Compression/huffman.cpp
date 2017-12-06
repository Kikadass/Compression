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

    int counter = 0;
    int max = 0;

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

    int separator = 0;
    ofile.write(reinterpret_cast<const char *>(&separator), sizeof(separator));

    counter = 0;

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


using byte = unsigned char ;
using bits_in_byte = std::bitset<8> ;


void readFrom2(string location){
    cout << "Reading!" << endl;
    ifstream input(location, ios::binary);
    string bitstring ;
    char c ;
    int x;

    while( input.get(c) ) { // read byte by byte
        bitstring += bits_in_byte(byte(c)).to_string();
        cout << bitstring << endl;
        cin >> x;
    }
}


void readFrom(string location) {
    cout << "Reading!" << endl;
    ifstream input(location, ios::binary);
    vector<int> buffer((istreambuf_iterator<char>(input)),
                       (istreambuf_iterator<char>()));

    input.close();


    int x;

    map<vector<bool>, int> iHMap;

    int i = 0;
    bool header = true;
    for (int j = 0; j < buffer.size(); i++) {

        if (header) {
            if (buffer[j] == 0){
                header = false;
                continue;
            }

            if (buffer[j] < 0) {
                buffer[j] += 256;
            }

            int value = buffer[j];
            cout << "value: " << value << endl;
            j++;


            int size = buffer[j];
            j++;


            vector<bool> code;
            for (int k = 0; k < size; k++){
                bitset< 8 > bitSetCode = bitset< 8 >( buffer[j]);
//                cout << "boolean " << bitSetCode << endl;

                // 1 is the only code that does not have 0s in the beggining
                // and 1s are added in the beggining of the codes in order to make 1byte
                if (buffer[j] == -1 && size == 1){
                    code.push_back(1);
                }
                else {
                    bool extraOnes = true;
                    if (k != 0){
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
        else{
            cin >> x;
            vector<bool> code;
            while ((buffer[j] == 0 || buffer[j] == 1) && j < buffer.size()) {
                code.push_back((bool) buffer[j]);

                int value = iHMap.find(code)->second;
                cout << "code: ";
                for (bool b : code) {
                     cout << b;
                }
                cout << endl << "value: " << value << endl;

                int x;
                cin >> x;
                if (value < 256){

                }
                //cout << code.back();
                j++;
            }

        }


    }


    vector<bool> code;
    code.push_back(1);
    cout << iHMap.find(code)->second << endl;

    // do the rest of the reading



}

