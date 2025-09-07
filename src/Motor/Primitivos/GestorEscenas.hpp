#pragma once
#include "Escena.hpp"

namespace CE
{
    class GestorEscenas
    {
        typedef std::map<std::string,std::shared_ptr<Escena>> HEscenas;
        public:
            static GestorEscenas& Get();
            void registrarEscena(const std::string& key, const std::shared_ptr<Escena>& escena);
            void cambiarEscena(const std::string& key);
            Escena& getEscenaActual();
            HEscenas& getTodasEscenas()
            {
                return escenas;
            }
            std::vector<std::string> getKeys()const
            {
                std::vector<std::string> keys;
                for(auto it= escenas.begin(); it!=escenas.end();++it)
                    keys.push_back(it->first);
                return keys;
            }
        private:
            std::string escena_actual;
            HEscenas escenas;
            static inline GestorEscenas* instancia=0;
    };
}
