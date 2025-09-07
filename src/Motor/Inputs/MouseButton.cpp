#include "MouseButton.hpp"

namespace CE
{
    MouseButton::MouseButton(const std::string& nombre, const TipoAccion& tipo, const sf::Mouse::Button& scan)
    :nombre{nombre},accion{tipo},button{scan}{}
    const std::string MouseButton::getNombre() const
    {
        return nombre;
    }
    const std::string MouseButton::getTipoString() const
    {
        switch(accion)
        {
            case TipoAccion::OnPress:
                return "OnPress";
            case TipoAccion::OnRelease:
                return "OnRelease";
            default:
                return "None";
        }
    }
    const sf::Mouse::Button& MouseButton::getButton() const
    {
        return button;
    }
    const MouseButton::TipoAccion& MouseButton::getTipo() const
    {
        return accion;
    }
    const std::string MouseButton::toString() const
    {
        return nombre + "--> "+getTipoString();
    }
}
