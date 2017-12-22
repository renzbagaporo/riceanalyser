### What is this repository for? ###

* This repository contains the C++ equivalent of the MATLAB segmentation algorithm and rice grain analysis logic.

### How do I get set up? ###

*The project in this repository was created using Qt Creator. To open, just navigate to the rice-analysis.pro file.

*This project uses OpenCV3 RC1. This version is necessary as this project uses a function, cv::connectedComponents(), not found in earlier versions. Reconfiguration of the rice-analysis.pro file might be necessary, depending on the location of the OpenCV include files and libraries in the system.
