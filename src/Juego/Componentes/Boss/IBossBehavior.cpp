#include "IBossBehavior.h"

namespace IVJ
{
    // TODO: add handling to be able to adjust
    // based on a "difficulty" parameter
    IBossBhvrMirage::IBossBhvrMirage() :
        currentAttackPhase{MELEE},
        currentMeleeAttack{NONE},
        currentTrapsDeployed{0},
        maxTrapsAllowed{10},
        meleeAttackDamage{1},
        quickMeleeAttackDamage{2},
        rangedAttackDamage{2},
        meleeAttackRange{80.0f},
        isWindingUp{false},
        hasLandedAttack{false},
        didTeleport{false},
        rangedKeepDistanceRange{250.0f},
        currentBurstCount{0},
        currentProjectilesInBurst{0},
        isShootingBurst{false}
    {
        // Initialize timer components (assuming 60 FPS)
        meleeAttackCooldownTimer = std::make_shared<CE::ITimer>(120);  // 2 seconds at 60 FPS
        simpleMeleeWindupTimer = std::make_shared<CE::ITimer>(72);     // 1.2 seconds at 60 FPS
        quickMeleeWindupTimer = std::make_shared<CE::ITimer>(60);      // 1.0 second at 60 FPS

        // Ranged attack timers
        rangedAttackTimer = std::make_shared<CE::ITimer>(180);  // 3 seconds at 60 FPS
        projectileBurstTimer = std::make_shared<CE::ITimer>(30);  // 30 frames between projectiles

        // Reset all timers to 0
        meleeAttackCooldownTimer->frame_actual = 0;
        simpleMeleeWindupTimer->frame_actual = 0;
        quickMeleeWindupTimer->frame_actual = 0;
        rangedAttackTimer->frame_actual = 0;
        projectileBurstTimer->frame_actual = 0;
    }

}