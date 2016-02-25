#include <iostream>
#include <fstream>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;
Mat frame;

typedef struct{
	int red, blue, green;
}RGB;


typedef struct{
	RGB rgb;
	int row, col;
}Pixel;

typedef struct{
	vector<Pixel> pixel;
	bool isFire;
}Images;

RGB rgbThreshold;
vector<Images> image;
vector<Mat> processedImage;
vector<Mat> trainingImage;

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

//store the pixels of the proccessed image in a vector
void storeTrainingData(){ 
	for (int x = 0; x < processedImage.size(); x++){
		int ctr = 0;
		image.push_back(Images());
		for (int i = 0; i < frame.rows; i++)
		{
			for (int j = 0; j < frame.cols; j++)
			{
				image[x].pixel.push_back(Pixel());
				image[x].pixel[ctr].rgb.red = processedImage[x].at<cv::Vec3b>(i, j)[2]; // store red
				image[x].pixel[ctr].rgb.green = processedImage[x].at<cv::Vec3b>(i, j)[1]; //store green
				image[x].pixel[ctr].rgb.blue = processedImage[x].at<cv::Vec3b>(i, j)[0]; // store blue
				image[x].pixel[ctr].row = i; // store row
				image[x].pixel[ctr].col = j; // store column
				ctr++;
			}
		}
	}
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
				double cB = 128 + r * -37.797 - g * 74.203 + b *
					112.0;

				double cR = 128 + r * 112.00 + g * -93.7864 + b *
					-18.214;

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
}

void redThreshold(){
	for (int x = 0; x < processedImage.size(); x++){
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
}

// subtract the background to get only the desired pixel for training
void processTrainingImages(){
	cout << "proccesed Image: " << processedImage.size() << endl;
	cout << "training IMage:" << trainingImage.size() << endl;
	cout << "training Image row: " << trainingImage[0].rows << endl;
	cout << "training image column: " << trainingImage[0].cols << endl;
	redThreshold();
	YCbCrThreshold();
	
}

void openTrainingImages(){
	vector<cv::String> fn;
	//cv::String  folder = "C:\\Thesis\\image\\positive";
    cv:String folder = "image\\";
	glob(folder, fn, false);
	size_t count = fn.size();
	for (size_t i = 0; i<fn.size(); i++)
	{
		cout << fn[i] << endl;
		Mat m = imread(fn[i]);		
		trainingImage.push_back(m);
		processedImage.push_back(Mat());
		processedImage[i] = cv::Mat(trainingImage[i].size().height, trainingImage[i].size().width, CV_8UC3);
	}
}

void train(){

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
	for (int i = 0; i<trainingImage.size(); i++){
		imshow("training Image", trainingImage[i]);
		imshow("processed Image", processedImage[i]);
		
		waitKey(0); //Note: wait for user input for every image
	}
	return 0;

}

