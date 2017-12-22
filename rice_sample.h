#ifndef RICE_SAMPLE_H
#define RICE_SAMPLE_H

#include <opencv2/opencv.hpp>
#include "rice_grain.h"

class rice_sample
{
private:
    cv::Mat orig_img;
    std::vector<rice_grain> rice_grains;
public:
    rice_sample(std::string file_path);
    ~rice_sample();
};

#endif // RICE_SAMPLE_H
