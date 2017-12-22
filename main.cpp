#include <QCoreApplication>
#include <QDebug>

#include "segmentation.h"
#include "rice_sample.h"

using namespace cv;
using namespace std;

int main()
{
    cv::Mat im = segment(cv::imread("../images/test/new.jpg"));
    show(im);
}
