#pragma once
#include "Juego/Maquinas/FSM.hpp"

#define NUMBER_OF_ANIMATION_FRAMES 4

namespace IVJ
{
    class MrgIdleState : public FSM
    {
    public:
        explicit MrgIdleState(bool flip);
        ~MrgIdleState() override {};
        FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
        void onEntrar(const Entidad& obj) override;
        void onSalir(const Entidad& obj) override;
        void onUpdate(const Entidad& obj, float dt) override;

    private:
        void flipSprite(const Entidad& obj);

        CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {};
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

    class MrgAttackState : public FSM
    {
    public:
        explicit MrgAttackState(bool flip);
        ~MrgAttackState() override {};
        FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
        void onEntrar(const Entidad& obj) override;
        void onSalir(const Entidad& obj) override;
        void onUpdate(const Entidad& obj, float dt) override;

    private:
        void flipSprite(const Entidad& obj);

        CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {};
        float attackSpriteRow = 64.f;
        bool shouldFlip;
        float time { };
        int current_id { };
        float max_time { };
        // sprite ref
        sf::Sprite* sprite { nullptr };
        int sprite_width { };
        int sprite_height { };
    };

    class MrgMovingState : public FSM
    {
    public:
        explicit MrgMovingState(bool flip);
        ~MrgMovingState() override {};
        FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
        void onEntrar(const Entidad& obj) override;
        void onSalir(const Entidad& obj) override;
        void onUpdate(const Entidad& obj, float dt) override;

    private:
        void flipSprite(const Entidad& obj);

        CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {};
        float movingSpriteRow = 128.f;
        bool shouldFlip;
        float time { };
        int current_id { };
        float max_time { };
        // sprite ref
        sf::Sprite* sprite { nullptr };
        int sprite_width { };
        int sprite_height { };
    };
}
