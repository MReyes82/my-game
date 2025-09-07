#pragma once
#include "../GUI/GLayer.hpp"
#include "../Utils/Utils.hpp"
#include "../Primitivos/Objetos.hpp"

namespace CE
{
    class GPaths : public GLayer
    {
        public:
            ~GPaths() override {};
            void OnInit(const MotorConfig& des) override;
            void OnUpdate(float dt) override;
            void OnRender(void) override;
            void setObjetoSelect(Objeto *ref);
        private:
            Objeto* objeto_select{nullptr};
    };
}
