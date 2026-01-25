#include "Game.h"
#include "Util/Logger.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

Game::Game()
    : window_(sf::VideoMode(sf::Vector2u(1280, 720)), "Algorithmic Arena") {
  Logger::get()->info("Game initialized");
}

void Game::run() {
  while (window_.isOpen()) {
    processEvents();
    render();
  }
}

void Game::processEvents() {
  while (const auto event = window_.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      Logger::get()->warn("Window closing event detected");
      window_.close();
    }
  }
}

void Game::render() {
  window_.clear(sf::Color::White);
  window_.display();
}
