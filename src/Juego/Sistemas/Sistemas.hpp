#pragma once

#include <string>
#include "../objetos/Entidad.hpp"
#include "../../Motor/Primitivos/Objetos.hpp"
#include "../../Motor/Primitivos/CEPool.hpp"
#include "Juego/Escenas/Overlay/InfoUI.hpp"
#include <set>

namespace IVJ
{
    [[maybe_unused]]void SistemaControl(CE::Objeto&ente, float dt);
    [[maybe_unused]]void SistemaMover(const std::vector<std::shared_ptr<CE::Objeto>>& entes, float dt);
    [[maybe_unused]] void SistemaMoverEntidad(const std::vector<std::shared_ptr<Entidad>>& entes, float dt);
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
    [[maybe_unused]] CE::WEAPON_TYPE SystemChooseRandWeapon();
    [[maybe_unused]] CE::UTILITY_TYPE SystemChooseRandUtility(); // separated so they can return an enum value directly
    [[maybe_unused]] void SystemAdjustEntityStats(std::shared_ptr<CE::Objeto> entity, int type);
    [[maybe_unused]] int SystemGetRandomPosition(std::array<CE::Vector2D, 20> positionsArr, std::set<int>& usedIndices);
    [[maybe_unused]] void SystemUpdatePlayerWeaponStats(CE::WEAPON_TYPE weaponType, const std::shared_ptr<Entidad>& player);
    [[maybe_unused]] void SystemUpdateLootItems(std::vector<std::shared_ptr<Entidad>>& lootItems,
        std::shared_ptr<Entidad>& player, CE::WEAPON_TYPE& tempRefWpn, CE::UTILITY_TYPE& tempRefUtil,
        std::array<CE::Vector2D, 20> positionsArr, bool& shouldChangeWeaponFlag, bool& shouldChangeUtilFlag, float dt);

    [[maybe_unused]] void SystemUpdateEnergyDrink(bool& isEnergyDrinkActive, std::shared_ptr<Entidad>& player, float dt);
    [[maybe_unused]] void SystemChangePlayerItems(bool& shouldChangeWeaponFlag, bool& shouldChangeUtilFlag,
        std::shared_ptr<Entidad>& player,
        CE::WEAPON_TYPE& tempRefWpn, CE::UTILITY_TYPE& tempRefUtil, InfoUI& sceneOverlayElements);
    [[maybe_unused]] void SystemCreateLootItems(std::vector<std::shared_ptr<Entidad>>& lootItems , const std::array<CE::Vector2D, 20>& positionsArr, int maxFrames, int maxLootItems);

    // -------------- BULLET GENERATION SYSTEMS --------------
    [[maybe_unused]] CE::Vector2D calculateProjectileVel(const sf::Vector2i& mousePos, const CE::Vector2D& projectilePos, float speed);
    [[maybe_unused]] void SystemGenerateBullets(bool isAttacking, std::shared_ptr<Entidad>& player, std::vector<std::shared_ptr<Entidad>>& bulletsShot);
    [[maybe_unused]] void SystemAddEntitiesToPool(std::vector<std::shared_ptr<Entidad>>& entities, CE::Pool& pool);
    [[maybe_unused]] void SystemUpdateBulletsState(std::vector<std::shared_ptr<Entidad>>& bulletsShot,
        std::vector<std::shared_ptr<Entidad>>& enemies,
        std::shared_ptr<Entidad>& player,
        CE::Pool& collisionPool,
        int& currentEnemiesInScene,
        float dt);
}
