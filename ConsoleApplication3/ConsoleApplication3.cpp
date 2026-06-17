#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp> 
#include <iostream>
#include <vector>
#include <random>
#include <memory>
#include <numeric>
#include <algorithm>
#include <optional>
#include <cmath>

// --- VISUAL STUDIO AUTO-LINKER ---
#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")
#pragma comment(lib, "sfml-audio.lib")

const int ROWS = 3;
const int COLS = 5;
const int TOTAL_WINDOWS = ROWS * COLS;
const int WIN_WIDTH = 180;
const int WIN_HEIGHT = 110;
const int GAP = 25;

const float PI = 3.14159265f;

struct FlagWindow {
    std::unique_ptr<sf::RenderWindow> window;
    sf::Sprite sprite;
    sf::Vector2f currentDesktopPos;
    sf::Vector2f targetDesktopPos;
    bool isCorrect = false;

    FlagWindow(const sf::Texture& texture, sf::Vector2f startPos)
        : sprite(texture), currentDesktopPos(startPos), targetDesktopPos(startPos) {

        window = std::make_unique<sf::RenderWindow>(
            sf::VideoMode({ static_cast<unsigned int>(WIN_WIDTH), static_cast<unsigned int>(WIN_HEIGHT) }),
            "FOCUS!",
            sf::Style::Titlebar
        );
        window->setFramerateLimit(60);

        sf::Vector2u texSize = texture.getSize();
        sprite.setScale({ static_cast<float>(WIN_WIDTH) / texSize.x, static_cast<float>(WIN_HEIGHT) / texSize.y });
        sprite.setPosition({ 0.f, 0.f });

        window->setPosition(sf::Vector2i({ static_cast<int>(currentDesktopPos.x), static_cast<int>(currentDesktopPos.y) }));
    }
};

sf::Vector2f calculateGridPosition(int index, unsigned int screenWidth, unsigned int screenHeight) {
    int row = index / COLS;
    int col = index % COLS;

    int totalGridWidth = (COLS * WIN_WIDTH) + ((COLS - 1) * GAP);
    int totalGridHeight = (ROWS * WIN_HEIGHT) + ((ROWS - 1) * GAP);

    int startX = (static_cast<int>(screenWidth) - totalGridWidth) / 2;
    int startY = (static_cast<int>(screenHeight) - totalGridHeight) / 2;

    float x = static_cast<float>(startX + col * (WIN_WIDTH + GAP));
    float y = static_cast<float>(startY + row * (WIN_HEIGHT + GAP));
    return { x, y };
}

