#include "Escena_Bestiary.hpp"
#include "../objetos/Texto.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../../Motor/Render/Render.hpp"
#include <array>
#include <vector>
#include <string>

#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorEscenas.hpp"

namespace IVJ
{
    // Helper: enemy names and descriptions
    std::array<std::string, 10> ENEMY_NAMES = {
        "Errant", "Crawler", "Berserker", "Shielded", "Summoner",
        "Burglar", "Blaster", "Jockey", "Burster", "Chongus"
    };
    std::array<std::string, 10> ENEMY_DESCRIPTIONS = {
        "A wandering zombie.", "Crawls quickly.", "Strong and aggressive.", "Has a shield.",
        "Summons more zombies.", "Steals items.", "Explodes on death.", "Jumps on player.",
        "Spits acid.", "Huge and tough."
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

            /*if (DEBUG)
            {
                // For testing purposes, mark some enemies as discovered
                (*enemiesKilledStats)[0] = 1; // Errant
            }*/
    }

    void Escena_Bestiary::onInit()
    {
        // TOOD: FIND WHY THE POSITIONS ARE NOT BEING CHANGED
        CE::GestorCamaras::Get().setCamaraActiva(0);
        if (!newInstance)
            return;

        if (!font.openFromFile(ASSETS "/fonts/NotJamSlab14.ttf"))
        {
            return;
        }

        registrarBotones(sf::Keyboard::Scancode::W, "arriba");
        registrarBotones(sf::Keyboard::Scancode::Up, "arriba");
        registrarBotones(sf::Keyboard::Scancode::S, "abajo");
        registrarBotones(sf::Keyboard::Scancode::Down, "abajo");
        registrarBotones(sf::Keyboard::Scancode::Escape, "esc");

        discoveredColor = sf::Color::White;
        undiscoveredColor = sf::Color(120,120,120);
        enemyNameTexts.clear();

        // desired layout
        float leftColumnX = 120.f;
        float leftColumnStartY = 120.f;
        float leftColumnOffsetY = 55.f;
        float cardX = 500.f;
        float cardNameY = 320.f;
        float cardDescY = 420.f;
        float cardStateY = 520.f;
        float cardSpriteX = 550.f;
        float cardSpriteY = 260.f;

        for (size_t i = 0; i < ENEMY_NAMES.size(); ++i)
        {
            auto txt = std::make_shared<Texto>(font, "???");
            txt->setPosicion(leftColumnX, leftColumnStartY + i * leftColumnOffsetY);
            txt->setTextCharacterSize(40);
            enemyNameTexts.push_back(txt);
            objetos.agregarPool(txt);
        }
        cardNameText = std::make_shared<Texto>(font, "???");
        cardNameText->setPosicion(cardX, cardNameY);
        cardNameText->setTextCharacterSize(64);
        cardDescText = std::make_shared<Texto>(font, "Not discovered yet.");
        cardDescText->setPosicion(cardX, cardDescY);
        cardDescText->setTextCharacterSize(36);
        cardStateText = std::make_shared<Texto>(font, "Not discovered");
        cardStateText->setPosicion(cardX, cardStateY);
        cardStateText->setTextCharacterSize(32);
        cardSprite = std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura(ENEMY_SPRITES[0]), 32, 32, 3.0f);
        cardSprite->m_sprite.setPosition({cardSpriteX, cardSpriteY});

        objetos.agregarPool(cardStateText);
        objetos.agregarPool(cardDescText);
        objetos.agregarPool(cardNameText);

        newInstance = false;
    }

    void Escena_Bestiary::onFinal() {}

    void Escena_Bestiary::onUpdate(float dt)
    {
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
                displayName = "???";
                displayColor = undiscoveredColor;
            }
            if (i == currentSelection)
            {
                displayColor = sf::Color::Yellow;
            }
            enemyNameTexts[i]->setTextString(displayName);
            enemyNameTexts[i]->setTextFillColor(displayColor);
        }
        bool isCardDiscovered = (*enemiesKilledStats)[currentSelection] > 0;
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
        CE::Render::Get().OnClearColor(sf::Color::Black);
        for (auto& txt : objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*txt);
        }
        if (cardSprite) CE::Render::Get().AddToDraw(cardSprite->m_sprite);
    }
}
