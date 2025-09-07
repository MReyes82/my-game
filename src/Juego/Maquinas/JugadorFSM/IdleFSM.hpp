#pragma once
#include "../FSM.hpp"

namespace IVJ
{
    class IdleFSM : public FSM
    {
        public:
            explicit IdleFSM();
            ~IdleFSM()override {};
            FSM* onInputs(const CE::IControl& control, const Entidad& obj)override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            CE::Vector2D ani_frames[2]{};
            float tiempo{};
            int id_actual{};
            float max_tiempo{};
            //sprite ref
            sf::Sprite * sprite{nullptr};
            int s_w{};
            int s_h{};

    };
}
