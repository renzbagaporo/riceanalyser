#ifndef RICE_CLUSTER_H
#define RICE_CLUSTER_H

#include <opencv2/opencv.hpp>
#include "segmentation.h"

enum grain_color_classes{
    chalky, sound, paddy, discolored, red, damaged, undefined_color
};

enum grain_length_classes{
    headrice, broken, brewers, undefined_length
};

enum grain_ratio_classes{
    short_grain, medium_grain, long_grain, undefined_ratio
};

struct length_params{
    float headrice_thresh;
    float broken_thresh;
    float brewers_thresh;
    float grain_length_ave;
};

struct color_params{
   float sat_limit;
   float val_limit;

   float sat_min;

   float paddy_hue_min;
   float paddy_hue_max;
   float paddy_val_min;
   float paddy_val_max;

   float discolored_hue_min;
   float discolored_hue_max;
   float discolored_val_min;
   float discolored_val_max;

   float red_val_min;
   float red_val_max;
   float red_sat_min;
   float red_sat_max;
};

struct ratio_params{
    float short_thresh;
    float medium_thresh;
    float long_thresh;
};

class rice_grain
{

private:
    cv::Mat grain_img;
    cv::Mat grain_img_mask;
    cv::Rect grain_rectangle;
    cv::RotatedRect grain_min_rectangle;

    grain_color_classes grain_color_class;
    grain_length_classes grain_length_class;
    grain_length_classes grain_ratio_class;

    length_params length_parameters;
    color_params color_parameters;
    ratio_params ratio_parameters;

    bool is_single_grain;

public:

    rice_grain(cv::Mat sample_img, blob grain_blob, length_params length_parameters, color_params color_parameters, ratio_params ratio_parameters);
    ~rice_grain();

    void classify_color();
    void classify_length();
    void classify_ratio();


    grain_color_classes get_color_class();
    grain_length_classes get_length_class();

    cv::Rect get_grain_rectangle();
    cv::RotatedRect get_grain_min_rectangle(bool apply_offset);
    cv::Mat get_grain_img();
    cv::Mat get_grain_mask();
};

#endif // RICE_CLUSTER_H
