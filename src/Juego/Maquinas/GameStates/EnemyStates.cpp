#include "EnemyStates.hpp"
#include <SFML/Graphics.hpp>
#include <Motor/Utils/Utils.hpp>
#include <SFML/Graphics.hpp>

#include "PlayerStates.hpp"

namespace IVJ
{
    IdleEnemyState::IdleEnemyState(bool flip)
        : FSM{}, shouldFlip{flip}
    {
        nombre = "IdleEnemyState";
        CE::printDebug("" + nombre + "\n");
    }

    FSM *IdleEnemyState::onInputs(const CE::IControl &control, const Entidad &obj)
    {
        // check here if the enemy should change state, since it cannot enter the onInputs method
        const auto& transform =  *obj.getComponente<CE::ITransform>();
        if (obj.getCollidedWithAnotherEntity())
        {
            return new AttackingEnemyState( !obj.getIsEntityFacingRight() );
        }
        if (transform.velocidad.x != 0.f || transform.velocidad.y != 0.f)
        {
            return new MovingEnemyState( !obj.getIsEntityFacingRight() );
        }

        return nullptr;
    }

    void IdleEnemyState::flipSprite(const Entidad &obj)
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

    void IdleEnemyState::onEntrar(const Entidad &obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        animation_frames[0] = {frame1pos, idleSpriteRow};
        animation_frames[1] = {frame2pos, idleSpriteRow};
        animation_frames[2] = {frame3pos, idleSpriteRow};
        animation_frames[3] = {frame4pos, idleSpriteRow};

        max_time = 0.2f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void IdleEnemyState::onUpdate(const Entidad &obj, float dt)
    {
        time = time - 1 * dt;

        if (time <= 0)
        {
            sprite->setTextureRect(
                sf::IntRect{
                    {// position
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x),
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y)
                    },
                    {// size
                        sprite_width,
                        sprite_height
                    }
                });

            current_id++;
            time = max_time;
        }
        flipSprite(obj);
    }

    AttackingEnemyState::AttackingEnemyState(bool flip)
        : FSM{}, shouldFlip{flip}
    {
        nombre = "AttackingEnemyState";
        CE::printDebug("" + nombre + "\n");
    }

    FSM* AttackingEnemyState::onInputs(const CE::IControl &control, const Entidad &obj)
    {
        const auto& transform =  *obj.getComponente<CE::ITransform>();

        if (obj.getCollidedWithAnotherEntity())
            return nullptr;
        // check if the entity is moving
        if (transform.velocidad.x != 0.f || transform.velocidad.y != 0.f)
            return new MovingEnemyState(!obj.getIsEntityFacingRight());
        // check if it is not moving
        if (transform.velocidad.x == 0.f && transform.velocidad.y == 0.f)
            return new IdleEnemyState(!obj.getIsEntityFacingRight());

        return nullptr;
    }

    void AttackingEnemyState::flipSprite(const Entidad &obj)
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

    void AttackingEnemyState::onEntrar(const Entidad &obj)
    {
        CE::printDebug("Entering " + nombre + "\n");
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        animation_frames[0] = {frame1pos, attackSpriteRow};
        animation_frames[1] = {frame2pos, attackSpriteRow};
        animation_frames[2] = {frame3pos, attackSpriteRow};
        animation_frames[3] = {frame4pos, attackSpriteRow};

        max_time = 0.2f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void AttackingEnemyState::onSalir(const Entidad &obj)
    {
        CE::printDebug("Exiting " + nombre + "\n");
    }

    void AttackingEnemyState::onUpdate(const Entidad &obj, float dt)
    {
        time = time - 1 * dt;
        flipSprite(obj);

        if (time <= 0)
        {
            sprite->setTextureRect(
                sf::IntRect{
                    {// position
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x),
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y)
                    },
                    {// size
                        sprite_width,
                        sprite_height
                    }
                });

            current_id++;
            time = max_time;
        }
    }

    MovingEnemyState::MovingEnemyState(bool flip)
        : FSM{}, shouldFlip{flip}
    {
        nombre =  "MovingEnemyState";
        CE::printDebug("" + nombre + "\n");
    }

    FSM* MovingEnemyState::onInputs(const CE::IControl &control, const Entidad &obj)
    {
        const auto& transform =  *obj.getComponente<CE::ITransform>();

        if (obj.getCollidedWithAnotherEntity())
        {
            return new AttackingEnemyState(!obj.getIsEntityFacingRight());
        }

        if (transform.velocidad.x == 0.f && transform.velocidad.y == 0.f)
        {
            return new IdleEnemyState(!obj.getIsEntityFacingRight());
        }

        return nullptr;
    }

    void MovingEnemyState::flipSprite(const Entidad &obj)
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

    void MovingEnemyState::onEntrar(const Entidad &obj)
    {
        CE::printDebug("Entering " + nombre + "\n");
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        animation_frames[0] = {frame1pos, movingSpriteRow};
        animation_frames[1] = {frame2pos, movingSpriteRow};
        animation_frames[2] = {frame3pos, movingSpriteRow};
        animation_frames[3] = {frame4pos, movingSpriteRow};

        max_time = 0.2f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void MovingEnemyState::onSalir(const Entidad &obj)
    {
        CE::printDebug("Exiting " + nombre + "\n");
    }

    void MovingEnemyState::onUpdate(const Entidad &obj, float dt)
    {
        time = time - 1 * dt;
        flipSprite(obj);

        if (time <= 0)
        {
            sprite->setTextureRect(
                sf::IntRect{
                    {// position
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x),
                        static_cast<int>(animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y)
                    },
                    {// size
                        sprite_width,
                        sprite_height
                    }
                });

            current_id++;
            time = max_time;
        }
    }
}