int main() {
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    unsigned int screenWidth = desktop.size.x;
    unsigned int screenHeight = desktop.size.y;

    sf::Texture flagTexture;
    if (!flagTexture.loadFromFile("poland.png")) {
        std::cerr << "Error: Place 'poland.png' inside your project directory!" << std::endl;
        return -1;
    }

    sf::Music backgroundMusic;
    bool musicLoaded = true;
    if (!backgroundMusic.openFromFile("Poland Limbo meme.mp3")) {
        std::cerr << "Warning: Could not open 'Poland Limbo meme.mp3'! Running game silent." << std::endl;
        musicLoaded = false;
    }

    std::vector<std::unique_ptr<FlagWindow>> flags;
    flags.reserve(TOTAL_WINDOWS);
    for (int i = 0; i < TOTAL_WINDOWS; ++i) {
        sf::Vector2f initialPos = calculateGridPosition(i, screenWidth, screenHeight);
        flags.push_back(std::make_unique<FlagWindow>(flagTexture, initialPos));
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> indexDist(0, TOTAL_WINDOWS - 1);

    int correctIndex = indexDist(gen);
    flags[correctIndex]->isCorrect = true;
    flags[correctIndex]->sprite.setColor(sf::Color::Green);
    flags[correctIndex]->window->setTitle("TRACK ME!");

    if (musicLoaded) {
        backgroundMusic.play();
    }

    sf::Clock gameClock;
    sf::Clock shuffleIntervalClock;

    bool isShuffling = false;
    bool isSpinningCircle = false;
    bool isWaitingForClick = false;
    bool gameEnded = false;

    float circleOrbitAngle = 0.0f;

    std::vector<int> slotMapping(TOTAL_WINDOWS);
    std::iota(slotMapping.begin(), slotMapping.end(), 0);

    while (true) {
        float elapsedTime = gameClock.getElapsedTime().asSeconds();
        bool anyWindowOpen = false;

        // --- ADJUSTED TIMELINE PHASES ---
        // 0.0s to 3.0s -> Look at the target green window
        // 3.0s to 13.0s -> Rapid Grid Shuffling buildup
        if (elapsedTime > 3.0f && elapsedTime < 13.0f && !isShuffling) {
            isShuffling = true;
            flags[correctIndex]->sprite.setColor(sf::Color::White);
            for (auto& flag : flags) {
                if (flag->window->isOpen()) flag->window->setTitle("???");
            }
        }

        // 13.0s to 18.0s -> Shifts EXACTLY on 13 seconds to enter the spinning orbit layout (The Drop!)
        if (elapsedTime > 13.0f && elapsedTime < 18.0f && isShuffling) {
            isShuffling = false;
            isSpinningCircle = true;
            for (auto& flag : flags) {
                if (flag->window->isOpen()) flag->window->setTitle("SPINNING...");
            }
        }

        // 18.0s+ -> Freeze and wait for choice
        if (elapsedTime > 18.0f && isSpinningCircle) {
            isSpinningCircle = false;
            isWaitingForClick = true;
            for (auto& flag : flags) {
                if (flag->window->isOpen()) flag->window->setTitle("CHOOSE IT!");
            }
        }

        // --- RHYTHMIC SHUFFLE BEAT ---
        if (isShuffling && shuffleIntervalClock.getElapsedTime().asSeconds() > 0.48f) {
            shuffleIntervalClock.restart();
            std::shuffle(slotMapping.begin(), slotMapping.end(), gen);
            for (int i = 0; i < TOTAL_WINDOWS; ++i) {
                flags[i]->targetDesktopPos = calculateGridPosition(slotMapping[i], screenWidth, screenHeight);
            }
        }

        // --- GRAND FINALE: 15-WINDOW CIRCLE ORBIT ---
        if (isSpinningCircle) {
            circleOrbitAngle += 0.025f;

            float screenCenterX = static_cast<float>(screenWidth) / 2.0f;
            float screenCenterY = static_cast<float>(screenHeight) / 2.0f;
            float radius = 420.0f;

            for (int i = 0; i < TOTAL_WINDOWS; ++i) {
                float currentWindowAngle = circleOrbitAngle + (i * (2.0f * PI / TOTAL_WINDOWS));
                float targetX = screenCenterX + (radius * std::cos(currentWindowAngle)) - (WIN_WIDTH / 2.0f);
                float targetY = screenCenterY + (radius * std::sin(currentWindowAngle)) - (WIN_HEIGHT / 2.0f);
                flags[i]->targetDesktopPos = { targetX, targetY };
            }
        }

        // --- MAIN SYSTEM ENGINE BLOCK ---
        for (int i = 0; i < TOTAL_WINDOWS; ++i) {
            if (!flags[i]->window->isOpen()) continue;
            anyWindowOpen = true;

            while (const std::optional event = flags[i]->window->pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    flags[i]->window->close();
                }

                if (isWaitingForClick && !gameEnded) {
                    if (event->is<sf::Event::MouseButtonPressed>()) {
                        gameEnded = true;

                        if (flags[i]->isCorrect) {
                            std::cout << "SUCCESS! FOCUS MASTER!" << std::endl;
                            flags[i]->sprite.setColor(sf::Color::Green);
                            flags[i]->window->setTitle("WINNER!");
                        }
                        else {
                            std::cout << "INCORRECT CHOICE!" << std::endl;
                            flags[i]->sprite.setColor(sf::Color::Red);
                            flags[i]->window->setTitle("LOSER!");
                            flags[correctIndex]->sprite.setColor(sf::Color::Green);
                            flags[correctIndex]->window->setTitle("HERE!");
                        }
                    }
                }
            }

            // Snappy Video-Speed Windows Movement Logic
            if (isShuffling || isSpinningCircle) {
                sf::Vector2f diff = flags[i]->targetDesktopPos - flags[i]->currentDesktopPos;
                float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                if (distance > 1.0f) {
                    flags[i]->currentDesktopPos += diff * 0.32f;
                }
                else {
                    flags[i]->currentDesktopPos = flags[i]->targetDesktopPos;
                }

                flags[i]->window->setPosition(sf::Vector2i({
                    static_cast<int>(flags[i]->currentDesktopPos.x),
                    static_cast<int>(flags[i]->currentDesktopPos.y)
                    }));
            }

            flags[i]->window->clear(sf::Color(20, 20, 20));
            flags[i]->window->draw(flags[i]->sprite);
            flags[i]->window->display();
        }

        if (!anyWindowOpen) break;
    }

    return 0;
}