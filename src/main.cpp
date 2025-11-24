#include <SFML/Graphics.hpp>
#include "Objects.h"
#include "Utils.h"
#include "Defines.h"
#include <cstdlib>
#include <ctime>
#include <string>

using namespace sf;
using namespace std;

// Maksimalus plokðteliø kiekis vienu metu
const int PLATES_AMOUNT = 100;
// Greitis, kuriuo krenta plokðtelës
const float PLATE_FALL_SPEED = 3.0f;

// Dezoderantø dydis
const float RAIN_PLATE_WIDTH = 40.f;
const float RAIN_PLATE_HEIGHT = 80.f;

// PlateEx struktûra – papildyta informacija apie plokðteles
struct PlateEx : Plate
{
    bool isRain = false; // ar tai lietaus plokðtelë (pavojinga)
};

// Funkcija, atnaujinanti plokðteliø padëtá ir þaidëjo sàveikà
void UpdatePlates(Player& player, PlateEx plates[], int platesAmount, float& score, int& missedPlates)
{
    for (int i = 0; i < platesAmount; ++i)
    {
        PlateEx& plate = plates[i];

        // jei plokðtelë neaktyvi, praleidþiame
        if (!plate.active) continue;

        // kiekvieno kadro plokðtelë juda þemyn
        plate.y += PLATE_FALL_SPEED;

        // nustatome plokðtelës dydá pagal tipà
        float plateWidth = plate.isRain ? RAIN_PLATE_WIDTH : PLATES_WIDTH;
        float plateHeight = plate.isRain ? RAIN_PLATE_HEIGHT : PLATES_HEIGHT;

        // tikriname ar þaidëjas palietë plokðtelæ galva
        bool hit = (player.x + PLAYER_WIDTH > plate.x) && (player.x < plate.x + plateWidth) &&
            (plate.y <= player.y && plate.y + plateHeight >= player.y);

        if (hit)
        {
            if (plate.isRain)
            {
                // lietaus plokðtelë: score reset
                score = 0;
                plate.active = false;
                plate.counted = true; // nebeskaièiuojame missed
            }
            else
            {
                // normalus platformø score
                score += 1;
                plate.active = false;
                plate.counted = true;
            }
        }

        // jei plokðtelë nukrito uþ ekrano ir nebuvo "counted", skaièiuojame missed
        if (plate.y > WINDOW_HEIGHT && !plate.counted)
        {
            if (!plate.isRain) missedPlates += 1;
            plate.counted = true;
            plate.active = false;
        }
    }
}

// Funkcija, spawninanti naujà plokðtelæ
void SpawnPlate(PlateEx plates[], int platesAmount, float newX, bool isRain)
{
    for (int i = 0; i < platesAmount; ++i)
    {
        if (!plates[i].active)
        {
            plates[i].x = newX; // atsitiktinë x koordinatë
            plates[i].y = -(isRain ? RAIN_PLATE_HEIGHT : PLATES_HEIGHT); // spawn virð ekrano
            plates[i].active = true;
            plates[i].counted = false;
            plates[i].isRain = isRain; // nurodome tipà
            break; // spawn tik vienos plokðtelës
        }
    }
}

