#include "SelbaWard/ElasticSprite.cpp"
#include "SelbaWard/Line.cpp"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <sfeMovie/Movie.hpp>

using nlohmann::json;

struct Illumination {
    json graphics;

    static void add_illumination_usertype_to_lua(sol::state &lua) {
        lua.new_usertype<Illumination>(
            "Illumination", // the name of the class, as you want it to be used in lua List the member
            "rectangle", &Illumination::rectangle,
            "ellipse", &Illumination::ellipse,
            "line", &Illumination::line,
            "text", &Illumination::text,
            "frame", &Illumination::frame,
            "image", &Illumination::image,
            "movie", &Illumination::movie);
    }

    std::vector<int> get_color_from_lua_table(sol::table &opts, std::string key, std::vector<int> fallbackColor) {
        std::vector<int> color = fallbackColor;
        auto luaTable = opts[key];
        if (luaTable.valid()) {
            sol::table t = luaTable;
            std::size_t sz = t.size();
            for (int i = 1; i <= sz; i++) {
                color[i - 1] = t[i];
            }
        }
        return color;
    }

    int get_int_from_lua_table(sol::table &opts, std::string key, int fallback) {
        double val = fallback;
        auto luaTable = opts[key];
        if (luaTable.valid()) {
            val = luaTable;
        }
        return (int)val;
    }
    float get_float_from_lua_table(sol::table &opts, std::string key, float fallback) {
        double val = fallback;
        auto luaTable = opts[key];
        if (luaTable.valid()) {
            val = luaTable;
        }
        return (float)val;
    }

    void rectangle(sol::table opts) {
        int x = get_int_from_lua_table(opts, "x", 0);
        int y = get_int_from_lua_table(opts, "y", 0);
        int w = get_int_from_lua_table(opts, "w", 10);
        int h = get_int_from_lua_table(opts, "h", 10);
        std::vector<int> fallbackColor = {255, 255, 255, 255};
        std::vector<int> fill = get_color_from_lua_table(opts, "fill", fallbackColor);
        std::vector<int> stroke = get_color_from_lua_table(opts, "stroke", fallbackColor);
        int stroke_width = get_int_from_lua_table(opts, "stroke_width", 1);
        graphics.push_back({{"type", "rectangle"}, {"options", {{"x", x}, {"y", y}, {"w", w}, {"h", h}, {"fill", fill}, {"stroke", stroke}, {"stroke_width", stroke_width}}}});
    }

    void ellipse(sol::table opts) {
        int x = get_int_from_lua_table(opts, "x", 0);
        int y = get_int_from_lua_table(opts, "y", 0);
        int w = get_int_from_lua_table(opts, "w", 10);
        int h = get_int_from_lua_table(opts, "h", 10);
        std::vector<int> fallbackColor = {255, 255, 255, 255};
        std::vector<int> fill = get_color_from_lua_table(opts, "fill", fallbackColor);
        std::vector<int> stroke = get_color_from_lua_table(opts, "stroke", fallbackColor);
        int stroke_width = get_int_from_lua_table(opts, "stroke_width", 1);
        graphics.push_back({{"type", "ellipse"}, {"options", {{"x", x}, {"y", y}, {"w", w}, {"h", h}, {"fill", fill}, {"stroke", stroke}, {"stroke_width", stroke_width}}}});
    }

    void line(sol::table opts) {
        int x1 = get_int_from_lua_table(opts, "x1", 0);
        int y1 = get_int_from_lua_table(opts, "y1", 0);
        int x2 = get_int_from_lua_table(opts, "x2", 0);
        int y2 = get_int_from_lua_table(opts, "y2", 0);
        int thickness = get_int_from_lua_table(opts, "thickness", 1);
        std::vector<int> fallbackColor = {255, 255, 255, 255};
        std::vector<int> color = get_color_from_lua_table(opts, "color", fallbackColor);
        graphics.push_back({{"type", "line"}, {"options", {{"x1", x1}, {"y1", y1}, {"x2", x2}, {"y2", y2}, {"color", color}, {"thickness", thickness}}}});
    }

    void text(sol::table opts) {
        int x = get_int_from_lua_table(opts, "x", 0);
        int y = get_int_from_lua_table(opts, "y", 0);
        int fontSize = get_int_from_lua_table(opts, "size", 12);
        std::vector<int> fallbackColor = {255, 255, 255, 255};
        std::vector<int> color = get_color_from_lua_table(opts, "color", fallbackColor);
        std::string text = opts["text"];
        graphics.push_back({{"type", "text"}, {"options", {{"x", x}, {"y", y}, {"text", text}, {"color", color}, {"size", fontSize}}}});
    }

