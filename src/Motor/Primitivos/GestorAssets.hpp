#pragma once
#include <SFML/Graphics.hpp>
#include<map>
#include<memory>
#include <string>
#include "../Utils/Vector2D.hpp"

namespace CE
{
    class GestorAssets
    {
        public:
            static GestorAssets& Get();
            void agregarFont(const std::string& key, const std::string& filepath);
            void agregarTextura(const std::string& key, const std::string& filepath,
                                const CE::Vector2D& pos_init,const CE::Vector2D& dim);
            sf::Font& getFont(const std::string& key)
            {
                return *hashFonts[key];
            }
            sf::Texture& getTextura(const std::string& key)
            {
                return *hashTexturas[key];
            }

        private:
            std::map<std::string, std::shared_ptr<sf::Font>> hashFonts;
            std::map<std::string, std::shared_ptr<sf::Texture>> hashTexturas;
            static inline GestorAssets* instancia=nullptr;
    };
}
