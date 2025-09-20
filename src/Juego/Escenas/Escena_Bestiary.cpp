#include "Escena_Bestiary.hpp"
#include "../objetos/Texto.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../../Motor/Render/Render.hpp"
#include <array>
#include <vector>
#include <string>

#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorEscenas.hpp"

#define BESTIARY_BG_WIDTH 1250.f
#define BESTIARY_BG_HEIGHT 700.f


namespace IVJ
{
    // Helper: enemy names and descriptions
    std::array<std::string, 10> ENEMY_NAMES = {
        "Errant", "Crawler", "Berserker", "Shielded", "Summoner",
        "Burglar", "Blaster", "Jockey", "Burster", "Chongus"
    };
    std::array<std::string, 10> ENEMY_DESCRIPTIONS = {
        "A wandering zombie.", "It crawls.", "Strong and aggressive.", "Has a shield.",
        "It summons zombies.", "Steals items.", "Explodes randomly.", "Extremely quick.",
        "Throws projectiles.", "Huge and tough."
    };
    std::array<std::string, 10> ENEMY_SPRITES = {
        "hojaErrante", "hojaCrawler", "hojaBerserker", "hojaShielded", "hojaSummoner",
        "hojaBurglar", "hojaKamikaze", "hojaJockey", "hojaSpitter", "hojaChongus"
    };

    Escena_Bestiary::Escena_Bestiary()
        : CE::Escena{}
    {
            enemiesKilledStats = std::make_shared<std::array<int, 10>>();
            enemiesKilledStats->fill(0); // mark 1 for testing as discovered
            // NOTE: currently marking everything as discovered causes undefined behavior, since

            /*if (!DEBUG)
            {
                // For testing purposes, mark some enemies as discovered
                (*enemiesKilledStats)[0] = 1; // Errant
            }*/
    }

    void Escena_Bestiary::addMenuOptPositions()
    {
        // TODO: adjust the X offset based from the text length so the icon
        // is always at the same distance from the text
        for (auto i = 0; i < enemyNameTexts.size(); ++i)
        {
            constexpr float yOffset = 33.f;
            constexpr float xOffset = 300.f;
            menuOptionsPositions[i] = {
                enemyNameTexts[i]->getTransformada()->posicion.x + xOffset,
                enemyNameTexts[i]->getTransformada()->posicion.y + yOffset
            };
        }
    }

    void Escena_Bestiary::onInit()
    {
        if (!newInstance)
        {
            CE::GestorCamaras::Get().setCamaraActiva(2);
            return;
        }

        CE::GestorCamaras::Get().agregarCamara(std::make_shared<CE::CamaraCuadro>(
            CE::Vector2D{BESTIARY_BG_WIDTH / 2.f, BESTIARY_BG_HEIGHT / 2.f}, CE::Vector2D{BESTIARY_BG_WIDTH, BESTIARY_BG_HEIGHT}));
        CE::GestorCamaras::Get().setCamaraActiva(2);

        if (!font.openFromFile(ASSETS "/fonts/NotJamSlab14.ttf"))
        {
            return;
        }

        if (!backgroundImage.loadTileMap(ASSETS "/mapas/bestiary_background.txt"))
            exit(EXIT_FAILURE);

        icon = std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura("iconoGrande"),160,150,0.5f);

        registrarBotones(sf::Keyboard::Scancode::W, "arriba");
        registrarBotones(sf::Keyboard::Scancode::Up, "arriba");
        registrarBotones(sf::Keyboard::Scancode::S, "abajo");
        registrarBotones(sf::Keyboard::Scancode::Down, "abajo");
        registrarBotones(sf::Keyboard::Scancode::Escape, "esc");

        discoveredColor = sf::Color::White;
        undiscoveredColor = sf::Color(120,120,120);
        enemyNameTexts.clear();

        // desired layout
        float leftColumnX = 140.f;
        float leftColumnStartY = 60.f;
        float leftColumnOffsetY = 55.f;
        float cardX = 660.f;
        float cardNameY = 320.f;
        float cardDescY = 420.f;
        float cardStateY = 520.f;
        float cardSpriteX = 720.f;
        float cardSpriteY = 240.f;