    void frame(sol::table opts) {
        int x = get_int_from_lua_table(opts, "x", 0);
        int y = get_int_from_lua_table(opts, "y", 0);
        int scale = get_int_from_lua_table(opts, "scale", 1);
        int clip_x = get_int_from_lua_table(opts, "clip_x", 0);
        int clip_y = get_int_from_lua_table(opts, "clip_y", 0);
        int clip_w = get_int_from_lua_table(opts, "clip_w", -1);
        int clip_h = get_int_from_lua_table(opts, "clip_h", -1);
        graphics.push_back({{"type", "frame"}, {"options", {{"x", x}, {"y", y}, {"scale", scale}, {"clip_x", clip_x}, {"clip_y", clip_y}, {"clip_w", clip_w}, {"clip_h", clip_h}}}});
    }

    void image(sol::table opts) {
        int x = get_int_from_lua_table(opts, "x", 0);
        int y = get_int_from_lua_table(opts, "y", 0);
        float scale = get_float_from_lua_table(opts, "scale", 1.0f);
        std::string filepath = opts["filepath"];
        graphics.push_back({{"type", "image"}, {"options", {{"x", x}, {"y", y}, {"scale", scale}, {"filepath", filepath}}}});
    }

    void movie(sol::table opts) {
        int x = get_int_from_lua_table(opts, "x", 0);
        int y = get_int_from_lua_table(opts, "y", 0);
        int w = get_int_from_lua_table(opts, "w", 10);
        int h = get_int_from_lua_table(opts, "h", 10);
        std::string filepath = opts["filepath"];
        graphics.push_back({{"type", "movie"}, {"options", {{"x", x}, {"y", y}, {"w", w}, {"h", h}, {"filepath", filepath}}}});
    }
};
std::ostream &operator<<(std::ostream &os, const Illumination &ill) {
    os << ill.graphics;
    return os;
}

class GraphicsManager {
  private:
    std::map<std::string, sf::Texture *> textureMap;
    std::map<std::string, sfe::Movie *> movieMap;
    sf::Font font;
    sf::RenderTexture renderTexture;
    sf::RenderTexture topRenderTexture;
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;
    sf::RenderWindow window;

  public:
    GraphicsManager(int _SCREEN_WIDTH, int _SCREEN_HEIGHT) : SCREEN_WIDTH(_SCREEN_WIDTH), SCREEN_HEIGHT(_SCREEN_HEIGHT),
                                                             window(sf::VideoMode(_SCREEN_WIDTH, _SCREEN_HEIGHT), "main") {}

    void init() {
        if (!font.loadFromFile("Inconsolata-Regular.ttf")) {
            std::cerr << "ERROR LOADING FONT" << std::endl;
        }
        if (!renderTexture.create(SCREEN_WIDTH, SCREEN_HEIGHT)) {
            // error...
        }
        if (!topRenderTexture.create(SCREEN_WIDTH, SCREEN_HEIGHT)) {
            // error...
        }
    }

    void update(Database &db,
                sf::Texture &latestFrameTexture) {
        handle_keyboard_and_window_events(db);
        draw_graphics(db, latestFrameTexture);
    }

