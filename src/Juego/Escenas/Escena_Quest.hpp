#pragma once

#include "Motor/Primitivos/Escena.hpp"
#include "../objetos/TileMap.hpp"
#include <memory>

namespace IVJ
{
    class Escena_Quest : public CE::Escena
    {
    public:

        explicit Escena_Quest(std::shared_ptr<Entidad>& pref);
        virtual ~Escena_Quest(){};
        void onInit() override;
        void onFinal() override;
        void onUpdate(float dt) override;
        void onInputs(const CE::Botones& accion) override;
        void onInputs(const CE::MouseButton& accion) override;
        void onRender() override;
        std::shared_ptr<Entidad> getJugador() {return jugador_ref;}
    private:
        int inicializar{1};
        std::shared_ptr<Entidad>& jugador_ref;
        TileMap bg[2];
    };
}
