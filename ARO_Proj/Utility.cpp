////////////////////
// Utility.cpp - implementation of the Utility namespace methods
// Last edited: 08/11/2021 by Andrew O'Kins
////////////////////

#include "stdafx.h"

#include <ctime>	// for getting current time for getCurDateTime and getCurLocalTime
#include <iostream> // std::cout in printLine

#include <opencv2\highgui\highgui.hpp>	//image processing used in FindAverageValue methods
#include <opencv2\imgproc\imgproc.hpp>

#include "Utility.h"

// [CONSOLE FEATURES]
// On a new line print a message formatted as <[LOCAL TIME]> - [MESSAGE]
const void Utility::printLine(std::string msg, bool isDebug) {
	// Comment out only when need to see debug type line printing
#ifndef _DEBUG // Only in release mode do we ignore debug only messages
	if (isDebug) {
		return;
	}
#endif
	std::cout << "\n<" << Utility::getCurLocalTime() << "> " << msg;
}

// [TIMING FEATURES]
// Return a string of formatted time label with current local date and time
std::string Utility::getCurDateTime() {
	time_t tt;
	struct tm curTime;
	time(&tt);
	//curTime = localtime(&tt); // Depreceated version
	localtime_s(&curTime, &tt);

	// https://en.cppreference.com/w/c/chrono/asctime
	char ascBuf[26]; // Buffer to hold output from asctime_s
	asctime_s(ascBuf, sizeof ascBuf, &curTime);
	std::vector<std::string> timeParts = seperateByDelim(ascBuf, ' ');
	std::vector<std::string> hourMinuteSecondParts = seperateByDelim(timeParts[3], ':');

	std::string finalTimeString = "";
	finalTimeString += timeParts[1] + "-"; // month
	finalTimeString += timeParts[2] + "-"; // date (day)
	finalTimeString += timeParts[4] + "_"; // year
	finalTimeString += std::to_string(std::stoi((std::string(hourMinuteSecondParts[0])))) + "-";	// hour in 24 format
	finalTimeString += hourMinuteSecondParts[1] + "-";  // minute
	finalTimeString += hourMinuteSecondParts[2];		// seconds

	finalTimeString.erase(std::remove(finalTimeString.begin(), finalTimeString.end(), '\n'), finalTimeString.end());

	return finalTimeString;
}

// Return a string of formatted time label with current local time
std::string Utility::getCurLocalTime() {
	time_t tt;
	struct tm curTime;
	time(&tt);
	//curTime = localtime(&tt); // Depreceated version
	localtime_s(&curTime, &tt);

	// https://en.cppreference.com/w/c/chrono/asctime
	char ascBuf[26]; // Buffer to hold output from asctime_s
	asctime_s(ascBuf, sizeof ascBuf, &curTime);
	std::vector<std::string> timeParts = seperateByDelim(ascBuf, ' ');
	std::vector<std::string> hourMinuteSecondParts = seperateByDelim(timeParts[3], ':');

	std::string finalTimeString = "";
	finalTimeString += std::to_string(std::stoi((std::string(hourMinuteSecondParts[0])))) + ":";	// hour in 24 format
	finalTimeString += hourMinuteSecondParts[1] + ":";  // minute
	finalTimeString += hourMinuteSecondParts[2];		// seconds

	finalTimeString.erase(std::remove(finalTimeString.begin(), finalTimeString.end(), '\n'), finalTimeString.end());

	return finalTimeString;
}

// Calculate an average intensity from an image taken by the camera  which can be used as a fitness value
// Input: image - pointer to the image data
//		  width - the width of the camera image in pixels
//		 height - the height of the camera image in pixels
//			  r - radius of area (centered in middle of image) to find average within
// Output: The average intensity within the calculated area
const double Utility::FindAverageValue(const void *image, const int width, const int height, const int r) {
	int value, ll, kk, cx, cy, ymin, ymax;
	double rdbl, rloop, sloop, xmin, xmax, area;
	cv::Mat m_ary = cv::Mat(int(height), int(width), CV_8UC1, (void*)image);
	rdbl = 0;
	rloop = 0;
	sloop = 0;
	cx = width / 2;
	cy = height / 2;
	area = 3.1416*pow(r, 2);
	ymin = cy - r;
	ymax = cy + r;

	//calculate average
	for (ll = ymin; ll < ymax; ll++) {
		xmin = cx - sqrt(pow(r, 2) - pow(ll - cy, 2));
		xmax = cx + sqrt(pow(r, 2) - pow(ll - cy, 2));

		for (kk = int(xmin); kk < int(xmax); kk++){
			value = m_ary.at<unsigned char>(ll, kk);
			rloop += value;
		}
	}

	rdbl = rloop / area;

	return rdbl;
}

//[STRING PROCCESING]
// Separate a string into a vector array, breaks in given character
// Input: fullString - string to seperate into parts
//		       delim - character that when found will be fulcrum to seperate the full string by
// Output: a vector of strings that are substrings of the inputted string broken up by the delim input
std::vector<std::string> Utility::seperateByDelim(std::string fullString, char delim) {
	std::vector<std::string> parts;
	std::string curPart = "";
	for (int i = 0; i < fullString.size(); i++)	{
		if (fullString[i] == delim && curPart != "") {
			parts.push_back(curPart);
			curPart = "";
		}
		else {
			curPart += fullString[i];
		}
	}
	if (curPart != "")
		parts.push_back(curPart);

	return parts;
}
