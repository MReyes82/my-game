#include "QuestSystems.hpp"

#include "Motor/Render/Render.hpp"
#include "Motor/Utils/Lerp.hpp"

namespace IVJ
{
    bool checkRayHit(CE::Objeto& npc, CE::Vector2D& p1, CE::Vector2D& p2)
    {
        if (!npc.tieneComponente<CE::IBoundingBox>())
            return false;

        auto midNpc = npc.getComponente<CE::IBoundingBox>()->mitad;
        auto posNpc = npc.getTransformada()->posicion;
        auto line = p2 - p1;

        float dx = std::abs(posNpc.x - p1.x);
        float dy = std::abs(posNpc.y - p1.y);

        float sumMidX = midNpc.x + std::abs(line.x);
        float sumMidY = midNpc.y + std::abs(line.y);

        return (sumMidX - dx > 0) && (sumMidY - dy > 0);
    }

    void SystemDialogues(IDialogo* dialogue, CE::Objeto& obj)
    {
        if (!dialogue)
            return;

        dialogue->activo = true;
        dialogue->onInteractuar(obj);
    }

    bool checkDistanceInteraction(CE::Objeto& player, CE::Objeto& npc, float maxDistance)
    {
        auto playerPos = player.getTransformada()->posicion;
        auto npcPos = npc.getTransformada()->posicion;
        float distance = playerPos.distancia(npcPos);
        return distance <= maxDistance;
    }
    // DEBUG
    void debugDrawRay(CE::Vector2D& p1, CE::Vector2D& p2, const sf::Color& color)
    {
        sf::CircleShape cp1 {2.f};
        cp1.setFillColor({0, 0, 0, 255});
        sf::CircleShape cp2 {2.f};
        cp2.setFillColor({0, 0, 0, 255});
        cp1.setPosition({p1.x, p1.y});
        cp2.setPosition({p2.x, p2.y});
        // draw guide lines
        for (float t = 0.f ; t <= 1.f ; t += 0.0015f)
        {
            sf::CircleShape pixel{1};
            pixel.setFillColor(color);
            auto pos = CE::lerp(p1, p2, t);
            pixel.setPosition({pos.x, pos.y});
            CE::Render::Get().AddToDraw(pixel);
        }
        CE::Render::Get().AddToDraw(cp1);
        CE::Render::Get().AddToDraw(cp2);
    }
}
