#define SOL_ALL_SAFETIES_ON 1
#include <future>
#include <iostream>
#include <map>
#include <math.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "calibration.cpp"
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

    std::vector<int> main_seen_program_ids;
    std::vector<std::vector<cv::Point2f>> main_seen_program_corners;
    cv::Mat main_latestFrame;
    sf::Image latestFrameImage;
    sf::Texture latestFrameTexture;
    sf::Sprite latestFrameSprite;
    CalibrationManager calibrationManager{SCREEN_WIDTH, SCREEN_HEIGHT, CAMERA_WIDTH, CAMERA_HEIGHT};
    DebugWindowManager debugWindowManager{CAMERA_WIDTH, CAMERA_HEIGHT};
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
                                         "../../scripts/21__showFps.lua"});
    sourceCodeManager.init(db);

    HTTPServer httpServerInstance(new MyRequestHandlerFactory{db}, ServerSocket(9090), new HTTPServerParams);
    httpServerInstance.start();

    sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "SFML works!");

    float fps;
    sf::Clock clock;
    sf::Time previousTime = clock.getElapsedTime();
    sf::Time currentTime;

    int loopCount = 0;
    window.setFramerateLimit(60);
    while (window.isOpen() && debugWindowManager.debugWindow.isOpen()) {
        currentTime = clock.getElapsedTime();
        fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds()); // the asSeconds returns a float
        // std::cout << "fps =" << floor(fps) << std::endl; // flooring it will make the frame rate a rounded number
        previousTime = currentTime;

        std::vector<int> newlySeenPrograms;
        std::vector<int> programsThatDied;
        {
            std::scoped_lock guard(myMutex);
            if (new_data_available) {
                new_data_available = false;
                for (auto &id : seen_program_ids) {
                    if (std::find(main_seen_program_ids.begin(), main_seen_program_ids.end(), id) == main_seen_program_ids.end()) {
                        newlySeenPrograms.emplace_back(id);
                    }
                }
                for (auto &id : main_seen_program_ids) {
                    if (std::find(seen_program_ids.begin(), seen_program_ids.end(), id) == seen_program_ids.end()) {
                        programsThatDied.emplace_back(id);
                    }
                }
                main_seen_program_ids = seen_program_ids;
                main_seen_program_corners = seen_program_corners;

                int index = 0;
                for (auto &id : main_seen_program_ids) {
                    cv::Point2f corner0 = main_seen_program_corners.at(index).at(0);
                    cv::Point2f corner1 = main_seen_program_corners.at(index).at(1);
                    cv::Point2f corner2 = main_seen_program_corners.at(index).at(2);
                    cv::Point2f corner3 = main_seen_program_corners.at(index).at(3);
                    db.claim("#0 program " + std::to_string(id) + " at " +
                             std::to_string(corner0.x) + " " + std::to_string(corner0.y) + " " +
                             std::to_string(corner1.x) + " " + std::to_string(corner1.y) + " " +
                             std::to_string(corner2.x) + " " + std::to_string(corner2.y) + " " +
                             std::to_string(corner3.x) + " " + std::to_string(corner3.y));
                    index += 1;
                }
                loopCount += 1;

                if (!latestFrame.empty()) {
                    cv::cvtColor(latestFrame, main_latestFrame, cv::COLOR_BGR2RGBA);
                }
                latestFrameImage.create(main_latestFrame.cols, main_latestFrame.rows, main_latestFrame.ptr());
                if (latestFrameTexture.loadFromImage(latestFrameImage)) {
                    latestFrameSprite.setTexture(latestFrameTexture);
                }
                // std::cout << "NEW DATA!" << std::endl;
            }
        }

        std::vector<QueryResult> genericGraphicsWishes;

        {
            std::scoped_lock guard(dbMutex);
            db.cleanup("0");

            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Enter) {
                        std::cout << "Enter pressed" << std::endl;
                        db.print();
                        db.claim(Fact{"#0 keyboard typed key ENTER"});
                    } else if (event.key.code == sf::Keyboard::Backspace) {
                        db.claim(Fact{"#0 keyboard typed key BACKSPACE"});
                    } else if (event.key.code == sf::Keyboard::Space) {
                        db.claim(Fact{"#0 keyboard typed key SPACE"});
                    } else if (event.key.code == sf::Keyboard::Tab) {
                        db.claim(Fact{"#0 keyboard typed key TAB"});
                    } else if (event.key.code == sf::Keyboard::Right) {
                        db.claim(Fact{"#0 keyboard typed key RIGHT"});
                    } else if (event.key.code == sf::Keyboard::Left) {
                        db.claim(Fact{"#0 keyboard typed key LEFT"});
                    } else if (event.key.code == sf::Keyboard::Up) {
                        db.claim(Fact{"#0 keyboard typed key UP"});
                    } else if (event.key.code == sf::Keyboard::Down) {
                        db.claim(Fact{"#0 keyboard typed key DOWN"});
                    } else if (event.key.control) {
                        if (event.key.code == sf::Keyboard::P) {
                            db.claim(Fact{"#0 keyboard typed key CONTROL-p"});
                        } else if (event.key.code == sf::Keyboard::S) {
                            db.claim(Fact{"#0 keyboard typed key CONTROL-s"});
                        }
                    }
                } else if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode > 31 && event.text.unicode < 127) {
                        sf::String c = event.text.unicode;
                        db.claim(Fact{{Term{"#0"}, Term{"keyboard"}, Term{"typed"}, Term{"key"}, Term{"", c}}});
                    } else {
                        std::cout << "unhandled key press unicode " << event.text.unicode << std::endl;
                    }
                }
            }

            db.claim("#0 fps is " + std::to_string((int)fps));
            db.claim("#0 clock time is " + std::to_string(loopCount));

            sourceCodeManager.update(db, programsThatDied, newlySeenPrograms);

            db.run_subscriptions();

            genericGraphicsWishes = db.select({"$source wish $target had graphics $graphics"});
        }

        calibrationManager.checkForArucoCalibration(main_seen_program_ids,
                                                    main_seen_program_corners);

        window.clear(sf::Color(0, 0, 255, 255));
        graphicsManager.draw_graphics(window,
                                      genericGraphicsWishes,
                                      main_seen_program_ids,
                                      main_seen_program_corners,
                                      calibrationManager.projection_corrected_world_corners,
                                      latestFrameTexture);

        window.display();

        debugWindowManager.update(calibrationManager, db);
        debugWindowManager.draw(latestFrameSprite, calibrationManager.calibration);
    }

    std::cout << "stopping http server" << std::endl;
    httpServerInstance.stop();
    std::cout << "http server stopped" << std::endl;
    stop_cv_thread = true;
    std::cout << "waiting for CV thread to end" << std::endl;
    // r.wait();

    return 0;
}