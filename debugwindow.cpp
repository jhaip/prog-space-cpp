class DebugWindowManager {
  public:
    int calibrationCorner = 0;
    sf::RenderWindow debugWindow;

    DebugWindowManager(int CAMERA_WIDTH, int CAMERA_HEIGHT) : debugWindow(sf::VideoMode(CAMERA_WIDTH, CAMERA_HEIGHT), "debug") {}

    void update(CalibrationManager &calibrationManager, Database &db) {
        sf::Event event;
        while (debugWindow.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                debugWindow.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                    std::cout << "Debug Enter pressed" << std::endl;
                    {
                        std::scoped_lock guard(dbMutex);
                        db.print();
                    }
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
                calibrationManager.calibration[calibrationCorner].first += 2;
                calibrationManager.recalculate();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                calibrationManager.calibration[calibrationCorner].first -= 2;
                calibrationManager.recalculate();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
                calibrationManager.calibration[calibrationCorner].second -= 2;
                calibrationManager.recalculate();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
                calibrationManager.calibration[calibrationCorner].second += 2;
                calibrationManager.recalculate();
            }
        }
    }
    void draw(sf::Sprite &latestFrameSprite, std::vector<std::pair<float, float>> &calibration) {
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