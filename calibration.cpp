#include <opencv2/core/core.hpp>

using namespace cv;

class CalibrationManager {
  public:
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;
    int CAMERA_WIDTH;
    int CAMERA_HEIGHT;
    std::vector<std::pair<float, float>> calibration; // TL TR BR BL
    std::vector<cv::Point2f> projection_corrected_world_corners;
    cv::Mat calibrationMatrix;

    CalibrationManager(int _SCREEN_WIDTH, int _SCREEN_HEIGHT, int _CAMERA_WIDTH, int _CAMERA_HEIGHT) : SCREEN_WIDTH(_SCREEN_WIDTH),
                                                                                                       SCREEN_HEIGHT(_SCREEN_HEIGHT),
                                                                                                       CAMERA_WIDTH(_CAMERA_WIDTH),
                                                                                                       CAMERA_HEIGHT(_CAMERA_HEIGHT) {
        calibration = {{0, 0}, {SCREEN_WIDTH, 0}, {SCREEN_WIDTH, SCREEN_HEIGHT}, {0, SCREEN_HEIGHT}}; // TL TR BR BL
        projection_corrected_world_corners = {
            cv::Point2f(0, 0),
            cv::Point2f(SCREEN_WIDTH, 0),
            cv::Point2f(SCREEN_WIDTH, SCREEN_HEIGHT),
            cv::Point2f(0, SCREEN_HEIGHT)}; // TL TR BR BL
    }

    void checkForArucoCalibration(std::vector<int> &main_seen_program_ids, std::vector<std::vector<cv::Point2f>> &main_seen_program_corners) {
        int index = 0;
        for (auto &id : main_seen_program_ids) {
            cv::Point2f corner0 = main_seen_program_corners.at(index).at(0);
            cv::Point2f corner1 = main_seen_program_corners.at(index).at(1);
            cv::Point2f corner2 = main_seen_program_corners.at(index).at(2);
            cv::Point2f corner3 = main_seen_program_corners.at(index).at(3);
            if (id == 990) {
                calibration[0].first = corner0.x;
                calibration[0].second = corner0.y;
            }
            if (id == 991) {
                calibration[1].first = corner1.x;
                calibration[1].second = corner1.y;
            }
            if (id == 992) {
                calibration[2].first = corner2.x;
                calibration[2].second = corner2.y;
            }
            if (id == 993) {
                calibration[3].first = corner3.x;
                calibration[3].second = corner3.y;
            }
            index += 1;
        }
    }

    void recalculate() {
        float size = 10;
        std::vector<cv::Point2f> screen_vertices{
            cv::Point2f{0 + size, 0 + size},                                      // topLeft
            cv::Point2f{(float)SCREEN_WIDTH - size, 0 + size},                    // Top Right
            cv::Point2f{(float)SCREEN_WIDTH - size, (float)SCREEN_HEIGHT - size}, // bottomRight
            cv::Point2f{0 + size, (float)SCREEN_HEIGHT - size}                    // bottomLeft
        };

        std::vector<cv::Point2f> camera_vertices{
            cv::Point2f{0, 0},                                      // topLeft
            cv::Point2f{(float)CAMERA_WIDTH, 0},                    // Top Right
            cv::Point2f{(float)CAMERA_WIDTH, (float)CAMERA_HEIGHT}, // bottomRight
            cv::Point2f{0, (float)CAMERA_HEIGHT}                    // bottomLeft
        };

        std::vector<cv::Point2f> dst_vertices{
            cv::Point2f{calibration[0].first, calibration[0].second},
            cv::Point2f{calibration[1].first, calibration[1].second},
            cv::Point2f{calibration[2].first, calibration[2].second},
            cv::Point2f{calibration[3].first, calibration[3].second}};

        calibrationMatrix = getPerspectiveTransform(dst_vertices, screen_vertices);
        std::cout << "cal matrix: " << calibrationMatrix << std::endl;

        perspectiveTransform(camera_vertices, projection_corrected_world_corners, calibrationMatrix);

        std::cout << projection_corrected_world_corners[0] << std::endl;
        std::cout << projection_corrected_world_corners[1] << std::endl;
        std::cout << projection_corrected_world_corners[2] << std::endl;
        std::cout << projection_corrected_world_corners[3] << std::endl;

        std::cout << "calibration:" << std::endl;
        std::cout << calibration[0].first << " " << calibration[0].second << std::endl;
        std::cout << calibration[1].first << " " << calibration[1].second << std::endl;
        std::cout << calibration[2].first << " " << calibration[2].second << std::endl;
        std::cout << calibration[3].first << " " << calibration[3].second << std::endl;
        std::cout << "---" << std::endl;
    }
};