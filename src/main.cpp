// main.cpp (suderinta versija)
#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>

// *** PASTABA: įsitikink, kad šie include atitinka tikslų failų vardą tavo diske (case-sensitive) ***
#include "Objects.h"
#include "Utils.h"
#include "Defines.h"

#include <cstdlib>
#include <ctime>
#include <string>
#include <optional>
#include <iostream>

using namespace sf;
using namespace std;

const int PLATES_AMOUNT = 100;
const float PLATE_FALL_SPEED = 3.0f;
const float RAIN_PLATE_WIDTH = 40.f;
const float RAIN_PLATE_HEIGHT = 80.f;

struct PlateEx : Plate
{
    bool isRain = false;
};

// Atnaujina plokšteles (judėjimas + collision)
void UpdatePlates(Player& player, PlateEx plates[], int platesAmount, float& score, int& missedPlates)
{
    for (int i = 0; i < platesAmount; ++i)
    {
        PlateEx& plate = plates[i];
        if (!plate.active) continue;

        plate.y += PLATE_FALL_SPEED;

        float plateWidth = plate.isRain ? RAIN_PLATE_WIDTH : PLATES_WIDTH;
        float plateHeight = plate.isRain ? RAIN_PLATE_HEIGHT : PLATES_HEIGHT;

        bool hit = (player.x + PLAYER_WIDTH > plate.x) &&
                   (player.x < plate.x + plateWidth) &&
                   (plate.y <= player.y && plate.y + plateHeight >= player.y);

        if (hit)
        {
            if (plate.isRain)
            {
                score = 0.f;               // dezikas resetina score
                plate.active = false;
                plate.counted = true;
            }
            else
            {
                score += 1.f;             // normaliai +1
                plate.active = false;
                plate.counted = true;
            }
        }

        if (plate.y > WINDOW_HEIGHT && !plate.counted)
        {
            if (!plate.isRain) missedPlates += 1;
            plate.counted = true;
            plate.active = false;
        }
    }
}

