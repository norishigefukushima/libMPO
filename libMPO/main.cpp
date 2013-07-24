#ifdef _DEBUG
#pragma comment(lib, "opencv_video232d.lib")
#pragma comment(lib, "opencv_ts232d.lib")
#pragma comment(lib, "opencv_objdetect232d.lib")
#pragma comment(lib, "opencv_ml232d.lib")
#pragma comment(lib, "opencv_legacy232d.lib")
#pragma comment(lib, "opencv_imgproc232d.lib")
#pragma comment(lib, "opencv_highgui232d.lib")
#pragma comment(lib, "opencv_haartraining_engine.lib")
#pragma comment(lib, "opencv_gpu232d.lib")
#pragma comment(lib, "opencv_flann232d.lib")
#pragma comment(lib, "opencv_features2d232d.lib")
#pragma comment(lib, "opencv_core232d.lib")
#pragma comment(lib, "opencv_contrib232d.lib")
#pragma comment(lib, "opencv_calib3d232d.lib")
#else
#pragma comment(lib, "opencv_video232.lib")
#pragma comment(lib, "opencv_ts232.lib")
#pragma comment(lib, "opencv_objdetect232.lib")
#pragma comment(lib, "opencv_ml232.lib")
#pragma comment(lib, "opencv_legacy232.lib")
#pragma comment(lib, "opencv_imgproc232.lib")
#pragma comment(lib, "opencv_highgui232.lib")
#pragma comment(lib, "opencv_haartraining_engine.lib")
#pragma comment(lib, "opencv_gpu232.lib")
#pragma comment(lib, "opencv_flann232.lib")
#pragma comment(lib, "opencv_features2d232.lib")
#pragma comment(lib, "opencv_core232.lib")
#pragma comment(lib, "opencv_contrib232.lib")
#pragma comment(lib, "opencv_calib3d232.lib")
#endif


#include "mpoio.h"

using namespace cv;
using namespace std;

int main(int argc, char** argv)
{
	//OpenCV1.0
	IplImage**a = cvLoadMPO("sakura.MPO");
	cvShowImage("L",a[0]);
	cvShowImage("R",a[1]);
	cvWaitKey();


	//OpenCV2.0
	vector<Mat> aa=mporead("sakura.MPO");
	imshow("L",aa[0]);
	imshow("R",aa[1]);
	waitKey();

	return 0;
}