#include "IJComponentes.hpp"

#include "Motor/Render/Render.hpp"

namespace IVJ
{

    IMaquinaEstado::IMaquinaEstado()
    {
        fsm = nullptr;
    }

    /*IRayo::IRayo(CE::Vector2D &pos, float &sig, const float magnitud)
        : magnitud {magnitud}, dir {sig}, lp1 {pos}, lp2 (CE::Vector2D{0.f, 0.f})
    {

    }*/

    CE::Vector2D &IRayo::getP1() const
    {
        return lp1;
    }

    CE::Vector2D IRayo::getP2()
    {
        // int direccion = (dir>0) ? 1 : (dir < 0) ? -1 : 0;
        int direccion = (dir > 0) ? 1 : -1;
        lp2 = CE::Vector2D{
            lp1.x + direccion * magnitud,
            lp1.y
        };
        return lp2;
    }

    // interactuables
    IDialogo::IDialogo()
        : IInteractable(), texto{L""}, id_texto{0}
    {
        texto = L"Texto de prueba ABC ABC"
        L"Hola papu"
        L"El texto del papu";
    }

    void IDialogo::onInteractuar(CE::Objeto &obj)
    {
        // detch el dialogo del arbol
        if (!activo)
        {
            return;
        }
        auto control = obj.getComponente<CE::IControl>();
        if (control->NPCinteract && !interactuado)
        {

        }
    }
}
