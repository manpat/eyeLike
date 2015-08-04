#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <cstdio>
#include <cmath>

#include "constants.h"
#include "findEyeCenter.h"
#include "findEyeCorner.h"

#include "PacketHandler.h"

/** Function Headers */
void detectAndDisplay( cv::Mat frame );

double GetVal();
void DoStuffWithPupils(cv::Point left, cv::Point right, double height);

/** Global variables */
cv::String face_cascade_name = "haarcascade_frontalface_alt.xml";
cv::CascadeClassifier face_cascade;
std::string main_window_name = "Capture - Face detection";
std::string face_window_name = "Capture - Face";
cv::RNG rng(12345);
cv::Mat debugImage;

double smoothingRatio = 5.0; 
bool faceCaptured = false;

int main( int argc, const char** argv ) {
	// Network stuff
	if(argc < 1) {
		std::cerr << "Missing port" << std::endl;
		return 1;
	}

	int port = atoi(argv[1]);
	std::cout << "Port: " << port << std::endl;

	std::string fullName(argv[0]);
	std::string path;
	auto lastSlash = fullName.find_last_of("\\/");
	path = fullName.substr(0, lastSlash+1);

	PacketHandler packetHandler(port);
	std::cout << fullName << " running in " << path << std::endl;

	cv::VideoCapture capture(0);
	cv::Mat frame;

	auto faceCascadePath = path + face_cascade_name;

	// Load the cascades
	if(!face_cascade.load(faceCascadePath)){ 
		std::cout 
			<< "Error loading face cascade (" 
			<< faceCascadePath << ")"
			<< std::endl;
		return -1; 
	};

	if(showWindows){
		cv::namedWindow(main_window_name,CV_WINDOW_NORMAL);
		cv::moveWindow(main_window_name, 400, 100);
		cv::namedWindow(face_window_name,CV_WINDOW_NORMAL);
		cv::moveWindow(face_window_name, 10, 100);
		cv::namedWindow("Right Eye",CV_WINDOW_NORMAL);
		cv::moveWindow("Right Eye", 10, 600);
		cv::namedWindow("Left Eye",CV_WINDOW_NORMAL);
		cv::moveWindow("Left Eye", 400, 600);
	}

	createCornerKernels();

	 // Read the video stream
	if( capture.isOpened() ) {
		while( true ) {
			auto packets = packetHandler.ReceivePackets();
			for(auto& p: packets){
				switch(p.type){
				case PacketType::Ack:
					packetHandler.SendPacket(Packet{PacketType::ServAck, 0.0});
					break;
				case PacketType::GetData:
					if(faceCaptured)
						packetHandler.SendPacket(Packet{PacketType::Data, GetVal()});
					else
						packetHandler.SendPacket(Packet{PacketType::CantFindFace, 0.0});

					break;
				case PacketType::SetSmooth:
					smoothingRatio = std::max(1.0, p.data);
					packetHandler.SendPacket(Packet{PacketType::SetSmooth, smoothingRatio});
					break;
				case PacketType::EnableDebug:
					showWindows = true;
					break;
				default:
					std::cout << "Server recieved unknown packet type " << (byte)p.type << std::endl;
				}
			}

			capture.read(frame);

			// mirror it
			cv::flip(frame, frame, 1);
			if(showWindows) frame.copyTo(debugImage);

			// Apply the classifier to the frame
			if( !frame.empty() ) {
				detectAndDisplay( frame );
			} else {
				printf(" --(!) No captured frame -- Break!");
				break;
			}

			if(showWindows) imshow(main_window_name,debugImage);

			int c = cv::waitKey(1);
			if( (char)c == 'c' ) break;
		}
	}

	releaseCornerKernels();

	return 0;
}

