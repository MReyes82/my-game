#pragma once
#include "../Inputs/Botones.hpp"
#include "../Inputs/MouseButton.hpp"
#include "CEPool.hpp"
#include <SFML/Graphics.hpp>
#include<map>

namespace CE
{
    class Escena
    {
        typedef std::map<sf::Keyboard::Scancode,std::string> HashBotones;
        typedef std::map<sf::Mouse::Button, std::string> HashMouse;

        public:
            virtual ~Escena(){};

            virtual void onInit()=0;
            virtual void onFinal()=0;
            virtual void onUpdate(float dt)=0;
            virtual void onInputs(const CE::Botones& accion) = 0;
            virtual void onInputs(const CE::MouseButton& accion) = 0;
            virtual void onRender()=0;

            void registrarBotones(sf::Keyboard::Scancode boton, const std::string &accion)
            {
                botones[boton] = accion;
            }

            HashBotones &getBotones()
            {
                return botones;
            }

            void registrarBotonesMouse(sf::Mouse::Button boton, const std::string &accion)
            {
                mouseButtons[boton] = accion;
            }

            HashMouse& getMouseButtons()
            {
                return mouseButtons;
            }

            Pool& getPool()
            {
                return objetos;
            }
            //input
            std::vector<sf::Keyboard::Scancode> 
            getBotonesRegistrados()
            {
                std::vector<sf::Keyboard::Scancode> b;
                b.reserve(botones.size());
                for(auto k = botones.begin();k!=botones.end();k++)
                {
                    b.emplace_back(k->first);
                }
                return b;
            }
            std::vector<sf::Mouse::Button> getRegisteredMouseButtons()
            {
                std::vector<sf::Mouse::Button> b;
                b.reserve(mouseButtons.size());
                for(auto k = mouseButtons.begin();k!=mouseButtons.end();k++)
                {
                    b.emplace_back(k->first);
                }
                return b;
            }

        public:
            HashBotones botones;
            HashMouse mouseButtons;
            HashBotones gui;
            std::string nombre;
        protected:
            Pool objetos{100}; // objects pool, replaces the EntityFactory from past engine version
    };
}
