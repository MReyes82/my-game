#pragma once
#include "Juego/Componentes/IJComponentes.hpp"
#include "Motor/Primitivos/Objetos.hpp"

namespace IVJ
{
    [[maybe_unused]] bool checkRayHit(CE::Objeto& npc, CE::Vector2D& p1, CE::Vector2D& p2);
    [[maybe_unused]] void SystemDialogues(IDialogo* dialogue, CE::Objeto& obj);
    [[maybe_unused]] bool checkDistanceInteraction(CE::Objeto& player, CE::Objeto& npc, float maxDistance);
    // DEBUG
    [[maybe_unused]] void debugDrawRay(CE::Vector2D& p1, CE::Vector2D& p2, const sf::Color& color = sf::Color::Red);
}
