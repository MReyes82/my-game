#pragma once

#include <string>
#include "../objetos/Entidad.hpp"
#include "../../Motor/Primitivos/Objetos.hpp"
#include "../../Motor/Primitivos/CEPool.hpp"

namespace IVJ
{
    [[maybe_unused]]void SistemaControl(CE::Objeto&ente, float dt);
    [[maybe_unused]]void SistemaMover(const std::vector<std::shared_ptr<CE::Objeto>>& entes, float dt);
    [[maybe_unused]]bool SistemaColAABB(CE::Objeto& A,CE::Objeto& B, bool resolucion =false);
    [[maybe_unused]]bool SistemaColAABBMid(CE::Objeto& A, CE::Objeto& B, bool resolucion =false);

    [[maybe_unused]] void SistemaSpawn(CE::Pool& spawns, std::vector<CE::IPaths> paths);
    [[maybe_unused]] void SistemaPaths(std::vector<std::shared_ptr<CE::Objeto>>& obj);
    [[maybe_unused]] void SistemaAsignarPath(std::vector<CE::IPaths> paths, std::shared_ptr<CE::Objeto> objeto);

    // -------------- MAIN SCENE SYSTEMS --------------
    //* enemies and entities
    [[maybe_unused]] std::string SystemChooseEnemyType(int num);
    [[maybe_unused]] void SystemFollowPlayer(std::vector<std::shared_ptr<CE::Objeto>>& chasers, CE::Objeto& target, float dt);
    [[maybe_unused]] std::vector<std::shared_ptr<Entidad>>& SystemGetEntityTypeVector(std::vector<std::shared_ptr<CE::Objeto>>& entities, CE::ENTITY_TYPE entityType);
    [[maybe_unused]] void SystemCheckLimits(std::vector<std::shared_ptr<CE::Objeto>>& entes, unsigned int dimensionX, unsigned int dimensionY);
    // loot methods
    [[maybe_unused]] int SystemChooseRandLootItem(bool isWeapon);
    [[maybe_unused]] void SystemAdjustEntityStats(std::shared_ptr<CE::Objeto> entity, int type);
    [[maybe_unused]] CE::Vector2D getRandomPosition(std::array<CE::Vector2D, 20> positionsArr);
    [[maybe_unused]] void SystemUpdatePlayerWeaponStats(CE::WEAPON_TYPE weaponType, const std::shared_ptr<Entidad>& player);
}
