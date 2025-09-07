#include "Texto.hpp"
namespace IVJ
{
    
    Texto::Texto(const sf::Font& f, const std::string &t)
        :CE::Objeto{},texto{CE::ITexto(f,t)}
    {
        texto.m_texto.setCharacterSize(60);
        texto.m_texto.setFillColor(sf::Color::Red);
        texto.m_texto.setStyle(sf::Text::Style::Bold);
    }
    void Texto::onUpdate(float dt)
    {
        auto pos  = getTransformada()->posicion;
        texto.m_texto.setPosition({pos.x,pos.y});
    }
    void Texto::draw(sf::RenderTarget& target, sf::RenderStates state) const
    {
        state.transform *= getTransform();
        
        target.draw(texto.m_texto);

    }
}
