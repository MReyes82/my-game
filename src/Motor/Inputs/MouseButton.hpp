#pragma once

#include <string>
#include <SFML/Graphics.hpp>

namespace CE
{
    class MouseButton
    {
    public:
        enum class TipoAccion
        {
            OnPress,
            OnRelease,
            None
        };
    public:
        MouseButton(const std::string& nombre, const TipoAccion& tipo, const sf::Mouse::Button& scan);
        const std::string getNombre() const;
        const std::string getTipoString() const;
        const TipoAccion& getTipo() const;
        const std::string toString() const;
        const sf::Mouse::Button& getButton() const;

    private:
        std::string nombre;
        TipoAccion accion{TipoAccion::None};
        sf::Mouse::Button button;
    };
}



