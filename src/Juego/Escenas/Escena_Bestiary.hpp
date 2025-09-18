#pragma once
#include "Juego/objetos/Texto.hpp"
#include "Juego/objetos/TileMap.hpp"
#include "Motor/Primitivos/Escena.hpp"

namespace IVJ
{
    enum class ENEMY_TYPE
    {
        ERRANT = 1,
        CRAWLER,
        BERSERKER,
        SHIELDED,
        SUMMONER,
        BURGLAR,
        KAMIKAZE,
        JOCKEY,
        SPITTER,
        CHONGUS
    };

    class Escena_Bestiary : public CE::Escena
    {
    public:
        explicit Escena_Bestiary();
        void onInit() override;
        void onFinal() override;
        void onUpdate(float dt) override;
        void onInputs(const CE::Botones& accion) override;
        void onInputs(const CE::MouseButton &accion) override;
        void onRender() override;

    private:

        void addMenuOptPositions(); // method to set up the menu options positions

        // table with the positions of the menu options
        // for the navigation system
        // positions to be calculated based on text positions on onInit
        std::array<CE::Vector2D, 10> menuOptionsPositions = std::array<CE::Vector2D, 10>{};
        // this is the array of number of entities killed per enemy type
        // to display the bestiary information, if its 0 it means the player hasn't encountered that enemy yet
        // and shall not be displayed (meaning, displayed as a silhouette)
        // it's also used for updating the stats of the enemies in the bestiary
        std::shared_ptr<std::array<int, 10>> enemiesKilledStats;

        bool newInstance = true;
        std::vector<std::shared_ptr<Texto>> enemyNameTexts;
        int currentSelection = 0;
        std::shared_ptr<Texto> cardNameText;
        std::shared_ptr<Texto> cardDescText;
        std::shared_ptr<Texto> cardStateText;
        std::shared_ptr<CE::ISprite> cardSprite;
        sf::Font font;
        sf::Color discoveredColor, undiscoveredColor;
        TileMap backgroundImage; // bestiary background image
        std::shared_ptr<CE::ISprite> icon; // not directly a sf::Sprite so i'm not obligated to initialize it on the constructor
    };
}
