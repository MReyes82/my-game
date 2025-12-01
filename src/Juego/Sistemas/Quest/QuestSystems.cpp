#include "QuestSystems.hpp"

#include "Motor/Render/Render.hpp"
#include "Motor/Utils/Lerp.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"

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

    void SysOnRenderDialogo(IDialogo* dialogo)
    {
        if (!dialogo)
            return;

        auto dim = CE::Render::Get().GetVentana().getSize();
        sf::RectangleShape background;

        background.setSize({dim.x * 0.7f, 150.f});
        background.setFillColor({0, 0, 0, 150});
        background.setPosition({(dim.x / 3.f) - 200.f, 120.f});

        sf::Font font = CE::GestorAssets::Get().getFont("NotJamSlab14");
        std::wstring text_with_linebreaks = agregarSaltoLinea(dialogo->texto, 76);
        sf::Text renderedText {font, text_with_linebreaks, 20};
        renderedText.setPosition({(dim.x / 3.f) - 180.f, 130.f});

        // add to render queue
        CE::Render::Get().AddToDraw(background);
        CE::Render::Get().AddToDraw(renderedText);
    }
}
