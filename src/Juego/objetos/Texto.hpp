#pragma once

#include "../../Motor/Primitivos/Objetos.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
    class Texto : public CE::Objeto
    {
        public:
            Texto(const sf::Font& f, const std::string &t);
            void onUpdate(float dt) override;
            void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
            // setters for text properties
            [[maybe_unused]] void setTextString(const std::string &t) { texto.m_texto.setString(t); };
            [[maybe_unused]] void setTextFont(const sf::Font &f) { texto.m_texto.setFont(f); };
            [[maybe_unused]] void setTextCharacterSize(const unsigned int size) { texto.m_texto.setCharacterSize(size); };
            [[maybe_unused]] void setTextFillColor(const sf::Color& color) { texto.m_texto.setFillColor(color); };
            [[maybe_unused]] void setTextStyle(const sf::Text::Style& style) { texto.m_texto.setStyle(style); };
            [[nodiscard]] sf::FloatRect getGlobalBoundsText() const { return texto.m_texto.getGlobalBounds(); };

        private:
            CE::ITexto texto;
    };
}
