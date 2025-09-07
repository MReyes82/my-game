#include "Figuras.hpp"

namespace IVJ
{
    Figuras::Figuras(int l, const sf::Color& relleno, const sf::Color& contorno)
        :CE::Objeto{},f_lados{l},f_crelleno{relleno},f_ccontorno{contorno}
    {
    }


    Rectangulo::Rectangulo(float ancho, float largo, const sf::Color& relleno, const sf::Color& contorno)
    :Figuras{4,relleno,contorno}, rect_img{sf::RectangleShape({ancho,largo})},w{ancho},h{largo}
    {
        rect_img.setFillColor(f_crelleno);
        rect_img.setOutlineColor(f_ccontorno);
        rect_img.setOutlineThickness(3);
        rect_img.setOrigin({ancho/2.f,largo/2.f});
    }


    void Rectangulo::draw(sf::RenderTarget& target, sf::RenderStates state) const 
    {
        state.transform *= getTransform();
        target.draw(rect_img);
        sf::CircleShape pivote{10.f};
        pivote.setFillColor(f_ccontorno);
        pivote.setOrigin({10.f,10.f});
        pivote.setPosition(rect_img.getPosition());

        target.draw(pivote);

    }
    void Rectangulo::onUpdate(float dt)
    {
        rect_img.setPosition({transform->posicion.x,transform->posicion.y});
    }

    Circulo::Circulo(float r, const sf::Color& relleno, const sf::Color& contorno)

        :Figuras{64,relleno,contorno},radio{r},cir_img{sf::CircleShape{r,64}}
    {

        cir_img.setFillColor(f_crelleno);
        cir_img.setOutlineColor(f_ccontorno);
        cir_img.setOutlineThickness(3);
        cir_img.setOrigin({radio/2.f,radio/2.f});
    }

    void Circulo::draw(sf::RenderTarget& target, sf::RenderStates state) const 
    {
        state.transform *= getTransform();
        target.draw(cir_img);
    }

    void Circulo::onUpdate(float dt)
    {
        cir_img.setPosition({transform->posicion.x,transform->posicion.y});
    }

    Triangulo::Triangulo(float r, const sf::Color& relleno, const sf::Color& contorno)

        :Figuras{3,relleno,contorno},radio{r},tri_img{sf::CircleShape{r,3}}
    {

        tri_img.setFillColor(f_crelleno);
        tri_img.setOutlineColor(f_ccontorno);
        tri_img.setOutlineThickness(3);
        tri_img.setOrigin({radio/2.f,radio/2.f});
    }

    void Triangulo::draw(sf::RenderTarget& target, sf::RenderStates state) const 
    {
        state.transform *= getTransform();
        target.draw(tri_img);
    }

    void Triangulo::onUpdate(float dt)
    {
        tri_img.setPosition({transform->posicion.x,transform->posicion.y});
    }
}
