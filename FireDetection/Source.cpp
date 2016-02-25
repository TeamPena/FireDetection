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
}Pixel;

typedef struct{
	vector<Pixel> pixel;
	bool isFire;
	int rows, cols;
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
void writeCSV(){
	ofstream myfile;
	myfile.open("data.csv");
	int ctr = 0;
	int isFire = 0;
	int sum = processedImage[0].rows * processedImage[0].cols;
	cout << sum << endl;
	myfile << "Pixel, Red, Green, Blue, Fire" << endl;
	for (int x = 0; x < processedImage.size(); x++){
		if (x > 1){
			isFire = 1;
		}
		for (int i = 0; i < processedImage[x].rows; i++)
		{
			for (int j = 0; j < processedImage[x].cols; j++)
			{
				int b = processedImage[x].at<cv::Vec3b>(i, j)[0];
				int g = processedImage[x].at<cv::Vec3b>(i, j)[1];
				int r = processedImage[x].at<cv::Vec3b>(i, j)[2];

				if (b != 0 && g != 0 && r != 0){

					myfile << ctr << "," << r << "," << g << "," << b  << "," << isFire << endl;
					ctr++;
				}
			}
		}
	}
	myfile.close();
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
	writeCSV();

	for (int i = 0; i<trainingImage.size(); i++){
		imshow("training Image", trainingImage[i]);
		imshow("processed Image", processedImage[i]);
		
		waitKey(0); //Note: wait for user input for every image
   	}
	return 0;

}  

