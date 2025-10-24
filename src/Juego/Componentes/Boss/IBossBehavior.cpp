#include "IBossBehavior.h"

namespace IVJ
{
    // TODO: add handling to be able to adjust
    // based on a "difficulty" parameter
    IBossBhvrMirage::IBossBhvrMirage()
    {
        currentTrapsDeployed = RANGED;
        currentTrapsDeployed = 0;
        maxTrapsAllowed = 10;
        meleeAttackDamage = 3;
        rangedAttackDamage = 2;
    }

}