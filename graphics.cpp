#include "SelbaWard/ElasticSprite.cpp"
#include "SelbaWard/Line.cpp"
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
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
            "image", &Illumination::image);
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
        int fontSize = get_int_from_lua_table(opts, "size", 30);
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
        int x = opts.get_or("x", 0);
        int y = opts.get_or("y", 0);
        int scale = opts.get_or("scale", 1);
        std::string filepath = opts["filepath"];
        graphics.push_back({{"type", "image"}, {"options", {{"x", x}, {"y", y}, {"scale", scale}, {"filepath", filepath}}}});
    }
};
std::ostream &operator<<(std::ostream &os, const Illumination &ill) {
    os << ill.graphics;
    return os;
}

class GraphicsManager {
  private:
    std::map<std::string, sf::Texture *> textureMap;
    sf::Font font;
    sf::RenderTexture renderTexture;
    sf::RenderTexture topRenderTexture;
    int SCREEN_WIDTH;
    int SCREEN_HEIGHT;

  public:
    GraphicsManager(int _SCREEN_WIDTH, int _SCREEN_HEIGHT) : SCREEN_WIDTH(_SCREEN_WIDTH), SCREEN_HEIGHT(_SCREEN_HEIGHT) {}

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

    void draw_graphics(sf::RenderWindow &window,
                       std::vector<QueryResult> &genericGraphicsWishes,
                       std::vector<int> &main_seen_program_ids,
                       std::vector<std::vector<cv::Point2f>> &main_seen_program_corners,
                       std::vector<cv::Point2f> &projection_corrected_world_corners,
                       sf::Texture &latestFrameTexture) {
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
                int main_seen_program_ids_index = 0;
                for (auto &id : main_seen_program_ids) {
                    if (id == source) {
                        cv::Point2f corner0 = main_seen_program_corners.at(main_seen_program_ids_index).at(0);
                        cv::Point2f corner1 = main_seen_program_corners.at(main_seen_program_ids_index).at(1);
                        programTransform.translate(corner0.x, corner0.y);
                        auto angleDegrees = atan2(corner1.y - corner0.y, corner1.x - corner0.x) * 180 / 3.14159265;
                        programTransform.rotate(angleDegrees);
                    }
                    main_seen_program_ids_index++;
                }
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
};