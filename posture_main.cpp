
#include "opencv2/opencv.hpp"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <ctime>
#include <chrono>
#include <thread>
#include <vector>
#include <numeric>
#include <cmath> 
using namespace cv;
using namespace std; 

// additional notes: for running as standalone exe, need to have opencv as static library, not just dynamically link from IDE



class calibrate
{
public:
	int y_slouch, y_normal;
	void calibrate_slouch()
	{
		VideoCapture cap;
		vector<double> vec;
		// open the default camera;
		cap.open(0);

		// initialize haar cascade classifier
		CascadeClassifier face_cascade;
		face_cascade.load("haarcascade_frontalface_default.xml");
		Mat img;
		cout << "First, we will calibrate your sitting arrangement to calibrate slouching from sitting up" << endl;

		cout << "Please sit up for around 5 seconds. Press any key when ready." << endl;
		system("pause");
		for (int i=0; i<10; i++)
		{
			cap >> img;
			resize(img, img, Size(1000, 640));
			// Container of faces
			vector<Rect> faces;
			// Detect faces
			//face_cascade.detectMultiScale(img, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(140, 140));
			face_cascade.detectMultiScale(img, faces, 1.1, 3, 0, Size(100, 100), Size(400, 400));
			vec.push_back(double(faces[0].y));
			this_thread::sleep_for(std::chrono::milliseconds(500)); // sleep for 0.5 sec before next capture						
		}
		double average = accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
		y_normal = (int)average;
		//cout << y_normal << endl;

		vec.clear();
		cout << "Please slouch for around 5 seconds. Press any key when ready." << endl;
		system("pause");
		for (int i = 0; i<10; i++)
		{
			cap >> img;
			resize(img, img, Size(1000, 640));
			// Container of faces
			vector<Rect> faces;
			// Detect faces
			//face_cascade.detectMultiScale(img, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(140, 140));
			face_cascade.detectMultiScale(img, faces, 1.1, 3, 0, Size(100, 100), Size(400, 400));
			vec.push_back(double(faces[0].y));
			this_thread::sleep_for(std::chrono::milliseconds(500)); // sleep for 0.5 sec before next capture						
		}
		average = accumulate(vec.begin(), vec.end(), 0.0) / vec.size();
		y_slouch = (int)average;
		//cout << y_slouch << endl;
		
	}

};

int is_sit_up(int y_val, int y_norm_calibrated, int y_slouch_calibrated)
{
	if (abs(y_val - y_norm_calibrated) < (abs(y_val - y_slouch_calibrated) + 10)) //likely not slouching
		return 1;
	else return 0; // else slouching
}

int main(int argc, char** argv)
{
	calibrate cal;
	cal.calibrate_slouch();

	// Notify that tracking has started
	cout << "Tracking sit/slouch has now started. Please try to STAND UP and exercise at least once every 30 minutes" <<
		", and please SIT UP when sitting" << endl;
	
	// initialize time related variables	
	system("pause");
	clock_t start_sit = clock();
	double sitting_duration;

	// initialize the sitted flag
	int sitted_current=0; // away = 0, sitted and slouched = 1, sitted and straight = 2 
	int sitted_prev = 0;
	int sit_time=0, slouch_time=0;

	//initialize webcam related variables
	VideoCapture cap;
	// open the default camera, use something different from 0 otherwise;
	if (!cap.open(0))
		return 0;
	// initialize haar cascade classifier
	CascadeClassifier face_cascade;
	if (!face_cascade.load("haarcascade_frontalface_default.xml")) { printf("--(!)Error loading\n"); return -1; };	
	
	// captured image will be stored in img
	Mat img;
	
	for (;;)
	{		
		cap >> img;
		if (img.empty()) break; // end of video stream		

		// Just resize input image if you want
		resize(img, img, Size(1000, 640));
		// Container of faces
		vector<Rect> faces;
		// Detect faces
		//face_cascade.detectMultiScale(img, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(140, 140));
		face_cascade.detectMultiScale(img, faces, 1.1, 3, 0, Size(50, 50), Size(400, 400));
		
		//intermediate display
		//cout << is_sit_up(faces[0].y, cal.y_normal, cal.y_slouch) << endl;
		//cout << cal.y_normal << '\n' << cal.y_slouch;


		// before checking next round, keep a copy of current state in prev state
		sitted_prev = sitted_current;
		// check if sitted or away, if away check multiple times for 15 seconds to tell with more certainty if away
		if (faces.size() == 0) // away, but then, is really away? do following computations
		{
			for (int i = 0; i < 15; i++)
			{				
				std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // sleep for 1 sec before next capture
				face_cascade.detectMultiScale(img, faces, 1.1, 3, 0, Size(50, 50), Size(400, 400));
				if (faces.size() != 0) // seems not away after all
				{					
					sitted_current = 1 +is_sit_up(faces[0].y, cal.y_normal, cal.y_slouch); // sitted=2 if sitting properly
					break;
				}
				sitted_current = 0;
			}
		}
		else {
			sitted_current = 1 +is_sit_up(faces[0].y, cal.y_normal, cal.y_slouch);
		} 
		//cout << sitted_current;

		// calculation for time slouched vs sitting up
		if (sitted_current == 2)
			sit_time++;
		else if (sitted_current == 1)
			slouch_time++;

		// messages for different states and change of states
		if (sitted_current == 0 && sitted_prev!=0) // now away, previously sitting
		{
			sitting_duration = (std::clock() - start_sit) / (double)CLOCKS_PER_SEC;
			cout << "Duration for last sit session: " << sitting_duration/60.0 << " minutes"<< endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(20000)); // sleep for 20 sec before next capture
		}
		else if (sitted_current != 0 && sitted_prev != 0) // now sitting, previously sitting 
		{
			sitting_duration = (std::clock() - start_sit) / (double)CLOCKS_PER_SEC;
			cout << "Duration for current sit session: " << sitting_duration/60.0 << " minutes. Improper posture percent: " << (slouch_time * 100 / (sit_time+ slouch_time)) <<endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(20000)); // sleep for 20 sec before next capture
		}
		else if (sitted_current != 0 && sitted_prev == 0) // now sitting, previously away
		{
			start_sit = clock(); 
			sitting_duration = (std::clock() - start_sit) / (double)CLOCKS_PER_SEC;
			//cout << "Duration for current sit session: " << sitting_duration / 60.0 << " minutes";
			std::this_thread::sleep_for(std::chrono::milliseconds(20000)); // sleep for 20 sec before next capture
		}
		
		/*
		//Show the results
		// Draw circles on the detected faces
		
		for (int i = 0; i < faces.size(); i++)
		{
			Point center(faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5);
			ellipse(img, center, Size(faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);
		}

		if (waitKey(10) == 27) break; // stop capturing by pressing ESC 
		imshow("this is you, smile! :)", img);
		*/
	} 

	// the camera will be closed automatically upon exit
	cap.release();
	return 0;
}
