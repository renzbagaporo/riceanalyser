#ifndef ZHANGSUEN
#define ZHANGSUEN

#include <opencv2/opencv.hpp>

void thinningIteration(cv::Mat& img, int iter);
void zhangsuen(const cv::Mat& src, cv::Mat& dst);

#endif // ZHANGSUEN


