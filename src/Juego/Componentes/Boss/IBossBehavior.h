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

        ATTACK_PHASE currentAttackPhase;
        int currentTrapsDeployed;
        int maxTrapsAllowed;
        int meleeAttackDamage;
        int rangedAttackDamage;

    };
}
