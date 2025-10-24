#pragma once
#include "Juego/objetos/Entidad.hpp"
#include "Juego/Componentes/Boss/IBossBehavior.h"

namespace IVJ
{
    //////// SYSTEMS FOR BOSS BEHAVIORS ////////
    //////// GENERAL SYSTEMS ////////
    [[maybe_unused]] void BSysChooseAttackPhase();
    [[maybe_unused]] void BSysMeleeAttack();
    [[maybe_unused]] void BSysRangedAttack();
    [[maybe_unused]] void BSysAdjustBossStats(std::shared_ptr<Entidad>& boss, BOSS_TYPE bossTyp);

    /////// MIRAGE SYSTEMS ///////
    [[maybe_unused]] void MirageInit(std::shared_ptr<Entidad>& boss, std::array<CE::Vector2D, 20>& positionsArr);
}
