/************* gesture controlled light program ************
 * Author: Justin Dobson ***********************************
 ***********************************************************/

#include "gl.h"
#define EVER (;;)

/* Required by opencv */
using namespace std;
using namespace cv;

/* file descriptor for wiringSerial */
int fd;


/* Method for setting up parameters used by contour algorithms */
void GestureRecog::setParams( GestureRecog::Param &p )
{
    param.area = p.area;
    param.cosThresh = p.cosThresh;
    param.equalThresh = p.equalThresh;
    param.r = p.r;
    param.step = p.step;
}

/* Checks if doubles are equal, fabs() returns abs value */
bool GestureRecog::isEqual(double a, double b)
{
    return fabs(a-b) <= param.equalThresh;
}

/* Handles calculations for rotations in the image */
signed int GestureRecog::rotation(std::vector<cv::Point>& contour, int pt, int r)
{
    int size =  contour.size();
    Point p0 = (pt>0)?contour[pt%size]:contour[size-1+pt];
    Point p1 = contour[(pt+r)%size];
    Point p2 = (pt>r)?contour[pt-r]:contour[size-1-r];

    double firstX = p0.x-p1.x;
    double firstY = p0.y-p1.y;
    double secondX = p0.x-p2.x;
    double secondY = p0.y-p2.y;

    return(firstX*secondY - secondX*firstY);
}

/* Finds angle of contour*/
double GestureRecog::angle(std::vector<Point>& contour, int pt, int r)
{
    int size = contour.size();
    Point p0 = (pt > 0)?contour[pt%size]:contour[size-1+pt];
    Point p1 = contour[(pt+r)%size];
    Point p2 = (pt > r)?contour[pt-r]:contour[size-1-r];

    double firstX = p0.x-p1.x;
    double firstY = p0.y-p1.y;
    double secondX = p0.x-p2.x;
    double secondY = p0.y-p2.y;

    return (firstX*secondY + firstY*secondY)/sqrt((firstX*firstX + firstY*firstY)*(secondX*secondX + secondY*secondY));
}

/* detects hands against background */
void GestureRecog::detect(Mat &mask, vector<Hand>& hands)
{
    hands.clear();
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;


    findContours( mask.clone(), contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0) );
    if(!contours.empty())
    {
        for( int i = 0; i < contours.size(); i++ )
        {
            if ( contourArea(contours[i]) > param.area )
            {
                Hand temp;
                Moments m = moments( contours[i] );

                temp.center.x = m.m10/m.m00;
                temp.center.y = m.m01/m.m00;

                for( int j = 0; j < contours[i].size(); j += param.step )
                {
                    double cos0 = angle(contours[i], j, param.r);

                    if( ( cos0 > 0.5 ) && ( j+param.step<contours[i].size() ) )
                    {
                        double cos1 = angle( contours[i], j - param.step, param.r ); 
                        double cos2 = angle( contours[i], j + param.step, param.r );
                        double maxCos = max( max(cos0, cos1), cos2 );

                        bool equal = isEqual( maxCos, cos0 );
                        signed int z = rotation(contours[i], j, param.r);
                        if ( equal && z < 0 )
                        {
                            /* push_back part of vector, adds new element after last, moves conect ot last into it */
                            temp.fingers.push_back( contours[i][j] );
                        }
                    }
                }
                temp.contour = contours[i];
                hands.push_back(temp);
            }
        }
    }
}


/* recognizes hands based on imgs recieved */
void drawHands(Mat& img, vector<Hand> &hands)
{
    int size = hands.size();
    vector< vector<Point> > c;
    for( int i = 0; i < size; i++ )
    {
        c.clear();
        c.push_back(hands[i].contour);
        circle(img, hands[i].center, 20, Scalar(0, 0, 255), 3);
        int fingerSize = hands[i].fingers.size();
        for( int j=0; j<fingerSize; j++ )
        {
            circle(img, hands[i].fingers[j], 10, Scalar(0, 0, 255), 3);
            line(img, hands[i].center, hands[i].fingers[j], Scalar(0, 0, 255), 3);
        }
        cout << hands[i].fingers.size() << endl;

        /* put value corrisoping to gesture on the wire */
	    serialPutchar(fd, (char)hands[i].fingers.size());
    }
}

int main()
{
    wiringPiSetup();


    if( (fd = serialOpen("/dev/ttyACM0", 9600)) < 0 )
    {
        printf( " Unable to open serial device specified" );
        return 1;
    }

    VideoCapture cap(0);

    if( !cap.isOpened() )
    {
        printf("video capture failed");
        return 1;
    }

    GestureRecog::Param p;
    p.area = 1000;
    p.cosThresh = 0.5;
    p.equalThresh = 1e-7;
    p.r = 40;
    p.step = 16;

    GestureRecog gr;
    gr.setParams(p);
    vector<Hand> hands;
    Mat color;
    namedWindow("edges", 0);
    resizeWindow("edges", 200,200);

    for EVER
    {
	    if ( cap.read(color) )
	    {
        	Mat img;
        	cap.grab();
        	cap.retrieve( color, CV_16UC1);
        	cap.retrieve( img, CV_32SC1);

        	Mat temp;

        	cvtColor( img, temp, COLOR_BGR2GRAY );
        
        	threshold( temp, temp, 60, 180, THRESH_BINARY );
        	gr.detect(temp, hands);

        	if( !hands.empty() )
        	{
            		drawHands( temp, hands );
            		drawHands( color, hands );
        	}	

        	imshow( "edges", color );

        	if( waitKey(30) >= 0 )  //use this as while instead of the for
		    { 
	    	    serialClose(fd);
	    	    break;
		    }
	    }	
    }
    return 0;
}
