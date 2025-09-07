#pragma once
#include "Camaras.hpp"
#include <memory>

namespace CE
{
    class GestorCamaras
    {
        public:
            static GestorCamaras& Get();
            void agregarCamara(const std::shared_ptr<Camara>& nueva);
            void setCamaraActiva(int id);
            void onUpdateCamaras(float dt);
            void onRenderCamara(sf::RenderTexture &tex);
            
            [[nodiscard]]Camara& getCamaraActiva()
            {
                return *m_camaraActiva.lock();
            }
            [[nodiscard]]Camara& getCamara(int id)
            {
                return *m_listaCamaras[id];
            }
            [[nodiscard]]std::vector<std::shared_ptr<Camara>>& getListaCamaras()
            {
                return m_listaCamaras;
            }
        private:
            static inline GestorCamaras* instancia = nullptr;
            std::weak_ptr<Camara> m_camaraActiva;
            std::vector<std::shared_ptr<Camara>> m_listaCamaras;

    };
}
