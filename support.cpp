#include "support.h"


void show(cv::Mat im){
    cv::imshow("img", im);
    cv::waitKey(0);
}

void write(cv::Mat im, std::string filename){
    std::stringstream ss;
    ss << "../images/test/" << filename << ".bmp";
    cv::imwrite(ss.str(), im);
}

void wait(){
    cv::waitKey(0);
}
