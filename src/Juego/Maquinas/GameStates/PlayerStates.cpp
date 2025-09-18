#include "PlayerStates.hpp"
#include "EnemyStates.hpp"
#include <Motor/Utils/Utils.hpp>
#include <SFML/Graphics.hpp>

namespace IVJ
{
    IdleState::IdleState(bool flip)
        : FSM{}, shouldFlip{flip}
    {
        nombre = "IdleState";
        CE::printDebug("" + nombre + "\n");
    }

    FSM *IdleState::onInputs(const CE::IControl &control, const Entidad& obj)
    {
        if (control.arr)
            return new MovingState(false);
        if (control.abj)
            return new MovingState(true);
        if (control.der)
            return new MovingState(false);
        if (control.izq)
            return new MovingState(true);
        if (control.atacar)
            return new AttackingStillState(!obj.getIsEntityFacingRight());

        return nullptr;
    }

    void IdleState::flipSprite(const Entidad &obj)
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

    void IdleState::onEntrar(const Entidad &obj)
    {
        // access the sprite from the object
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        // TODO: implement weapon component, for now, we will add the offset variable
        // if (obj.tieneComponente<IVJ::IArma>())
        float currentWeapon = 0.f;

        // extract frames from sprite sheet
        animation_frames[0] = {frame1pos, currentWeapon};
        animation_frames[1] = {frame2pos, currentWeapon};
        animation_frames[2] = {frame3pos, currentWeapon};
        animation_frames[3] = {frame4pos, currentWeapon};

        max_time = 0.2f; // in seconds
        time = max_time; // timer
        current_id = 0; // start from the first frame
        flipSprite(obj);
    }

    void IdleState::onSalir(const Entidad &obj)
    {
        // No specific action on exit, just debug message
        CE::printDebug("Exiting " + nombre + "\n");
    }

    void IdleState::onUpdate(const Entidad &obj, float dt)
    {
        // animation framerate
        time = time - 1 * dt;

        if (time <= 0)
        {
            // update the sprite frame
            sprite->setTextureRect(
                sf::IntRect{
                    {// position
                        (int)animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x,
                        (int)animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y
                    },
                    {// sprite size
                        sprite_width,
                        sprite_height
                    }
                });

            time = max_time; // reset timer
            current_id++;

            //flipSprite(obj);
        }
    }

    MovingState::MovingState(bool flip)
        : FSM{}, shouldFlip{flip}
    {
        nombre = "MovingState";
        CE::printDebug("" + nombre + "\n");
    }

    FSM* MovingState::onInputs(const CE::IControl &control, const Entidad& obj)
    {
        if (!control.arr && !control.abj && !control.der && !control.izq && !control.atacar)
        {
            return new IdleState(shouldFlip);
        }

        if (control.der)
            shouldFlip = false;
        else if (control.izq)
            shouldFlip = true;

        if (control.atacar)
            return new AttackingMovingState(!obj.getIsEntityFacingRight());

        return nullptr;
    }

    void MovingState::flipSprite(const Entidad &obj)
    {
        auto c_sprite = obj.getComponente<CE::ISprite>();

        if (shouldFlip)
            sprite->setScale({-c_sprite->escala, c_sprite->escala});
        else
            //sprite->setScale({c_sprite->escala, c_sprite->escala});
                sprite->setScale({c_sprite->escala, c_sprite->escala});

        const auto w = c_sprite->width;
        const auto h = c_sprite->height;
        const sf::Vector2f origin = {static_cast<float>(w) / 2.f, static_cast<float>(h) / 2.f};
        sprite->setOrigin(origin);
    }

