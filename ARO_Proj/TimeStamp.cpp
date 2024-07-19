////////////////////
// TimeStamp.cpp - implementation file for TimeStamp.h
////////////////////

#include "stdafx.h"
#include "TimeStamp.h"

#include <iomanip> // setprecision
#include <sstream> // std::ostringstream in GetLabel()

//Constructor
TimeStamp::TimeStamp(double duration, std::string label) {
	this->duration = duration;
	this->label = label;
}

//Geting time taken
double TimeStamp::GetDurationSec() {
	return duration / 1000;
}

double TimeStamp::GetDurationMSec() {
	return duration;
}

// Get a string format label with time in milliseconds
std::string TimeStamp::GetLabel() {
	//Properly convert interval amount to string
	std::ostringstream ss;
	// references to remove trailing zeros from double to string converion: https://thispointer.com/c-convert-double-to-string-and-manage-precision-scientific-notation/
	ss << std::fixed << std::setprecision(4) << duration;
	std::string curNum = ss.str();

	//Add white space to small intervals with no thousandth value (needs to be atleast 8 charecters in length)
	if (curNum.length() < 10) {
		while (curNum.length() < 10) {
			curNum = " " + curNum;
		}
	}
	return  curNum + " MS - " + label;
}
