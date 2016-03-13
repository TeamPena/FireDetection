#include <iostream>
#include <fstream>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>


using namespace cv;
using namespace std;
Mat frame;
RNG rng(12345);
int thresh = 100;
int max_thresh = 255;
typedef struct{
	int red, blue, green;
}RGB;


typedef struct{
	RGB rgb;
}Pixel;

typedef struct{
	vector<Pixel> pixel;
	bool isFire;
	int rows, cols;
	float meanR;
	float meanG;
	float meanB;
	float varianceR;
	float varianceG;
	float varianceB;
}Images;



RGB rgbThreshold;
vector<Images> image;
vector<Mat> processedImage;
vector<Mat> trainingImage;
Mat processedImages;

RGB readFile(){
	RGB rgb;
	ifstream input_file("rgb.data", ios::binary);
	input_file.read((char*)&rgb, sizeof(rgb));
	return rgb;
}

void writeFile(RGB rgb){
	ofstream out("rgb.data", ios::binary);
	if (!out)
	{
		cout << "Cannot load file" << endl;
	}
	else
	{
		out.write((char*)&rgb, sizeof(rgb));
	}
	out.close();
}



void YCbCrThreshold()
{
	for (int x = 0; x < processedImage.size(); x++){
		for (int i = 0; i < processedImage[x].rows; i++)
		{
			for (int j = 0; j < processedImage[x].cols; j++)
			{
				bool isFire = false;
				double bT = processedImage[x].at<cv::Vec3b>(i, j)[0]; // B		
				double gT = processedImage[x].at<cv::Vec3b>(i, j)[1]; // G
				double rT = processedImage[x].at<cv::Vec3b>(i, j)[2]; // R

				double b = bT, g = gT, r = rT;

				double y = 16 + r *  65.481 + g * 128.553 + b *24.996;
				double cB = 128 + r * -37.797 - g * 74.203 + b * 112.0;
				double cR = 128 + r * 112.00 + g * -93.7864 + b * -18.214;

				isFire = (y >= cR >= cB);


				if (isFire)
				{
					processedImage[x].at<cv::Vec3b>(i, j)[0] = bT;
					processedImage[x].at<cv::Vec3b>(i, j)[1] = gT;
					processedImage[x].at<cv::Vec3b>(i, j)[2] = rT;
				}
				else
				{
					processedImage[x].at<cv::Vec3b>(i, j)[0] = 0;
					processedImage[x].at<cv::Vec3b>(i, j)[1] = 0;
					processedImage[x].at<cv::Vec3b>(i, j)[2] = 0;
				}
			}
		}
	}
	cout << "nahuman na ycbcr threshold" << endl;
}

void redThreshold(){
	for (int x = 0; x < processedImage.size(); x++){
		//cout << x << endl;
		for (int i = 0; i < trainingImage[x].rows; i++)
		{
			for (int j = 0; j<trainingImage[x].cols; j++)
			{
				bool isFire = false;

				int b = trainingImage[x].at<cv::Vec3b>(i, j)[0];
				int g = trainingImage[x].at<cv::Vec3b>(i, j)[1];
				int r = trainingImage[x].at<cv::Vec3b>(i, j)[2];

				if ((r > g && g > b))
				{
					//check if red is over a threshold
					if (r > rgbThreshold.red && g > rgbThreshold.green && b < rgbThreshold.blue)
					{
						isFire = true;
					}
				}

				if (isFire)
				{
					processedImage[x].at<cv::Vec3b>(i, j)[0] = b;
					processedImage[x].at<cv::Vec3b>(i, j)[1] = g;
					processedImage[x].at<cv::Vec3b>(i, j)[2] = r;

				}
				
			}
		}
	}
	cout << "nahuman na red threshold" << endl;
}

void storeTrainingImage(){
	for (int x = 0; x < processedImage.size(); x++){
		int ctr = 0;
		image.push_back(Images());
		for (int i = 0; i < processedImage[x].rows; i++)
		{
			for (int j = 0; j < processedImage[x].cols; j++)
			{
				int b = processedImage[x].at<cv::Vec3b>(i, j)[0];
				int g = processedImage[x].at<cv::Vec3b>(i, j)[1];
				int r = processedImage[x].at<cv::Vec3b>(i, j)[2];
				if (b != 0 && g != 0 && r != 0){
					image[x].pixel.push_back(Pixel());
					image[x].pixel[ctr].rgb.blue = b;
					image[x].pixel[ctr].rgb.green = g;
					image[x].pixel[ctr].rgb.red = r;
					ctr++;
				}
			}
		}
	}
}

// subtract the background to get only the desired pixel for training
void processTrainingImages(){
	cout << "proccesed Image: " << processedImage.size() << endl;
	cout << "training IMage:" << trainingImage.size() << endl;
	redThreshold();
	YCbCrThreshold();
	storeTrainingImage();
	cout << "mana" << endl;
	
}


