#pragma once
#include "../../Motor/Primitivos/Objetos.hpp" 
#include "../Maquinas/FSM.hpp"
#include <memory>
#include <Motor/Utils/Utils.hpp>

#include "Motor/Primitivos/Objetos.hpp"

namespace IVJ
{
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
            bool hasTimerReachedMax(CE::ITimer* timer) const;
            void resetTimer(CE::ITimer* timer);
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
            void resetSpeed(float currentBoost);
            /////// METHODS ONLY FOR THE PLAYER ///////
            bool weaponIsEmpty(); // check if the current weapon has no ammo left
            void updateReloadStatus(); // update the status of the reload
            void handleReload(); // handle the reload process
            void attackWithKnife(bool isAttacking, std::shared_ptr<Entidad>& enemyToAttack);

    public:
            // weapon for the player entity only, this is so the player can switch weapons from the KNIFE (default and any weapon)
            std::shared_ptr<CE::IWeapon> carryingWeapon = nullptr;
            std::shared_ptr<CE::ITimer> damageTimer = nullptr;
            std::shared_ptr<CE::ITimer> velocityBoostTimer = nullptr; // for energy drink effect in player and other effects for other entities
            std::shared_ptr<CE::ITimer> reloadTimer = nullptr; // for weapon reloading (player only)
            std::shared_ptr<CE::ITimer> fireRateTimer = nullptr; // for controlling fire rate of weapons (player only)
            bool shouldChangeWeapon = false; // flag to indicate if the player weapon needs to be changed (plauyer only)
            bool shouldChangeUtility = false; // flag to indicate if the player utility item needs to be changed (player only)
            bool isVelocityBoostActive = false; // flag to indicate if the velocity boost effect is active (player only)
            bool isReloading = false; // flag to indicate if the weapon is reloading (player only)
            bool finishedAttackAnimation = false; // flag to indicate if the attack animation has finished (enemies only)

    private:
            bool isEntityFacingRight = false;
            bool collidedWithAnotherEntity = false;
    };
}
