#pragma once
#include <SFML/Graphics.hpp>
#include <optional>

namespace CE
{
    class InputBuffer
    {
        public:
            explicit InputBuffer(int size);
            void pushData(std::optional<sf::Event>& evento);
            std::optional<sf::Event>& popData();
            void clearData();
        private:
            std::vector<std::optional<sf::Event>> buffer;
            int tope;
            int salir;
            int ele{0};
        public:
            const int size;
            bool estaVacio() const {return ele==0;}
    };
}

