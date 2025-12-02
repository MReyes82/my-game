#include "QuestSystems.hpp"

#include "Motor/Render/Render.hpp"
#include "Motor/Utils/Lerp.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"

namespace IVJ
{
    // Helper function for adding line breaks
    static std::wstring agregarSaltoLinea(const std::wstring& str, size_t max_len)
    {
        std::wstring resultado;
        size_t pos = 0;

        while (pos < str.size())
        {
            size_t chunk_size = std::min(max_len, str.size() - pos);
            resultado += str.substr(pos, chunk_size) + L'\n';
            pos += chunk_size;

            if (pos < str.size())
            {
                resultado += L"\n";
            }
        }
        return resultado;
    }

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
        SysOnInteractuarDialogo(dialogue, obj);
    }

    bool checkDistanceInteraction(CE::Objeto& player, CE::Objeto& npc, float maxDistance)
    {
        auto playerPos = player.getTransformada()->posicion;
        auto npcPos = npc.getTransformada()->posicion;
        float distance = playerPos.distancia(npcPos);
        return distance <= maxDistance;
    }

    // System functions for NPC quest handling
    void SysUpdateQuestNPCs(std::vector<std::shared_ptr<Entidad>>& npcs,
                            std::shared_ptr<Entidad>& player,
                            float dt)
    {
        if (!player || !player->tieneComponente<IRayo>())
            return;

        auto rayo = player->getComponente<IRayo>();

        for (auto& npc : npcs)
        {
            if (!npc->tieneComponente<IVJ::IDialogo>())
                continue;

            // Check if player is in interaction range
            bool in_range = checkDistanceInteraction(*player, *npc, 100.f);
            bool in_raycast = checkRayHit(*npc, rayo->getP1(), rayo->getP2());

            auto dialogo = npc->getComponente<IVJ::IDialogo>();

            if (in_range && in_raycast)
            {
                // Player is in range and facing NPC
                // Activate dialogue on first interaction (when pressing F)
                if (player->getComponente<CE::IControl>()->NPCinteract && !dialogo->activo)
                {
                    dialogo->activo = true;
                }

                // Always call onInteractuar to track key state (for edge detection)
                if (dialogo->activo)
                {
                    dialogo->onInteractuar(*player);
                }
            }
            else if (!in_range)
            {
                // Player walked away - reset dialogue
                dialogo->activo = false;
                SysResetearDialogo(dialogo);
            }

            npc->onUpdate(dt);
        }
    }

    void SysRenderQuestDialogues(std::vector<std::shared_ptr<Entidad>>& npcs)
    {
        for (auto& npc : npcs)
        {
            if (npc->tieneComponente<IVJ::IDialogo>())
            {
                auto dialogo = npc->getComponente<IVJ::IDialogo>();
                if (!dialogo->activo)
                    continue;
                SysOnRenderDialogo(dialogo, *npc);
            }
        }
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

    // Dialogue-specific system functions
    void SysCargarTextoDesdeID(IDialogo* dialogo, int dialogue_id)
    {
        if (!dialogo)
            return;

        dialogo->id_texto = dialogue_id;
        dialogo->texto = CE::GestorAssets::Get().getDialogue(dialogue_id);
    }

    void SysAvanzarDialogo(IDialogo* dialogo)
    {
        if (!dialogo)
            return;

        if (dialogo->indice_actual < dialogo->max_dialogos)
        {
            dialogo->indice_actual++;
            int next_id = dialogo->id_inicial + dialogo->indice_actual - 1;
            SysCargarTextoDesdeID(dialogo, next_id);
        }
    }

    void SysResetearDialogo(IDialogo* dialogo)
    {
        if (!dialogo)
            return;

        dialogo->indice_actual = 1;
        SysCargarTextoDesdeID(dialogo, dialogo->id_inicial);
        dialogo->activo = false;
        dialogo->primera_vez = true;
        dialogo->last_interact_state = false;
    }

    void SysOnInteractuarDialogo(IDialogo* dialogo, CE::Objeto& obj)
    {
        if (!dialogo || !dialogo->activo)
            return;

        auto control = obj.getComponente<CE::IControl>();
        if (!control)
            return;

        bool current_interact = control->NPCinteract;

        // Detect rising edge: key just pressed (was false, now true)
        bool key_just_pressed = current_interact && !dialogo->last_interact_state;

        if (key_just_pressed)
        {
            // On first press, just show the current dialogue (don't advance)
            if (dialogo->primera_vez)
            {
                dialogo->primera_vez = false; // Next press will advance
            }
            else
            {
                // On subsequent presses, advance to next dialogue
                SysAvanzarDialogo(dialogo);
            }
        }

        // Update last state for next frame
        dialogo->last_interact_state = current_interact;
    }

    void SysOnRenderDialogo(IDialogo* dialogo, CE::Objeto& npc)
    {
        if (!dialogo)
            return;

        // Get NPC position to position dialogue box above NPC
        auto npcPos = npc.getTransformada()->posicion;

        // Calculate NPC sprite bounds to position above head
        float npcHeight = 85.f * 0.5f; // NPC sprite height * scale

        sf::Font font = CE::GestorAssets::Get().getFont("PressStart");
        // Reduce max line length for smaller box
        std::wstring text_with_linebreaks = agregarSaltoLinea(dialogo->texto, 35);
        sf::Text renderedText {font, text_with_linebreaks, 8}; // Font size 8 for PressStart2P

        // Get text bounds to size background properly
        auto textBounds = renderedText.getLocalBounds();
        float padding = 12.f; // Increased padding for larger background box

        sf::RectangleShape background;
        // Size background to fit text with padding
        background.setSize(sf::Vector2f{textBounds.size.x + padding * 2, textBounds.size.y + padding * 2});
        background.setFillColor({0, 0, 0, 180});
        // Position centered above NPC head
        background.setPosition(sf::Vector2f{
            npcPos.x - (textBounds.size.x + padding * 2) / 2.f,
            npcPos.y - npcHeight - 65.f
        });

        // Position text inside background with padding
        renderedText.setPosition(sf::Vector2f{
            background.getPosition().x + padding,
            background.getPosition().y + padding
        });

        // add to render queue
        CE::Render::Get().AddToDraw(background);
        CE::Render::Get().AddToDraw(renderedText);
    }
}
