#ifndef _GL__H_
#define _GL__H_

/* all the libraries */
#include <cstdlib>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <wiringPi.h>
#include <wiringSerial.h>

/* define a hand */
class Hand
{
    public:
        Hand(){};
        ~Hand(){};
        std::vector<cv::Point> fingers;
        cv::Point center;
        std::vector<cv::Point> contour;
};

/* Class for detecting hands and gestures for OpenCV */
class GestureRecog
{
    public:
        struct Param
        {
            int area;
            int r;
            int step;
            double cosThresh;
            double equalThresh;
        };

    public:
        GestureRecog(){};
        ~GestureRecog(){};

        void detect(cv::Mat& mask, std::vector<Hand>& hands);
        void setParams(Param &p);

    private:
        Param param;
        signed int rotation( std::vector<cv::Point>& contour, int pt, int r );
        double angle( std::vector<cv::Point>& contour,int pt, int r );
        bool isEqual( double a, double b );
};

void drawHands(cv::Mat& image, std::vector<Hand>& hands);

#endif
