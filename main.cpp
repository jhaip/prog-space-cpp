#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <SFML/Graphics.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <map>
#include <thread>

#include "db.cpp"

using namespace std::chrono;
using namespace cv;

std::atomic_bool stop_cv_thread{false};
std::mutex myMutex;
std::atomic_bool new_data_available{false};
std::vector<int> seen_program_ids;
std::vector<std::vector<cv::Point2f>> seen_program_corners;
cv::Mat latestFrame;

std::string my_function( int a, std::string b ) {
        // Create a string with the letter 'D' "a" times,
        // append it to 'b'
        return b + std::string( 'D', a );
}

void my_query(std::string a, sol::protected_function f) {
    std::string result1 = a; // lua.create_table_with(a, "50");
    std::vector<std::string> results = {result1};
    for (auto& result : results) {
        f(result);
    }
}

void cvLoop() {
    VideoCapture inputVideo;
    inputVideo.open(0);
    cv::Mat cameraMatrix;
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_1000);
    while (!stop_cv_thread && inputVideo.grab()) {
        cv::Mat image, imageCopy;
        inputVideo.retrieve(image);
        image.copyTo(imageCopy);
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);
        if (ids.size() > 0) {
            cv::aruco::drawDetectedMarkers(imageCopy, corners, ids);
            // std::cout << "Seen ID" << ids[0] << std::endl;
        }
        // do something with imageCopy
        std::lock_guard<std::mutex> guard(myMutex);
        seen_program_ids = ids;
        seen_program_corners = corners;
        imageCopy.copyTo(latestFrame);
        new_data_available = true;
    }
    std::cout << "thread died" << std::endl;
}

int main () {
        std::thread cvThread(cvLoop);

        Database db{};

        sol::state lua;

        // open those basic lua libraries 
        // again, for print() and other basic utilities
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::os);

        lua["my_func"] = my_function; // way 1
        lua.set("my_func", my_function); // way 2
        lua.set_function("my_func", my_function); // way 3

        lua.set_function("claim", &Database::claim, &db);
        lua.set_function("when", &Database::when, &db);
        lua.set_function("cleanup", &Database::cleanup, &db);

        sol::load_result script1 = lua.load_file("foxisred.lua");
        sol::load_result script2 = lua.load_file("youisafox.lua");
        sol::load_result script3 = lua.load_file("whensomeoneisafox.lua");
        sol::load_result script4 = lua.load_file("whentime.lua");
        sol::load_result script5 = lua.load_file("isananimal.lua");
        sol::load_result script6 = lua.load_file("timeis.lua");
        sol::load_result script9 = lua.load_file("9__animation.lua");
        sol::load_result script10 = lua.load_file("10__outlinePrograms.lua");
        // std::vector<sol::load_result> scripts = {
        //     lua.load_file("foxisred.lua"),
        //     lua.load_file("isananimal.lua"),
        //     lua.load_file("whensomeoneisafox.lua"),
        //     lua.load_file("whentime.lua"),
        //     lua.load_file("youisafox.lua"),
        // };

        // for (auto& script : scripts) {
        //     script();
        // }

        std::vector<int> main_seen_program_ids;
        std::vector<std::vector<cv::Point2f>> main_seen_program_corners;
        cv::Mat main_latestFrame;
        sf::Image latestFrameImage;
        sf::Texture latestFrameTexture;
        sf::Sprite latestFrameSprite;

        sf::RenderWindow window(sf::VideoMode(800, 800), "SFML works!");
        sf::Font font;
        // TODO: find font into project and load that
        if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
            // error...
        }

        float fps;
        sf::Clock clock;
        sf::Time previousTime = clock.getElapsedTime();
        sf::Time currentTime;

        int loopCount = 0;
        while (window.isOpen())
        {
            // std::cout << "1";
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed) {
                    window.close();
                } else if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Enter) {
                        std::cout << "Enter pressed" << std::endl;
                        db.print();
                    }
                }
            }

            currentTime = clock.getElapsedTime();
            if (currentTime.asSeconds() - previousTime.asSeconds() > 1.0f/60.0f) {
                fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds()); // the asSeconds returns a float
                // std::cout << "fps =" << floor(fps) << std::endl; // flooring it will make the frame rate a rounded number
                previousTime = currentTime;

                {
                    std::lock_guard<std::mutex> guard(myMutex);
                    if (new_data_available) {
                        new_data_available = false;
                        main_seen_program_ids = seen_program_ids;
                        main_seen_program_corners = seen_program_corners;
                        cv::cvtColor(latestFrame, main_latestFrame, cv::COLOR_BGR2RGBA);
                        latestFrameImage.create(main_latestFrame.cols, main_latestFrame.rows, main_latestFrame.ptr());
                        if (latestFrameTexture.loadFromImage(latestFrameImage)) {
                            latestFrameSprite.setTexture(latestFrameTexture);
                        }
                        // std::cout << "NEW DATA!" << std::endl;
                    }
                }

                db.cleanup("0");
                db.claim("#0 clock time is " + std::to_string(loopCount));
                int index = 0;
                for (auto& id : main_seen_program_ids) {
                    cv::Point2f corner = main_seen_program_corners.at(index).at(0);
                    db.claim("#0 program "+std::to_string(id)+" at "+std::to_string(corner.x)+" "+std::to_string(corner.y));
                    index += 1;
                }
                loopCount += 1;
                script1();
                script2();
                script3();
                // script4();
                script5();
                // script6();
                script9();
                script10();

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                    script4();
                } else {
                    db.cleanup("5");
                }
                if (std::find(main_seen_program_ids.begin(), main_seen_program_ids.end(), 49) != main_seen_program_ids.end()) {
                    script6();
                } else {
                    db.cleanup("3");
                }

                window.clear();

                auto textGraphicsWishes = db.select({"$ wish text $text at $x $y"});
                for (const auto &wish : textGraphicsWishes) {
                    sf::Text text;
                    text.setFont(font);
                    text.setString(wish.Result.at("text").value);
                    text.setFillColor(sf::Color::Red);
                    text.setPosition(std::stod(wish.Result.at("x").value), std::stod(wish.Result.at("y").value));
                    window.draw(text);
                }
                auto lineGraphicsWishes = db.select({"$ wish line from $x1 $y1 to $x2 $y2"});
                for (const auto &wish : lineGraphicsWishes) {
                    sf::Vertex line[] =
                    {
                        sf::Vertex(sf::Vector2f(std::stod(wish.Result.at("x1").value), std::stod(wish.Result.at("y1").value))),
                        sf::Vertex(sf::Vector2f(std::stod(wish.Result.at("x2").value), std::stod(wish.Result.at("y2").value))),
                    };
                    window.draw(line, 2, sf::Lines);
                }
                auto frameGraphicsWishes = db.select({"$ wish frame at $x $y scale $s"});
                for (const auto &wish : frameGraphicsWishes) {
                    latestFrameSprite.setPosition(sf::Vector2f(std::stod(wish.Result.at("x").value), std::stod(wish.Result.at("y").value)));
                    float s = std::stod(wish.Result.at("s").value);
                    latestFrameSprite.setScale(s, s);
                    window.draw(latestFrameSprite);
                }

                sf::Text fpsText;
                fpsText.setFont(font);
                fpsText.setString(std::to_string(fps));
                fpsText.setFillColor(sf::Color::Yellow);
                fpsText.setPosition(0, 50);
                window.draw(fpsText);


                window.display();
            }
        }

        stop_cv_thread = true;
        cvThread.join();

        return 0;
}