void findEyes(cv::Mat frame_gray, cv::Rect face) {
	cv::Mat faceROI = frame_gray(face);
	cv::Mat debugFace = faceROI;

	if (kSmoothFaceImage) {
		double sigma = kSmoothFaceFactor * face.width;
		GaussianBlur( faceROI, faceROI, cv::Size( 0, 0 ), sigma);
	}
	//-- Find eye regions and draw them
	int eye_region_width = face.width * (kEyePercentWidth/100.0);
	int eye_region_height = face.width * (kEyePercentHeight/100.0);
	int eye_region_top = face.height * (kEyePercentTop/100.0);
	cv::Rect leftEyeRegion(face.width*(kEyePercentSide/100.0),
												 eye_region_top,eye_region_width,eye_region_height);
	cv::Rect rightEyeRegion(face.width - eye_region_width - face.width*(kEyePercentSide/100.0),
													eye_region_top,eye_region_width,eye_region_height);

	//-- Find Eye Centers
	cv::Point leftPupil = findEyeCenter(faceROI,leftEyeRegion,"Left Eye");
	cv::Point rightPupil = findEyeCenter(faceROI,rightEyeRegion,"Right Eye");
	// get corner regions
	cv::Rect leftRightCornerRegion(leftEyeRegion);
	leftRightCornerRegion.width -= leftPupil.x;
	leftRightCornerRegion.x += leftPupil.x;
	leftRightCornerRegion.height /= 2;
	leftRightCornerRegion.y += leftRightCornerRegion.height / 2;
	cv::Rect leftLeftCornerRegion(leftEyeRegion);
	leftLeftCornerRegion.width = leftPupil.x;
	leftLeftCornerRegion.height /= 2;
	leftLeftCornerRegion.y += leftLeftCornerRegion.height / 2;
	cv::Rect rightLeftCornerRegion(rightEyeRegion);
	rightLeftCornerRegion.width = rightPupil.x;
	rightLeftCornerRegion.height /= 2;
	rightLeftCornerRegion.y += rightLeftCornerRegion.height / 2;
	cv::Rect rightRightCornerRegion(rightEyeRegion);
	rightRightCornerRegion.width -= rightPupil.x;
	rightRightCornerRegion.x += rightPupil.x;
	rightRightCornerRegion.height /= 2;
	rightRightCornerRegion.y += rightRightCornerRegion.height / 2;
	rectangle(debugFace,leftRightCornerRegion,200);
	rectangle(debugFace,leftLeftCornerRegion,200);
	rectangle(debugFace,rightLeftCornerRegion,200);
	rectangle(debugFace,rightRightCornerRegion,200);

	DoStuffWithPupils(leftPupil, rightPupil, (leftLeftCornerRegion.height + rightRightCornerRegion.height));
	if(!showWindows) return;

	// change eye centers to face coordinates
	rightPupil.x += rightEyeRegion.x;
	rightPupil.y += rightEyeRegion.y;
	leftPupil.x += leftEyeRegion.x;
	leftPupil.y += leftEyeRegion.y;
	// draw eye centers
	circle(debugFace, rightPupil, 3, 1234);
	circle(debugFace, leftPupil, 3, 1234);

	//-- Find Eye Corners
	if (kEnableEyeCorner) {
		cv::Point2f leftRightCorner = findEyeCorner(faceROI(leftRightCornerRegion), true, false);
		leftRightCorner.x += leftRightCornerRegion.x;
		leftRightCorner.y += leftRightCornerRegion.y;
		cv::Point2f leftLeftCorner = findEyeCorner(faceROI(leftLeftCornerRegion), true, true);
		leftLeftCorner.x += leftLeftCornerRegion.x;
		leftLeftCorner.y += leftLeftCornerRegion.y;
		cv::Point2f rightLeftCorner = findEyeCorner(faceROI(rightLeftCornerRegion), false, true);
		rightLeftCorner.x += rightLeftCornerRegion.x;
		rightLeftCorner.y += rightLeftCornerRegion.y;
		cv::Point2f rightRightCorner = findEyeCorner(faceROI(rightRightCornerRegion), false, false);
		rightRightCorner.x += rightRightCornerRegion.x;
		rightRightCorner.y += rightRightCornerRegion.y;
		circle(faceROI, leftRightCorner, 3, 200);
		circle(faceROI, leftLeftCorner, 3, 200);
		circle(faceROI, rightLeftCorner, 3, 200);
		circle(faceROI, rightRightCorner, 3, 200);
	}

	imshow(face_window_name, faceROI);
}

/**
 * @function detectAndDisplay
 */
void detectAndDisplay( cv::Mat frame ) {
	std::vector<cv::Rect> faces;

	std::vector<cv::Mat> rgbChannels(3);
	cv::split(frame, rgbChannels);
	cv::Mat frame_gray = rgbChannels[2];

	face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE|CV_HAAR_FIND_BIGGEST_OBJECT, cv::Size(150, 150) );

	if(showWindows){
		for( int i = 0; i < faces.size(); i++ ) {
			rectangle(debugImage, faces[i], 1234);
		}
	}

	//-- Show what you got
	if (faces.size() > 0) {
		findEyes(frame_gray, faces[0]);
		faceCaptured = true;
	}else{
		faceCaptured = false;
	}
}

double gavg = 50.0;
double prev = 50.0;
double avgdiff = 0.0;

void DoStuffWithPupils(cv::Point left, cv::Point right, double height){
	double val = 100.0 - (left.y + right.y)/height*50.0;
	gavg = (val + gavg*3.0)*0.25;
	double diff = (gavg - prev);
	diff *= diff;

	avgdiff = (diff + avgdiff*(smoothingRatio-1.0))/smoothingRatio;

	prev = gavg;
	if(avgdiff > 20)
		fprintf(stderr, "Diff: %f\n", avgdiff);
}

double GetVal(){
	return (gavg-50.0)/50.0;
}