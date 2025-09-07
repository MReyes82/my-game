#pragma once

#include "../FSM.hpp"

#define NUMBER_OF_ANIMATION_FRAMES 4

namespace IVJ
{
       class IdleState : public FSM
       {
       public:
              explicit IdleState(bool flip);
              ~IdleState() override {};
              FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
              void onEntrar(const Entidad& obj) override;
              void onSalir(const Entidad& obj) override;
              void onUpdate(const Entidad& obj, float dt) override;

       private:
              void flipSprite(const Entidad& obj);

              CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {}; // 4 frames for every animation
              bool shouldFlip;
              float time { };
              int current_id { };
              float max_time { };
              // sprite ref
              sf::Sprite* sprite { nullptr };
              int sprite_width { };
              int sprite_height { };
       };

       class MovingState : public FSM
       {
       public:
              explicit MovingState(bool flip);
              ~MovingState() override {};
              FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
              void onEntrar(const Entidad& obj) override;
              void onSalir(const Entidad& obj) override;
              void onUpdate(const Entidad& obj, float dt) override;

       private:
              void flipSprite(const Entidad& obj);

       private:
              bool shouldFlip;
              CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {};
              float time { };
              int current_id { };
              float max_time { };
              // sprite ref
              sf::Sprite* sprite { nullptr };
              int sprite_width { };
              int sprite_height { };
       };

       class AttackingStillState : public FSM
       {
       public:
              explicit AttackingStillState(bool flip);
              ~AttackingStillState() override {};
              FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
              void onEntrar(const Entidad& obj) override;
              void onSalir(const Entidad& obj) override;
              void onUpdate(const Entidad& obj, float dt) override;

       private:
              void flipSprite(const Entidad& obj);
       private:
              bool shouldFlip;
              CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {};
              float time { };
              int current_id { };
              float max_time { };
              // sprite ref
              sf::Sprite* sprite { nullptr };
              int sprite_width { };
              int sprite_height { };
       };

       class AttackingMovingState : public FSM
       {
       public:
              explicit AttackingMovingState(bool flip);
              ~AttackingMovingState() override {};
              FSM* onInputs(const CE::IControl& control, const Entidad& obj) override;
              void onEntrar(const Entidad& obj) override;
              void onSalir(const Entidad& obj) override;
              void onUpdate(const Entidad& obj, float dt) override;
       private:
              void flipSprite(const Entidad& obj);
       private:
              bool shouldFlip;
              CE::Vector2D animation_frames[NUMBER_OF_ANIMATION_FRAMES] {};
              float time { };
              int current_id { };
              float max_time { };
              // sprite ref
              sf::Sprite* sprite { nullptr };
              int sprite_width { };
              int sprite_height { };
       };
}

