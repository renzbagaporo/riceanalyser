#include "segmentation.h"
#include "guohall.h"
#include "tracker.h"


cv::Mat segment(const cv::Mat orig){

    cv::Mat orig_img;
    cv::Mat gray_img;
    cv::Mat bw_img;

    orig_img = orig.clone();

    //Pre-process the image to obtain a grayscale and BW image
    preprocess(orig_img, gray_img, bw_img);
    cv::Mat bw_img_segmented = bw_img.clone();
    cv::threshold(bw_img_segmented, bw_img_segmented, 0, 0, CV_THRESH_BINARY);

    //Find the 4-connected blobs in bw_img
    std::vector<blob> rice_blobs = find_blobs(bw_img);

    //Iterate through each blob
    for(uint blob_iterator = 0; blob_iterator < rice_blobs.size(); blob_iterator++){
        cv::Mat blob_img = rice_blobs[blob_iterator].blob_img.clone();

        if(blob_img.rows > 3 && blob_img.cols > 3)
        {
            cv::Mat blob_inverted;
            cv::threshold(blob_img, blob_inverted, 0, 255, CV_THRESH_BINARY_INV);

            //Thin the blob
            cv::Mat blob_img_thinned;
            guohall(blob_inverted, blob_img_thinned); //Thinning using the Guo-Hall Algorithm

            //Find the endpoints
            std::vector<cv::Point2i> end_points = find_endpoints(blob_img_thinned);

            if(end_points.size() > 0){

                //Trace back from endpoint for a number of steps (default=3) or until branch point is reached, storing it as an origin point
                std::vector<cv::Point2i> origin_points;
                for(uint end_points_iterator = 0; end_points_iterator < end_points.size(); end_points_iterator++){
                    origin_points.push_back(trace(blob_img_thinned, end_points[end_points_iterator], 3));
                }

                std::vector<tracker> trackers;
                std::vector<pointpair> point_pairs;

                uint paired_trackers_no = 0;
                uint unpaired_trackers_no = 0;

                //Pair an origin point with an endpoint,and create tracker objects to push to vector
                cv::Point2i bounds(blob_img_thinned.cols, blob_img_thinned.rows);
                for(uint end_points_iterator = 0; end_points_iterator < end_points.size(); end_points_iterator++){
                    tracker t = tracker(end_points_iterator, end_points[end_points_iterator], origin_points[end_points_iterator], bounds);
                    trackers.push_back(t);
                }

                //Start pairing endpoints using trackers
                int step_multiplier = 1;
                while(true){

                    //Move each tracker
                    for(uint trackers_iterator = 0; trackers_iterator < trackers.size(); trackers_iterator++){
                        if(trackers[trackers_iterator].move(step_multiplier)){
                            trackers[trackers_iterator].set_status(1);
                            unpaired_trackers_no++;
                        }
                    }

                    //Check for collisions
                    for(uint tracker_iter_out = 0; tracker_iter_out < trackers.size()-1; tracker_iter_out++){
                        if(trackers[tracker_iter_out].get_status() == 0){
                            for(uint tracker_iter_in = tracker_iter_out+1;  tracker_iter_in < trackers.size(); tracker_iter_in++){
                                if(trackers[tracker_iter_in].get_status() == 0){
                                    if(trackers[tracker_iter_out].check_collision_with(trackers[tracker_iter_in], 5)){
                                        trackers[tracker_iter_out].set_status(2);
                                        trackers[tracker_iter_in].set_status(2);

                                        pointpair point_pair;
                                        point_pair.a = trackers[tracker_iter_out].get_reference_point();
                                        point_pair.b = trackers[tracker_iter_in].get_reference_point();

                                        point_pairs.push_back(point_pair);

                                        paired_trackers_no += 2;
                                    }
                                }
                            }
                        }
                    }
                    if(paired_trackers_no + unpaired_trackers_no >= trackers.size())
                        break;
                }

                //Generate segmenting pixels
                for(uint point_pairs_iterator = 0; point_pairs_iterator < point_pairs.size(); point_pairs_iterator++){
                    std::vector<cv::Point2i> segmenting_pixels =
                            generate_segmenting_pixels(point_pairs[point_pairs_iterator].a, point_pairs[point_pairs_iterator].b);
                    for(uint segmenting_pixels_iterator = 0; segmenting_pixels_iterator < segmenting_pixels.size(); segmenting_pixels_iterator++){
                        blob_img.at<uchar>(segmenting_pixels[segmenting_pixels_iterator].x, segmenting_pixels[segmenting_pixels_iterator].y) = 0;
                    }
                }
            }
        }

        //Combine the segmented blobs into the whole image
        cv::bitwise_or(bw_img_segmented(rice_blobs[blob_iterator].blob_rectangle), blob_img, bw_img_segmented(rice_blobs[blob_iterator].blob_rectangle));
    }
    return bw_img_segmented;
}

