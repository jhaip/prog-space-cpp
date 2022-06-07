#include <opencv2/aruco.hpp>
#include <opencv2/core/core.hpp>

void cvLoop() {
    VideoCapture inputVideo;
    inputVideo.open(0);
    cv::Mat cameraMatrix;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_1000);
    while (!stop_cv_thread && inputVideo.grab()) {
        cv::Mat image, imageCopy;
        inputVideo.retrieve(image);
        image.copyTo(imageCopy);
        // std::cout << "width: " << imageCopy.cols << " height: " << imageCopy.rows << std::endl;
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);
        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);
            // std::cout << "Seen ID" << ids[0] << std::endl;
        }
        {
            std::scoped_lock guard(myMutex);
            seen_program_ids = ids;
            seen_program_corners = corners;
            imageCopy.copyTo(latestFrame);
            new_data_available = true;
        }
    }
    std::cout << "cv thread died" << std::endl;
}

void fakeCvLoop() {
    std::scoped_lock guard(myMutex);
    std::vector<int> my_seen_program_ids{17, 3, 21};
    std::vector<std::vector<cv::Point2f>> my_seen_program_corners{
        {cv::Point2f{0, 50}, cv::Point2f{1, 50}, cv::Point2f{1, 51}, cv::Point2f{0, 51}},
        {cv::Point2f{0, 0}, cv::Point2f{1, 0}, cv::Point2f{1, 1}, cv::Point2f{0, 1}},
        {cv::Point2f{0, 300}, cv::Point2f{1, 300}, cv::Point2f{1, 301}, cv::Point2f{0, 301}}};
    seen_program_ids = my_seen_program_ids;
    seen_program_corners = my_seen_program_corners;
    new_data_available = true;
}