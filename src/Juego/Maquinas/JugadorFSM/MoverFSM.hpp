#pragma once
#include "../FSM.hpp"

namespace IVJ
{
    class MoverFSM : public FSM
    {
        public:
            explicit MoverFSM(bool flip_sprite);
            ~MoverFSM()override {};
            FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override;
            void onUpdate(const Entidad& obj,float dt) override;
        private:
            void flipSprite(const Entidad& obj);
        private:
            bool flip{};
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
