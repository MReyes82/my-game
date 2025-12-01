#include "IJComponentes.hpp"

#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Render/Render.hpp"

namespace IVJ
{
    IMaquinaEstado::IMaquinaEstado()
    {
        fsm = nullptr;
    }

    IRayo::IRayo(CE::Vector2D &pos, float &sig, const float magnitud)
        : magnitud {magnitud}, dir {sig}, lp1 {pos}, lp2 {0.f, 0.f}
    {}

    CE::Vector2D &IRayo::getP1() const
    {
        return lp1;
    }

    CE::Vector2D& IRayo::getP2()
    {
        // int direccion = (dir>0) ? 1 : (dir < 0) ? -1 : 0;
        const int direction = (dir > 0) ? 1 : -1; // if dir is 0, default to -1
        lp2.x = lp1.x + direction * magnitud; // extend in x direction based on dir sign
        lp2.y = lp1.y; // keep same y position

        return lp2; // we return by reference to avoid copying
    }

    // interactuables
    IDialogo::IDialogo()
        : IInteractable(), texto{L""}, id_texto{0}, id_inicial{0},
          indice_actual{1}, max_dialogos{1}
    {
        texto = L"Texto de Prueba ABC ABC, se debe cargar por el arbol, al presionar una tecla"
            L"se avanza al siguiente texto del dialogo, esto es solo una demostración."
            L" El texto debe tener saltos de linea automáticos para no salirse de la caja"
            L"  de dialogo que se renderiza en pantalla."
            L"Espero que funcione correctamente y se vea bien en pantalla.";
    }

    IDialogo::IDialogo(int dialogue_id)
        : IInteractable(), texto{L""}, id_texto{dialogue_id}, id_inicial{dialogue_id},
          indice_actual{1}, max_dialogos{1}
    {
        cargarTextoDesdeID(dialogue_id);
    }

    IDialogo::IDialogo(int initial_dialogue_id, int max_dialogue_count)
        : IInteractable(), texto{L""}, id_texto{initial_dialogue_id},
          id_inicial{initial_dialogue_id}, indice_actual{1}, max_dialogos{max_dialogue_count}
    {
        cargarTextoDesdeID(initial_dialogue_id);
    }

    void IDialogo::cargarTextoDesdeID(int dialogue_id)
    {
        id_texto = dialogue_id;
        texto = CE::GestorAssets::Get().getDialogue(dialogue_id);
    }

    void IDialogo::avanzarDialogo()
    {
        if (indice_actual < max_dialogos)
        {
            indice_actual++;
            int next_id = id_inicial + indice_actual - 1;
            cargarTextoDesdeID(next_id);
        }
    }

    void IDialogo::resetearDialogo()
    {
        indice_actual = 1;
        cargarTextoDesdeID(id_inicial);
        activo = false;
        primera_vez = true; // Reset so next interaction shows first dialogue
        last_interact_state = false; // Reset interaction state
    }

    void IDialogo::onInteractuar(CE::Objeto &obj)
    {
        if (!activo)
        {
            return;
        }

        auto control = obj.getComponente<CE::IControl>();
        bool current_interact = control->NPCinteract;

        // Detect rising edge: key just pressed (was false, now true)
        bool key_just_pressed = current_interact && !last_interact_state;

        if (key_just_pressed)
        {
            // On first press, just show the current dialogue (don't advance)
            if (primera_vez)
            {
                primera_vez = false; // Next press will advance
            }
            else
            {
                // On subsequent presses, advance to next dialogue
                avanzarDialogo();
            }
        }

        // Update last state for next frame
        last_interact_state = current_interact;
    }

    std::wstring IDialogo::agregarSaltoLinea(const std::wstring& str, size_t max_len)
    {
        std::wstring resultado;
        size_t pos = 0;

        while (pos < str.size())
        {
            size_t chunk_size = std::min(max_len, str.size() - pos);
            resultado += str.substr(pos, chunk_size) + L'\n'; // extract the chunk and add newline
            pos += chunk_size; // move to the next chunk

            if (pos < str.size())
            {
                resultado += L"\n"; // add extra newline for paragraph spacing;
            }
        }
        return resultado;
    }

    void IDialogo::onRender()
    {
        auto dim = CE::Render::Get().GetVentana().getSize();
        sf::RectangleShape background;

        background.setSize({dim.x * 0.7f, 150.f});
        background.setFillColor({0, 0, 0, 150});
        background.setPosition({(dim.x / 3.f) - 200.f, 120.f});

        sf::Font font = CE::GestorAssets::Get().getFont("NotJamSlab14");
        std::wstring text_with_linebreaks = agregarSaltoLinea(texto, 76);
        sf::Text renderedText {font, text_with_linebreaks, 20};
        renderedText.setPosition({(dim.x / 3.f) - 180.f, 130.f});
        // add to render queue
        CE::Render::Get().AddToDraw(background);
        CE::Render::Get().AddToDraw(renderedText);
    }
}
