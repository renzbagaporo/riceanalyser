#include "rice_grain.h"
#include "support.h"

rice_grain::rice_grain(cv::Mat orig, blob b, length_params len_params, color_params col_params, ratio_params rat_params)
{
    grain_img = orig(b.blob_rectangle).clone();
    grain_img_mask = b.blob_img;
    grain_rectangle = b.blob_rectangle;

    //Find the minimum area rotated rectangle
    std::vector<cv::Point2i> nonzero_pixels;
    cv::findNonZero(grain_img_mask, nonzero_pixels);
    grain_min_rectangle = cv::minAreaRect(nonzero_pixels);

    //Initialize analysis parameters
    length_parameters = len_params;
    color_parameters = col_params;
    ratio_parameters = rat_params;

    //Determine whether the grain is single grain

    //Begin grain analysis of ratio, length and color
    classify_color();
    classify_length();
    classify_ratio();
}

void rice_grain::classify_color(){

    if(is_single_grain){
        //Convert the color space from BGR(OpenCV default) to HSV
        cv::Mat grain_img_hsv;
        cv::cvtColor(grain_img, grain_img_hsv, cv::COLOR_BGR2HSV);

        //Get the average value of each channel
        cv::Scalar channels_sum = cv::sum(grain_img_hsv);
        int nonzero_pixels_count = cv::countNonZero(grain_img_mask);
        double hue_ave = channels_sum[0]/nonzero_pixels_count;
        double sat_ave = channels_sum[1]/nonzero_pixels_count;
        double val_ave = channels_sum[2]/nonzero_pixels_count;

        //Classify based on obtained values
        if(sat_ave < color_parameters.sat_limit){
            if(val_ave > color_parameters.val_limit){
                grain_color_class = chalky;
            }else{
                grain_color_class = sound;
            }
        }else{
            if(hue_ave > color_parameters.paddy_hue_min && hue_ave < color_parameters.paddy_hue_max && val_ave > color_parameters.paddy_val_min && val_ave < color_parameters.paddy_val_max){
                grain_color_class = paddy;
            }else if(hue_ave > color_parameters.discolored_hue_min && hue_ave < color_parameters.discolored_hue_max && val_ave > color_parameters.discolored_val_min && val_ave < color_parameters.discolored_val_max){
                grain_color_class = discolored;
            }else if(val_ave > color_parameters.red_val_min && val_ave < color_parameters.red_val_max && sat_ave > color_parameters.red_sat_min && sat_ave < color_parameters.red_sat_max){
                grain_color_class = red;
            }else grain_color_class = undefined_color;
        }
    }

    grain_color_class = undefined_color;
}

void rice_grain::classify_length(){
    if(is_single_grain){
        double grain_length = grain_min_rectangle.size.height;

        //Classify according to length
        if(grain_length >= length_parameters.headrice_thresh * length_parameters.grain_length_ave) grain_length_class = headrice;
        else if(grain_length >= length_parameters.broken_thresh * length_parameters.grain_length_ave) grain_length_class =  broken;
        else if (grain_length >= length_parameters.brewers_thresh * length_parameters.grain_length_ave) grain_length_class =  brewers;
        else grain_length_class = undefined_length;
    }

   grain_length_class = undefined_length;
}

void rice_grain::classify_ratio(){
    if(is_single_grain){
        //Put code for classifying the ratio here
    }
    grain_ratio_class = undefined_ratio;
}

cv::Mat rice_grain::get_grain_img(){
    return grain_img.clone();
}

cv::Mat rice_grain::get_grain_mask(){
    return grain_img_mask.clone();
}

cv::Rect rice_grain::get_grain_rectangle(){
    return grain_rectangle;
}

cv::RotatedRect rice_grain::get_grain_min_rectangle(bool apply_offset){

    if(apply_offset){
        cv::RotatedRect min_rectangle_with_offset = grain_min_rectangle;
        min_rectangle_with_offset.center.x += grain_rectangle.x;
        min_rectangle_with_offset.center.y += grain_rectangle.y;
    }

    return grain_min_rectangle;
}

rice_grain::~rice_grain()
{

}





