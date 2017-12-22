#ifndef TRACKER_H
#define TRACKER_H

#include <opencv2/opencv.hpp>

class tracker
{
private:

    uint tracker_tag;

    cv::Point2d current_point;
    cv::Point2i bounds_point;
    cv::Point2i reference_point;
    cv::Point2i origin_point;

    std::vector<cv::Point2i> previous_points;
    double row_increment;
    double column_increment;

    /* Status
     * 0 - No collision
     * 1 - Collision with border
     * 2 - Collision with another tracker
     */
    ushort status;

public:
    tracker();
    tracker(int tracker_tag, cv::Point2i reference_point, cv::Point2i origin_point, cv::Point2i bounds_point);
    ~tracker();

    void calculate_increments();
    bool move(double step);
    bool check_collision_with(tracker tracker_to_check, double radius_to_check);

    int get_status();
    void set_status(int status);
    cv::Point2d get_current_point();
    cv::Point2i get_reference_point();
    std::vector<cv::Point2i> get_previous_points();
};

#endif // TRACKER_H
