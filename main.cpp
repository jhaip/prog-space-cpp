#define SOL_ALL_SAFETIES_ON 1
#include <future>
#include <iostream>
#include <map>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "db.cpp"
#include "graphics.cpp"
#include "http.cpp"
#include "sourcecode.cpp"

// hack moving the definition before importing httpserver.cpp
// because httpserver.cpp depends on it as a global variable
std::mutex dbMutex;
std::mutex myMutex;
std::atomic_bool stop_cv_thread{false};
std::atomic_bool new_data_available{false};
std::vector<int> seen_program_ids;
std::vector<std::vector<cv::Point2f>> seen_program_corners;
cv::Mat latestFrame;

#include "calibration.cpp"
#include "debugwindow.cpp"
#include "httpserver.cpp"
#include "vision.cpp"

using namespace std::chrono;
using namespace cv;

int main() {
    auto r = std::async(std::launch::async, cvLoop);
    // fakeCvLoop();

    Database db{};

    int SCREEN_WIDTH = 1920;
    int SCREEN_HEIGHT = 1080;
    int CAMERA_WIDTH = 1920;
    int CAMERA_HEIGHT = 1080;

    cv::Mat main_latestFrame;
    sf::Image latestFrameImage;
    sf::Texture latestFrameTexture;
    CalibrationManager calibrationManager{SCREEN_WIDTH, SCREEN_HEIGHT, CAMERA_WIDTH, CAMERA_HEIGHT};
    DebugWindowManager debugWindowManager{SCREEN_WIDTH, SCREEN_HEIGHT, CAMERA_WIDTH, CAMERA_HEIGHT};
    GraphicsManager graphicsManager{SCREEN_WIDTH, SCREEN_HEIGHT};
    graphicsManager.init();
    SourceCodeManager sourceCodeManager({"", // 0
                                         "../../scripts/1__foxisred.lua",
                                         "../../scripts/2__isananimal.lua",
                                         "../../scripts/3__timeis.lua",
                                         "../../scripts/4__whensomeoneisafox.lua",
                                         "../../scripts/5__whentime.lua",
                                         "../../scripts/6__youisafox.lua",
                                         "../../scripts/7__darksky.lua",
                                         "", // 8
                                         "../../scripts/9__animation.lua",
                                         "../../scripts/10__outlinePrograms.lua",
                                         "../../scripts/11__counting.lua",
                                         "../../scripts/12__particle.lua",
                                         "../../scripts/13__particleFast.lua",
                                         "../../scripts/14__showWeather.lua",
                                         "../../scripts/15__drawFrame.lua",
                                         "../../scripts/16__subframeAnimation.lua",
                                         "../../scripts/17__textEditor.lua",
                                         "../../scripts/18__controlLights.lua",
                                         "../../scripts/19__controlLights2.lua",
                                         "../../scripts/20__calibration.lua",
                                         "../../scripts/21__showFps.lua",
                                         "", // 22
                                         "", // 23
                                         "", // 24
                                         "", // 25
                                         "", // 26
                                         "../../scripts/27__movie.lua",
                                         "../../scripts/28__movie.lua",
                                         "../../scripts/29__pointingAt.lua",
                                         "../../scripts/30__showPointingAt.lua"});
    sourceCodeManager.init(db);

    HTTPServer httpServerInstance(new MyRequestHandlerFactory{db}, ServerSocket(9090), new HTTPServerParams);
    httpServerInstance.start();

    float fps;
    sf::Clock clock;
    sf::Time previousTime = clock.getElapsedTime();
    sf::Time currentTime;

    int loopCount = 0;
    // while (window.isOpen() && debugWindowManager.debugWindow.isOpen()) {
    while (true) {
        currentTime = clock.getElapsedTime();
        fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds()); // the asSeconds returns a float
        // std::cout << "fps =" << floor(fps) << std::endl; // flooring it will make the frame rate a rounded number
        previousTime = currentTime;

        {
            std::scoped_lock guard(myMutex);
            if (new_data_available) {
                new_data_available = false;

                {
                    std::scoped_lock guard(dbMutex);
                    db.remove_claims_from_source("0cv");
                    int index = 0;
                    for (auto &id : seen_program_ids) {
                        cv::Point2f corner0 = seen_program_corners.at(index).at(0);
                        cv::Point2f corner1 = seen_program_corners.at(index).at(1);
                        cv::Point2f corner2 = seen_program_corners.at(index).at(2);
                        cv::Point2f corner3 = seen_program_corners.at(index).at(3);
                        db.claim("#0cv program " + std::to_string(id) + " at " +
                                 std::to_string(corner0.x) + " " + std::to_string(corner0.y) + " " +
                                 std::to_string(corner1.x) + " " + std::to_string(corner1.y) + " " +
                                 std::to_string(corner2.x) + " " + std::to_string(corner2.y) + " " +
                                 std::to_string(corner3.x) + " " + std::to_string(corner3.y));
                        index += 1;
                    }
                }

                if (!latestFrame.empty()) {
                    cv::cvtColor(latestFrame, main_latestFrame, cv::COLOR_BGR2RGBA);
                }
                latestFrameImage.create(main_latestFrame.cols, main_latestFrame.rows, main_latestFrame.ptr());
                latestFrameTexture.loadFromImage(latestFrameImage);
                // std::cout << "NEW DATA!" << std::endl;
            }
        }

        {
            std::scoped_lock guard(dbMutex);
            db.remove_claims_from_source("0");
            db.claim("#0 fps is " + std::to_string((int)fps));
            db.claim("#0 clock time is " + std::to_string(loopCount));
            loopCount += 1;
            sourceCodeManager.update(db);
            calibrationManager.update(db);
            graphicsManager.update(db, latestFrameTexture);
            debugWindowManager.update(db, latestFrameTexture);

            db.run_subscriptions();
        }

        sf::sleep(sf::seconds(1 / 60.0f) - sf::seconds(clock.getElapsedTime().asSeconds() - currentTime.asSeconds()));
    }

    std::cout << "stopping http server" << std::endl;
    httpServerInstance.stop();
    std::cout << "http server stopped" << std::endl;
    stop_cv_thread = true;
    std::cout << "waiting for CV thread to end" << std::endl;
    // r.wait();

    return 0;
}