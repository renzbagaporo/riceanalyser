#ifndef GUOHALL
#define GUOHALL

#include <opencv2/opencv.hpp>

void thinningGuoHallIteration(cv::Mat& im, int iter);
void guohall(cv::Mat& src, cv::Mat& im);

#endif // GUOHALL


