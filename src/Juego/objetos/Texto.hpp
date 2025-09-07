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

        private:
            CE::ITexto texto;
    };
}
