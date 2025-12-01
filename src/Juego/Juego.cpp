#include "Juego.hpp"
#include<iostream>

#include<SFML/Graphics.hpp>
#include <string>

#include "../Motor/Primitivos/GestorEscenas.hpp"
#include "Escenas/Escena_Menu.hpp"
#include "Escenas/EscenaCuadro.hpp"
#include "Escenas/Escena_zombie.hpp"
#include "Escenas/EscenaShaders.hpp"
#include "Escenas/EscenaMain.hpp"
#include "Escenas/Escena_Bestiary.hpp"
#include "Escenas/Escena_Quest.hpp"

namespace IVJ
{

    Juego::Juego()
        :CE::GameLayer{}
    {
        termino=false;
    }
    void Juego::OnInit(void)
    {
        std::cout<<"Inicializando Juego\n";
        jugador_ref = std::make_shared<Entidad>();
        
        CE::GestorEscenas::Get().registrarEscena("Menu",std::make_shared<EscenaMenu>());
        //CE::GestorEscenas::Get().registrarEscena("Cuadros",std::make_shared<EscenaCuadros>());
        //CE::GestorEscenas::Get().registrarEscena("Shaders",std::make_shared<EscenaShaders>());
        //CE::GestorEscenas::Get().registrarEscena("Zombie",std::make_shared<Escena_zombie>());
        CE::GestorEscenas::Get().registrarEscena("Main", std::make_shared<EscenaMain>());
        CE::GestorEscenas::Get().registrarEscena("Bestiary", std::make_shared<Escena_Bestiary>());
        CE::GestorEscenas::Get().registrarEscena("Quest", std::make_shared<Escena_Quest>(jugador_ref));
        //CE::GestorEscenas::Get().cambiarEscena("Shaders"); //ejecutar onInit()
        CE::GestorEscenas::Get().cambiarEscena("Main"); //ejecutar onInit()
        escena_actual = &CE::GestorEscenas::Get().getEscenaActual();
        
    }
    void Juego::OnInputs(float dt)
    {
        auto br = escena_actual->getBotonesRegistrados();

        CE::Botones::TipoAccion tipo_accion = CE::Botones::TipoAccion::OnRelease;
        std::string strAccion ="None";
        sf::Keyboard::Scancode scan = sf::Keyboard::Scancode::Comma;

        for(auto &sk : br)
        {
            if(sf::Keyboard::isKeyPressed(sk))
            {
                tipo_accion = CE::Botones::TipoAccion::OnPress;
                scan = sk;
                strAccion = escena_actual->getBotones().at(scan);
            }
        }
        escena_actual->onInputs(CE::Botones(strAccion,tipo_accion,scan));
    }
    void Juego::OnInputs(float dt ,std::optional<sf::Event>& eventos)
    {
        if(eventos->getIf<sf::Event::Closed>())
            termino = true;

        if(eventos->getIf<sf::Event::MouseMoved>())
        {
            
        }

    }
    void Juego::OnInputs(float dt, CE::InputBuffer& buffer)
    {
        //procesar la lista circular
        while(!buffer.estaVacio())
        {
            auto evento_ptr = buffer.popData();
            if(!evento_ptr)
                continue;
            auto& evento = *evento_ptr;
            //eventos de la ventana
            if(evento.getIf<sf::Event::Closed>())
            {
                termino=true;
                continue; //o break si no necesitamos procesar más
            }
            //procesar inputs
            sf::Keyboard::Scancode scan = sf::Keyboard::Scancode::Unknown;
            CE::Botones::TipoAccion b_tipo;
            bool procesado=false;

            if(const auto* tecla_presionada = evento.getIf<sf::Event::KeyPressed>())
            {
                scan = tecla_presionada->scancode;
                b_tipo = CE::Botones::TipoAccion::OnPress;
                procesado = true;
            }
            else if(const auto* tecla_liberada = evento.getIf<sf::Event::KeyReleased>())
            {
                scan = tecla_liberada->scancode;
                b_tipo = CE::Botones::TipoAccion::OnRelease;
                procesado = true;
            }

            if(procesado)
            {
                auto iterador = escena_actual->getBotones().find(scan);
                if(iterador!=escena_actual->getBotones().end())
                {
                    //se encontro el input del control
                    std::string strAccion = iterador->second;
                    escena_actual->onInputs(CE::Botones(strAccion,b_tipo,scan));
                }
            }

            // process inputs for mouse buttons
            procesado = false;
            sf::Mouse::Button m_button;
            CE::MouseButton::TipoAccion mouseButtonTipo;

            if (const auto* mouseButtonPressed = evento.getIf<sf::Event::MouseButtonPressed>())
            {
                //CE::printDebug("[JUEGO] Mouse Button Pressed");
                m_button = mouseButtonPressed->button;
                mouseButtonTipo = CE::MouseButton::TipoAccion::OnPress;
                procesado = true;
            }
            else if (const auto* mouseButtonReleased = evento.getIf<sf::Event::MouseButtonReleased>())
            {
                //CE::printDebug("[JUEGO] Mouse Button Released");
                m_button = mouseButtonReleased->button;
                mouseButtonTipo = CE::MouseButton::TipoAccion::OnRelease;
                procesado = true;
            }

            if (procesado)
            {
                auto iterador = escena_actual->getMouseButtons().find(m_button);
                if (iterador != escena_actual->getMouseButtons().end())
                {
                    // se encontro el input del control
                    //CE::printDebug("[JUEGO] Mouse Button Action Found");
                    std::string strAccion = iterador->second;
                    escena_actual->onInputs(CE::MouseButton(strAccion, mouseButtonTipo, m_button));
                }
            }
        }
    }
    void Juego::OnUpdate(float dt)
    {
        escena_actual = &CE::GestorEscenas::Get().getEscenaActual();
        escena_actual->onUpdate(dt);
    }

    void Juego::OnRender(float dt)
    {
        escena_actual->onRender();
    }

}
