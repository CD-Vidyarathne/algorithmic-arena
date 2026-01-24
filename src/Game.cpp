#include "Game.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

Game::Game()
    : m_window(sf::VideoMode(sf::Vector2u(1280, 720)), "Algorithmic Arena") {}

void Game::run() {
  while (m_window.isOpen()) {
    processEvents();
    render();
  }
}

void Game::processEvents() {
  while (const auto event = m_window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      m_window.close();
    }
  }
}

void Game::render() {
  m_window.clear(sf::Color::Cyan);
  m_window.display();
}
