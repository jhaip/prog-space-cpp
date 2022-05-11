#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <SFML/Graphics.hpp>
#include "SelbaWard/ElasticSprite.cpp"
// #include "SelbaWard/Sprite3d.cpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <map>
#include <thread>
#include <nlohmann/json.hpp>

#include "db.cpp"
#include "http.cpp"

using nlohmann::json;

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

// from https://github.com/ThePhD/sol2/issues/86
sol::object jsonToLuaObject(const json &j, sol::state_view &lua)
{
    if (j.is_null())
    {
        return sol::make_object(lua, sol::nil);
    }
    else if (j.is_boolean())
    {
        return sol::make_object(lua, j.get<bool>());
    }
    else if (j.is_number())
    {
        return sol::make_object(lua, j.get<double>());
    }
    else if (j.is_string())
    {
        return sol::make_object(lua, j.get<std::string>().c_str());
    }
    else if (j.is_object())
    {
        sol::table obj = lua.create_table();
        for (auto &el : j.items())
        {
            auto thing = jsonToLuaObject(el.value(), lua);
            obj.set(el.key(), thing);
        }
        return obj.as<sol::object>();
    }
    else if (j.is_array())
    {
        sol::table obj = lua.create_table();
        unsigned long i = 0;
        for (auto &el : j.items())
        {
            obj.set(i++, jsonToLuaObject(el.value(), lua));
        }
        return obj;
    }
    return sol::make_object(lua, sol::nil);
}

void http_request_thread(std::vector<std::string> query_parts, sol::protected_function callback_func, sol::this_state ts) {
    if (auto response_json = do_http_request())
    {
        sol::state_view lua = ts;
        auto r = jsonToLuaObject(response_json.value(), lua);
        callback_func(r);
    }
    std::cout << "making http request == done" << std::endl;
}

