class DebugWindowManager {
  public:
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;
    int calibrationCorner = 0;
    sf::RenderWindow debugWindow;

    DebugWindowManager(int _SCREEN_WIDTH, int _SCREEN_HEIGHT, int CAMERA_WIDTH, int CAMERA_HEIGHT) : SCREEN_WIDTH(_SCREEN_WIDTH), SCREEN_HEIGHT(_SCREEN_HEIGHT),
                                                                                                     debugWindow(sf::VideoMode(CAMERA_WIDTH, CAMERA_HEIGHT), "debug") {}

    void update(Database &db, sf::Sprite &latestFrameSprite) {
        auto calibrationResults = db.select({"$ calibration points $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"});
        std::vector<std::pair<float, float>> calibration = {
            {0, 0},
            {SCREEN_WIDTH, 0},
            {SCREEN_WIDTH, SCREEN_HEIGHT},
            {0, SCREEN_HEIGHT}}; // TL TR BR BL
        if (calibrationResults.size() > 0) {
            auto r = calibrationResults[0];
            calibration[0].first = std::stof(r.get("x1").value);
            calibration[0].second = std::stof(r.get("y1").value);
            calibration[1].first = std::stof(r.get("x2").value);
            calibration[1].second = std::stof(r.get("y2").value);
            calibration[2].first = std::stof(r.get("x3").value);
            calibration[2].second = std::stof(r.get("y3").value);
            calibration[3].first = std::stof(r.get("x4").value);
            calibration[3].second = std::stof(r.get("y4").value);
        }
        bool calibrationChanged = false;

        sf::Event event;
        while (debugWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                debugWindow.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                    std::cout << "Debug Enter pressed" << std::endl;
                    db.print();
                }
            }
        }
        if (debugWindow.hasFocus()) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                calibrationCorner = 0;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                calibrationCorner = 1;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
                calibrationCorner = 2;
            } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) {
                calibrationCorner = 3;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                calibration[calibrationCorner].first += 2;
                calibrationChanged = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                calibration[calibrationCorner].first -= 2;
                calibrationChanged = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                calibration[calibrationCorner].second -= 2;
                calibrationChanged = true;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                calibration[calibrationCorner].second += 2;
                calibrationChanged = true;
            }
        }
        if (calibrationChanged) {
            db.claim(Fact{{Term{"#0debug"}, Term{"wish"}, Term{"calibration"}, Term{"is"},
                           Term{"", std::to_string(calibration[0].first)}, Term{"", std::to_string(calibration[0].second)},
                           Term{"", std::to_string(calibration[1].first)}, Term{"", std::to_string(calibration[1].second)},
                           Term{"", std::to_string(calibration[2].first)}, Term{"", std::to_string(calibration[2].second)},
                           Term{"", std::to_string(calibration[3].first)}, Term{"", std::to_string(calibration[3].second)}}});
        }

        debugWindow.clear();
        debugWindow.draw(latestFrameSprite);
        int cornerIndex = 0;
        sf::VertexArray verticesOfCalibration(sf::LinesStrip, 5);
        for (const auto &corner : calibration) {
            sf::CircleShape circle(10);
            circle.setPosition(sf::Vector2f(corner.first - 10.f, corner.second - 10.f));
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
    }
};