void preprocess(const cv::Mat orig, cv::Mat& gray, cv::Mat& bw)
{
    //Get a grayscale version of the image
    cv::cvtColor(orig, gray, cv::COLOR_BGR2GRAY);

    //Get a BW version of the image, with threshold level using Otsu's algorithm
    cv::threshold(gray, bw, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    //Remove isolated pixels by morphological opening, using a radius-5 circular kernel
    cv::Mat opening_kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5,5), cv::Point(2,2));
    cv::morphologyEx(bw, bw, cv::MORPH_OPEN, opening_kernel);

}

std::vector<blob> find_blobs(const cv::Mat bw)
{
    std::vector<blob> blobs;

    //Label connected components(blobs)
    cv::Mat bw_labeled = bw.clone();
    int no_of_labeled = cv::connectedComponents(bw, bw_labeled, 4, 4);

    //Iterate through labels
    for(int label_iterator = 1; label_iterator < no_of_labeled; label_iterator++)
    {
        //Isolate a labeled cluster
        cv::Mat bw_labeled_and_isolated;
        cv::inRange(bw_labeled, label_iterator, label_iterator,bw_labeled_and_isolated);

        //Compute for the minimum bounding rectangle, and get the rectangle subset image from bw_labeled_and_isolated
        cv::Rect cluster_bounding_rectangle = cv::boundingRect(bw_labeled_and_isolated);
        cv::Mat connected_component = bw_labeled_and_isolated(cluster_bounding_rectangle).clone();

        //Push the blob to the vector
        blobs.push_back(blob(connected_component, cv::Point2i(cluster_bounding_rectangle.x, cluster_bounding_rectangle.y)));
    }
    return blobs;
}

std::vector<cv::Point2i> find_endpoints(const cv::Mat blob_img)
{
    cv::Mat blob_img_clone = blob_img.clone();
    cv::copyMakeBorder(blob_img_clone, blob_img_clone, 1,1,1,1, cv::BORDER_CONSTANT);
    std::vector<cv::Point2i> endpoints;

    //Find the white pixels
    std::vector<cv::Point2i> white_pixels;
    cv::findNonZero(blob_img_clone, white_pixels);

    //Loop through each white pixel
    for(uint i = 0; i < white_pixels.size(); i++){
        //Find the 3x3 neighbor hood of the pixel
        cv::Mat roi = blob_img_clone(cv::Range(white_pixels[i].y-1, white_pixels[i].y +2),
                         cv::Range(white_pixels[i].x-1, white_pixels[i].x +2));

        bool is_a_border_pixel = (white_pixels[i].x-1) == 0 ||
                                 (white_pixels[i].y-1) == 0 ||
                                 (blob_img_clone.rows-2) - white_pixels[i].y == 0 ||
                                 (blob_img_clone.cols-2) - white_pixels[i].x == 0;

        if(!is_a_border_pixel)
        {
            if(white_pixels[i].x - (blob_img_clone.cols-1) != 0 && white_pixels[i].x != 0 &&
                    white_pixels[i].y - (blob_img_clone.rows-1) != 0 && white_pixels[i].y != 0)
            {
                int no_of_nonzero = cv::countNonZero(roi);
                if(no_of_nonzero == 2){
                    white_pixels[i].x -= 1;
                    white_pixels[i].y -= 1;
                    endpoints.push_back(white_pixels[i]);
                }else if(no_of_nonzero == 3){
                    bool cond1 = roi.at<uchar>(0,1) && (roi.at<uchar>(0,0) || roi.at<uchar>(0,2));
                    bool cond2 = roi.at<uchar>(1,2) && (roi.at<uchar>(0,2) || roi.at<uchar>(2,2));
                    bool cond3 = roi.at<uchar>(2,1) && (roi.at<uchar>(2,0) || roi.at<uchar>(2,2));
                    bool cond4 = roi.at<uchar>(1,0) && (roi.at<uchar>(0,0) || roi.at<uchar>(2,0));
                    if(cond1 || cond2 || cond3 || cond4){
                        white_pixels[i].x -= 1;
                        white_pixels[i].y -= 1;
                        endpoints.push_back(white_pixels[i]);
                    }
                }else if(no_of_nonzero == 4){
                    bool cond1 = roi.at<uchar>(0,1) && (roi.at<uchar>(0,0) && roi.at<uchar>(0,2));
                    bool cond2 = roi.at<uchar>(1,2) && (roi.at<uchar>(0,2) && roi.at<uchar>(2,2));
                    bool cond3 = roi.at<uchar>(2,1) && (roi.at<uchar>(2,0) && roi.at<uchar>(2,2));
                    bool cond4 = roi.at<uchar>(1,0) && (roi.at<uchar>(0,0) && roi.at<uchar>(2,0));
                    if(cond1 || cond2 || cond3 || cond4){
                        white_pixels[i].x -= 1;
                        white_pixels[i].y -= 1;
                        endpoints.push_back(white_pixels[i]);
                    }
                }else continue;
            }
        }
    }

    return endpoints;
}

