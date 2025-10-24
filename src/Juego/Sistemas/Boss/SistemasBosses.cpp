#include "SistemasBosses.h"
#include "../Sistemas.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"

namespace IVJ
{
    // system to adjust the stats of the boss based on its type
    void BSysAdjustBossStats(std::shared_ptr<Entidad>& boss, BOSS_TYPE bossTyp)
    {
        float baseSpeed = 80.f;
        const auto stats = boss->getStats();

        switch (bossTyp)
        {
            case BOSS_TYPE::MIRAGE:
                const auto bossBehaviorComp = boss->getComponente<IBossBhvrMirage>();
                if (!bossBehaviorComp)
                {
                    CE::printDebug("[SISTEMAS BOSS] Error: boss behavior component not found for Mirage boss");
                    return;
                }

                stats->hp = 200;
                stats->hp_max = 200;
                stats->damage = bossBehaviorComp->rangedAttackDamage; // projectile phase is default
                stats->maxSpeed = baseSpeed * 1.2f;
                break;
        }
    }
    // constructor for the Mirage boss
    // this function assumes that the boss pointer passed as parameter it's already created
    void MirageInit(std::shared_ptr<Entidad>& boss, std::array<CE::Vector2D, 20>& positionsArr)
    {
        std::set<int> usedIndices;
        int posIndex = SystemGetRandomPosition(positionsArr, usedIndices);
        CE::Vector2D spawnPos = positionsArr[posIndex];
        boss->setPosicion(spawnPos.x, spawnPos.y);
        boss->addComponente(std::make_shared<IBossBhvrMirage>());
        BSysAdjustBossStats(boss, BOSS_TYPE::MIRAGE);
        boss->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("MirageSprite"),
            64, 64, 1.f))
        .addComponente(std::make_shared<CE::IBoundingBox>(
            CE::Vector2D{64.f, 64.f}));
        //.addComponente();
    }
}