void http_request(std::vector<std::string> query_parts, sol::protected_function callback_func, sol::this_state ts)
{
    std::cout << "making http request" << std::endl;
    std::thread { http_request_thread, query_parts, callback_func, ts }.detach();
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
        lua.set_function("retract", &Database::retract, &db);
        lua.set_function("register_when", &Database::register_when, &db);
        lua.set_function("remove_subs", &Database::remove_subs, &db);
        lua.set_function("http_request", http_request);

        sol::load_result script1 = lua.load_file("../../scripts/foxisred.lua");
        sol::load_result script2 = lua.load_file("../../scripts/youisafox.lua");
        sol::load_result script3 = lua.load_file("../../scripts/whensomeoneisafox.lua");
        sol::load_result script4 = lua.load_file("../../scripts/whentime.lua");
        sol::load_result script5 = lua.load_file("../../scripts/isananimal.lua");
        sol::load_result script6 = lua.load_file("../../scripts/timeis.lua");
        sol::load_result script7 = lua.load_file("../../scripts/7__darksky.lua");
        sol::load_result script9 = lua.load_file("../../scripts/9__animation.lua");
        sol::load_result script10 = lua.load_file("../../scripts/10__outlinePrograms.lua");
        sol::load_result script11 = lua.load_file("../../scripts/11__counting.lua");
        sol::load_result script12 = lua.load_file("../../scripts/12__particle.lua"); 
        sol::load_result script13 = lua.load_file("../../scripts/13__particleFast.lua");
        sol::load_result script14 = lua.load_file("../../scripts/14__showWeather.lua");
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
        std::vector<std::pair<float, float>> calibration = {{10, 10}, {1200, 10}, {1200, 700}, {10, 700}};
        int calibrationCorner = 0;

        sf::RenderWindow window(sf::VideoMode(800, 800), "SFML works!");
        sf::RenderWindow debugWindow(sf::VideoMode(1280, 720), "debug");
        sf::Font font;
        // TODO: find font into project and load that
        if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
            // error...
        }

        float fps;
        sf::Clock clock;
        sf::Time previousTime = clock.getElapsedTime();
        sf::Time currentTime;

        script11();
        // script12();
        script13();

        script7();
        script14();

        int loopCount = 0;
        window.setFramerateLimit(60);
        while (window.isOpen() && debugWindow.isOpen())
        {
            // currentTime = clock.getElapsedTime();
            // if (currentTime.asSeconds() - previousTime.asSeconds() > 1.0f/60.0f) {
                currentTime = clock.getElapsedTime();
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
                // script6(); // also mapped to aruco card
                script9();
                script10();

                db.run_subscriptions();

                sf::Event event;
                while (window.pollEvent(event))
                {
                    if (event.type == sf::Event::Closed)
                    {
                        window.close();
                    }
                    else if (event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Enter)
                        {
                            std::cout << "Enter pressed" << std::endl;
                            db.print();
                        }
                    }
                }
                while (debugWindow.pollEvent(event))
                {
                    if (event.type == sf::Event::Closed)
                    {
                        debugWindow.close();
                    }
                    else if (event.type == sf::Event::KeyPressed)
                    {
                        if (event.key.code == sf::Keyboard::Enter)
                        {
                            std::cout << "Debug Enter pressed" << std::endl;
                            db.print();
                        }
                    }
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                    calibrationCorner = 0;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                    calibrationCorner = 1;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                    calibrationCorner = 2;
                } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
                    calibrationCorner = 3;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
                {
                    calibration[calibrationCorner].first++;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                {
                    calibration[calibrationCorner].first--;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                {
                    calibration[calibrationCorner].second--;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                {
                    calibration[calibrationCorner].second++;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num9)) {
                    script4();
                } else {
                    db.cleanup("5");
                }
                if (std::find(main_seen_program_ids.begin(), main_seen_program_ids.end(), 49) != main_seen_program_ids.end()) {
                    script6();
                } else {
                    db.cleanup("3");
                }
                // TODO: need to update CV to only run programs when program first seen
                // if (std::find(main_seen_program_ids.begin(), main_seen_program_ids.end(), 44) != main_seen_program_ids.end())
                // {
                //     script14();
                // }
                // else
                // {
                //     db.cleanup("14");
                // }

                window.clear();

                auto textGraphicsWishes = db.select({"$ wish text $text at $x $y"});
                for (const auto &wish : textGraphicsWishes) {
                    sf::Text text;
                    text.setFont(font);
                    text.setString(wish.get("text").value);
                    text.setFillColor(sf::Color::Red);
                    text.setPosition(std::stod(wish.get("x").value), std::stod(wish.get("y").value));
                    window.draw(text);
                }
                auto lineGraphicsWishes = db.select({"$ wish line from $x1 $y1 to $x2 $y2"});
                for (const auto &wish : lineGraphicsWishes) {
                    sf::Vertex line[] =
                    {
                        sf::Vertex(sf::Vector2f(std::stod(wish.get("x1").value), std::stod(wish.get("y1").value))),
                        sf::Vertex(sf::Vector2f(std::stod(wish.get("x2").value), std::stod(wish.get("y2").value))),
                    };
                    window.draw(line, 2, sf::Lines);
                }
                auto frameGraphicsWishes = db.select({"$ wish frame at $x $y scale $s"});
                for (const auto &wish : frameGraphicsWishes) {
                    latestFrameSprite.setPosition(sf::Vector2f(std::stod(wish.get("x").value), std::stod(wish.get("y").value)));
                    float s = std::stod(wish.get("s").value);
                    latestFrameSprite.setScale(s, s);
                    window.draw(latestFrameSprite);

                    // sf::VertexArray quad(sf::Quads, 4);

                    // // define it as a rectangle, located at (10, 10) and with size 100x100
                    // quad[0].position = sf::Vector2f(20.f, 0.f);
                    // quad[1].position = sf::Vector2f(80.f, 0.f);
                    // quad[2].position = sf::Vector2f(120.f, 100.f);
                    // quad[3].position = sf::Vector2f(-20.f, 100.f);

                    // // define its texture area to be a 25x50 rectangle starting at (0, 0)
                    // quad[0].texCoords = sf::Vector2f(0.f, 0.f);
                    // quad[1].texCoords = sf::Vector2f(1280.f, 0.f);
                    // quad[2].texCoords = sf::Vector2f(1280.f, 720.f);
                    // quad[3].texCoords = sf::Vector2f(0.f, 720.f);
                    // window.draw(quad, &latestFrameTexture);

                    sw::ElasticSprite sprite{latestFrameTexture};
                    // sprite.setScale(100.f/1280.f, 100.f/720.f);
                    sprite.setVertexOffset(0, {100.0f, 0.f}); // TL
                    sprite.setVertexOffset(1, {0.0f, -720.f+300.f}); // BL
                    sprite.setVertexOffset(2, {-1280.f+400.0f, -720.f+300.f}); // BR
                    sprite.setVertexOffset(3, {-1280.f + 300.0f, 0.f}); // TR
                    window.draw(sprite);
                }

                sf::Text fpsText;
                fpsText.setFont(font);
                fpsText.setString(std::to_string(fps));
                fpsText.setFillColor(sf::Color::Yellow);
                fpsText.setPosition(0, 50);
                window.draw(fpsText);


                window.display();

                debugWindow.clear();
                debugWindow.draw(latestFrameSprite);
                int cornerIndex = 0;
                sf::VertexArray verticesOfCalibration(sf::LinesStrip, 5);
                for (const auto &corner : calibration) {
                    sf::CircleShape circle(10);
                    circle.setPosition(sf::Vector2f(corner.first-10.f, corner.second-10.f));
                    if (cornerIndex == calibrationCorner) {
                        circle.setFillColor(sf::Color(255, 255, 0));
                    } else {
                        circle.setFillColor(sf::Color(0, 255, 0));
                    }
                    verticesOfCalibration[cornerIndex].position = sf::Vector2f(corner.first, corner.second);
                    verticesOfCalibration[cornerIndex].color = sf::Color::Green;
                    debugWindow.draw(circle);
                    cornerIndex++;
                }
                verticesOfCalibration[4] = verticesOfCalibration[0];
                debugWindow.draw(verticesOfCalibration);
                debugWindow.display();
            // }
        }

        stop_cv_thread = true;
        cvThread.join();

        return 0;
}