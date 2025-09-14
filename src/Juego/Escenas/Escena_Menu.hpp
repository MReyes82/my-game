#pragma once

#include "../../Motor/Primitivos/Escena.hpp"
#include "../objetos/Texto.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"

namespace IVJ
{
    class EscenaMenu :public CE::Escena
    {
        public:
            // table with the positions of the menu options
            // for the navigation system
            // positions to be calculated based on text positions on onInit
            std::array<CE::Vector2D, 3> menuOptionsPositions = std::array<CE::Vector2D, 3>{};

            EscenaMenu()
        :CE::Escena{}
            {

            };
            void onInit();
            void onFinal();
            void onUpdate(float dt);
            void onInputs(const CE::Botones& accion);
            void onInputs(const CE::MouseButton &accion);
            void onRender();

        private:
            void setSceneTexts(); // method to set up the texts in the scene
            void addMenuOptPositions(); // method to set up the menu options positions

            bool inicializar{true};
            int currentSelection = 0;
            int optionsCount = 3; // number of menu options
            // vector of texts to handle layout, drawing still handled by the pool
            std::shared_ptr<Texto> titleText;
            std::shared_ptr<Texto> continueText;
            std::shared_ptr<Texto> exitGameText;
            std::shared_ptr<Texto> goToBestiaryText;
            sf::Color bgColor;
            std::shared_ptr<CE::ISprite> icon; // not directly a sf::Sprite so i'm not obligated to initialize it on the constructor
    };
}
