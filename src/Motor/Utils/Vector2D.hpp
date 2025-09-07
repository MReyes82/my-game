#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>

namespace CE
{
    class Vector2D
    {   
        public:
            Vector2D();
            Vector2D(const float x, const float y);
            Vector2D(const float v);
            Vector2D(const sf::Vector2f& cpy);

            //métodos
            float longitud() const;
            float magnitud() const;
            float distancia(const Vector2D& o) const;
            Vector2D& normalizacion();
            Vector2D& escala(float s);
            Vector2D& suma(const Vector2D& o);
            Vector2D& resta(const Vector2D& o);

            //sobre carga operadores
            bool operator==(const Vector2D& v)const;
            Vector2D operator+(const Vector2D& v)const;
            Vector2D operator-(const Vector2D& v)const;
            void operator+=(const Vector2D& v);
            friend std::ostream& operator<<(std::ostream& os, const Vector2D& vec);
        public:
            float x;
            float y;
    };
}
