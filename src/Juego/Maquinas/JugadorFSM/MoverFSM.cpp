#include "MoverFSM.hpp"
#include "IdleFSM.hpp"

namespace IVJ
{
    
    MoverFSM::MoverFSM(bool flip_sprite)
        :FSM{},flip{flip_sprite}
    {
        nombre="MoverFSM";
        std::cout<<nombre<<"\n";
    }
    FSM* MoverFSM::onInputs(const CE::IControl& control, const Entidad& obj)
    {
        if(!control.arr && !control.abj && !control.der && !control.izq)
        {
            return new IdleFSM();
        }

        if(control.der)
            flip=false;
        else if(control.izq)
            flip=true;

        return nullptr;
    }
    void MoverFSM::onEntrar(const Entidad& obj)
    {
        //podemos acceder al sprite desde obj
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        s_w = c_sprite->width; 
        s_h = c_sprite->height;
        //los valores siguientes son el sprite_sheet del alien_pink el frame que dice
        //walk_1 y 2
        ani_frames[0]= {69.f,193.f};
        ani_frames[1]= {0.f,0.f};
        max_tiempo = 0.4f; //en segundos
        tiempo = max_tiempo; //timer
        id_actual = 0;
        //flip
        flipSprite(obj);
    }
    void MoverFSM::flipSprite(const Entidad& obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();
        if(flip)
            sprite->setScale({-c_sprite->escala,c_sprite->escala});
        else
            sprite->setScale({c_sprite->escala,c_sprite->escala});
    }
    void MoverFSM::onSalir(const Entidad& obj)
    {
    }
    void MoverFSM::onUpdate(const Entidad& obj,float dt)
    {
        //el framerate de la animación
        tiempo = tiempo - 1*dt;
        flipSprite(obj);
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