// Spawnina plokštelę virš ekrano
void SpawnPlate(PlateEx plates[], int platesAmount, float newX, bool isRain)
{
    for (int i = 0; i < platesAmount; ++i)
    {
        if (!plates[i].active)
        {
            plates[i].x = newX;
            plates[i].y = -(isRain ? RAIN_PLATE_HEIGHT : PLATES_HEIGHT);
            plates[i].active = true;
            plates[i].counted = false;
            plates[i].isRain = isRain;
            break;
        }
    }
}

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    // VideoMode: aiškiai perduodame Vector2u (suderinama su SFML 2.6+)
    RenderWindow app(VideoMode(Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Indus Simulator - Survive In Vilnius DLC");
    app.setFramerateLimit(60);

    if (!app.isOpen())
    {
        cerr << "KLAIDA: nepavyko sukurti lango." << endl;
        return 1;
    }

    // Textures
    Texture tBackground, tPlayer, tPlatform, tPlatformAlt, tDezikas;
    if (!tBackground.loadFromFile("resources/background.png")) cerr << "Fail: background.png\n";
    if (!tPlayer.loadFromFile("resources/him.png")) cerr << "Fail: him.png\n";
    if (!tPlatform.loadFromFile("resources/bolt.png")) cerr << "Fail: bolt.png\n";
    if (!tPlatformAlt.loadFromFile("resources/wolt.png")) cerr << "Fail: wolt.png\n";
    if (!tDezikas.loadFromFile("resources/dezikas.png")) cerr << "Fail: dezikas.png\n";

    Font font;
    if (!font.openFromFile("resources/arialbd.ttf")) cerr << "Fail: arialbd.ttf\n";

    // Text objects (naudojame Vector2f su setPosition)
    Text scoreText(font), missedText(font), bestScoreText(font), rainWarning(font);
    scoreText.setCharacterSize(20);
    missedText.setCharacterSize(20);
    bestScoreText.setCharacterSize(20);
    rainWarning.setCharacterSize(28);

    scoreText.setFillColor(Color::Green);
    missedText.setFillColor(Color::Green);
    bestScoreText.setFillColor(Color::Yellow);
    rainWarning.setFillColor(Color::Blue);
    rainWarning.setOutlineColor(Color::White);
    rainWarning.setOutlineThickness(2);
    rainWarning.setString("CAUTION!!! DEODORANT!!!");

    // Dėmesio: setPosition prašo vieno Vector2 parameterio
    scoreText.setPosition(Vector2f(10.f, 10.f));
    missedText.setPosition(Vector2f(10.f, 50.f));
    bestScoreText.setPosition(Vector2f(10.f, 90.f));
    rainWarning.setPosition(Vector2f(WINDOW_WIDTH / 2.f - 180.f, WINDOW_HEIGHT / 2.f - 20.f));

    Sprite sprBackground(tBackground);
    Sprite sprPlayer(tPlayer);
    Sprite sprPlatform(tPlatform);
    Sprite sprRain(tDezikas);

    Player player;
    player.x = WINDOW_WIDTH / 2.f;
    player.y = MAX_PLAYER_Y;

    PlateEx plates[PLATES_AMOUNT];
    for (int i = 0; i < PLATES_AMOUNT; ++i)
    {
        plates[i].x = 0.f;
        plates[i].y = -PLATES_HEIGHT;
        plates[i].counted = false;
        plates[i].active = false;
        plates[i].isRain = false;
    }

    Clock clock;
    float spawnTimer = 0.f;
    const float spawnInterval = 1.0f;

    float score = 0.f, bestScore = 0.f;
    int missedPlates = 0;

    bool rainActive = false;
    int rainCount = 0;
    bool altTexture = false; // toggle only changes platform texture
    int lastScoreFor7Dezikas = -1;

    // vienas debounce kintamasis SPACE toggle
    bool spacePressedLastFrame = false;

    while (app.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // Event loop (patikriname Close)
        while (auto ev = app.pollEvent())
        {
            if (ev->is<Event::Closed>()) app.close();
        }

        // SPACE debounce (vienas paspaudimas = vienas toggle)
        bool spaceNow = Keyboard::isKeyPressed(Keyboard::Key::Space);
        if (spaceNow && !spacePressedLastFrame)
            altTexture = !altTexture;
        spacePressedLastFrame = spaceNow;

        // Player movement (polling)
        const float dx = 3.5f;
        if (Keyboard::isKeyPressed(Keyboard::Key::Left) || Keyboard::isKeyPressed(Keyboard::Key::A))
            player.x -= dx;
        if (Keyboard::isKeyPressed(Keyboard::Key::Right) || Keyboard::isKeyPressed(Keyboard::Key::D))
            player.x += dx;

        // clamp to screen
        if (player.x < 0.f) player.x = 0.f;
        if (player.x + PLAYER_WIDTH > WINDOW_WIDTH) player.x = WINDOW_WIDTH - PLAYER_WIDTH;

        // rain sequence trigger (20,40,...)
        int scoreInt = static_cast<int>(score);
        if (!rainActive && scoreInt >= 20 && (scoreInt % 20) == 0)
        {
            rainActive = true;
            rainCount = 0;
        }

        // spawn logic
        spawnTimer += deltaTime;
        if (spawnTimer >= spawnInterval)
        {
            spawnTimer = 0.f;
            float newX = static_cast<float>(rand() % (WINDOW_WIDTH - PLATES_WIDTH));

            if (rainActive && rainCount < 10)
            {
                SpawnPlate(plates, PLATES_AMOUNT, newX, true);
                rainCount++;
                if (rainCount >= 10) rainActive = false;
            }
            else
            {
                if (scoreInt > 0 && (scoreInt % 7) == 0 && scoreInt != lastScoreFor7Dezikas && !rainActive)
                {
                    SpawnPlate(plates, PLATES_AMOUNT, newX, true);
                    lastScoreFor7Dezikas = scoreInt;
                }
                else
                {
                    SpawnPlate(plates, PLATES_AMOUNT, newX, false);
                }
            }
        }

        // update plates (movement + collisions)
        UpdatePlates(player, plates, PLATES_AMOUNT, score, missedPlates);

        if (score > bestScore) bestScore = score;

        // drawing
        app.clear();
        app.draw(sprBackground);

        for (int i = 0; i < PLATES_AMOUNT; ++i)
        {
            PlateEx& p = plates[i];
            if (!p.active) continue;
            if (p.y < 0.f || p.y > WINDOW_HEIGHT) continue;

            if (p.isRain)
            {
                sprRain.setPosition(Vector2f(p.x, p.y));
                sprRain.setScale(Vector2f(RAIN_PLATE_WIDTH / tDezikas.getSize().x,
                                          RAIN_PLATE_HEIGHT / tDezikas.getSize().y));
                app.draw(sprRain);
            }
            else
            {
                sprPlatform.setPosition(Vector2f(p.x, p.y));
                sprPlatform.setTexture(altTexture ? tPlatformAlt : tPlatform);
                app.draw(sprPlatform);
            }
        }

        // player sprite: HIM only (we do NOT switch player -> dezikas)
        sprPlayer.setTexture(tPlayer);
        sprPlayer.setPosition(Vector2f(player.x, player.y));
        app.draw(sprPlayer);

        // UI text
        scoreText.setString("Deliveries: " + to_string(static_cast<int>(score)));
        missedText.setString("Got called by the n-word: " + to_string(missedPlates));
        bestScoreText.setString("Best score: " + to_string(static_cast<int>(bestScore)));

        if (altTexture)
        {
            scoreText.setFillColor(Color::Cyan);
            missedText.setFillColor(Color::Cyan);
        }
        else
        {
            scoreText.setFillColor(Color::Green);
            missedText.setFillColor(Color::Green);
        }

        app.draw(scoreText);
        app.draw(missedText);
        app.draw(bestScoreText);

        if (rainActive) app.draw(rainWarning);

        app.display();
    }

    return 0;
}