    void handle_keyboard_and_window_events(Database &db) {
        db.remove_claims_from_source("0key");
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Enter) {
                    db.claim(Fact{"#0key keyboard typed key ENTER"});
                } else if (event.key.code == sf::Keyboard::Backspace) {
                    db.claim(Fact{"#0key keyboard typed key BACKSPACE"});
                } else if (event.key.code == sf::Keyboard::Space) {
                    db.claim(Fact{"#0key keyboard typed key SPACE"});
                } else if (event.key.code == sf::Keyboard::Tab) {
                    db.claim(Fact{"#0key keyboard typed key TAB"});
                } else if (event.key.code == sf::Keyboard::Right) {
                    db.claim(Fact{"#0key keyboard typed key RIGHT"});
                } else if (event.key.code == sf::Keyboard::Left) {
                    db.claim(Fact{"#0key keyboard typed key LEFT"});
                } else if (event.key.code == sf::Keyboard::Up) {
                    db.claim(Fact{"#0key keyboard typed key UP"});
                } else if (event.key.code == sf::Keyboard::Down) {
                    db.claim(Fact{"#0key keyboard typed key DOWN"});
                } else if (event.key.control) {
                    if (event.key.code == sf::Keyboard::P) {
                        db.claim(Fact{"#0key keyboard typed key CONTROL-p"});
                    } else if (event.key.code == sf::Keyboard::S) {
                        db.claim(Fact{"#0key keyboard typed key CONTROL-s"});
                    }
                }
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode > 31 && event.text.unicode < 127) {
                    sf::String c = event.text.unicode;
                    db.claim(Fact{{Term{"#0key"}, Term{"keyboard"}, Term{"typed"}, Term{"key"}, Term{"", c}}});
                } else {
                    std::cout << "unhandled key press unicode " << event.text.unicode << std::endl;
                }
            }
        }
    }

    void draw_graphics(Database &db,
                       sf::Texture &latestFrameTexture) {
        auto genericGraphicsWishes = db.select({"$source wish $target had graphics $graphics"});
        auto seenProgramResults = db.select({"$ program $id at $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"});
        auto projectedWorldCornersResults = db.select({"$ calibration projected corners $x1 $y1 $x2 $y2 $x3 $y3 $x4 $y4"});
        std::vector<cv::Point2f> projection_corrected_world_corners = {
            cv::Point2f(0, 0),
            cv::Point2f(SCREEN_WIDTH, 0),
            cv::Point2f(SCREEN_WIDTH, SCREEN_HEIGHT),
            cv::Point2f(0, SCREEN_HEIGHT)}; // TL TR BR BL
        if (projectedWorldCornersResults.size() > 0) {
            auto r = projectedWorldCornersResults[0];
            projection_corrected_world_corners[0].x = std::stof(r.get("x1").value);
            projection_corrected_world_corners[0].y = std::stof(r.get("y1").value);
            projection_corrected_world_corners[1].x = std::stof(r.get("x2").value);
            projection_corrected_world_corners[1].y = std::stof(r.get("y2").value);
            projection_corrected_world_corners[2].x = std::stof(r.get("x3").value);
            projection_corrected_world_corners[2].y = std::stof(r.get("y3").value);
            projection_corrected_world_corners[3].x = std::stof(r.get("x4").value);
            projection_corrected_world_corners[3].y = std::stof(r.get("y4").value);
        }

        window.clear(sf::Color(0, 0, 255, 255));
        renderTexture.clear();
        topRenderTexture.clear(sf::Color::Transparent);
        for (const auto &wish : genericGraphicsWishes) {
            auto sourceStr = wish.get("source").value;
            auto source = std::stoi(sourceStr);
            auto targetStr = wish.get("target").value;
            json j;
            try {
                j = json::parse(wish.get("graphics").value);
            } catch (json::parse_error &ex) {
                std::cout << "WARNING: json parse from program " << sourceStr << ": " << ex.what() << std::endl;
                continue;
            }
            // std::cout << "wish you had graphics" << source << std::endl;
            sf::Transform programTransform;
            sf::RenderTexture *textureTarget = &topRenderTexture;
            if (targetStr == "you") {
                for (const auto &seenProgramResult : seenProgramResults) {
                    auto id = std::stoi(seenProgramResult.get("id").value);
                    if (id == source) {
                        auto x1 = std::stoi(seenProgramResult.get("x1").value);
                        auto y1 = std::stoi(seenProgramResult.get("y1").value);
                        auto x2 = std::stoi(seenProgramResult.get("x2").value);
                        auto y2 = std::stoi(seenProgramResult.get("y2").value);
                        programTransform.translate(x1, y1);
                        auto angleDegrees = atan2(y2 - y1, x2 - x1) * 180 / 3.14159265;
                        programTransform.rotate(angleDegrees);
                    }
                }
                textureTarget = &renderTexture;
            } else if (targetStr == "all") {
                textureTarget = &renderTexture;
            }

            for (auto &g : j) {
                try {
                    const auto typ = g["type"];
                    if (typ == "rectangle") {
                        auto x = g["options"]["x"];
                        auto y = g["options"]["y"];
                        auto w = g["options"]["w"];
                        auto h = g["options"]["h"];
                        auto stroke_width = g["options"]["stroke_width"];
                        sf::RectangleShape rectangle(sf::Vector2f(w, h));
                        rectangle.setPosition(x, y);
                        rectangle.setFillColor(sf::Color{g["options"]["fill"][0], g["options"]["fill"][1], g["options"]["fill"][2], g["options"]["fill"][3]});
                        rectangle.setOutlineColor(sf::Color{g["options"]["stroke"][0], g["options"]["stroke"][1], g["options"]["stroke"][2], g["options"]["stroke"][3]});
                        rectangle.setOutlineThickness(stroke_width);
                        textureTarget->draw(rectangle, programTransform);
                    } else if (typ == "ellipse") {
                        auto x = g["options"]["x"].get<double>();
                        auto y = g["options"]["y"].get<double>();
                        auto w = g["options"]["w"].get<double>() * 0.5;
                        auto h = g["options"]["h"].get<double>() * 0.5;
                        sf::CircleShape circle{};
                        circle.setPosition(x, y);
                        circle.setRadius(w); // TODO: support ellipse
                        circle.setFillColor(sf::Color{g["options"]["fill"][0], g["options"]["fill"][1], g["options"]["fill"][2], g["options"]["fill"][3]});
                        circle.setOutlineColor(sf::Color{g["options"]["stroke"][0], g["options"]["stroke"][1], g["options"]["stroke"][2], g["options"]["stroke"][3]});
                        circle.setOutlineThickness(g["options"]["stroke_width"]);
                        textureTarget->draw(circle, programTransform);
                    } else if (typ == "line") {
                        auto x1 = g["options"]["x1"].get<double>();
                        auto y1 = g["options"]["y1"].get<double>();
                        auto x2 = g["options"]["x2"].get<double>();
                        auto y2 = g["options"]["y2"].get<double>();
                        auto thickness = g["options"]["thickness"].get<double>();
                        sw::Line line{sf::Vector2f(x1, y1), sf::Vector2f(x2, y2), thickness, sf::Color{g["options"]["color"][0], g["options"]["color"][1], g["options"]["color"][2], g["options"]["color"][3]}};
                        textureTarget->draw(line, programTransform);
                    } else if (typ == "text") {
                        auto x = g["options"]["x"];
                        auto y = g["options"]["y"];
                        auto size = g["options"]["size"].get<int>();
                        sf::Text text;
                        std::string textContents = g["options"]["text"];
                        sf::String sfTmp = sf::String::fromUtf8(textContents.begin(), textContents.end());
                        text.setFont(font);
                        text.setString(sfTmp);
                        text.setFillColor(sf::Color{g["options"]["color"][0], g["options"]["color"][1], g["options"]["color"][2], g["options"]["color"][3]});
                        text.setCharacterSize(size);
                        text.setPosition(x, y);
                        textureTarget->draw(text, programTransform);
                    } else if (typ == "frame") {
                        auto x = g["options"]["x"];
                        auto y = g["options"]["y"];
                        auto scale = g["options"]["scale"];
                        auto clipx = g["options"]["clip_x"];
                        auto clipy = g["options"]["clip_y"];
                        auto clipw = g["options"]["clip_w"];
                        auto cliph = g["options"]["clip_h"];
                        if (clipw < 0) {
                            clipw = latestFrameTexture.getSize().x;
                            cliph = latestFrameTexture.getSize().y;
                        }
                        sf::RenderTexture subframeTexture;
                        if (!subframeTexture.create(clipw, cliph)) {
                            // error...
                        }
                        subframeTexture.clear();
                        // do we need to create a new sprite here?
                        sf::Sprite innerSprite{latestFrameTexture};
                        innerSprite.setOrigin(clipx, clipy);
                        // todo rotate
                        subframeTexture.draw(innerSprite);
                        subframeTexture.display();
                        const sf::Texture &subframeTextureCopy = subframeTexture.getTexture();
                        sf::Sprite sprite{subframeTextureCopy};
                        sprite.setPosition(sf::Vector2f(x, y));
                        sprite.setScale(scale, scale);
                        textureTarget->draw(sprite, programTransform);
                    } else if (typ == "image") {
                        auto x = g["options"]["x"];
                        auto y = g["options"]["y"];
                        auto scale = g["options"]["scale"];
                        auto filepath = g["options"]["filepath"];
                        sf::Sprite sprite;
                        sprite.setTexture(*getTexture(textureMap, filepath));
                        sprite.setPosition(sf::Vector2f(x, y));
                        sprite.setScale(scale, scale);
                        textureTarget->draw(sprite, programTransform);
                    } else if (typ == "movie") {
                        auto x = g["options"]["x"];
                        auto y = g["options"]["y"];
                        auto w = g["options"]["w"];
                        auto h = g["options"]["h"];
                        auto filepath = g["options"]["filepath"];
                        auto movie = getMovie(movieMap, filepath);
                        movie->fit(x, y, w, h);
                        if (movie->getStatus() != sfe::Playing) {
                            movie->play();
                        }
                        movie->update();
                        textureTarget->draw(*movie, programTransform);
                    }
                } catch (const std::exception &e) {
                    std::cout << "Exception when running graphics " << g << " " << e.what() << std::endl;
                }
            }
        }

        renderTexture.display();
        topRenderTexture.display();
        const sf::Texture &renderTextureCopy = renderTexture.getTexture();
        sw::ElasticSprite sprite{renderTextureCopy};
        sprite.setTextureFlipY(true);
        // sprite.setUseShader(false);
        sprite.activatePerspectiveInterpolation();
        sprite.setVertexOffset(0, {projection_corrected_world_corners[0].x, projection_corrected_world_corners[0].y});                                // TL
        sprite.setVertexOffset(1, {projection_corrected_world_corners[3].x, projection_corrected_world_corners[3].y - SCREEN_HEIGHT});                // BL
        sprite.setVertexOffset(2, {projection_corrected_world_corners[2].x - SCREEN_WIDTH, projection_corrected_world_corners[2].y - SCREEN_HEIGHT}); // BR
        sprite.setVertexOffset(3, {projection_corrected_world_corners[1].x - SCREEN_WIDTH, projection_corrected_world_corners[1].y});                 // TR
        // sprite.setVertexOffset(0, {-calibration[0].first, -calibration[0].second}); // TL
        // sprite.setVertexOffset(1, {-calibration[3].first, CAMERA_HEIGHT - calibration[3].second});               // BL
        // sprite.setVertexOffset(2, {CAMERA_WIDTH - calibration[2].first, CAMERA_HEIGHT - calibration[2].second}); // BR
        // sprite.setVertexOffset(3, {CAMERA_WIDTH - calibration[1].first, -calibration[1].second});  // TR
        window.draw(sprite);

        const sf::Texture &topRenderTextureCopy = topRenderTexture.getTexture();
        sf::Sprite topRenderTextureSprite(topRenderTextureCopy);
        window.draw(topRenderTextureSprite);
        window.display();
    }

  private:
    sf::Texture *
    getTexture(std::map<std::string, sf::Texture *> &m_textureMap, const std::string filePath) {
        // copied from https://github.com/Jfeatherstone/SFMLResource/blob/master/src/ResourceManager.cpp
        // Search through the map to see if there is already an entry
        for (auto element : m_textureMap) {
            // We also want to check that the path is not invalid, as otherwise it would just be
            // stuck as invalid, because it would technically have an entry in the map
            if (element.first == filePath) // && element.first != m_invalidTexture)
                return element.second;
        }

        // If the code has made it to this point, it hasn't found a matching entry
        // in the map. We use the new keyword because we want to store these variables
        // outside of the stack
        sf::Texture *texture = new sf::Texture();

        // If the texture doesn't load properly, we assign our invalid texture to it
        if (!texture->loadFromFile(filePath)) {
            // TODO:
            // texture->loadFromFile(m_invalidTexture);
        }

        m_textureMap[filePath] = texture;

        return m_textureMap[filePath];
    }

    sfe::Movie *
    getMovie(std::map<std::string, sfe::Movie *> &m_movieMap, const std::string filePath) {
        // copied from https://github.com/Jfeatherstone/SFMLResource/blob/master/src/ResourceManager.cpp
        // Search through the map to see if there is already an entry
        for (auto element : m_movieMap) {
            // We also want to check that the path is not invalid, as otherwise it would just be
            // stuck as invalid, because it would technically have an entry in the map
            if (element.first == filePath) // && element.first != m_invalidTexture)
                return element.second;
        }

        // If the code has made it to this point, it hasn't found a matching entry
        // in the map. We use the new keyword because we want to store these variables
        // outside of the stack
        sfe::Movie *movie = new sfe::Movie();

        // If the texture doesn't load properly, we assign our invalid texture to it
        if (!movie->openFromFile(filePath)) {
            std::cout << "ERROR LOADING MOVIE" << std::endl;
            // TODO: show some visual error if file not found
        }

        m_movieMap[filePath] = movie;

        return m_movieMap[filePath];
    }
};