    void MovingState::onEntrar(const Entidad &obj)
    {
        CE::printDebug("Entering " + nombre + "\n");
        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        constexpr float currentRow = 64.f; // row for walking animation
        float currentWeapon = 0.f;
        // if (obj.tieneComponente<IVJ::IArma>())

        animation_frames[0] = {frame1pos, currentRow + currentWeapon};
        animation_frames[1] = {frame2pos, currentRow + currentWeapon};
        animation_frames[2] = {frame3pos, currentRow + currentWeapon};
        animation_frames[3] = {frame4pos, currentRow + currentWeapon};

        max_time = 0.2f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void MovingState::onSalir(const Entidad &obj)
    {
        CE::printDebug("Exiting " + nombre + "\n");
    }

    void MovingState::onUpdate(const Entidad &obj, float dt)
    {
        // animation framerate
        time = time - 1 * dt;
        //flipSprite(obj);

        if (time <= 0)
        {
            // update the sprite frame
            sprite->setTextureRect(
                sf::IntRect{
                    {// position
                        (int)animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x,
                        (int)animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y
                    },
                    {// sprite size
                        sprite_width,
                        sprite_height
                    }
                });

            time = max_time; // reset timer
            current_id++;
        }
    }

    AttackingStillState::AttackingStillState(bool flip)
        : FSM{}, shouldFlip{flip}
    {
        nombre = "AttackingStillState";
        CE::printDebug("" + nombre + "\n");
    }

    FSM* AttackingStillState::onInputs(const CE::IControl &control, const Entidad& obj)
    {
        // if the attack button is released, return to idle or moving state
        if (!control.atacar)
            return new IdleState(shouldFlip);

        if (control.der)
            shouldFlip = false;
        if (control.izq)
            shouldFlip = true;

        bool playerBeganMoving = control.arr || control.abj || control.der || control.izq;
        if (playerBeganMoving)
            return new AttackingMovingState(!obj.getIsEntityFacingRight());

        return nullptr;
    }

    void AttackingStillState::flipSprite(const Entidad &obj)
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

    void AttackingStillState::onEntrar(const Entidad &obj)
    {
        CE::printDebug("Entering " + nombre + "\n");

        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        constexpr float currentRow = 32.f; // row for attacking animation
        float currentWeapon = 0.f;
        // if (obj.tieneComponente<IVJ::IArma>())

        animation_frames[0] = {frame1pos, currentRow + currentWeapon};
        animation_frames[1] = {frame2pos, currentRow + currentWeapon};
        animation_frames[2] = {frame3pos, currentRow + currentWeapon};
        animation_frames[3] = {frame4pos, currentRow + currentWeapon};

        max_time = 0.2f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void AttackingStillState::onSalir(const Entidad &obj)
    {
        CE::printDebug("Exiting " + nombre + "\n");
    }

    void AttackingStillState::onUpdate(const Entidad &obj, float dt)
    {
        // animation framerate
        time = time - 1 * dt;
        //flipSprite(obj);

        if (time <= 0)
        {
            // update the sprite frame
            sprite->setTextureRect(
                sf::IntRect{
                    {// position
                        (int)animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x,
                        (int)animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y
                    },
                    {// sprite size
                        sprite_width,
                        sprite_height
                    }
                });

            time = max_time; // reset timer
            current_id++;
        }
    }

    AttackingMovingState::AttackingMovingState(bool flip)
        : FSM{}, shouldFlip{flip}
    {
        nombre = "AttackingMovingState";
        CE::printDebug("" + nombre + "\n");
    }

    void AttackingMovingState::flipSprite(const Entidad &obj)
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

    FSM* AttackingMovingState::onInputs(const CE::IControl &control, const Entidad& obj)
    {
        bool playerStoppedMoving = !control.arr && !control.abj && !control.der && !control.izq;
        if (playerStoppedMoving)
            return new AttackingStillState(shouldFlip);

        if (control.der)
            shouldFlip = false;
        if (control.izq)
            shouldFlip = true;
        // if the attack button is released, return to idle or moving state
        if (!control.atacar)
            return new MovingState(shouldFlip); // !obj->getIsEntityFacingRight()

        return nullptr;
    }

    void AttackingMovingState::onEntrar(const Entidad &obj)
    {
        CE::printDebug("Entering " + nombre + "\n");

        auto c_sprite = obj.getComponente<CE::ISprite>();
        sprite = &c_sprite->m_sprite;
        sprite_width = c_sprite->width;
        sprite_height = c_sprite->height;

        constexpr float currentRow = 96.f; // row for attacking while moving animation
        float currentWeapon = 0.f;
        // if (obj.tieneComponente<IVJ::IArma>())
        animation_frames[0] = {frame1pos, currentRow + currentWeapon};
        animation_frames[1] = {frame2pos, currentRow + currentWeapon};
        animation_frames[2] = {frame3pos, currentRow + currentWeapon};
        animation_frames[3] = {frame4pos, currentRow + currentWeapon};

        max_time = 0.2f;
        time = max_time;
        current_id = 0;

        flipSprite(obj);
    }

    void AttackingMovingState::onSalir(const Entidad &obj)
    {
        CE::printDebug("Exiting " + nombre + "\n");
    }

    void AttackingMovingState::onUpdate(const Entidad &obj, float dt)
    {
        time = time - 1 * dt;
        //flipSprite(obj);

        if (time <= 0)
        {
            // update the sprite frame
            sprite->setTextureRect(
                sf::IntRect{
                    {// position
                        (int)animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].x,
                        (int)animation_frames[current_id % NUMBER_OF_ANIMATION_FRAMES].y
                    },
                    {// sprite size
                        sprite_width,
                        sprite_height
                    }
                });

            time = max_time; // reset timer
            current_id++;
        }
    }
}
