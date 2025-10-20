#pragma once

#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
    /*
     * Class to be used by the overlay class, basically contains all the info
     * to be displayed on the screen, as a primitive type, so it can be decoupled
     * from the rendering system, and work as a data container only
     * meaning it will work as a "builder" object for the UI information
     */
    class InfoUI
    {
    public:
        InfoUI()
            : score{0}, healthAmount{10}, totalHealth{10}, maxAmmo{-1}, currentAmmo{-1}, weapon{CE::WEAPON_TYPE::KNIFE}, utility{CE::UTILITY_TYPE::NONE}
        {}

        // setters and getters
        void setScore(const int scoreToSet) { this->score = scoreToSet; }
        [[nodiscard]] int getScore() const { return score; }
        void setHealth(const int healthToSet) {this->healthAmount = healthToSet; };
        [[nodiscard]] int getHealth() const { return healthAmount; }
        [[nodiscard]] int getTotalHealth() const { return totalHealth; }
        void setMaxAmmo(const int maxAmmoToSet) {this->maxAmmo = maxAmmoToSet; } ;
        [[nodiscard]] int getMaxAmmo() const { return maxAmmo; }
        void setCurrentAmmo(const int currentAmmoToSet) {this->currentAmmo = currentAmmoToSet; } ;
        [[nodiscard]] int getCurrentAmmo() const { return currentAmmo; }
        void setWeapon(const CE::WEAPON_TYPE weaponToSet) { this->weapon = weaponToSet; }
        [[nodiscard]] CE::WEAPON_TYPE getWeapon() const { return weapon; }
        void setUtility(const CE::UTILITY_TYPE utilityToSet) { this->utility = utilityToSet; }
        [[nodiscard]] CE::UTILITY_TYPE getUtility() const { return utility; }

    private:
        int score;
        int healthAmount;
        int totalHealth;
        CE::WEAPON_TYPE weapon;
        int maxAmmo;
        int currentAmmo;
        CE::UTILITY_TYPE utility;
    };
}



