#pragma once

#include <Motor/Inputs/MouseButton.hpp>

#include "../../Motor/Primitivos/Escena.hpp"


namespace IVJ
{
    class Escena_zombie : public CE::Escena
    {
    public:
        virtual ~Escena_zombie(){}

        void onInit() override;
        void onUpdate(float deltaTime) override;
        void onRender() override;
        void onFinal() override;
        void onInputs(const CE::Botones& accion) override;
        void onInputs(const CE::MouseButton &accion) override;

    private:
        int inicializar {1};
        const unsigned int SPAWNS_MAX_ENTITIES = 5;
        std::vector<CE::IPaths> prefixed_paths; // Vector for the prefixed paths, from which enemies will choose randomly
    };
}