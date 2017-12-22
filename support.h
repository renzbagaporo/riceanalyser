#ifndef SUPPORT
#define SUPPORT

#include <opencv2/opencv.hpp>

void show(cv::Mat im);
void write(cv::Mat im, std::string filename);
void write(cv::Mat im, std::string filename, int seq_no);

#endif // SUPPORT

