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
        std::shared_ptr<CE::ITimer> attackModeTimer; // Timer to switch between melee and ranged phases (3 minutes)

        // HP threshold tracking for mode changes (10% intervals)
        int lastHpThresholdCrossed;  // Last HP threshold that triggered a mode change (in percentage: 90, 80, 70, etc.)
        int hpThresholdInterval;     // Interval for HP thresholds (default: 10%)

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
        bool isExecutingMeleeAttack;  // Flag for FSM to trigger melee attack animation

        // Ranged attack properties
        float rangedKeepDistanceRange;  // Distance to maintain from player (~200 units)
        std::shared_ptr<CE::ITimer> rangedAttackTimer;  // 3 second timer for ranged attacks
        std::shared_ptr<CE::ITimer> projectileBurstTimer;  // Timer for burst cadence (30 frames)
        int currentBurstCount;  // Current burst number (0-2, for 3 bursts)
        int currentProjectilesInBurst;  // Current projectile in burst (0-4, for 5 projectiles)
        bool isShootingBurst;  // Is currently shooting a burst of projectiles
        bool isExecutingRangedAttack;  // Flag for FSM to trigger ranged attack animation

    };
}
