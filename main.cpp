#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>

#include "db.cpp"

using namespace std::chrono;

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

int main () {

        Database db{};

        sol::state lua;

        // open those basic lua libraries 
        // again, for print() and other basic utilities
        lua.open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::io, sol::lib::os);

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

        sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
        sf::Font font;
        // TODO: find font into project and load that
        if (!font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
            // error...
        }

        while (window.isOpen())
        {
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

            script1();
            script2();
            script3();
            // script4();
            script5();
            // script6();

            window.clear();

            auto graphicsWishes = db.select({"$ wish text $text at $x $y"});
            for (const auto &wish : graphicsWishes) {
                sf::Text text;
                text.setFont(font);
                text.setString(wish.Result.at("text").value);
                text.setFillColor(sf::Color::Red);
                text.setPosition(std::stod(wish.Result.at("x").value), std::stod(wish.Result.at("y").value));
                window.draw(text);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                sf::CircleShape shape(100.f);
                shape.setFillColor(sf::Color::Green);
                window.draw(shape);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                sf::Text text;
                text.setFont(font);
                text.setString("Hello");
                text.setFillColor(sf::Color::Red);
                window.draw(text);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
                script4();
            } else {
                db.cleanup("5");
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
                script6();
            } else {
                db.cleanup("3");
            }
            window.display();
        }

        return 0;
}