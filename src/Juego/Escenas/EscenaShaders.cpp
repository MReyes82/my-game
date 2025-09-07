#include "EscenaShaders.hpp"
#include "../Figuras/Figuras.hpp"
#include "../../Motor/Camaras/CamarasGestor.hpp"
#include "../../Motor/Render/Render.hpp"
#include "../../Motor/Primitivos/GestorEscenas.hpp"
#include "../../Motor/Utils/Vector2D.hpp"
#include "../../Motor/Utils/Lerp.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../Sistemas/Sistemas.hpp"
#include "../Componentes/IJComponentes.hpp"
#include "../Maquinas/JugadorFSM/IdleFSM.hpp"
#include <memory>
#include <SFML/Graphics.hpp>

namespace IVJ 
{
    void EscenaShaders::onInit()
    {
        if(!inicializar) return;

        //cargamos las texturas correspondientes a esta escena
        CE::GestorAssets::Get().agregarTextura("hoja_pink",ASSETS "/sprites/aliens/alienPink.png",
                CE::Vector2D{0,0},CE::Vector2D{256,512});
        CE::GestorAssets::Get().agregarTextura("hoja_yellow",ASSETS "/sprites/aliens/alienYellow.png",
                CE::Vector2D{0,0},CE::Vector2D{256,512});
        CE::GestorAssets::Get().agregarTextura("nubes",ASSETS "/texturas/nube1.png",
                CE::Vector2D{0,0},CE::Vector2D{400,400});

        if(!bg[0].loadTileMap(ASSETS "/mapas/mapa_1_layer_1_col.txt",objetos)) exit(EXIT_FAILURE);
        if(!bg[1].loadTileMap(ASSETS "/mapas/mapa_1_layer_2_col.txt",objetos)) exit(EXIT_FAILURE);

        //registramos el esquema de los botones
        registrarBotones(sf::Keyboard::Scancode::W,"arriba");
        registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
        registrarBotones(sf::Keyboard::Scancode::S,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
        registrarBotones(sf::Keyboard::Scancode::A,"izquierda");
        registrarBotones(sf::Keyboard::Scancode::Left,"izquierda");
        registrarBotones(sf::Keyboard::Scancode::D,"derecha");
        registrarBotones(sf::Keyboard::Scancode::Right,"derecha");
        registrarBotones(sf::Keyboard::Scancode::Escape,"circulos");
        registrarBotones(sf::Keyboard::Scancode::LShift,"correr");


        //creamos la entidad para probar el sprite
        jugador = std::make_shared<Entidad>();
        jugador->getStats()->hp = 100;
        jugador->setPosicion(500.f,500.f);
        jugador->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("hoja_pink"),
                    68,92,
                    2.0f));
        jugador->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{68*2.f,92*2.f}));
        //shader
        jugador->addComponente(std::make_shared<CE::IShader>(
                    ASSETS "/shaders/pruebaVert.vert",
                    ASSETS "/shaders/prueba.frag"));
        static float delay = 1.0f;
        jugador->getComponente<CE::IShader>()->setEscalar("delay",&delay);

        jugador->addComponente(std::make_shared<IVJ::IMaquinaEstado>());
        jugador->addComponente(std::make_shared<CE::IControl>());

        auto &fsm_init = jugador->getComponente<IMaquinaEstado>()->fsm;
        fsm_init  = std::make_shared<IdleFSM>();
        fsm_init->onEntrar(*jugador);



        //edificio?
        int ida=26;
        int ax0=(ida%40)*64;
        int ay0=(ida/40)*64;
        //GAME_ASSETS "/sprites/atlas/atlas1.png",ax0,ay0,64*4,64*4,1)
        CE::GestorAssets::Get().agregarTextura("techo",ASSETS "/atlas/atlas1.png",
                CE::Vector2D{(float)ax0,(float)ay0},CE::Vector2D{64*4,64*4});
        for(int i=0;i<3;i++)
        {
            auto lago = std::make_shared<Entidad>();
            lago->getStats()->hp = 100;
            int minx= 500+std::rand()%500;
            int miny= 500+std::rand()%500;
            lago->setPosicion(minx+rand()%200,miny+std::rand()%300);
            lago->addComponente(std::make_shared<CE::ISprite>(
                        CE::GestorAssets::Get().getTextura("techo"),1.f));
            lago->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{64*4,64*4}));
            objetos.agregarPool(lago);
        }

        objetos.agregarPool(jugador);

        //agregamos una camara
        CE::GestorCamaras::Get().agregarCamara(std::make_shared<CE::CamaraLERP>(
                    CE::Vector2D{540,360},CE::Vector2D{1080.f,720.f}));
        CE::GestorCamaras::Get().setCamaraActiva(1);
        //lockeamos la camara en un objeto
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(jugador);

        inicializar = false;

    }
    void EscenaShaders::onFinal(){}
    void EscenaShaders::onUpdate(float dt)
    {

        SistemaControl(*jugador,dt);
        //shader
        static float frame = 0;
        auto shader = jugador->getComponente<CE::IShader>();
        float delay = *static_cast<float*>(shader->m_vars["delay"].second);
        frame += dt*delay;

        if(frame >1.0)
            frame = 0;
        
        shader->setEscalar("dt",&frame);
        
        auto sprite = jugador->getComponente<CE::ISprite>();
        
        static sf::Texture textura = sprite->m_sprite.getTexture();
        shader->setTextura("personaje",&textura);

        shader->setTextura("nubes",
                &CE::GestorAssets::Get().getTextura("nubes"));


        SistemaMover(objetos.getPool(),dt);
        jugador->inputFSM();

        //---- LERP---|
        for(auto &f: objetos.getPool())
        {
            if(f!=jugador)
                SistemaColAABB(*jugador,*f,true);
                //SistemaColAABBMid(*jugador,*f,true);
            f->onUpdate(dt);
        }
        objetos.borrarPool();



    }
    void EscenaShaders::onInputs(const CE::Botones& accion)
    {
        auto c = jugador->getComponente<CE::IControl>();

        if(accion.getTipo()==CE::Botones::TipoAccion::OnPress)
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
            else if(accion.getNombre()=="circulos")
            {
                CE::GestorEscenas::Get().cambiarEscena("Circulos");
            }
        }
        else if(accion.getTipo()==CE::Botones::TipoAccion::OnRelease)
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
        }
    }

    void EscenaShaders::onInputs(const CE::MouseButton &accion)
    {
        //no implementado
    }

    void EscenaShaders::onRender()
    {
        for(auto& b: bg)
            CE::Render::Get().AddToDraw(b);

        for(auto &f: objetos.getPool())
            CE::Render::Get().AddToDraw(*f);
    }
}

