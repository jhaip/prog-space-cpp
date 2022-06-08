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

    void update(Database &db) {
        bool shouldRecalculate = false;

        auto calibrationWishResults = db.select({"$source wish calibration is $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"});
        if (calibrationWishResults.size() > 0) {
            auto r = calibrationWishResults[0];
            calibration[0].first = std::stof(r.get("x1").value);
            calibration[0].second = std::stof(r.get("y1").value);
            calibration[1].first = std::stof(r.get("x2").value);
            calibration[1].second = std::stof(r.get("y2").value);
            calibration[2].first = std::stof(r.get("x3").value);
            calibration[2].second = std::stof(r.get("y3").value);
            calibration[3].first = std::stof(r.get("x4").value);
            calibration[3].second = std::stof(r.get("y4").value);
            shouldRecalculate = true;
            db.retract("#" + r.get("source").value + " wish calibration is $ $ $ $ $ $ $ $");
        }

        auto seenProgramResults = db.select({"$ program $id at $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"});

        for (const auto &seenProgramResult : seenProgramResults) {
            auto id = std::stoi(seenProgramResult.get("id").value);
            if (id == 990) {
                calibration[0].first = std::stoi(seenProgramResult.get("x1").value);
                calibration[0].second = std::stoi(seenProgramResult.get("y1").value);
                shouldRecalculate = true;
            }
            if (id == 991) {
                calibration[1].first = std::stoi(seenProgramResult.get("x2").value);
                calibration[1].second = std::stoi(seenProgramResult.get("y2").value);
                shouldRecalculate = true;
            }
            if (id == 992) {
                calibration[2].first = std::stoi(seenProgramResult.get("x3").value);
                calibration[2].second = std::stoi(seenProgramResult.get("y3").value);
                shouldRecalculate = true;
            }
            if (id == 993) {
                calibration[3].first = std::stoi(seenProgramResult.get("x4").value);
                calibration[3].second = std::stoi(seenProgramResult.get("y4").value);
                shouldRecalculate = true;
            }
        }
        if (shouldRecalculate) {
            recalculate(db);
        }
    }

    void recalculate(Database &db) {
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

        db.remove_claims_from_source("09");
        db.claim(Fact{{Term{"#09"}, Term{"calibration"}, Term{"points"},
                       Term{"", std::to_string(calibration[0].first)}, Term{"", std::to_string(calibration[0].second)},
                       Term{"", std::to_string(calibration[1].first)}, Term{"", std::to_string(calibration[1].second)},
                       Term{"", std::to_string(calibration[2].first)}, Term{"", std::to_string(calibration[2].second)},
                       Term{"", std::to_string(calibration[3].first)}, Term{"", std::to_string(calibration[3].second)}}});
        db.claim(Fact{{Term{"#09"}, Term{"calibration"}, Term{"projected"}, Term{"corners"},
                       Term{"", std::to_string(projection_corrected_world_corners[0].x)}, Term{"", std::to_string(projection_corrected_world_corners[0].y)},
                       Term{"", std::to_string(projection_corrected_world_corners[1].x)}, Term{"", std::to_string(projection_corrected_world_corners[1].y)},
                       Term{"", std::to_string(projection_corrected_world_corners[2].x)}, Term{"", std::to_string(projection_corrected_world_corners[2].y)},
                       Term{"", std::to_string(projection_corrected_world_corners[3].x)}, Term{"", std::to_string(projection_corrected_world_corners[3].y)}}});
    }
};