#include "GestorAssets.hpp"
#include <iostream>
namespace CE
{
    GestorAssets& GestorAssets::Get()
    {
        if(!instancia)
            instancia = new GestorAssets();
        return *instancia;
    }
    void GestorAssets::agregarFont(const std::string& key, const std::string& filepath)
    {
        auto font = std::make_shared<sf::Font>();
        if(!font->openFromFile(filepath))
        {
            std::cerr<<"No se pudo abrir el archivo "<<filepath<<"\n";
            exit(EXIT_FAILURE);
        }

        hashFonts[key] = font;

    }
    void GestorAssets::agregarTextura(const std::string& key, const std::string& filepath,
            const CE::Vector2D& pos_init,const CE::Vector2D& dim)
    {
        auto textura = std::make_shared<sf::Texture>();
        if(!textura->loadFromFile(filepath,false,
                    sf::IntRect({(int)pos_init.x,(int)pos_init.y},{(int)dim.x,(int)dim.y})
                    ))
        {
            std::cerr<<"No se pudo cargar la textura "<<filepath<<"\n";
            exit(EXIT_FAILURE);
        }

        textura->setSmooth(true);
        hashTexturas[key] = textura;
    }
}
