#include "Escena_Menu.hpp"
#include "../../Motor/Camaras/CamarasGestor.hpp"
#include "../../Motor/Primitivos/GestorEscenas.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../../Motor/Render/Render.hpp"
#include "../objetos/Texto.hpp"
namespace IVJ
{
    void EscenaMenu::onInit()
    {
        CE::GestorCamaras::Get().setCamaraActiva(0);
        if(!inicializar) return;
        //agregar el font
        CE::GestorAssets::Get().agregarFont("Eater",ASSETS "/fonts/Eater.ttf");

        //registramos el esquema de los botones
        registrarBotones(sf::Keyboard::Scancode::W,"arriba");
        registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
        registrarBotones(sf::Keyboard::Scancode::S,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Escape,"esc");
        

        auto texto = std::make_shared<Texto>(CE::GestorAssets::Get().getFont("Eater"),"Continuar");
        texto->setPosicion(100.f,100.f);

        objetos.agregarPool(texto);


        inicializar = false;

    }
    void EscenaMenu::onFinal(){}
    void EscenaMenu::onUpdate(float dt)
    {
        for(auto &f: objetos.getPool())
            f->onUpdate(dt);
        //objetos.borrarPool();
    }
    void EscenaMenu::onInputs(const CE::Botones& accion)
    {
        if(accion.getTipo()==CE::Botones::TipoAccion::OnPress)
        {
            if(accion.getNombre() == "esc")
            {
                CE::GestorEscenas::Get().cambiarEscena("Shaders");
            }
        }
    }

    void EscenaMenu::onInputs(const CE::MouseButton &accion)
    {
        //no implementado
    }

    void EscenaMenu::onRender()
    {
        sf::Text mitexto{CE::GestorAssets::Get().getFont("Eater"),"HOLA!!"};
        mitexto.setCharacterSize(50);
        mitexto.setFillColor(sf::Color::Yellow);
        mitexto.setPosition({500.f,500.f});

        CE::Render::Get().AddToDraw(mitexto);

        for(auto &f: objetos.getPool())
            CE::Render::Get().AddToDraw(*f);
    }
}
