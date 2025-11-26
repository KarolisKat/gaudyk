#include <SFML/Graphics.hpp>
#include <SFML/System/Vector2.hpp>
#include "Objects.h" // NOTE: Patikrinkite antraštės failo pavadinimą (Objects.h ar objects.h)
#include "Utils.h" // NOTE: Patikrinkite antraštės failo pavadinimą (Utils.h ar utils.h)
#include "Defines.h" // NOTE: Patikrinkite antraštės failo pavadinimą (Defines.h ar defines.h)
#include <cstdlib>
#include <ctime>
#include <string>
#include <optional> // Reikalingas SFML 3.x pollEvent()
#include <iostream> // Pridėta klaidų pranešimams

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
    RenderWindow app(VideoMode(Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), "Indus Simulator - Survive In Vilnius DLC");
    app.setFramerateLimit(60);

    // Patikriname, ar langas sėkmingai sukurtas.
    if (!app.isOpen()) {
        cerr << "KLAIDA: SFML langas nepavyko sukurti. Patikrinkite WINDOW_WIDTH/HEIGHT konstantes arba SFML konfigūraciją." << endl;
        return 1;
    }

    Texture tBackground, tPlayer1, tPlayer2, tPlatform, tPlatformAlt;

    // Tikriname tekstūrų įkėlimą, kad nebūtų "nodiscard" įspėjimų
    if (!tBackground.loadFromFile("resources/background.png")) {
        cerr << "KLAIDA: Nepavyko įkelti fono tekstūros: resources/background.png" << endl;
    }
    if (!tPlayer1.loadFromFile("resources/him.png")) {
        cerr << "KLAIDA: Nepavyko įkelti žaidėjo 1 tekstūros: resources/him.png" << endl;
    }
    if (!tPlayer2.loadFromFile("resources/dezikas.png")) {
        cerr << "KLAIDA: Nepavyko įkelti žaidėjo 2/Deziko tekstūros: resources/dezikas.png" << endl;
    }
    if (!tPlatform.loadFromFile("resources/bolt.png")) {
        cerr << "KLAIDA: Nepavyko įkelti platformos 1 tekstūros: resources/bolt.png" << endl;
    }
    if (!tPlatformAlt.loadFromFile("resources/wolt.png")) {
        cerr << "KLAIDA: Nepavyko įkelti platformos 2 tekstūros: resources/wolt.png" << endl;
    }

    Font font;
    if (!font.openFromFile("resources/arialbd.ttf"))
    {
        cerr << "KLAIDA: Nepavyko įkelti šrifto: resources/arialbd.ttf" << endl;
        // Tęsiama be šrifto, jei nepavyko, bet tekstas gali nerodyti
    }

    Text scoreText(font), missedText(font), rainWarning(font), bestScoreText(font);

    scoreText.setFont(font); scoreText.setCharacterSize(20);
    scoreText.setFillColor(Color::Green); scoreText.setOutlineThickness(1); scoreText.setOutlineColor(Color::Black);
    scoreText.setPosition(Vector2f(10.f, 10.f));

    missedText.setFont(font); missedText.setCharacterSize(20);
    missedText.setFillColor(Color::Green); missedText.setOutlineThickness(1); missedText.setOutlineColor(Color::Black);
    missedText.setPosition(Vector2f(10.f, 50.f));

    bestScoreText.setFont(font); bestScoreText.setCharacterSize(20);
    bestScoreText.setFillColor(Color::Yellow); bestScoreText.setOutlineThickness(1); bestScoreText.setOutlineColor(Color::Black);
    bestScoreText.setPosition(Vector2f(10.f, 90.f));

    rainWarning.setFont(font); rainWarning.setCharacterSize(25);
    rainWarning.setFillColor(Color::Blue); rainWarning.setOutlineThickness(2); rainWarning.setOutlineColor(Color::White);
    rainWarning.setString("CAUTION!!! DEODORANT!!!");

    // Kadangi jūsų SFML versija negali pasiekti sf::FloatRect narių (left, top, width, height),
    // paliekame be setOrigin centravimo, kad išvengtume kompiliavimo klaidos.
    rainWarning.setPosition(Vector2f(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f));

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
    bool altTexture = false; // Būsena, ar naudojama alternatyvi tekstūra

    int lastScoreFor7Dezikas = -1;

    while (app.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // Naudojama teisinga SFML 3.x įvykių ciklo struktūra
        while (auto optionalEvent = app.pollEvent())
        {
            const Event& e = *optionalEvent;

            if (e.is<Event::Closed>())
            {
                app.close();
            }
            // Klavišo tikrinimas dabar tvarkomas toliau, naudojant isKeyPressed, nes Event handler'is neveikia
        }

        // Laikinas klavišo perjungimo tvarkymas, kol event::KeyPressed neveikia
        // ĮSPĖJIMAS: tai perjungia tekstūrą kiekviename kadre, kol laikomas SPACE
        if (Keyboard::isKeyPressed(Keyboard::Key::Space)) {
            // Šiek tiek geresnis perjungimas: tikrinti, ar klavišas buvo paspaustas tik vieną kartą
            static bool space_pressed = false;
            if (!space_pressed) {
                altTexture = !altTexture;
                space_pressed = true;
            }
        } else {
            static bool space_pressed = false;
            space_pressed = false;
        }

        const float dx = 3.5f;
        if (Keyboard::isKeyPressed(Keyboard::Key::Left) || Keyboard::isKeyPressed(Keyboard::Key::A)) player.x -= dx;
        if (Keyboard::isKeyPressed(Keyboard::Key::Right) || Keyboard::isKeyPressed(Keyboard::Key::D)) player.x += dx;

        if (player.x < 0) player.x = 0;
        if (player.x + PLAYER_WIDTH > WINDOW_WIDTH) player.x = WINDOW_WIDTH - PLAYER_WIDTH;

        if (!rainActive && score >= 20 && ((int)score % 20) == 0)
        {
            rainActive = true;
            rainCount = 0;
        }

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

        app.clear();
        app.draw(sprBackground);

        for (int i = 0; i < PLATES_AMOUNT; ++i)
        {
            if (plates[i].active && plates[i].y >= 0 && plates[i].y <= WINDOW_HEIGHT)
            {
                if (plates[i].isRain)
                {
                    sprRainPlate.setPosition(Vector2f(plates[i].x, plates[i].y));
                    sprRainPlate.setScale(Vector2f(RAIN_PLATE_WIDTH / tPlayer2.getSize().x, RAIN_PLATE_HEIGHT / tPlayer2.getSize().y));
                    app.draw(sprRainPlate);
                }
                else
                {
                    sprPlatform.setPosition(Vector2f(plates[i].x, plates[i].y));
                    if (altTexture) sprPlatform.setTexture(tPlatformAlt); else sprPlatform.setTexture(tPlatform);
                    app.draw(sprPlatform);
                }
            }
        }

        sprPlayer.setPosition(Vector2f(player.x, player.y));
        if (altTexture) sprPlayer.setTexture(tPlayer2); else sprPlayer.setTexture(tPlayer1);
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