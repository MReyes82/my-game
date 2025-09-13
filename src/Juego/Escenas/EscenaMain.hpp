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
        void loadUIAssets();
        void updatePlayerUI();

        void initPlayerPointer();
        void registerButtons();

    private: // scene variables and pointers
        bool inicializar {true}; // flag to initialize only once
        std::shared_ptr<Entidad> player; // player entity
        //std::vector<std::shared_ptr<Entidad>> enemies; // vector of enemy entities
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

        std::array<CE::Vector2D, 10> lootPositions = {
            CE::Vector2D{300.f, 300.f}, CE::Vector2D{600.f, 600.f}, CE::Vector2D{900.f, 900.f},
            CE::Vector2D{1200.f, 1200.f}, CE::Vector2D{500.f, 500.f}, CE::Vector2D{800.f, 800.f},
            CE::Vector2D{100.f, 100.f}, CE::Vector2D{400.f, 400.f}, CE::Vector2D{700.f, 700.f},
            CE::Vector2D{800.f, 300.f}
        };
    };

} // IVJ