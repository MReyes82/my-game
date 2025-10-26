#pragma once
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
    enum class BOSS_TYPE
    {
        MIRAGE,
    };

    // list of the components to define boss behaviors
    // every single one carrying the prefix "IBossBhvr"
    class IBossBhvrMirage : public CE::IComponentes
    {
    public:
        explicit IBossBhvrMirage();
        ~IBossBhvrMirage() override = default;

        enum ATTACK_PHASE
        {
            MELEE,
            RANGED,
        };

        enum MELEE_ATTACK_TYPE
        {
            SIMPLE,  // Slow, telegraphed attack with higher damage
            QUICK,   // Fast attack with lower damage
            NONE     // Not currently attacking
        };

        ATTACK_PHASE currentAttackPhase;
        MELEE_ATTACK_TYPE currentMeleeAttack;

        int currentTrapsDeployed;
        int maxTrapsAllowed;
        int meleeAttackDamage;
        int quickMeleeAttackDamage;
        int rangedAttackDamage;

        // Melee attack timing (using ITimer components)
        std::shared_ptr<CE::ITimer> meleeAttackCooldownTimer;    // Cooldown between attacks
        std::shared_ptr<CE::ITimer> simpleMeleeWindupTimer;      // Windup time for simple attack
        std::shared_ptr<CE::ITimer> quickMeleeWindupTimer;       // Windup time for quick attack

        // Attack range
        float meleeAttackRange;
        bool isWindingUp;  // Is the boss currently winding up an attack?
        bool hasLandedAttack; // Has the current attack landed?
        bool didTeleport; // Did the boss teleport for the current attack?

    };
}
