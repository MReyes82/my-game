#pragma once
#include "../../Motor/Primitivos/Escena.hpp"
#include "../objetos/Entidad.hpp"
#include "../objetos/TileMap.hpp"
#include <memory>
namespace IVJ
{
    class EscenaShaders :public CE::Escena
    {
        public:
            EscenaShaders():CE::Escena{}{};
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
        //private:
            //varibles del para el shader
    };
}
