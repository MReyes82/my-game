#pragma once
#include <Motor/Inputs/MouseButton.hpp>

#include "Juego/objetos/TileMap.hpp"
#include "Motor/Primitivos/Escena.hpp"
#include "Overlay/InfoUI.hpp"


namespace IVJ
{
    class OverlayMain;

    class EscenaMain : public CE::Escena
    {
    public:
        explicit EscenaMain()= default;
        ~EscenaMain() override= default;

        void onInit() override;
        void onUpdate(float deltaTime) override;
        void onRender() override;
        void onFinal() override;
        void onInputs(const CE::Botones& accion) override;
        void onInputs(const CE::MouseButton &accion) override;

    private: // scene methods
        void checkRoundEnd();
        void summonEnemies(int maxEnemies);
        void movePlayerPointer();
        static void loadUIAssets();
        void updatePlayerUI();

        void initPlayerPointer();
        void registerButtons();

    private: // scene variables and pointers
        bool newInstance {true}; // flag to initialize only once
        bool gameState {false}; // false = playing, true = paused
        std::shared_ptr<Entidad> player; // player entity
        //std::vector<std::shared_ptr<Entidad>> enemies; // vector of enemy entities

        // weapon an utilities functions
        CE::WEAPON_TYPE newWeaponType {CE::WEAPON_TYPE::NONE};
        CE::UTILITY_TYPE newUtilityType {CE::UTILITY_TYPE::NONE};
        std::vector<std::shared_ptr<Entidad>> lootItems; // vector of loot items in the scene
        std::vector<std::shared_ptr<Entidad>> bulletsShot; // vector of projectiles in the scene (soon to be implemented)

        signed int currentRound {0};
        int MAX_ROUND_ENEMIES = 9;
        int currentEnemiesInScene = 0;
        bool shouldShowNewRoundText = true; // todo: implement a UIInfo class to show text on screen

        // map and camera
        TileMap background[2];
        std::shared_ptr<OverlayMain> sceneOverlay;
        InfoUI UIsceneOverlayElements;
        /*
         * POSITION TABLES FOR ENEMIES AND LOOT SPAWNING
         */
        std::array<CE::Vector2D, 10> spawnPositions = { // fixed definition, could be loaded from a file later
            CE::Vector2D{1000.f, 2100.f}, CE::Vector2D{1820.f, 1090.f}, CE::Vector2D{1040.f, 10.f},
            CE::Vector2D{1620.f, 1090.f}, CE::Vector2D{180.f, 100.f}, CE::Vector2D{100.f, 120.f},
            CE::Vector2D{120.f, 120.f}, CE::Vector2D{540.f, 920.f}, CE::Vector2D{960.f, 120.f},
            CE::Vector2D{380.f, 820.f}
        };

        std::array<CE::Vector2D, 20> lootPositions = {
            CE::Vector2D{2879.f, 2896.f}, CE::Vector2D{951.f, 993.f}, CE::Vector2D{901.f, 1995.f},
            CE::Vector2D{3339.f, 1920.f}, CE::Vector2D{1920.f, 3264.f}, CE::Vector2D{559.f, 1928.f},
            CE::Vector2D{425.f, 985.f}, CE::Vector2D{1869.f, 242.f}, CE::Vector2D{2896.f, 484.f},
            CE::Vector2D{3397.f, 1001.f}, CE::Vector2D{2779.f, 976.f}, CE::Vector2D{1853.f, 726.f},
            CE::Vector2D{1452.f, 976.f}, CE::Vector2D{1544.f, 1861.f}, CE::Vector2D{934.f, 3347.f},
            CE::Vector2D{1460.f, 3589.f}, CE::Vector2D{959.f, 2888.f}, CE::Vector2D{1435.f, 1936.f},
            CE::Vector2D{2445.f, 3238.f}, CE::Vector2D{2000.f, 2000.f}
        };
    };

} // IVJ