cv::Point2i trace(const cv::Mat thin, const cv::Point2i start,const int traceback)
{
    cv::Point2i current_point = start;
    cv::Mat thin_img = thin.clone();

    //Pad to avoid errors
    cv::copyMakeBorder(thin_img, thin_img, 1,1,1,1, cv::BORDER_CONSTANT);

    //Compensate for the padding
    current_point.x++;
    current_point.y++;

    for(int i = 0; i < traceback-1; i++)
    {
        //Create a 3 by 3 ROI in thin_img, whose center is the current_point
        cv::Mat roi = thin_img(cv::Range(current_point.y-1, current_point.y+2),
                               cv::Range(current_point.x-1, current_point.x+2)).clone();

        int nonzero_no = cv::countNonZero(roi);

        if(nonzero_no == 2){
            //Set the center to 0
            roi.at<uchar>(1,1) = 0;

            //Find the nonzero pixel
            std::vector<cv::Point2i> nonzero_pixel;
            cv::findNonZero(roi, nonzero_pixel);

            //Set the current point to be 0
            thin_img.at<uchar>(current_point.y, current_point.x) = 0;

            //Set the location of the next current point to be the nonzero pixel
            current_point = cv::Point2i(current_point.x+(nonzero_pixel[0].x - 1),
                                        current_point.y+(nonzero_pixel[0].y - 1));
        }else if(nonzero_no == 3){
            bool cond1 = roi.at<uchar>(0,1) && (roi.at<uchar>(0,0) || roi.at<uchar>(0,2));
            bool cond2 = roi.at<uchar>(1,2) && (roi.at<uchar>(0,2) || roi.at<uchar>(2,2));
            bool cond3 = roi.at<uchar>(2,1) && (roi.at<uchar>(2,0) || roi.at<uchar>(2,2));
            bool cond4 = roi.at<uchar>(1,0) && (roi.at<uchar>(0,0) || roi.at<uchar>(2,0));
            if(cond1 || cond2 || cond3 || cond4){
                //Set the current point to be 0
                thin_img.at<uchar>(current_point.y, current_point.x) = 0;

                if(cond1) current_point = cv::Point2i(current_point.x, current_point.y-1);
                else if(cond2) current_point = cv::Point2i(current_point.x+1, current_point.y);
                else if(cond3) current_point = cv::Point2i(current_point.x, current_point.y+1);
                else if(cond4) current_point = cv::Point2i(current_point.x-1 , current_point.y);
                else break;
            }else break;
        }else if(nonzero_no == 4){
            bool cond1 = roi.at<uchar>(0,1) && (roi.at<uchar>(0,0) && roi.at<uchar>(0,2));
            bool cond2 = roi.at<uchar>(1,2) && (roi.at<uchar>(0,2) && roi.at<uchar>(2,2));
            bool cond3 = roi.at<uchar>(2,1) && (roi.at<uchar>(2,0) && roi.at<uchar>(2,2));
            bool cond4 = roi.at<uchar>(1,0) && (roi.at<uchar>(0,0) && roi.at<uchar>(2,0));
            if(cond1 || cond2 || cond3 || cond4){
                //Set the current point to be 0
                thin_img.at<uchar>(current_point.y, current_point.x) = 0;

                if(cond1) current_point = cv::Point2i(current_point.x, current_point.y-1);
                else if(cond2) current_point = cv::Point2i(current_point.x+1, current_point.y);
                else if(cond3) current_point = cv::Point2i(current_point.x, current_point.y+1);
                else if(cond4) current_point = cv::Point2i(current_point.x-1 , current_point.y);
                else break;

                break;
            }else break;
        }else break;
    }

    //Account for the earlier increment
    current_point.x--;
    current_point.y--;
    return current_point; //The last current_point is the end point of the trace.
}

std::vector<cv::Point2i> generate_segmenting_pixels(cv::Point a, cv::Point b){

    std::vector<cv::Point2i> segmenting_pixels;

    int no_of_pixels;
    int no_of_row_pixels = abs(a.y-b.y);
    int no_of_col_pixels = abs(a.x-b.x);

    //Determine the number of pixel points to be generated.
    if(no_of_row_pixels > no_of_col_pixels) no_of_pixels = no_of_row_pixels;
    else if(no_of_row_pixels < no_of_col_pixels) no_of_pixels = no_of_col_pixels;
    else no_of_pixels = no_of_row_pixels = no_of_col_pixels;

    //Generate no_of_pixels equally spaced values
    std::vector<int> row_vector = rounded_linspace(a.x, b.x, no_of_pixels);
    std::vector<int> column_vector = rounded_linspace(a.y, b.y, no_of_pixels);

    //Loop through each element, and create a cv::Point2i object
    for(uint points_iterator = 0; points_iterator < row_vector.size(); points_iterator++)
        segmenting_pixels.push_back(cv::Point2i(column_vector[points_iterator], row_vector[points_iterator]));

    return segmenting_pixels;
}

std::vector<int> rounded_linspace(double min, double max, int n){
    std::vector<int> result;
    n++;
    int iterator = 0;
    for (int i = 0; i <= n-2; i++){
        double temp = round(min + i*(max-min)/(floor((double)n) - 1));
        result.insert(result.begin() + iterator, temp);
        iterator += 1;
    }

    result.insert(result.begin() + iterator, round(max));
    return result;
}

