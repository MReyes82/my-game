#pragma once
#include "Juego/Componentes/IJComponentes.hpp"
#include "Motor/Primitivos/Objetos.hpp"
#include "Motor/Primitivos/CEPool.hpp"

namespace IVJ
{
    [[maybe_unused]] bool checkRayHit(CE::Objeto& npc, CE::Vector2D& p1, CE::Vector2D& p2);
    [[maybe_unused]] bool checkDistanceInteraction(CE::Objeto& player, CE::Objeto& npc, float maxDistance);

    // System functions for NPC quest handling (work with Entidad vectors from SystemGetEntityTypeVector)
    [[maybe_unused]] void SysUpdateQuestNPCs(std::vector<std::shared_ptr<Entidad>>& npcs,
                                              std::shared_ptr<Entidad>& player,
                                              float dt);
    [[maybe_unused]] void SysRenderQuestDialogues(std::vector<std::shared_ptr<Entidad>>& npcs);

    // Dialogue-specific system functions
    [[maybe_unused]] void SysCargarTextoDesdeID(IDialogo* dialogo, int dialogue_id);
    [[maybe_unused]] void SysAvanzarDialogo(IDialogo* dialogo, std::shared_ptr<Entidad>& player);
    [[maybe_unused]] void SysResetearDialogo(IDialogo* dialogo);
    [[maybe_unused]] void SysOnInteractuarDialogo(IDialogo* dialogo, CE::Objeto& obj, std::shared_ptr<Entidad>& player);
    [[maybe_unused]] void SysOnRenderDialogo(IDialogo* dialogo, CE::Objeto& npc);

    // Quest system functions
    [[maybe_unused]] void SysOnInteractuarSignalJammer(ISignalJammer* jammer, CE::Objeto& jammerEntity);
    [[maybe_unused]] void SysResetJammerStabilization(std::vector<std::shared_ptr<Entidad>>& jammers);
    [[maybe_unused]] void SysRenderStabilizationText(std::vector<std::shared_ptr<Entidad>>& jammers);
    [[maybe_unused]] void SysUpdateQuestState(std::shared_ptr<Entidad>& player);
    [[maybe_unused]] void SysUpdateSignalJammers(std::vector<std::shared_ptr<Entidad>>& jammers,
                                                  std::shared_ptr<Entidad>& player,
                                                  CE::Pool& pool,
                                                  float dt);

    // Phase 1: Trap Lesson - spawn traps around jammer
    [[maybe_unused]] void SysSpawnTrapsAroundJammer(ISignalJammer* jammer, CE::Objeto& jammerEntity, CE::Pool& pool);
    [[maybe_unused]] void SysUpdateQuestTraps(std::vector<std::shared_ptr<Entidad>>& traps,
                                               std::shared_ptr<Entidad>& player, float dt);
    [[maybe_unused]] void SysCheckPhase1Complete(ISignalJammer* jammer, IQuest* quest, CE::Objeto& jammerEntity);

    // Phase 2: Ranged Attack Lesson - burst shooting mechanics
    [[maybe_unused]] void SysJammerShootProjectileBursts(ISignalJammer* jammer, CE::Objeto& jammerEntity,
                                                          std::shared_ptr<Entidad>& player,
                                                          CE::Pool& pool, float dt);
    [[maybe_unused]] void SysUpdateQuestProjectiles(std::vector<std::shared_ptr<Entidad>>& projectiles,
                                                     std::shared_ptr<Entidad>& player, float dt);
    [[maybe_unused]] void SysCheckPhase2Complete(ISignalJammer* jammer, IQuest* quest, CE::Objeto& jammerEntity);

    // Phase 3: Teleport Lesson - teleportation mechanics
    [[maybe_unused]] void SysInitializeTeleportPositions(ISignalJammer* jammer, CE::Objeto& jammerEntity);
    [[maybe_unused]] void SysJammerTeleport(ISignalJammer* jammer, CE::Objeto& jammerEntity, float dt);
    [[maybe_unused]] void SysCheckPhase3Complete(ISignalJammer* jammer, IQuest* quest, CE::Objeto& jammerEntity);

    // Navigation Arrow System - guide player to next objective
    [[maybe_unused]] void SysUpdateQuestNavigationTarget(std::shared_ptr<Entidad>& player,
                                                          std::vector<std::shared_ptr<Entidad>>& npcs,
                                                          std::vector<std::shared_ptr<Entidad>>& jammers);
    [[maybe_unused]] void SysInitNavigationArrow(std::shared_ptr<Entidad>& player, CE::Pool& pool);
    [[maybe_unused]] void SysUpdateNavigationArrow(std::shared_ptr<Entidad>& player,
                                                    std::vector<std::shared_ptr<Entidad>>& npcs);
    [[maybe_unused]] void SysRenderNavigationArrow(std::shared_ptr<Entidad>& player);

    // DEBUG
    [[maybe_unused]] void debugDrawRay(CE::Vector2D& p1, CE::Vector2D& p2, const sf::Color& color = sf::Color::Red);
}
