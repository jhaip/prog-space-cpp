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
#include <math.h>
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
cv::Mat calibrationMatrix;

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
        // std::cout << "width: " << imageCopy.cols << " height: " << imageCopy.rows << std::endl;
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

        std::vector<std::string> scriptPaths = {
            "", // 0
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
            "../../scripts/14__showWeather.lua"};

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
        std::vector<std::pair<float, float>> calibration = {{226, 124}, {1442, 108}, {1458, 830}, {198, 810}}; // TL TR BR BL
        int calibrationCorner = 0;
        std::vector<cv::Point2f> projection_corrected_world_corners = {
            cv::Point2f(0, 0),
            cv::Point2f(SCREEN_WIDTH, 0),
            cv::Point2f(SCREEN_WIDTH, SCREEN_HEIGHT),
            cv::Point2f(0, SCREEN_HEIGHT)
        }; // TL TR BR BL
        bool shouldRecalculate = true;

        sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "SFML works!");
        sf::RenderWindow debugWindow(sf::VideoMode(CAMERA_WIDTH, CAMERA_HEIGHT), "debug");
        sf::Font font;
        // TODO: find font into project and load that
        if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
            // error...
        }
        sf::RenderTexture renderTexture;
        if (!renderTexture.create(SCREEN_WIDTH, SCREEN_HEIGHT))
        {
            // error...
        }

        float fps;
        sf::Clock clock;
        sf::Time previousTime = clock.getElapsedTime();
        sf::Time currentTime;

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

                std::vector<int> newlySeenPrograms;
                std::vector<int> programsThatDied;
                {
                    std::lock_guard<std::mutex> guard(myMutex);
                    if (new_data_available) {
                        new_data_available = false;
                        for (auto &id : seen_program_ids)
                        {
                            if (std::find(main_seen_program_ids.begin(), main_seen_program_ids.end(), id) == main_seen_program_ids.end()) {
                                newlySeenPrograms.emplace_back(id);
                            }
                        }
                        for (auto &id : main_seen_program_ids)
                        {
                            if (std::find(seen_program_ids.begin(), seen_program_ids.end(), id) == seen_program_ids.end())
                            {
                                programsThatDied.emplace_back(id);
                            }
                        }
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

                for (auto &id : programsThatDied)
                {
                    std::cout << id << " died." << std::endl;
                    db.cleanup(std::to_string(id));
                    db.remove_subs(std::to_string(id));
                }
                for (auto &id : newlySeenPrograms)
                {
                    std::cout << "running " << id << std::endl;
                    lua.script_file(scriptPaths[id]);
                }

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
                    calibration[calibrationCorner].first+=2;
                    shouldRecalculate = true;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
                {
                    calibration[calibrationCorner].first-=2;
                    shouldRecalculate = true;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
                {
                    calibration[calibrationCorner].second-=2;
                    shouldRecalculate = true;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
                {
                    calibration[calibrationCorner].second+=2;
                    shouldRecalculate = true;
                }
                if (shouldRecalculate) {
                    std::vector<cv::Point2f> screen_vertices{
                        cv::Point2f{0, 0},                                     // topLeft
                        cv::Point2f{(float)SCREEN_WIDTH, 0},                   // Top Right
                        cv::Point2f{(float)SCREEN_WIDTH, (float)SCREEN_HEIGHT}, // bottomRight
                        cv::Point2f{0, (float)SCREEN_HEIGHT}                  // bottomLeft
                    };

                    std::vector<cv::Point2f> camera_vertices{
                        cv::Point2f{0, 0},                                     // topLeft
                        cv::Point2f{(float)CAMERA_WIDTH, 0},                   // Top Right
                        cv::Point2f{(float)CAMERA_WIDTH, (float)CAMERA_HEIGHT}, // bottomRight
                        cv::Point2f{0, (float)CAMERA_HEIGHT}                  // bottomLeft
                    };

                    std::vector<cv::Point2f> dst_vertices{
                        cv::Point2f{calibration[0].first, calibration[0].second},
                        cv::Point2f{calibration[1].first, calibration[1].second},
                        cv::Point2f{calibration[2].first, calibration[2].second},
                        cv::Point2f{calibration[3].first, calibration[3].second}
                    };

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

                    shouldRecalculate = false;
                }

                window.clear(sf::Color(0,0,255, 255));
                renderTexture.clear();

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
                    // window.draw(line, 2, sf::Lines);
                    renderTexture.draw(line, 2, sf::Lines);
                }
                auto genericGraphicsWishes = db.select({"$source wish $target had graphics $graphics"});
                for (const auto &wish : genericGraphicsWishes)
                {
                    auto j = json::parse(wish.get("graphics").value);
                    auto sourceStr = wish.get("source").value;
                    auto targetStr = wish.get("target").value;
                    auto source = std::stoi(sourceStr);
                    // std::cout << "wish you had graphics" << source << std::endl;
                    sf::Transform programTransform;
                    if (targetStr == "you") {
                        int main_seen_program_ids_index = 0;
                        for (auto& id : main_seen_program_ids) {
                            if (id == source) {
                                cv::Point2f corner0 = main_seen_program_corners.at(main_seen_program_ids_index).at(0);
                                cv::Point2f corner1 = main_seen_program_corners.at(main_seen_program_ids_index).at(1);
                                programTransform.translate(corner0.x, corner0.y);
                                auto angleDegrees = atan2(corner1.y-corner0.y, corner1.x-corner0.x) * 180 / 3.14159265;
                                programTransform.rotate(angleDegrees);
                            }
                            main_seen_program_ids_index++;
                        }
                    }
                    
                    for (auto &g : j)
                    {
                        const auto typ = g["type"];
                        if (typ == "rectangle") {
                            auto x = g["options"]["x"];
                            auto y = g["options"]["y"];
                            auto w = g["options"]["w"];
                            auto h = g["options"]["h"];
                            sf::RectangleShape rectangle(sf::Vector2f(w, h));
                            rectangle.setPosition(x, y);
                            rectangle.setFillColor(sf::Color(100, 250, 50)); // todo
                            renderTexture.draw(rectangle, programTransform);
                        }
                        else if (typ == "ellipse")
                        {
                            auto x = g["options"]["x"].get<double>() + g["options"]["w"].get<double>() * 0.5;
                            auto y = g["options"]["y"].get<double>() + g["options"]["h"].get<double>() * 0.5;
                            auto w = g["options"]["w"].get<double>() * 0.5;
                            auto h = g["options"]["h"].get<double>() * 0.5;
                            sf::CircleShape circle{};
                            circle.setPosition(x, y);
                            circle.setRadius(w); // TODO: support ellipse
                            circle.setFillColor(sf::Color(250, 100, 50)); // todo
                            renderTexture.draw(circle, programTransform);
                        }
                        else if (typ == "line")
                        {
                            auto x1 = g["options"][0];
                            auto y1 = g["options"][1];
                            auto x2 = g["options"][2];
                            auto y2 = g["options"][3];
                            sf::VertexArray line(sf::Lines, 2);
                            line[0] = sf::Vertex(sf::Vector2f(x1, y1));
                            line[1] = sf::Vertex(sf::Vector2f(x2, y2));
                            renderTexture.draw(line, programTransform);
                        }
                        else if (typ == "text")
                        {
                            auto x = g["options"]["x"];
                            auto y = g["options"]["y"];
                            sf::Text text;
                            text.setFont(font);
                            text.setString(g["options"]["text"]);
                            text.setFillColor(sf::Color::Red); // xxx
                            text.setPosition(x, y);
                            renderTexture.draw(text, programTransform);
                        }
                    }
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

                    // sw::ElasticSprite sprite{latestFrameTexture};
                    // // sprite.setScale(100.f/1280.f, 100.f/720.f);
                    // sprite.setVertexOffset(0, {100.0f, 0.f}); // TL
                    // sprite.setVertexOffset(1, {0.0f, -720.f+300.f}); // BL
                    // sprite.setVertexOffset(2, {-1280.f+400.0f, -720.f+300.f}); // BR
                    // sprite.setVertexOffset(3, {-1280.f + 300.0f, 0.f}); // TR
                    // window.draw(sprite);
                }

                renderTexture.display();
                const sf::Texture &renderTextureCopy = renderTexture.getTexture();
                sw::ElasticSprite sprite{renderTextureCopy};
                sprite.setTextureFlipY(true);
                // sprite.setUseShader(false);
                sprite.activatePerspectiveInterpolation();
                sprite.setVertexOffset(0, {projection_corrected_world_corners[0].x, projection_corrected_world_corners[0].y});                                // TL
                sprite.setVertexOffset(1, {projection_corrected_world_corners[3].x, projection_corrected_world_corners[3].y - SCREEN_HEIGHT});                // BL
                sprite.setVertexOffset(2, {projection_corrected_world_corners[2].x - SCREEN_WIDTH, projection_corrected_world_corners[2].y - SCREEN_HEIGHT}); // BR
                sprite.setVertexOffset(3, {projection_corrected_world_corners[1].x - SCREEN_WIDTH, projection_corrected_world_corners[1].y});     // TR
                // sprite.setVertexOffset(0, {-calibration[0].first, -calibration[0].second}); // TL
                // sprite.setVertexOffset(1, {-calibration[3].first, CAMERA_HEIGHT - calibration[3].second});               // BL
                // sprite.setVertexOffset(2, {CAMERA_WIDTH - calibration[2].first, CAMERA_HEIGHT - calibration[2].second}); // BR
                // sprite.setVertexOffset(3, {CAMERA_WIDTH - calibration[1].first, -calibration[1].second});  // TR
                window.draw(sprite);

                // test projecting directly
                // for (const auto &wish : lineGraphicsWishes)
                // {
                //     std::vector<cv::Point2f> line_vertices{
                //         cv::Point2f{std::stof(wish.get("x1").value), std::stof(wish.get("y1").value)},
                //         cv::Point2f{std::stof(wish.get("x2").value), std::stof(wish.get("y2").value)}
                //     };
                //     std::vector<cv::Point2f> projection_line_points = {
                //         cv::Point2f(0, 0),
                //         cv::Point2f(0, 0)};
                //     perspectiveTransform(line_vertices, projection_line_points, calibrationMatrix);
                //     sf::Vertex line[] =
                //     {
                //         sf::Vertex(sf::Vector2f(projection_line_points[0].x, projection_line_points[0].y), sf::Color::Green),
                //         sf::Vertex(sf::Vector2f(projection_line_points[1].x, projection_line_points[1].y), sf::Color::Green)
                //     };
                //     window.draw(line, 2, sf::Lines);
                // }

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
                for (const auto &wish : lineGraphicsWishes)
                {
                    sf::Vertex line[] =
                        {
                            sf::Vertex(sf::Vector2f(std::stod(wish.get("x1").value), std::stod(wish.get("y1").value))),
                            sf::Vertex(sf::Vector2f(std::stod(wish.get("x2").value), std::stod(wish.get("y2").value))),
                        };
                    debugWindow.draw(line, 2, sf::Lines);
                }
                debugWindow.display();
            // }
        }

        stop_cv_thread = true;
        cvThread.join();

        return 0;
}