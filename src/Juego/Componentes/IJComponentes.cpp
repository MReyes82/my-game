#include "IJComponentes.hpp"

#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Render/Render.hpp"
#include "Juego/Sistemas/Quest/QuestSystems.hpp"

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
        SysCargarTextoDesdeID(this, dialogue_id);
    }

    IDialogo::IDialogo(int initial_dialogue_id, int max_dialogue_count)
        : IInteractable(), texto{L""}, id_texto{initial_dialogue_id},
          id_inicial{initial_dialogue_id}, indice_actual{1}, max_dialogos{max_dialogue_count}
    {
        SysCargarTextoDesdeID(this, initial_dialogue_id);
    }

    void IDialogo::onInteractuar(CE::Objeto &obj)
    {
        // Delegate to system function
        SysOnInteractuarDialogo(this, obj);
    }
}
