#include "tracker.h"

tracker::tracker(){}

tracker::tracker(int tag, cv::Point2i ref, cv::Point2i start, cv::Point2i bounds)
{
    tracker_tag = tag;

    reference_point = ref;
    origin_point = start;
    current_point = cv::Point2d(start.x, start.y);
    bounds_point = bounds;

    status = 0;

    calculate_increments();
}

void tracker::calculate_increments()
{
    double rise = reference_point.y - origin_point.y;
    double run = reference_point.x - origin_point.x;

    if(rise == 0 && run != 0){
        row_increment = 0;
        (run > 0) ? column_increment = 1 : column_increment = -1;
    }
    else if (run == 0 && rise != 0){
        column_increment = 0;
        (rise > 0) ? row_increment = 1 : row_increment = -1;
    }
    else if (run != 0 && rise != 0){
        double slope_abs = fabs(rise/run);
        if(slope_abs > 1) //Slope is rise dominant
        {
            double slope_reciprocal = 1/slope_abs;
            if(rise > 0 && run > 0){
                row_increment = 1;
                column_increment = slope_reciprocal;
            }
            else if (rise > 0 && run < 0){
                row_increment = 1;
                column_increment = -slope_reciprocal;
            }
            else if (rise < 0 && run > 0){
                row_increment = -1;
                column_increment = slope_reciprocal;
            }
            else if (rise < 0 && run < 0){
                row_increment = -1;
                column_increment = -slope_reciprocal;
            }
            else return;
        } else //Slope is run dominant
        {
            if(rise > 0 && run > 0){
                row_increment = slope_abs;
                column_increment = 1;
            }
            else if (rise > 0 && run < 0){
                row_increment =slope_abs;
                column_increment = -1;
            }
            else if (rise < 0 && run > 0){
                row_increment = -slope_abs;
                column_increment = 1;
            }
            else if (rise < 0 && run < 0){
                row_increment = -slope_abs;
                column_increment = -1;
            }
            else return;
        }
    }
    else
        return;
}

bool tracker::move(double step)
{
    if(status == 0) // Only move when the tracker has not yet collided
    {
        current_point.y += row_increment * step;
        current_point.x += column_increment * step;

        cv::Point to_push = cv::Point(current_point.x, current_point.y);
        previous_points.push_back(to_push);
        //Check if point is still within bounds
        bool not_exceeding_left  = current_point.x >= 1;
        bool not_exceeding_right  = bounds_point.x - current_point.x >= 1;
        bool not_exceeding_top  = current_point.y - 0 >= 1;
        bool not_exceeding_bottom  = bounds_point.y - current_point.y >= 1;
        if(!(not_exceeding_left && not_exceeding_right && not_exceeding_top && not_exceeding_bottom)){
            //Tracker has already reached bounds; Undo the previous step
            current_point.y -= row_increment * step;
            current_point.x -= column_increment * step;

            previous_points.pop_back();
            return true;
        }
    }
    return false;
}

bool tracker::check_collision_with(tracker t, double rad)
{
    std::vector<cv::Point2i> t_previous_points = t.get_previous_points();
    for(uint t_iter = 0; t_iter < t_previous_points.size(); t_iter++){
        //Pythagorean theorem
        double distance = sqrt(pow(current_point.x - t_previous_points[t_iter].x, 2.0) + pow(current_point.y - t_previous_points[t_iter].y, 2.0));
        if(distance <= rad){
            return true;
        }
    }
    return false;
}

int tracker::get_status(){
    return status;
}

void tracker::set_status(int stat){
    status = stat;
}

cv::Point2d tracker::get_current_point(){
    return current_point;
}

cv::Point2i tracker::get_reference_point(){
    return reference_point;
}

std::vector<cv::Point2i> tracker::get_previous_points(){
    return previous_points;
}

tracker::~tracker(){}
