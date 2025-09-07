#pragma once
#include "../FSM.hpp"

#define NUMBER_OF_ANIMATION_FRAMES 4

namespace IVJ
{
    class IdleEnemyState : public FSM
    {
        public:
            explicit IdleEnemyState(bool flip);
            ~IdleEnemyState() override {};
            [[maybe_unused]] FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
            void onEntrar(const Entidad& obj) override;
            void onSalir(const Entidad& obj) override {};
            void onUpdate(const Entidad& obj, float dt) override;

        private:
            void flipSprite(const Entidad& obj);
        private:
            CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {}; // 4 frames for every animation
            float idleSpriteRow = 0.f;
            bool shouldFlip;
            float time { };
            int current_id { };
            float max_time { };
            // sprite ref
            sf::Sprite* sprite { nullptr };
            int sprite_width { };
            int sprite_height { };
    };

    class AttackingEnemyState : public FSM
    {
        public:
                explicit AttackingEnemyState(bool flip);
                ~AttackingEnemyState() override {};
                FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
                void onEntrar(const Entidad& obj) override;
                void onSalir(const Entidad& obj) override;
                void onUpdate(const Entidad& obj, float dt) override;

        private:
                void flipSprite(const Entidad& obj);
        private:
                bool shouldFlip;
                CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {};
                float attackSpriteRow = 32.f;
                float time { };
                int current_id { };
                float max_time { };
                // sprite ref
                sf::Sprite* sprite { nullptr };
                int sprite_width { };
                int sprite_height { };
    };

    class MovingEnemyState : public FSM
    {
        public:
                explicit MovingEnemyState(bool flip);
                ~MovingEnemyState() override {};
                FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
                void onEntrar(const Entidad& obj) override;
                void onSalir(const Entidad& obj) override;
                void onUpdate(const Entidad& obj, float dt) override;

        private:
                void flipSprite(const Entidad& obj);
        private:
                bool shouldFlip;
                CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {};
                float movingSpriteRow = 64.f;
                float time { };
                int current_id { };
                float max_time { };
                // sprite ref
                sf::Sprite* sprite { nullptr };
                int sprite_width { };
                int sprite_height { };
    };
}


