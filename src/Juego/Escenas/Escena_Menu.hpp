#pragma once
#include "../../Motor/Primitivos/Escena.hpp"
namespace IVJ
{
    class EscenaMenu :public CE::Escena
    {
        public:
            EscenaMenu():CE::Escena{}{};
            void onInit();
            void onFinal();
            void onUpdate(float dt);
            void onInputs(const CE::Botones& accion);
            void onInputs(const CE::MouseButton &accion);
            void onRender();
        private:
            bool inicializar{true};
    };
}
