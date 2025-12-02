#pragma once
#include "Juego/Componentes/IJComponentes.hpp"
#include "Motor/Primitivos/Objetos.hpp"

namespace IVJ
{
    [[maybe_unused]] bool checkRayHit(CE::Objeto& npc, CE::Vector2D& p1, CE::Vector2D& p2);
    [[maybe_unused]] void SystemDialogues(IDialogo* dialogue, CE::Objeto& obj);
    [[maybe_unused]] bool checkDistanceInteraction(CE::Objeto& player, CE::Objeto& npc, float maxDistance);

    // System functions for NPC quest handling (work with Entidad vectors from SystemGetEntityTypeVector)
    [[maybe_unused]] void SysUpdateQuestNPCs(std::vector<std::shared_ptr<Entidad>>& npcs,
                                              std::shared_ptr<Entidad>& player,
                                              float dt);
    [[maybe_unused]] void SysRenderQuestDialogues(std::vector<std::shared_ptr<Entidad>>& npcs);

    // Dialogue-specific system functions
    [[maybe_unused]] void SysCargarTextoDesdeID(IDialogo* dialogo, int dialogue_id);
    [[maybe_unused]] void SysAvanzarDialogo(IDialogo* dialogo);
    [[maybe_unused]] void SysResetearDialogo(IDialogo* dialogo);
    [[maybe_unused]] void SysOnInteractuarDialogo(IDialogo* dialogo, CE::Objeto& obj);
    [[maybe_unused]] void SysOnRenderDialogo(IDialogo* dialogo, CE::Objeto& npc);

    // DEBUG
    [[maybe_unused]] void debugDrawRay(CE::Vector2D& p1, CE::Vector2D& p2, const sf::Color& color = sf::Color::Red);
}
