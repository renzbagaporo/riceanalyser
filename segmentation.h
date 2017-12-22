#ifndef SEGMENTATION
#define SEGMENTATION

#include <opencv2/opencv.hpp>

struct pointpair{
    cv::Point2i a;
    cv::Point2i b;
};

struct blob{
    cv::Mat blob_img;
    cv::Point2i blob_location;
    cv::Rect blob_rectangle;

    blob(const cv::Mat blobimg, cv::Point2i location){
        blob_img = blobimg;
        blob_location = location;
        blob_rectangle = cv::Rect(location.x, location.y, blobimg.cols, blobimg.rows);
    }
};

cv::Mat segment(const cv::Mat);
void preprocess(const cv::Mat orig_img,cv::Mat& gray_img,cv::Mat& bw_img);
std::vector<blob> find_blobs(const cv::Mat bw_img);
std::vector<cv::Point2i> find_endpoints(const cv::Mat blob_img);
cv::Point2i trace(const cv::Mat thin_img, const cv::Point2i start_point, const int traceback_no);
std::vector<cv::Point2i> generate_segmenting_pixels(cv::Point a, cv::Point b);
std::vector<int> rounded_linspace(double min, double max, int n);
void show(cv::Mat);
void write(cv::Mat, std::string);

#endif // SEGMENTATION

