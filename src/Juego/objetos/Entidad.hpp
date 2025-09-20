#pragma once
#include "../../Motor/Primitivos/Objetos.hpp" 
#include "../Maquinas/FSM.hpp"
#include <memory>
#include <Motor/Utils/Utils.hpp>

#include "Motor/Primitivos/Objetos.hpp"

namespace IVJ
{
    /*enum class EntityType
    {
        PLAYER = 0xAAA,
        ENEMY,
        STATIC,
        DYNAMIC,
        PROJECTILE,
        LOOT,
    };*/

    class FSM; //referencia circular
    class Entidad: public CE::Objeto
    {
        public:
            void onUpdate(float dt) override;
            void draw(sf::RenderTarget &target, sf::RenderStates state) const override;

            //fsm
            void setFSM(const std::shared_ptr<FSM>& mq);
            void inputFSM();

            //methods for game mechanics
            bool hasTimerReachedMax(std::shared_ptr<CE::ITimer> timer) const;
            bool checkPlayerFacingRight(const sf::RenderWindow& window);
            void setIsEntityFacingRight(const bool facingRight) { isEntityFacingRight = facingRight; }
            bool getIsEntityFacingRight() const { return isEntityFacingRight; }
            void setCollidedWithAnotherEntity(const bool collided) { collidedWithAnotherEntity = collided; }
            bool getCollidedWithAnotherEntity() const { return collidedWithAnotherEntity; }
            void applyKnockback(CE::Vector2D direction, float force);

            // stat modifiers
            bool hasBeenHit = false;
            void checkAndApplyDamage(std::uint8_t damage);
            bool isDamageAnimationActive = false;
            void checkDamageAnimation();

    public:
            // weapon for the player entity only, this is so the player can switch weapons from the KNIFE (default and any weapon)
            std::shared_ptr<CE::IWeapon> carryingWeapon = nullptr;

    private:
            bool isEntityFacingRight = false;
            bool collidedWithAnotherEntity = false;
            std::shared_ptr<CE::ITimer> damageTimer = nullptr;
    };
}
