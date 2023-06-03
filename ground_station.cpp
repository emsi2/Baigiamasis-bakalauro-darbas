#include <opencv2\opencv.hpp>

#include <iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<stdio.h>
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#include <string>
#include <cstring>

#define CVUI_IMPLEMENTATION
#include "cvui.h"

#define WINDOW_NAME "Camera transmission"

using namespace cv;
using namespace std;
//using namespace std::chrono_literals;

struct initRoi {
	//initial coordination based on EVENT_LBUTTONDOWN
	int initX;
	int initY;

	// actual coordination 
	int actualX;
	int actualY;

	// final coordinates 
	int finalX;
	int finalY;
	int finalWidth;
	int finalHeight;

	int displayRoi;
}SelectedRoi;



int main() {

	Mat image;
	Mat src;
	int count = 0;
	SelectedRoi.displayRoi = 0;
	int csrt = 0;
	int mosse = 0;
	namedWindow(WINDOW_NAME);
	cvui::init(WINDOW_NAME);

	cv::Point cursor = cvui::mouse();
	//VideoCapture cap(0, CAP_DSHOW);
	VideoCapture cap("udpsrc port=5001 ! application/x-rtp,width=(int)640,height=(int)480,media=(string)video,clock-rate=(int)90000,encoding-name=(string)JPEG,payload=(int)26 ! rtpjpegdepay ! jpegdec ! videoconvert ! appsink", CAP_GSTREAMER);
	//VideoCapture cap("udpsrc port=5001 ! application/x-rtp,width=(int)640,height=(int)480,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H264,payload=(int)96 ! rtph264depay  ! decodebin ! videoconvert ! appsink", CAP_GSTREAMER);
	cap >> src;
	bool isColor = (src.type() == CV_8UC3);
	//--- INITIALIZE VIDEOWRITER
	VideoWriter video;
	int codec = VideoWriter::fourcc('d', 'i', 'v', 'x');  // select desired codec (must be available at runtime)
	double fps = 25.0;                          // framerate of the created video stream
	
	string filename = "server_test.avi";             // name of the output video file
	video.open(filename, codec, fps, src.size(), isColor);

		if (!cap.isOpened()) {
			cout << "cannot open camera";
		}

	while (true) {
		cap >> image;

		// render a message in the frame at position (10, 15)
		cvui::text(image, 10, 15, "Webcam");

		// Mygtukas atvaizduojamos koordinatese (110, 80)
		if (cvui::button(image, 0, 80, "Reset")) { // pridedamas "reset" mygtukas atšaukti sekimui
			
			count++;
			SelectedRoi.initX = 1;
			SelectedRoi.initY = 2;
			SelectedRoi.displayRoi = 1;
			SelectedRoi.finalWidth = 3;
			SelectedRoi.finalHeight = 4;
			SelectedRoi.displayRoi = 0;
		}

		// Show how many times the button has been clicked.
		// Text at position (250, 90), sized 0.4, in red.
		cvui::printf(image, 0, 110, 0.4, 0xff0000, "Button click count: %d", count);

		if (cvui::button(image, 0, 130, "CSRT tracker")) {
			// The button was clicked, so let's increment our counter.
			;
			csrt = 1;
			mosse = 0;
			SelectedRoi.initX = 2;
			SelectedRoi.initY = 1;
			SelectedRoi.displayRoi = 1;
			SelectedRoi.finalWidth = 3;
			SelectedRoi.finalHeight = 4;
			SelectedRoi.displayRoi = 0;
		}
		if (cvui::button(image, 0, 160, "Descend")) {
			// The button was clicked, so let's increment our counter.
			;
			
			SelectedRoi.initX = 1;
			SelectedRoi.initY = 1;
			SelectedRoi.displayRoi = 1;
			SelectedRoi.finalWidth = 3;
			SelectedRoi.finalHeight = 4;
			SelectedRoi.displayRoi = 0;
		}
		if (cvui::button(image, 0, 190, "Move to target")) {
			// The button was clicked, so let's increment our counter.
			;
			
			SelectedRoi.initX = 1;
			SelectedRoi.initY = 0;
			SelectedRoi.displayRoi = 1;
			SelectedRoi.finalWidth = 3;
			SelectedRoi.finalHeight = 4;
			SelectedRoi.displayRoi = 0;
		}
		if (csrt == 1) {
			cvui::printf(image, 0, 190, 0.4, 0xff0000, "CSRT tracker");
		}
		if (mosse == 1) {
			cvui::printf(image, 0, 190, 0.4, 0xff0000, "MOSSE tracker");
		}
		//setMouseCallback(WINDOW_NAME, CallBackFunc, NULL);
		// Save the mouse cursor to a point object
		//std::cout << "x: " << cursor.x << " y: " << cursor.y << std::endl;
		// or access the position directly
		//std::cout << "x: " << cvui::mouse().x << " y: " << cvui::mouse().y << std::endl;
		if (cvui::mouse(cvui::DOWN)) { // Užregistruojamas kairiojo peles klavišo paspaudimas
			
			SelectedRoi.initX = cvui::mouse().x; // iššsaugoma x koordinate peles klavišo paspaudimo vietoje
			SelectedRoi.initY = cvui::mouse().y; // iššsaugoma y koordinate peles klavišo paspaudimo vietoje
			SelectedRoi.displayRoi = 1;
		}
		if (cvui::mouse(cvui::UP)) {
			
			SelectedRoi.finalWidth = cvui::mouse().x - SelectedRoi.initX; // iššsaugoma x koordinate peles klavišo atleidimo vietoje
			SelectedRoi.finalHeight = cvui::mouse().y - SelectedRoi.initY; // iššsaugoma y koordinate peles klavišo atleidimo vietoje
			SelectedRoi.displayRoi = 0;

			//////////////////////////////////////////////
			//Tik yuo atveju veiks siuntimas jei paþymëta sritis yra tvarkinga atliekame susijungima ir duomenu persiuntima, be to netrukdys mygtuku paspaudimams
			if (SelectedRoi.initX > 0 && SelectedRoi.initY >= 0 && SelectedRoi.finalWidth > 0 && SelectedRoi.finalHeight > 0) {
				WSADATA wsa;
				SOCKET s;
				struct sockaddr_in server;
				//char message[] = "test";


				string str1 = to_string(SelectedRoi.initX);
				string str2 = to_string(SelectedRoi.initY);
				string str3 = to_string(SelectedRoi.finalWidth);
				string str4 = to_string(SelectedRoi.finalHeight);
				string str = str1 + ',' + str2 + ',' + str3 + ',' + str4 + ',';
				const char* message = str.c_str();

				printf("\nInitialising Winsock...\n");
				if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
				{
					printf("Failed. Error Code : %d", WSAGetLastError());
					return 1;
				}
				printf("Initialised.\n");

				if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
				{
					printf("Could not create socket : %d", WSAGetLastError());
				}

				printf("Socket created.\n");

				server.sin_addr.s_addr = inet_addr("192.168.0.3");
				server.sin_family = AF_INET;
				server.sin_port = htons(5005);

				//Connect to remote server
				if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0)
				{
					puts("connect error");
					return 1;
				}

				puts("Connected");

				//Send some data
				if (send(s, message, strlen(message) + 1, 0) < 0) // +1 padaro NUL ir tuomet gavëjas þino kada bagti rinkti duomenys
				{
					puts("Send failed");
					return 1;
				}
				puts("Data Send\n");
				shutdown(s, SD_BOTH);
				closesocket(s);
				
			}
			///////////////////////////////////////////////////////////
			
		}


		if (SelectedRoi.displayRoi != 0) {
			cv::Point cursor = cvui::mouse();
			rectangle(image, Rect(SelectedRoi.initX, SelectedRoi.initY, cursor.x - SelectedRoi.initX, cursor.y - SelectedRoi.initY), Scalar(255, 255, 255), 4, 8, 0);
		}

		// Update cvui internal stuff
		cvui::update();
		cvui::imshow(WINDOW_NAME, image);
		video.write(image);

		waitKey(25);
	}
	video.release();
	return 0;

}
