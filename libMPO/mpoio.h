#include <opencv2/opencv.hpp>


void cvReleaseMPO(IplImage** src);
IplImage** cvLoadMPO(char* name);
std::vector<cv::Mat> mporead(char* name,int flags=1);
