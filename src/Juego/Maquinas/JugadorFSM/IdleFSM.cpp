#include "IdleFSM.hpp"
#include <SFML/Graphics.hpp>
#include<iostream>
#include "MoverFSM.hpp"
namespace IVJ
{
    IdleFSM::IdleFSM()
        :FSM{}
    {
        nombre ="IdleFSM";
        std::cout<<nombre<<"\n";

    }
    FSM* IdleFSM::onInputs(const CE::IControl& control, const Entidad& obj)
    {
        if(control.arr)
            return new MoverFSM(false);
        if(control.abj)
            return new MoverFSM(true);
        if(control.der)
            return new MoverFSM(false);
        if(control.izq)
            return new MoverFSM(true);

        return nullptr;
    }
    void IdleFSM::onEntrar(const Entidad& obj)
    {
        //podemos acceder al sprite desde obj
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        s_w = c_sprite->width; 
        s_h = c_sprite->height;
        //std::cout<<s_w<<","<<s_h<<"\n";
        //dependiendo de como este acomodado el sprite
        //cambian los valores, por lo que se recomienda
        //que la hoja de sprite esten todas iguales x->animacion_frame y^la_animacion
        //los valores siguientes son el sprite_sheet del alien_pink el frame que dice
        //swimn_1 y 2
        ani_frames[0]= {0.f,193.f};
        ani_frames[1]= {0.f,96.f};
        max_tiempo = 0.2f; //en segundos
        tiempo = max_tiempo; //timer
        id_actual = 0;
    }
    void IdleFSM::onSalir(const Entidad& obj){}
    void IdleFSM::onUpdate(const Entidad& obj,float dt)
    {
        //el framerate de la animación
        tiempo = tiempo - 1*dt;
        if(tiempo <=0)
        {
            //std::cout<<ani_frames[id_actual%2]<<"\n";
            //mover el cuadro
            sprite->setTextureRect(
                    sf::IntRect{
                    {//posición
                        (int)ani_frames[id_actual%2].x,
                        (int)ani_frames[id_actual%2].y
                    },
                    {//tamaño
                        s_w,
                        s_h
                    }});
            tiempo = max_tiempo;
            id_actual++;
        }
    }
}

