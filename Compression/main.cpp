#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv/cv.hpp>
#include "huffman.cpp"


bool debug = false;

double QYarray[8][8] =  {{16, 11, 10, 16, 24, 40, 51, 61},
                         {12, 12, 14, 19, 26, 48, 60, 55},
                         {14, 13, 16, 24, 40, 57, 69, 56},
                         {14, 17, 22, 29, 51, 87, 80, 62},
                         {18, 22, 37, 56, 68, 109, 103, 77},
                         {24, 35, 55, 64, 81, 104, 113, 92},
                         {49, 64, 78, 87, 103, 121, 120, 101},
                         {72, 92, 95, 98, 112, 100, 103, 99}};

double QCarray[8][8]=  {{17, 18, 24, 47, 99, 99, 99, 99},
                        {18, 21, 26, 66, 99, 99, 99, 99},
                        {24, 26, 56, 99, 99, 99, 99, 99},
                        {47, 66, 99, 99, 99, 99, 99, 99},
                        {99, 99, 99, 99, 99, 99, 99, 99},
                        {99, 99, 99, 99, 99, 99, 99, 99},
                        {99, 99, 99, 99, 99, 99, 99, 99},
                        {99, 99, 99, 99, 99, 99, 99, 99}};


float average_error(Mat& originalImage, Mat& image){

    float error = 0;

    for (int r = 0; r < image.rows; r++) {
        for (int c = 0; c < image.cols; c++) {

            for (int i = 0; i < 3; i++){
                float tmp = abs(originalImage.at<Vec3b>(r,c)[i] - image.at<Vec3b>(r,c)[i]);
                error += tmp*tmp;


            }
        }
    }

    error /= image.rows*image.cols*3;


    return error;
}

void getQuantizationTables(int qualityFactor, vector<Mat> &quantizationTables){
    double scale;

    Mat QY = Mat(8, 8, CV_64FC1, &QYarray);
    Mat QC = Mat(8, 8, CV_64FC1, &QCarray);


    if (qualityFactor < 50) scale = 5000 / qualityFactor;
    else scale = 200 - 2 * qualityFactor;


    scale = scale / 100.0;

    cout << "scale: " << scale << endl;

	multiply(QY, scale, QY);
	multiply(QC, scale, QC);

    cout << "quantization: " << QY << endl;
    cout << "quantization: " << QC << endl;
    quantizationTables = {QY, QC};

}


void goDct(Mat& image, bool inverse, vector<Mat> quantizationTables){
    int height = image.rows;
    int width = image.cols;


    // split in 3 planes RGB
    vector<Mat> planes;
    split(image, planes);


    for (int i = 0; i < (height/8)*8; i += 8) {
        for (int j = 0; j < (width/8)*8; j+= 8) {
            for (size_t k = 0; k < planes.size(); k++) {
                planes[k].convertTo(planes[k], CV_64FC1);


                //get 8x8 block from image from (j,i) coordinates
                Mat block = planes[k](Rect(j, i, 8, 8));

                block.convertTo(block, CV_64FC1);


                if (inverse){
                    if (j == 0 && i == 0) cout << "block: " << block << endl;

                    subtract(block, 128, block);

                    if (k == 0) multiply(block, quantizationTables[0], block);
                    else multiply(block, quantizationTables[1], block);

                    if (j == 0 && i == 0 && debug) cout << "outblock6453: " << block << endl;

                    idct(block, block);

                    if (j == 0 && i == 0 && debug) cout << "outblock6452: " << block << endl;


                    add(block, 128, block);
                    if (j == 0 && i == 0 && debug) cout << "outblock+128: " << block << endl;

                    block.copyTo(planes[k](Rect(j, i, 8, 8)));
                    if (j == 0 && i == 0) cout << "outblock: " << block << endl;

                }
                else {

                    if (j == 0 && i == 0) cout << "block: " << block << endl;
                    subtract(block, 128, block);
                    if (j == 0 && i == 0 && debug) cout << "block132: " << block << endl;

                    dct(block, block);

                    if (j == 0 && i == 0 && debug) cout << "block132: " << block << endl;

                    if (k == 0) divide(block, quantizationTables[0], block);
                    else divide(block, quantizationTables[1], block);

                    if (j == 0 && i == 0 && debug) cout << "block132: " << block << endl;

                    add(block, 128, block);
                    block.convertTo(block, CV_32S);


                    block.copyTo(planes[k](Rect(j, i, 8, 8)));
                    if (j == 0 && i == 0 && debug) cout << "outblock312: " << block << endl;
                    if (j == 0 && i == 0) cout << "outblock312: " << planes[k](Rect(j, i, 8, 8)) << endl;

                }
            }
        }
    }

    merge(planes, image);
}



int main(int argc, char** argv) {
	Mat original;
    Mat yCbCrImage;
    Mat iYCbCrImage;
    Mat dctImage;
    Mat idctImage;
	int x;      // needed for Visual Studio
    string fileLocation = "../Compression/Images/6-gs.ppm";


	// Read the file
    cout << "Reading file: " << fileLocation << endl;
	original = imread(fileLocation, CV_LOAD_IMAGE_COLOR);


    // Check for invalid input
	if (!original.data) {
		cout << "Could not open or find the image" << endl;
		cin >> x;       // needed for Visual Studio
		return -1;
	}


    // turn into YCbCr:
    cout << "Turning into YCbCr" << endl;
    cvtColor(original, yCbCrImage, CV_BGR2YCrCb);

    cout << "Creating DCT" << endl;

    int qualityFactor = -1;

    while (qualityFactor > 74 || qualityFactor <= 0) {
        cout << "Please enter a Quality Factor. It must be in the range [1..74]" << endl;

        string str;
        cin >> str;
        if(sscanf(str.c_str(), "%d", &qualityFactor) != 1){
            qualityFactor = -1;
        }
    }

    vector<Mat> quantizationTables;
    getQuantizationTables(qualityFactor, quantizationTables);

    dctImage = yCbCrImage.clone();
    goDct(dctImage, 0, quantizationTables);


    writeTo("../Compression/Images/compressedImage.kike", dctImage);
    Mat image = readFrom("../Compression/Images/compressedImage.kike");


    cout << "Inverting DCT" << endl;
    idctImage = image.clone();
    goDct(idctImage, 1, quantizationTables);

    cout << "Inverting YCrCb" << endl;
    idctImage.convertTo(idctImage, CV_8U);
    cvtColor(idctImage, iYCbCrImage, CV_YCrCb2BGR);


    cout << "Done" << endl;

    // Create a window for display.
    namedWindow("ORIGINAL", CV_WINDOW_AUTOSIZE);
    namedWindow("YCbCr", CV_WINDOW_AUTOSIZE);
    namedWindow("DCT", CV_WINDOW_AUTOSIZE);
    namedWindow("Inverted DCTImage", CV_WINDOW_AUTOSIZE);
    imshow("ORIGINAL", original);
    imshow("YCbCr", yCbCrImage);
    imshow("DCT", dctImage);
    imshow("Inverted DCTImage", idctImage);
    imshow("FINISHED", iYCbCrImage);

    // put each image next to each other
    moveWindow("ORIGINAL", 0, 0);
    moveWindow("DCT", 50, 0);
    moveWindow("Inverted DCTImage", 100, 0);

    cout << "mean square error: " << average_error(original, iYCbCrImage) << endl;

    //infinite Exit loop
    while (1) {
        // if pressed ESC it closes the program
        if (waitKeyEx(10) == 27) {
            return 0;
        }
    }
}