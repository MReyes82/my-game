#include "MirageStates.h"

// constant declared here because I'm too lazy to modify the logic of the FSM base class
#define FRAME_OFFSET 32.f

namespace IVJ
{
    MrgIdleState::MrgIdleState(bool flip)
        : FSM {}, shouldFlip{flip}
    {
        nombre = "MrgIdleState";
        CE::printDebug("" + nombre + "\n");
    }

    FSM* MrgIdleState::onInputs(const CE::IControl &control, const Entidad &obj)
    {
        const auto& transform =  *obj.getTransformadaC();
        if (obj.getCollidedWithAnotherEntity())
        {
            return new MrgAttackState(!obj.getIsEntityFacingRight());
        }
        if (transform.velocidad.x != 0.f || transform.velocidad.y != 0.f)
        {
            return new MrgMovingState(!obj.getIsEntityFacingRight());
        }

        return nullptr;
    }
    void MrgIdleState::flipSprite(const Entidad &obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();

        if (shouldFlip)
            sprite->setScale({-c_sprite->escala, c_sprite->escala});
        else
            sprite->setScale({c_sprite->escala, c_sprite->escala});

        const auto w = c_sprite->width;
        const auto h = c_sprite->height;
        const sf::Vector2f origin = {static_cast<float>(w) / 2.f, static_cast<float>(h) / 2.f};
        sprite->setOrigin(origin);
    }

    void MrgIdleState::onEntrar(const Entidad &obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        animation_frames[0] = {frame1pos, idleSpriteRow};
        animation_frames[1] = {frame2pos + FRAME_OFFSET, idleSpriteRow};
        animation_frames[2] = {frame3pos + FRAME_OFFSET * 2, idleSpriteRow};
        animation_frames[3] = {frame4pos + FRAME_OFFSET * 3, idleSpriteRow};

        max_time = 0.8f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void MrgIdleState::onSalir(const Entidad& obj)
    {
        //CE::printDebug("Exiting " + nombre + "\n");
    }

    void MrgIdleState::onUpdate(const Entidad &obj, float dt)
    {
        time = time - 1 * dt;

        if (time <= 0.f)
        {
            sprite->setTextureRect(
                sf::IntRect{
                    {
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x),
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y)
                    },
                    {
                        sprite_width,
                        sprite_height
                    }
                });
            time = max_time;
            current_id++;
        }
    }

    MrgAttackState::MrgAttackState(bool flip)
        : FSM {}, shouldFlip{flip}
    {
        nombre =  "MrgAttackState";
        CE::printDebug("" + nombre + "\n");
    }

    FSM* MrgAttackState::onInputs(const CE::IControl& control, const Entidad& obj)
    {
        const auto& transform =  *obj.getTransformadaC();

        if (!obj.getCollidedWithAnotherEntity())
        {
            return nullptr;
        }
        if (transform.velocidad.x != 0.f || transform.velocidad.y != 0.f)
        {
            return new MrgMovingState(!obj.getIsEntityFacingRight());
        }
        if (transform.velocidad.x == 0.f && transform.velocidad.y == 0.f)
        {
            return new MrgIdleState(!obj.getIsEntityFacingRight());
        }

        return nullptr;
    }

    void MrgAttackState::flipSprite(const Entidad &obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();

        if (shouldFlip)
            sprite->setScale({-c_sprite->escala, c_sprite->escala});
        else
            sprite->setScale({c_sprite->escala, c_sprite->escala});

        const auto w = c_sprite->width;
        const auto h = c_sprite->height;
        const sf::Vector2f origin = {static_cast<float>(w) / 2.f, static_cast<float>(h) / 2.f};
        sprite->setOrigin(origin);
    }

    void MrgAttackState::onEntrar(const Entidad &obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        animation_frames[0] = {frame1pos + FRAME_OFFSET, attackSpriteRow};
        animation_frames[1] = {frame2pos + FRAME_OFFSET, attackSpriteRow};
        animation_frames[2] = {frame3pos + FRAME_OFFSET, attackSpriteRow};
        animation_frames[3] = {frame4pos + FRAME_OFFSET, attackSpriteRow};

        max_time = 0.2f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void MrgAttackState::onSalir(const Entidad& obj)
    {
        //CE::printDebug("Exiting " + nombre + "\n");
    }

    void MrgAttackState::onUpdate(const Entidad &obj, float dt)
    {
        time = time - 1 * dt;

        if (time <= 0.f)
        {
            sprite->setTextureRect(
                sf::IntRect{
                    {
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x),
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y)
                    },
                    {
                        sprite_width,
                        sprite_height
                    }
                });
            time = max_time;
            current_id++;
        }
    }

    MrgMovingState::MrgMovingState(bool flip)
        : FSM {}, shouldFlip{flip}
    {
        nombre = "MrgMovingState";
        CE::printDebug("" + nombre + "\n");
    }


    FSM* MrgMovingState::onInputs(const CE::IControl& control, const Entidad& obj)
    {
        const auto& transform =  *obj.getTransformadaC();

        if (obj.getCollidedWithAnotherEntity())
        {
            return new MrgAttackState(!obj.getIsEntityFacingRight());
        }
        if (transform.velocidad.x == 0.f && transform.velocidad.y == 0.f)
        {
            return new MrgIdleState(!obj.getIsEntityFacingRight());
        }

        return nullptr;
    }

    void MrgMovingState::flipSprite(const Entidad &obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();

        if (shouldFlip)
            sprite->setScale({-c_sprite->escala, c_sprite->escala});
        else
            sprite->setScale({c_sprite->escala, c_sprite->escala});

        const auto w = c_sprite->width;
        const auto h = c_sprite->height;
        const sf::Vector2f origin = {static_cast<float>(w) / 2.f, static_cast<float>(h) / 2.f};
        sprite->setOrigin(origin);
    }

    void MrgMovingState::onEntrar(const Entidad &obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        animation_frames[0] = {frame1pos + FRAME_OFFSET, movingSpriteRow};
        animation_frames[1] = {frame2pos + FRAME_OFFSET, movingSpriteRow};
        animation_frames[2] = {frame3pos + FRAME_OFFSET, movingSpriteRow};
        animation_frames[3] = {frame4pos + FRAME_OFFSET, movingSpriteRow};

        max_time = 0.2f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void MrgMovingState::onSalir(const Entidad& obj)
    {
        //CE::printDebug("Exiting " + nombre + "\n");
    }

    void MrgMovingState::onUpdate(const Entidad &obj, float dt)
    {
        time = time - 1 * dt;

        if (time <= 0.f)
        {
            sprite->setTextureRect(
                sf::IntRect{
                    {
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x),
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y)
                    },
                    {
                        sprite_width,
                        sprite_height
                    }
                });
            time = max_time;
            current_id++;
        }
    }
}