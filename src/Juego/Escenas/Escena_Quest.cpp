#include "Escena_Quest.hpp"

#include "Juego/Maquinas/JugadorFSM/IdleFSM.hpp"
#include "Juego/objetos/Entidad.hpp"
#include "Juego/Sistemas/Sistemas.hpp"
#include "Juego/Sistemas/Quest/QuestSystems.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Render/Render.hpp"


namespace IVJ
{
    Escena_Quest::Escena_Quest(std::shared_ptr<Entidad>& pref)
        : CE::Escena(), jugador_ref{pref}
    {

    }

    void Escena_Quest::onInit()
    {
        CE::GestorCamaras::Get().setCamaraActiva(0);

        if(!inicializar) return;

        //probar sonidos
        /*CE::GestorAssets::Get().agregarMusica("mainTema",ASSETS "/musica/MainTheme.ogg");
        CE::GestorAssets::Get().agregarSonido("pasos",ASSETS "/sonidos/footsteps_short.wav");*/

        //registrar botones
        registrarBotones(sf::Keyboard::Scancode::W,"arriba");
        registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
        registrarBotones(sf::Keyboard::Scancode::S,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
        registrarBotones(sf::Keyboard::Scancode::A,"izquierda");
        registrarBotones(sf::Keyboard::Scancode::Left,"izquierda");
        registrarBotones(sf::Keyboard::Scancode::D,"derecha");
        registrarBotones(sf::Keyboard::Scancode::Right,"derecha");
        registrarBotones(sf::Keyboard::Scancode::LShift,"correr");
        registrarBotones(sf::Keyboard::Scancode::F,"rayo");

        inicializar=false;

        CE::GestorAssets::Get().agregarTextura("hoja_pink", ASSETS "/sprites/aliens/alienPink.png",
                                         CE::Vector2D{0, 0}, CE::Vector2D{256, 512});
        CE::GestorAssets::Get().agregarTextura("hoja_yellow", ASSETS "/sprites/aliens/alienYellow.png",
                                         CE::Vector2D{0, 0}, CE::Vector2D{256, 512});

        /*if(!bg[0].loadTileMap(ASSETS "/mapas/mapa_quest_layer1.txt")) exit(EXIT_FAILURE);
        if(!bg[1].loadTileMap(ASSETS "/mapas/mapa_quest_layer2.txt")) exit(EXIT_FAILURE);*/


        jugador_ref->setPosicion(500.f,500.f);
        jugador_ref->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("hoja_pink"),
                    68,92,
                    1.0f));
        //shader
        //jugador_ref->addComponente(std::make_shared<CE::IShader>(
        //            ASSETS "/shaders/wave.vert",
        //            ASSETS "/shaders/prueba.frag"));
        jugador_ref->addComponente(std::make_shared<IVJ::IMaquinaEstado>());
        jugador_ref->addComponente(std::make_shared<CE::IControl>());
        jugador_ref->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{68,92}));

        auto &fsm_init = jugador_ref->getComponente<IMaquinaEstado>()->fsm;
        fsm_init  = std::make_shared<IdleFSM>();
        fsm_init->onEntrar(*jugador_ref);

        jugador_ref->addComponente(std::make_shared<IRayo>(
            jugador_ref->getTransformada()->posicion,
            jugador_ref->getTransformada()->velocidad.x,
            100.f
        ));
        jugador_ref->getStats()->maxSpeed = 150.f;


        //NPC
        auto npc = std::make_shared<Entidad>();

        npc->getStats()->hp = 100;
        npc->setPosicion(296,464);
        npc->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("hoja_yellow"),
            68,85,
            1.f));
        npc->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{68,85}));
        npc->addComponente(std::make_shared<CE::IControl>());
        npc->addComponente(std::make_shared<IMaquinaEstado>());
        //auto &npc_fsm = npc->getComponente<IMaquinaEstado>()->fsm;
        /*npc_fsm = std::make_shared<IdleFSM>(
            CE::Vector2D{0.f, 173.f},
            CE::Vector2D{0.f, 86.f}
        );
        npc_fsm->onEntrar(*npc);*/
        // Add dialogue component with ID 1 as start, and 4 total dialogues (1-4)
        npc->addComponente(std::make_shared<IVJ::IDialogo>(1, 4));

        objetos.agregarPool(npc);

        //CE::GestorAssets::Get().getMusica("mainTema").play();

    }
    void Escena_Quest::onFinal()
    {
    }
    void Escena_Quest::onUpdate(float dt)
    {

        SistemaControl(*jugador_ref,dt);
        //SistemaMover({jugador_ref},dt);
        SistemaMoverEntidad({jugador_ref}, dt);
        SistemaMover(objetos.getPool(),dt);
        jugador_ref->inputFSM();
        jugador_ref->onUpdate(dt);
        for(auto &f: objetos.getPool())
        {
            // Check if player is in interaction range
            bool in_range = checkDistanceInteraction(*jugador_ref,*f,200.f);
            bool in_raycast = checkRayHit(*f,
                jugador_ref->getComponente<IRayo>()->getP1(),
                jugador_ref->getComponente<IRayo>()->getP2());

            if(f->tieneComponente<IVJ::IDialogo>())
            {
                auto dialogo = f->getComponente<IVJ::IDialogo>();

                if(in_range && in_raycast)
                {
                    // Player is in range and facing NPC
                    // Activate dialogue on first interaction (when pressing F)
                    if(jugador_ref->getComponente<CE::IControl>()->NPCinteract && !dialogo->activo)
                    {
                        dialogo->activo = true;
                    }

                    // Always call onInteractuar to track key state (for edge detection)
                    if(dialogo->activo)
                    {
                        dialogo->onInteractuar(*jugador_ref);
                    }
                }
                else if(!in_range)
                {
                    // Player walked away - reset dialogue
                    dialogo->activo = false;
                    IVJ::SysResetearDialogo(dialogo);
                }
            }

            f->onUpdate(dt);
        }
        objetos.borrarPool();
    }
    void Escena_Quest::onInputs(const CE::Botones& accion)
    {
        auto c = jugador_ref->getComponente<CE::IControl>();

        if(accion.getTipo()==CE::Botones::TipoAccion::OnPress && c->isActivo())
        {
            if(accion.getNombre()=="correr")
            {
                c->run=true;
            }
            if(accion.getNombre()=="arriba")
            {
                c->arr=true;
            }
            else if(accion.getNombre()=="derecha")
            {
                c->der=true;
            }
            else if(accion.getNombre()=="abajo")
            {
                c->abj=true;
            }
            else if(accion.getNombre()=="izquierda")
            {
                c->izq=true;
            }
            else if(accion.getNombre()=="pausa")
            {
                //CE::GestorEscenas::Get().cambiarEscena("pausa");
            }
            else if(accion.getNombre()=="rayo")
            {
                c->NPCinteract=true;
            }
        }
        else if(accion.getTipo()==CE::Botones::TipoAccion::OnRelease && c->isActivo())
        {
            if(accion.getNombre()=="correr")
            {
                c->run=false;
            }
            if(accion.getNombre()=="arriba")
            {
                c->arr=false;
            }
            else if(accion.getNombre()=="derecha")
            {
                c->der=false;
            }
            else if(accion.getNombre()=="abajo")
            {
                c->abj=false;
            }
            else if(accion.getNombre()=="izquierda")
            {
                c->izq=false;
            }
            else if(accion.getNombre()=="rayo")
            {
                c->NPCinteract=false;
            }
        }
        if(!c->isActivo())
        {
            c->arr=false;
            c->der=false;
            c->abj=false;
            c->izq=false;
            c->run=false;
            c->NPCinteract=false;
        }
    }

    void Escena_Quest::onInputs(const CE::MouseButton& accion)
    {
        //just to shut off the fucking compiler
    }

    void Escena_Quest::onRender()
    {
        /*for(auto &tm : bg)
        {
            CE::Render::Get().AddToDraw(tm);
        }*/
        for(auto &obj: objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*obj);
        }
        CE::Render::Get().AddToDraw(*jugador_ref);


        for(auto &f: objetos.getPool())
        {
            if(f->tieneComponente<IVJ::IDialogo>())
            {
                auto dialogo = f->getComponente<IVJ::IDialogo>();
                if(!dialogo->activo) continue;
                IVJ::SysOnRenderDialogo(dialogo, *f);
            }
        }
#if DEBUG
        //DEBUG RAYO
        if(jugador_ref->getComponente<CE::IControl>()->NPCinteract)
        {
            if(jugador_ref->tieneComponente<IRayo>())
            {
                auto rayo = jugador_ref->getComponente<IRayo>();
                debugDrawRay(rayo->getP1(),rayo->getP2(),sf::Color::Red);
            }
        }
#endif
    }
}
