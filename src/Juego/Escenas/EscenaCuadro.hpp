#pragma once
#include "../../Motor/Primitivos/Escena.hpp"
#include "../objetos/Entidad.hpp"
#include "../objetos/TileMap.hpp"
#include <memory>
namespace IVJ
{
    class EscenaCuadros :public CE::Escena
    {
        public:
            EscenaCuadros():CE::Escena{}{};
            void onInit();
            void onFinal();
            void onUpdate(float dt);
            void onInputs(const CE::Botones& accion);
            void onInputs(const CE::MouseButton &accion);
            void onRender();
        private:
            bool inicializar{true};
            std::shared_ptr<Entidad> jugador;
            TileMap bg[2];
        private:
            //varibles del para el shader
            float wave_phase{0};
            sf::Vector2f wave_ampli{};
    };
}