int main()
{
    srand((unsigned)time(nullptr));
    RenderWindow app(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Indus Simulator - Survive In Vilnius DLC");
    app.setFramerateLimit(60);

    // Tekstûros
    Texture tBackground, tPlayer1, tPlayer2, tPlatform;
    tBackground.loadFromFile("resources/background.png"); // fonas
    tPlayer1.loadFromFile("resources/him.png");          // þaidëjas
    tPlayer2.loadFromFile("resources/dezikas.png");      // lietaus plokðtelës
    tPlatform.loadFromFile("resources/bolt.png");        // normalios plokðtelës

    // Ðriftas
    Font font;
    font.loadFromFile("resources/arialbd.ttf");

    // Teksto elementai score, missed ir rain warning
    Text scoreText, missedText, rainWarning;

    // Score tekstas
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(Color::Green);
    scoreText.setOutlineThickness(1);
    scoreText.setOutlineColor(Color::Black);
    scoreText.setPosition(10.f, 10.f);

    // Missed plokðteliø tekstas
    missedText.setFont(font);
    missedText.setCharacterSize(30);
    missedText.setFillColor(Color::Green);
    missedText.setOutlineThickness(1);
    missedText.setOutlineColor(Color::Black);
    missedText.setPosition(10.f, 50.f);

    // Lietaus plokðteliø áspëjimas ekrano viduryje
    rainWarning.setFont(font);
    rainWarning.setCharacterSize(25);
    rainWarning.setFillColor(Color::Blue);
    rainWarning.setOutlineThickness(2);
    rainWarning.setOutlineColor(Color::White);
    rainWarning.setString("CAUTION!!! DEODORANT!!!");
    FloatRect textRect = rainWarning.getLocalBounds();
    rainWarning.setOrigin(textRect.left + textRect.width / 2.0f,
        textRect.top + textRect.height / 2.0f);
    rainWarning.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f);

    // Sprite
    Sprite sprBackground(tBackground);
    Sprite sprPlayer(tPlayer1);
    Sprite sprPlatform(tPlatform);
    Sprite sprRainPlate(tPlayer2);

    // Þaidëjas
    Player player;
    player.x = WINDOW_WIDTH / 2;
    player.y = MAX_PLAYER_Y;

    // Plokðtelës masyvas
    PlateEx plates[PLATES_AMOUNT];
    for (int i = 0; i < PLATES_AMOUNT; ++i)
    {
        plates[i].x = 0;
        plates[i].y = -PLATES_HEIGHT; // pradþioje uþ ekrano
        plates[i].counted = false;
        plates[i].active = false;
        plates[i].isRain = false;
    }

    Clock clock;
    float spawnTimer = 0.0f;
    const float spawnInterval = 1.0f;

    float score = 0;
    int missedPlates = 0;

    bool rainActive = false; // ar vyksta lietaus plokðèiø seka
    int rainCount = 0;

    while (app.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // Eventø tikrinimas
        Event e;
        while (app.pollEvent(e))
            if (e.type == Event::Closed) app.close();

        // Þaidëjo judëjimas
        const float dx = 3.5f;
        if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A))
            player.x -= dx;
        if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D))
            player.x += dx;

        // ribojame þaidëjà prie ekrano kraðtø
        if (player.x < 0)
            player.x = 0;
        if (player.x + PLAYER_WIDTH > WINDOW_WIDTH)
            player.x = WINDOW_WIDTH - PLAYER_WIDTH;

        // Ar prasideda lietaus plokðteliø seka
        if (!rainActive && score >= 20 && ((int)score % 20) == 0)
        {
            rainActive = true;
            rainCount = 0;
        }

        // Spawn naujos plokðtelës kas spawnInterval sekundþiø
        spawnTimer += deltaTime;
        if (spawnTimer >= spawnInterval)
        {
            spawnTimer = 0.0f;
            float newX = float(rand() % (WINDOW_WIDTH - PLATES_WIDTH));

            if (rainActive && rainCount < 10)
            {
                SpawnPlate(plates, PLATES_AMOUNT, newX, true);
                rainCount++;
                if (rainCount >= 10) rainActive = false; // baigëme lietaus sekà
            }
            else if (!rainActive)
            {
                SpawnPlate(plates, PLATES_AMOUNT, newX, false);
            }
        }

        // Atkuriame plokðteliø judëjimà ir sàveikà su þaidëju
        UpdatePlates(player, plates, PLATES_AMOUNT, score, missedPlates);

        // Pieðimas
        app.clear();
        app.draw(sprBackground);

        // Visø plokðteliø pieðimas
        for (int i = 0; i < PLATES_AMOUNT; ++i)
        {
            if (plates[i].active && plates[i].y >= 0 && plates[i].y <= WINDOW_HEIGHT)
            {
                if (plates[i].isRain)
                {
                    sprRainPlate.setPosition(plates[i].x, plates[i].y);
                    sprRainPlate.setScale(RAIN_PLATE_WIDTH / tPlayer2.getSize().x,
                        RAIN_PLATE_HEIGHT / tPlayer2.getSize().y);
                    app.draw(sprRainPlate);
                }
                else
                {
                    sprPlatform.setPosition(plates[i].x, plates[i].y);
                    app.draw(sprPlatform);
                }
            }
        }

        // Þaidëjas
        sprPlayer.setPosition(player.x, player.y);
        app.draw(sprPlayer);

        // Score ir missed
        scoreText.setString("Deliveries: " + to_string((int)score));
        missedText.setString("Got called the n-word: " + to_string(missedPlates));
        app.draw(scoreText);
        app.draw(missedText);

        // Lietaus áspëjimas ekrano viduryje
        if (rainActive)
        {
            app.draw(rainWarning);
        }

        app.display();
    }

    return 0;
}
