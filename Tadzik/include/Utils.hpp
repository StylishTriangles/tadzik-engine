#ifndef UTILS_HPP
#define UTILS_HPP

#include <SFML/Graphics.hpp>
#include <sstream>

namespace Utils{

sf::FloatRect getBiggerBoundingBox(sf::Sprite& sp);
void drawBoundingBox(sf::Sprite& sp, sf::RenderWindow* window);
float randF(float from, float to);
bool isMouseOnSprite(sf::Sprite& sp, sf::RenderWindow* window);
std::string stringify(int x);
float det3f(sf::Vector3f top, sf::Vector3f mid, sf::Vector3f bot);

}

#endif //UTILS_HPP
