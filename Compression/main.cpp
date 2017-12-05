#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv/cv.hpp>
#include "huffman.cpp"




bool debug = true;

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



// distance between point 1 and point 2
double distance(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void scaleUp(Mat& image, string window_name, int scale){
    resize((image), (image), (image).size()*scale, scale, scale);   //resize image
    resizeWindow(window_name, (image).cols, (image).rows);    // resize window

    imshow(window_name, image);                   // Show our image inside it.
}

void scaleDown(Mat& image, string window_name, int scale){
    resize((image), (image), (image).size()/scale, 1/scale, 1/scale);   //resize image
    resizeWindow(window_name, (image).cols, (image).rows);    // resize window

    imshow(window_name, (image));                   // Show our image inside it.
}

float average_error(Mat* originalImage, Mat* image){
    float Rerror = 0;
    float Gerror = 0;
    float Berror = 0;

    for (int r = 0; r < (*image).rows; r++) {
        for (int c = 0; c < (*image).cols; c++) {

            for (int i = 0; i < 3; i++){
                float tmp = (*originalImage).at<Vec3b>(r,c)[i] - (*image).at<Vec3b>(r,c)[i];

                if (i == 0) Berror += tmp*tmp;
                if (i == 1) Gerror += tmp*tmp;
                if (i == 2) Rerror += tmp*tmp;

            }
        }
    }

    Berror /= (*image).rows*(*image).cols;
    Gerror /= (*image).rows*(*image).cols;
    Rerror /= (*image).rows*(*image).cols;

    return (Rerror+Gerror+Berror)/3;
}

void getQuantizationTables(int qualityFactor, vector<Mat> &quantizationTables){
    double scale;

    Mat QY = Mat(8, 8, CV_64FC1, &QYarray);
    Mat QC = Mat(8, 8, CV_64FC1, &QCarray);


    scale = 200 - 2 * qualityFactor;



    scale = scale / 100.0;

    cout << "scale: " << scale << endl;

	multiply(QY, scale, QY);
	multiply(QC, scale, QC);

    cout << "quantization: " << QY << endl;
    cout << "quantization: " << QC << endl;
    quantizationTables = {QY, QC};

}


void goDct(Mat& image, bool inverse){
    int height = image.rows;
    int width = image.cols;

    int qualityFactor = -1;

    while (qualityFactor > 100 || qualityFactor <= 0) {
        cout << "Please enter a Quality Factor. It must be in the range [1..100]" << endl;
        cin >> qualityFactor;
    }

    vector<Mat> quantizationTables;
    getQuantizationTables(qualityFactor, quantizationTables);


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


                Mat outblock(block);

                if (inverse){
                    subtract(block, 128, block);

                    if (k == 0) multiply(block, quantizationTables[0], block);
                    else multiply(block, quantizationTables[1], block);
                    if (j == 0 && i == 0 && debug) cout << "outblock6453: " << outblock << endl;

                    idct(outblock, outblock);
                    if (j == 0 && i == 0 && debug) cout << "outblock6452: " << outblock << endl;

                    if (j == 0 && i == 0 && debug) cout << "outblock6451: " << outblock << endl;

                    add(outblock, 128, outblock);
                    if (j == 0 && i == 0 && debug) cout << "outblock+218: " << outblock << endl;

                    outblock.copyTo(planes[k](Rect(j, i, 8, 8)));
                    if (j == 0 && i == 0) cout << "outblock: " << outblock << endl;

                }
                else {

                    if (j == 0 && i == 0) cout << "block: " << block << endl;
                    subtract(block, 128, block);
                    if (j == 0 && i == 0 && debug) cout << "block132: " << block << endl;

                    dct(block, outblock);

                    if (k == 0) divide(outblock, quantizationTables[0], outblock);
                    else divide(outblock, quantizationTables[1], outblock);

                    add(outblock, 128, outblock);
                    outblock.convertTo(outblock, CV_8U);


                    outblock.copyTo(planes[k](Rect(j, i, 8, 8)));
                    if (j == 0 && i == 0 && debug) cout << "outblock312: " << outblock << endl;
                    if (j == 0 && i == 0 && debug) cout << "outblock312: " << planes[k](Rect(j, i, 8, 8)) << endl;

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
    string fileLocation = "../Compression/Images/1.ppm";


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
    dctImage = yCbCrImage.clone();
    goDct(dctImage, 0);

    cout << "Huffman: " << endl;
    huffman(dctImage);

    cout << "Inverting DCT" << endl;
    idctImage = dctImage.clone();
    goDct(idctImage, 1);

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

    //cout << iYCbCrImage << endl;

    // put each image next to each other
    moveWindow("ORIGINAL", 0, 0);
    moveWindow("DCT", 50, 0);
    moveWindow("Inverted DCTImage", 100, 0);


    //normalize(modified7, modified7, 0, 1, CV_MINMAX);
    //normalize(originalBnW, originalBnW, 0, 1, CV_MINMAX);


    //save image
    //imwrite("DFT2.ppm", iYCbCrImage);

    //cout << "Noisy: " << average_error(&original, &ppm) << endl;




    //infinite Exit loop
    while (1) {
        // if pressed ESC it closes the program
        if (waitKeyEx(10) == 27) {
            return 0;
        }
    }
}