void openTrainingImages(){
	vector<cv::String> fn;
	//cv::String  folder = "C:\\Thesis\\image\\positive";
    cv::String folder = "dataPics\\";
	glob(folder, fn, false);
	size_t count = fn.size();
	size_t i;
	for (i = 0; i<fn.size(); i++)
	{
		Mat m = imread(fn[i]);		
		trainingImage.push_back(m);
		processedImage.push_back(Mat());
		processedImage[i] = cv::Mat(trainingImage[i].size().height, trainingImage[i].size().width, CV_8UC3);
	}
}


void writeCSV(){
	ofstream myfile;
	myfile.open("train9.csv");
	int ctr = 1;
	int isFire = 0;
	int sum = processedImage[0].rows * processedImage[0].cols;
	myfile << "Red,Green,Blue,Fire" << endl;
	for (int x = 0; x < processedImage.size(); x++){
		if (x > 279){
			isFire = 1;
		}
		cout << x << ": " << isFire << endl;
		for (int i = 0; i < processedImage[x].rows; i++)
		{
			for (int j = 0; j < processedImage[x].cols; j++)
			{
				int b = processedImage[x].at<cv::Vec3b>(i, j)[0];
				int g = processedImage[x].at<cv::Vec3b>(i, j)[1];
				int r = processedImage[x].at<cv::Vec3b>(i, j)[2];

				if (b != 0 && g != 0 && r != 0){

					myfile << r << "," << g << "," << b  << "," << isFire << endl;
					ctr++;
				}
			}
		}
	}
	myfile.close();
}
void thresh_callback(int, void*)
{
	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	/// Detect edges using canny
	Canny(processedImages, canny_output, thresh, thresh * 2, 3);
	/// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Get the moments
	cv::Moments mom = cv::moments(contours[0]);
	double hu[7];
	cv::HuMoments(mom, hu); // now in hu are your 7 Hu-Moments

	for (int z = 0; z < 7; z++){
		cout <<"hu moment["<< z << "]:" << hu[z] << endl;
	}


	/// Show in a window
	namedWindow("Contours", CV_WINDOW_AUTOSIZE);
	imshow("Contours", processedImages);

	
}

void mean(){
	static float retval[3] = { 0, 0, 0 };
	for (int y = 0; y < image.size();y++){
		for (int x = 0; x < image[y].pixel.size(); x++){
			retval[0] = retval[0] + image[y].pixel[x].rgb.blue;
			retval[1] = retval[1] + image[y].pixel[x].rgb.green;
			retval[2] = retval[2] + image[y].pixel[x].rgb.red;
		}
		image[y].meanB = retval[0] / image[y].pixel.size();
		image[y].meanG = retval[1] / image[y].pixel.size();
		image[y].meanR = retval[2] / image[y].pixel.size();
		cout << "Red mean: " << image[y].meanR << endl;
		cout << "Green mean: " << image[y].meanG << endl;
		cout << "Blue mean: " << image[y].meanB << endl;
	}
}
void variance(){
	float varianceR, varianceG, varianceB;
	 varianceR = varianceG = varianceB = 0;
	for (int x = 0; x < image.size(); x++){
		for (int y = 0; y < image[x].pixel.size(); y++){
			varianceR = varianceR + pow(image[x].pixel[y].rgb.red - image[x].meanR,2.0);
			varianceG = varianceG + pow(image[x].pixel[y].rgb.green - image[x].meanG, 2.0);
			varianceB = varianceB + pow(image[x].pixel[y].rgb.blue - image[x].meanB, 2.0);
			
		}
		image[x].varianceR = sqrt(varianceR / image[x].pixel.size());
		image[x].varianceG = sqrt(varianceG / image[x].pixel.size());
		image[x].varianceB = sqrt(varianceB / image[x].pixel.size());
		cout <<"Red variance: "<< image[x].varianceR << endl;
		cout << "Green variance: " << image[x].varianceG << endl;
		cout << "Blue variance: " << image[x].varianceB << endl;
	}
}

int main()
{
	rgbThreshold = readFile();
	cout << "Initial thresholds:" << endl;
	cout << "red: " << rgbThreshold.red << endl;
	cout << "blue: " << rgbThreshold.green << endl;
	cout << "greens: " << rgbThreshold.blue << endl;
	
    openTrainingImages();
	processTrainingImages();
		//writeCSV();
	mean();
	variance();
	cvtColor(processedImage[0], processedImages, CV_BGR2GRAY);
	for (int i = 0; i<trainingImage.size(); i++){
		imshow("training Image", trainingImage[i]);
		imshow("processed Image", processedImage[i]);
		//createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
		thresh_callback(0, 0);
		
		waitKey(0); //Note: wait for user input for every image
   	}
	return 0;

}  

