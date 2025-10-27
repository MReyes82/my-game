#pragma once
#include "Juego/objetos/Entidad.hpp"
#include "Juego/Componentes/Boss/IBossBehavior.h"
#include "Motor/Primitivos/CEPool.hpp"
#include <vector>

namespace IVJ
{
    //////// SYSTEMS FOR BOSS BEHAVIORS ////////
    //////// GENERAL SYSTEMS ////////
    [[maybe_unused]] void BSysAdjustBossStats(std::shared_ptr<Entidad>& boss, BOSS_TYPE bossTyp);

    /////// MIRAGE SYSTEMS ///////
    [[maybe_unused]] void MirageInit(std::shared_ptr<Entidad>& boss, std::array<CE::Vector2D, 20>& positionsArr);
    [[maybe_unused]] void BSysMrgMovement(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player,
                                          std::vector<std::shared_ptr<Entidad>>& bossProjectiles,
                                          std::vector<std::shared_ptr<Entidad>>& traps,
                                          CE::Pool& pool, float worldWidth, float worldHeight, float dt);

    // Ranged attack systems
    [[maybe_unused]] void BSysMrgRangedAttack(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player,
                                              std::vector<std::shared_ptr<Entidad>>& bossProjectiles,
                                              CE::Pool& pool, float dt);
    [[maybe_unused]] void BSysMrgCreateTrap(std::shared_ptr<Entidad>& boss, std::vector<std::shared_ptr<Entidad>>& traps,
                                            CE::Pool& pool);
    [[maybe_unused]] void BSysMrgMaintainDistance(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player,
                                                  float distance, float worldWidth, float worldHeight);
    [[maybe_unused]] void BSysUpdateProjectiles(std::vector<std::shared_ptr<Entidad>>& bossProjectiles,
                               std::shared_ptr<Entidad>& boss,
                               std::vector<std::shared_ptr<Entidad>>& bossTraps,
                               std::shared_ptr<Entidad>& player,
                               CE::Pool& pool, float dt);
    [[maybe_unused]] void BSysUpdateTraps(std::vector<std::shared_ptr<Entidad>>& bossTraps,
                                          std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player, float dt);
}
