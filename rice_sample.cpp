#include "rice_sample.h"
#include "rice_grain.h"
#include "segmentation.h"


rice_sample::rice_sample(std::string file_path)
{
    //Read image
    orig_img = cv::imread(file_path, cv::IMREAD_COLOR);

    //Segment the image
    cv::Mat bw_img_segmented = segment(orig_img);

    //Identify the rice blobs in the image
    std::vector<blob> blobs = find_blobs(bw_img_segmented);

    //From the blobs, make the grains
    /*
    for(uint blobs_iterator = 0; blobs_iterator < blobs.size(); blobs_iterator++){
        rice_grain  grain = rice_grain(orig_img, blobs[blobs_iterator]);
        rice_grains.push_back(grain);
    }*/
}

rice_sample::~rice_sample()
{

}
