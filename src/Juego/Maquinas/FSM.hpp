#pragma once

#include "../Componentes/IJComponentes.hpp"
#include "../objetos/Entidad.hpp"
#include <SFML/Graphics.hpp>

namespace IVJ
{
    class Entidad; //referencia circular
    class FSM
    {
        public:
            virtual ~FSM()=default;
            virtual FSM* onInputs(const CE::IControl& control, const Entidad& obj) = 0;
            virtual void onEntrar(const Entidad& obj)=0;
            virtual void onSalir(const Entidad& obj)=0;
            virtual void onUpdate(const Entidad& obj,float dt)=0;
        protected:
            std::string nombre;
            // frame position constants (player and enemies)
            const float frame1pos = 0.f;
            const float frame2pos = 32.f;
            const float frame3pos = 64.f;
            const float frame4pos = 96.f;
    };

}
