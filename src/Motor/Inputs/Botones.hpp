#pragma once

#include <SFML/Graphics.hpp>
#include <string>
namespace CE
{
    class Botones
    {
        public:
            enum class TipoAccion
            {
                OnPress,
                OnRelease,
                None
            };
        public:
            Botones(const std::string& nombre, const TipoAccion& tipo,const sf::Keyboard::Scancode& scan);
            const std::string getNombre() const;
            const std::string getTipoString() const;
            const TipoAccion& getTipo() const;
            const std::string toString() const;
            const sf::Keyboard::Scancode& getScancode() const;
        private:
            std::string nombre;
            TipoAccion accion{TipoAccion::None};
            sf::Keyboard::Scancode tecla;
    };
}