        for (size_t i = 0; i < ENEMY_NAMES.size(); ++i)
        {
            auto txt = std::make_shared<Texto>(font, "???");
            txt->setPosicion(leftColumnX, leftColumnStartY + i * leftColumnOffsetY);
            txt->setTextCharacterSize(50);
            enemyNameTexts.push_back(txt);
            objetos.agregarPool(txt);
        }
        cardNameText = std::make_shared<Texto>(font, "???");
        cardNameText->setPosicion(cardX, cardNameY);
        cardNameText->setTextCharacterSize(80);
        cardDescText = std::make_shared<Texto>(font, "Not discovered yet.");
        cardDescText->setPosicion(cardX, cardDescY);
        cardDescText->setTextCharacterSize(46);
        cardStateText = std::make_shared<Texto>(font, "Not discovered");
        cardStateText->setPosicion(cardX, cardStateY);
        cardStateText->setTextCharacterSize(42);
        cardSprite = std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura(ENEMY_SPRITES[0]), 32, 32, 3.0f);
        cardSprite->m_sprite.setPosition({cardSpriteX, cardSpriteY});

        objetos.agregarPool(cardStateText);
        objetos.agregarPool(cardDescText);
        objetos.agregarPool(cardNameText);

        addMenuOptPositions();

        newInstance = false;
    }

    void Escena_Bestiary::onFinal() {}

    void Escena_Bestiary::onUpdate(float dt)
    {
        icon->m_sprite.setPosition({menuOptionsPositions[currentSelection].x, menuOptionsPositions[currentSelection].y});

        // onUpdate call so the positions are updated
        // (though they should be static in this scene, if it's not called they appear all
        // stacked at 0,0)
        for (auto& obj : objetos.getPool())
        {
            obj->onUpdate(dt);
        }

        for (size_t i = 0; i < enemyNameTexts.size(); ++i)
        {
            bool isDiscovered = (*enemiesKilledStats)[i] > 0;
            std::string displayName;
            sf::Color displayColor;
            if (isDiscovered)
            {
                displayName = ENEMY_NAMES[i];
                displayColor = discoveredColor;
            }
            else
            {
                constexpr float offsetToCenter = 40.f; // small offset for when it has to display "???", so it's centered
                displayName = "???";
                displayColor = undiscoveredColor;
                // todo: fix this, add conditional so it doesn't keep adding the offset every frame
                //enemyNameTexts[i]->setPosicion(enemyNameTexts[i]->getTransformada()->posicion.x + offsetToCenter, enemyNameTexts[i]->getTransformada()->posicion.y);
            }
            if (i == currentSelection)
            {
                displayColor = sf::Color::Yellow; // not neccesary anymore, the icon indicates the selection
            }
            enemyNameTexts[i]->setTextString(displayName);
            enemyNameTexts[i]->setTextFillColor(displayColor);
        }
        const bool isCardDiscovered = (*enemiesKilledStats)[currentSelection] > 0;
        std::string cardName;
        std::string cardDesc;
        std::string cardState;
        sf::Color cardStateColor;
        if (isCardDiscovered)
        {
            cardName = ENEMY_NAMES[currentSelection];
            cardDesc = ENEMY_DESCRIPTIONS[currentSelection];
            cardState = "Discovered";
            cardStateColor = sf::Color::Green;
            cardSprite->m_sprite.setColor(sf::Color::White);
            cardSprite->m_sprite.setTexture(CE::GestorAssets::Get().getTextura(ENEMY_SPRITES[currentSelection]));
        }
        else
        {
            cardName = "???";
            cardDesc = "Not discovered yet.";
            cardState = "Not discovered";
            cardStateColor = sf::Color::Red;
            cardSprite->m_sprite.setColor(sf::Color(0,0,0,150));
            cardSprite->m_sprite.setTexture(CE::GestorAssets::Get().getTextura(ENEMY_SPRITES[currentSelection]));
        }
        cardNameText->setTextString(cardName);
        cardDescText->setTextString(cardDesc);
        cardStateText->setTextString(cardState);
        cardStateText->setTextFillColor(cardStateColor);
    }

    void Escena_Bestiary::onInputs(const CE::Botones& accion)
    {
        if (accion.getTipo() != CE::Botones::TipoAccion::OnPress) return;

        if (accion.getNombre() == "arriba")
        {
            currentSelection = (currentSelection - 1 + 10) % 10;
        }
        else if (accion.getNombre() == "abajo")
        {
            currentSelection = (currentSelection + 1) % 10;
        }
        else if (accion.getNombre() == "esc")
        {
            CE::GestorEscenas::Get().cambiarEscena("Menu");
        }
    }
    void Escena_Bestiary::onInputs(const CE::MouseButton &accion) {}

    void Escena_Bestiary::onRender()
    {
        CE::Render::Get().AddToDraw(backgroundImage);
        for (auto& txt : objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*txt);
        }
        CE::Render::Get().AddToDraw(cardSprite->m_sprite);
        CE::Render::Get().AddToDraw(icon->m_sprite);
    }
}
