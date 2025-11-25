#include <SFML/Graphics.hpp>
#include "Objects.h"
#include "Utils.h"
#include "Defines.h"
#include <cstdlib>
#include <ctime>
#include <string>

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

// Atnaujina plokðteliø padëtá ir sàveikà su þaidëju
void UpdatePlates(Player& player, PlateEx plates[], int platesAmount, float& score, int& missedPlates)
{
    for (int i = 0; i < platesAmount; ++i)
    {
        PlateEx& plate = plates[i];
        if (!plate.active) continue;

        plate.y += PLATE_FALL_SPEED;

        float plateWidth = plate.isRain ? RAIN_PLATE_WIDTH : PLATES_WIDTH;
        float plateHeight = plate.isRain ? RAIN_PLATE_HEIGHT : PLATES_HEIGHT;

        bool hit = (player.x + PLAYER_WIDTH > plate.x) && (player.x < plate.x + plateWidth) &&
            (plate.y <= player.y && plate.y + plateHeight >= player.y);

        if (hit)
        {
            if (plate.isRain)
            {
                score = 0;
                plate.active = false;
                plate.counted = true;
            }
            else
            {
                score += 1;
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

// Spawnina naujà plokðtelæ
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
    srand((unsigned)time(nullptr));
    RenderWindow app(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Indus Simulator - Survive In Vilnius DLC");
    app.setFramerateLimit(60);

    Texture tBackground, tPlayer1, tPlayer2, tPlatform, tPlatformAlt;
    tBackground.loadFromFile("resources/background.png");
    tPlayer1.loadFromFile("resources/him.png");
    tPlayer2.loadFromFile("resources/dezikas.png");
    tPlatform.loadFromFile("resources/bolt.png");
    tPlatformAlt.loadFromFile("resources/wolt.png");

    Font font;
    font.loadFromFile("resources/arialbd.ttf");

    Text scoreText, missedText, rainWarning, bestScoreText;

    scoreText.setFont(font); scoreText.setCharacterSize(20);
    scoreText.setFillColor(Color::Green); scoreText.setOutlineThickness(1); scoreText.setOutlineColor(Color::Black);
    scoreText.setPosition(10.f, 10.f);

    missedText.setFont(font); missedText.setCharacterSize(20);
    missedText.setFillColor(Color::Green); missedText.setOutlineThickness(1); missedText.setOutlineColor(Color::Black);
    missedText.setPosition(10.f, 50.f);

    bestScoreText.setFont(font); bestScoreText.setCharacterSize(20);
    bestScoreText.setFillColor(Color::Yellow); bestScoreText.setOutlineThickness(1); bestScoreText.setOutlineColor(Color::Black);
    bestScoreText.setPosition(10.f, 90.f);

    rainWarning.setFont(font); rainWarning.setCharacterSize(25);
    rainWarning.setFillColor(Color::Blue); rainWarning.setOutlineThickness(2); rainWarning.setOutlineColor(Color::White);
    rainWarning.setString("CAUTION!!! DEODORANT!!!");
    FloatRect textRect = rainWarning.getLocalBounds();
    rainWarning.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    rainWarning.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);

    Sprite sprBackground(tBackground);
    Sprite sprPlayer(tPlayer1);
    Sprite sprPlatform(tPlatform);
    Sprite sprRainPlate(tPlayer2);

    Player player; player.x = WINDOW_WIDTH / 2; player.y = MAX_PLAYER_Y;

    PlateEx plates[PLATES_AMOUNT];
    for (int i = 0; i < PLATES_AMOUNT; ++i)
    {
        plates[i].x = 0; plates[i].y = -PLATES_HEIGHT;
        plates[i].counted = false; plates[i].active = false; plates[i].isRain = false;
    }

    Clock clock;
    float spawnTimer = 0.0f;
    const float spawnInterval = 1.0f;

    float score = 0, bestScore = 0;
    int missedPlates = 0;

    bool rainActive = false;
    int rainCount = 0;
    bool altTexture = false;

    int lastScoreFor7Dezikas = -1; // fiksuoja paskutiná score, kai spawnintas dezikas kas 7

    while (app.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        Event e;
        while (app.pollEvent(e))
        {
            if (e.type == Event::Closed) app.close();
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Space)
                altTexture = !altTexture;
        }

        const float dx = 3.5f;
        if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) player.x -= dx;
        if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) player.x += dx;

        if (player.x < 0) player.x = 0;
        if (player.x + PLAYER_WIDTH > WINDOW_WIDTH) player.x = WINDOW_WIDTH - PLAYER_WIDTH;

        // Deziko lietus startas
        if (!rainActive && score >= 20 && ((int)score % 20) == 0)
        {
            rainActive = true;
            rainCount = 0;
        }

        // Spawn
        spawnTimer += deltaTime;
        if (spawnTimer >= spawnInterval)
        {
            spawnTimer = 0.0f;
            float newX = float(rand() % (WINDOW_WIDTH - PLATES_WIDTH));

            if (rainActive && rainCount < 10)
            {
                SpawnPlate(plates, PLATES_AMOUNT, newX, true);
                rainCount++;
                if (rainCount >= 10) rainActive = false;
            }
            else
            {
                // Dezikas kas 7 score vienà kartà
                if ((int)score > 0 && ((int)score % 7 == 0) && ((int)score != lastScoreFor7Dezikas))
                {
                    SpawnPlate(plates, PLATES_AMOUNT, newX, true);
                    lastScoreFor7Dezikas = (int)score;
                }
                else
                {
                    SpawnPlate(plates, PLATES_AMOUNT, newX, false);
                }
            }
        }

        UpdatePlates(player, plates, PLATES_AMOUNT, score, missedPlates);

        if (score > bestScore) bestScore = score;

        // Pieðimas
        app.clear();
        app.draw(sprBackground);

        for (int i = 0; i < PLATES_AMOUNT; ++i)
        {
            if (plates[i].active && plates[i].y >= 0 && plates[i].y <= WINDOW_HEIGHT)
            {
                if (plates[i].isRain)
                {
                    sprRainPlate.setPosition(plates[i].x, plates[i].y);
                    sprRainPlate.setScale(RAIN_PLATE_WIDTH / tPlayer2.getSize().x, RAIN_PLATE_HEIGHT / tPlayer2.getSize().y);
                    app.draw(sprRainPlate);
                }
                else
                {
                    sprPlatform.setPosition(plates[i].x, plates[i].y);
                    if (altTexture) sprPlatform.setTexture(tPlatformAlt); else sprPlatform.setTexture(tPlatform);
                    app.draw(sprPlatform);
                }
            }
        }

        sprPlayer.setPosition(player.x, player.y);
        app.draw(sprPlayer);

        scoreText.setString("Deliveries: " + to_string((int)score));
        missedText.setString("Got called the n-word: " + to_string(missedPlates));
        bestScoreText.setString("Best score: " + to_string((int)bestScore));

        if (altTexture) { scoreText.setFillColor(Color::Cyan); missedText.setFillColor(Color::Cyan); }
        else { scoreText.setFillColor(Color::Green); missedText.setFillColor(Color::Green); }

        app.draw(scoreText); app.draw(missedText); app.draw(bestScoreText);

        if (rainActive) app.draw(rainWarning);

        app.display();
    }

    return 0;
}
