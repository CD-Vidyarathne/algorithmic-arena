#include "Game.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <spdlog/spdlog.h>

Game::Game()
    : m_window(sf::VideoMode(sf::Vector2u(1280, 720)), "Algorithmic Arena") {
  spdlog::info("Game Initialized");
}

void Game::run() {
  while (m_window.isOpen()) {
    processEvents();
    render();
  }
}

void Game::processEvents() {
  while (const auto event = m_window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      spdlog::warn("Window close event detected");
      m_window.close();
    }
  }
}

void Game::render() {
  m_window.clear(sf::Color::White);
  m_window.display();
}
