#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <opencv/cv.hpp>

using namespace cv;
using namespace std;

// distance between point 1 and point 2
double distance(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
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

float average_errorBnW(Mat* originalImage, Mat* image){
    float error = 0;

    for (int r = 0; r < (*image).rows; r++) {
        for (int c = 0; c < (*image).cols; c++) {

            float tmp = (*originalImage).at<uint8_t>(r,c) - (*image).at<uint8_t>(r,c);
            //cout << tmp;
            error += tmp*tmp;

        }
    }

    error /= (*image).rows*(*image).cols;

    return error;
}

// make images ready to display but not take into account the changes made.
void getVisualDFT(Mat& image, Mat& destImage, bool dftImage){

    //make 2 planes, and fill it up with zeros
    Mat planes[2] = { Mat::zeros(image.size(), CV_32F), Mat::zeros(image.size(), CV_32F) };

    //MAGNITUDE
    // take the image and split it into real and imaginary and put both separate in planes ([0] and [1] respectively)
    split(image, planes);

    magnitude(planes[0], planes[1], planes[0]);
    Mat magnitudeImage = planes[0];

    // switch to logarithmic scale
    magnitudeImage += Scalar::all(1);


    // do the log only for the dft and not other images that need this function
    if (dftImage) {
        // => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
        // this log messes up the brightness
        log(magnitudeImage, magnitudeImage);
    }

    // crop the spectrum, if it has an odd number of rows or columns
    magnitudeImage = magnitudeImage(Rect(0, 0, magnitudeImage.cols & -2, magnitudeImage.rows & -2));


    // Transform the matrix with float values into a viewable image form (float between values 0 and 1).
    normalize(magnitudeImage, magnitudeImage, 0, 1, CV_MINMAX);

    destImage = magnitudeImage;
}

// make images ready to display but not take into account the changes made.
Mat showImage(String name, Mat image, bool dftImage){
    Mat imageShow;
    getVisualDFT(image, imageShow, dftImage);
    imshow(name, imageShow);
    return imageShow;
}

// rearrange the quadrants of Fourier image so that the origin is at the image center
// its stored again in image
void rearrangeDFT(Mat& image){
    int centerX = image.cols/2;
    int centerY = image.rows/2;


    Mat quadrant0(image, Rect(0, 0, centerX, centerY));   // Top-Left
    Mat quadrant1(image, Rect(centerX, 0, centerX, centerY));  // Top-Right
    Mat quadrant2(image, Rect(0, centerY, centerX, centerY));  // Bottom-Left
    Mat quadrant3(image, Rect(centerX, centerY, centerX, centerY)); // Bottom-Right

    // swap quadrants Top-Left with Bottom-Right
    Mat tmp;
    quadrant0.copyTo(tmp);
    quadrant3.copyTo(quadrant0);
    tmp.copyTo(quadrant3);

    // swap quadrant Top-Right with Bottom-Left
    quadrant1.copyTo(tmp);
    quadrant2.copyTo(quadrant1);
    tmp.copyTo(quadrant2);
}

void invertDFT(Mat& image, Mat& destImage){
    Mat inverted;
    dft(image, inverted, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);
    destImage = inverted;
}

// take the image and make it dft in order to apply filters later
void readyDFT(String location, Mat& destImage){
    Mat image = imread(location, CV_LOAD_IMAGE_GRAYSCALE);

    Mat padded;
    //expand image to an exponential of 2
    // if size it is an exponential of 2 it is a lot quicker to process
    int m = getOptimalDFTSize( image.rows );
    int n = getOptimalDFTSize( image.cols );

    // to do that we need to add 0s to the extra pixels
    copyMakeBorder(image, padded, 0, m - image.rows, 0, n - image.cols, BORDER_CONSTANT, Scalar::all(0));


    Mat planes[] = {Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F)};

    Mat complexImage;

    // Add to the expanded image another plane with zeros
    merge(planes, 2, complexImage);

    // this way the result may fit in the source matrix
    dft(complexImage, complexImage);

    destImage = complexImage;
}

Mat createDFT() {
    Mat dftImage;
    readyDFT("../PandaNoise.bmp", dftImage);

    rearrangeDFT(dftImage);

    showImage("spectrum magnitude", dftImage, true);


    rearrangeDFT(dftImage);


    Mat inverted;
    invertDFT(dftImage, inverted);

    return inverted;

}

int main( int argc, char** argv ) {
    Mat original;
    Mat originalBnW;
    Mat noisy;



    // Read the file
    original = imread("../PandaOriginal.bmp", CV_LOAD_IMAGE_COLOR);
    originalBnW = imread("../PandaOriginal.bmp", CV_LOAD_IMAGE_GRAYSCALE);
    noisy = imread("../PandaNoise.bmp", CV_LOAD_IMAGE_COLOR);


    if(! noisy.data ){                              // Check for invalid input
        cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }


    namedWindow("ORIGINAL", CV_WINDOW_AUTOSIZE);// Create a window for display.
    namedWindow("NOISY", CV_WINDOW_AUTOSIZE);// Create a window for display.

    imshow("ORIGINAL", original);
    imshow("NOISY", noisy);


    // put each image next to each other
    moveWindow("ORIGINAL", 0, 0);            // put window in certain position in the screen
    moveWindow("NOISY", 0, noisy.rows+50);            // put window in certain position in the screen


    //make images blurry taking random noise away

    Mat modified7 = createDFT();
    modified7 = showImage("DFT", modified7, false);

    //normalize(modified7, modified7, 0, 1, CV_MINMAX);
    //normalize(originalBnW, originalBnW, 0, 1, CV_MINMAX);

    //Mat modified8(modified7.rows, modified7.cols, CV_8UC4, modified7.data);
    //imwrite("DFT2.bmp", modified7);

    cout << "Noisy: " << average_error(&original, &noisy) << endl;


    //infinite loop
    while (1) {
        // if pressed ESC it closes the program
        if (waitKeyEx(10) == 27) {
            return 0;
        }
    }
}