#include "EscenaCuadro.hpp"
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



namespace IVJ
{
    void EscenaCuadros::onInit()
    {
        if(!inicializar) return;

        //cargamos las texturas correspondientes a esta escena
        CE::GestorAssets::Get().agregarTextura("hoja_pink",ASSETS "/sprites/aliens/alienPink.png",
                CE::Vector2D{0,0},CE::Vector2D{256,512});
        CE::GestorAssets::Get().agregarTextura("hoja_yellow",ASSETS "/sprites/aliens/alienYellow.png",
                CE::Vector2D{0,0},CE::Vector2D{256,512});

        if(!bg[0].loadTileMap(ASSETS "/mapas/mapa_1_layer_1.txt")) exit(EXIT_FAILURE);
        if(!bg[1].loadTileMap(ASSETS "/mapas/mapa_1_layer_2.txt")) exit(EXIT_FAILURE);

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
        registrarBotones(sf::Keyboard::Scancode::Space,"brincar");

        //vamos hacer 3 figuras
        auto fig1 = std::make_shared<Rectangulo>(
                100,100,sf::Color(255,0,0,255),
                sf::Color(0,0,0,255));
        fig1->setPosicion(100,400);
        fig1->getStats()->hp=100;
        auto fig2 = std::make_shared<Rectangulo>(
                200,100,sf::Color(169,169,0,255),
                sf::Color(0,0,0,255));
        fig2->setPosicion(300,100);
        fig2->getStats()->hp=100;
        auto fig3 = std::make_shared<Rectangulo>(
                100,200,sf::Color(0,0,255,255),
                sf::Color(0,0,0,255));
        fig3->setPosicion(600,400);
        fig3->getStats()->hp=100;

        auto fig4 = std::make_shared<Circulo>(
                10,sf::Color(255,255,255,255),
                sf::Color(0,0,0,255));
        fig4->setPosicion(100,400);
        fig4->getStats()->hp=100;

        objetos.agregarPool(fig1);
        objetos.agregarPool(fig2);
        objetos.agregarPool(fig3);
        objetos.agregarPool(fig4);

        //creamos la entidad para probar el sprite
        jugador = std::make_shared<Entidad>();
        jugador->getStats()->hp = 100;
        jugador->setPosicion(500.f,500.f);
        jugador->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("hoja_pink"),
                    68,92,
                    2.0f));
        //shader
        jugador->addComponente(std::make_shared<CE::IShader>(
                    ASSETS "/shaders/wave.vert",
                    ASSETS "/shaders/prueba.frag"));
        jugador->addComponente(std::make_shared<IVJ::IMaquinaEstado>());
        jugador->addComponente(std::make_shared<CE::IControl>());

        auto &fsm_init = jugador->getComponente<IMaquinaEstado>()->fsm;
        fsm_init  = std::make_shared<IdleFSM>();
        fsm_init->onEntrar(*jugador);
        //jugador->setFSM(std::make_shared<IdleFSM>());

        //variables shader por defecto
        wave_ampli={1.5,1.f};

        static sf::Texture textura{
            CE::GestorAssets::Get().getTextura("hoja_yellow")};

        jugador->getComponente<CE::IShader>()->setTextura("textura",&CE::GestorAssets::Get().getTextura("hoja_pink"));
        jugador->getComponente<CE::IShader>()->setTextura("textura_prueba",&textura);

        static float blend = 0.51f;
        jugador->getComponente<CE::IShader>()->setEscalar("blend",&blend);


        //SHADER 2
        int ida=26;
        int ax0=(ida%40)*64;
        int ay0=(ida/40)*64;
        //GAME_ASSETS "/sprites/atlas/atlas1.png",ax0,ay0,64*4,64*4,1)
        CE::GestorAssets::Get().agregarTextura("techo",ASSETS "/atlas/atlas1.png",
                CE::Vector2D{(float)ax0,(float)ay0},CE::Vector2D{64*4,64*4});
        auto niebla = std::make_shared<Entidad>();
        niebla->getStats()->hp = 100;
        niebla->setPosicion(835.f,275.f);
        niebla->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("techo"),1.f));

        niebla->addComponente(std::make_shared<CE::IShader>(
                    "",
                    ASSETS "/shaders/fog.frag"));

        //variables para el shader
        auto * shader = niebla->getComponente<CE::IShader>();

        static sf::Vector2f lp ={0.f,0.f};
        shader->setVector2("lampPos",&lp);
        static float lr = 0.4f;
        shader->setEscalar("lampRadio",&lr);
        static float ls = 0.1f;
        shader->setEscalar("lampSoft",&ls);
        static sf::Glsl::Vec4 fc{0,0,0,0.8};
        shader->setVector4("fogColor",&fc);
        static float t = 0.f; 
        shader->setEscalar("tiempo",&t);
        static float re = 20.f; 
        shader->setEscalar("ruido_escala",&re);
        static float rmm = 0.35f; 
        shader->setEscalar("ruido_min",&rmm);
        static float rm = 0.6f; 
        shader->setEscalar("ruido_max",&rm);
        static float ro= 0.5f; 
        shader->setEscalar("opacidad_offset",&ro);

        objetos.agregarPool(niebla);
        //for(int i=0;i<500;i++)
        //{
        //auto figx = std::make_shared<Circulo>(
        //        10,sf::Color(5,5,5,255),
        //        sf::Color(0,0,0,255));
        //figx->setPosicion(100,400);
        //figx->getStats()->hp=100;
        //objetos.agregarPool(figx);
        //}

        objetos.agregarPool(jugador);


        //agregamos una camara
        CE::GestorCamaras::Get().agregarCamara(std::make_shared<CE::CamaraLERP>(
                    CE::Vector2D{540,360},CE::Vector2D{1080.f,720.f}));
        CE::GestorCamaras::Get().setCamaraActiva(1);
        //lockeamos la camara en un objeto
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(jugador);

        inicializar = false;

    }
    void EscenaCuadros::onFinal(){}
    void EscenaCuadros::onUpdate(float dt)
    {
        SistemaControl(*jugador,dt);
        SistemaMover(objetos.getPool(),dt);
        jugador->inputFSM();

        //LERP CUADRADA
        const int max_frames = 120;
        static int tiempo_frame = 0;
        static unsigned char reversa = 0;
        const CE::Vector2D inicio = objetos[0]->getTransformada()->posicion;
        const CE::Vector2D control = objetos[1]->getTransformada()->posicion;
        const CE::Vector2D destino = objetos[2]->getTransformada()->posicion;

        auto npos =  CE::lerp2(inicio,control,destino,(float)tiempo_frame/max_frames); 
        objetos[3]->setPosicion(npos.x,npos.y);

        if(tiempo_frame >max_frames-2)
            reversa = 1;
        if(tiempo_frame < 0)
            reversa = 0;
        if(reversa == 1)
            tiempo_frame--;
        else
            tiempo_frame++;
        //---- LERP---|
        for(auto &f: objetos.getPool())
            f->onUpdate(dt);
        objetos.borrarPool();


        static float norm_frame=0;
        norm_frame=(tiempo_frame%max_frames)/(float)max_frames;

        wave_phase+=0.01f;
        //vertex variables
        jugador->getComponente<CE::IShader>()->setEscalar("wave_phase",&wave_phase);
        jugador->getComponente<CE::IShader>()->setVector2("wave_amplitude",&wave_ampli);
        //fragment variable
        jugador->getComponente<CE::IShader>()->setTextura("textura",&CE::GestorAssets::Get().getTextura("hoja_pink"));
        jugador->getComponente<CE::IShader>()->setTextura("textura_prueba",&CE::GestorAssets::Get().getTextura("hoja_yellow"));
        jugador->getComponente<CE::IShader>()->setEscalar("frame",&norm_frame);
        auto key_blend =jugador->getComponente<CE::IShader>()->m_vars["blend"].second;
        jugador->getComponente<CE::IShader>()->setEscalar("blend",(float*)key_blend);


        //SHADER 2

        auto * shader = objetos[4]->getComponente<CE::IShader>();
        auto jp = jugador->getTransformada()->posicion;
        auto jdw = jugador->getComponente<CE::ISprite>()->width;
        auto jdh = jugador->getComponente<CE::ISprite>()->height;
        auto np = objetos[4]->getTransformada()->posicion;
        auto nw = objetos[4]->getComponente<CE::ISprite>()->width;
        auto nh = objetos[4]->getComponente<CE::ISprite>()->height;
        auto lp = static_cast<sf::Vector2f*>(shader->m_vars["lampPos"].second);
        lp->x= (jp.x+jdw/0.5f-np.x)/(nw);
        lp->y= (jp.y+jdh/0.75f-np.y)/(nh);
        shader->setVector2("lampPos",lp);
        auto lr = static_cast<float*>(shader->m_vars["lampRadio"].second);
        shader->setEscalar("lampRadio",lr);
        auto ls = static_cast<float*>(shader->m_vars["lampSoft"].second);
        shader->setEscalar("lampSoft",ls);
        auto fc = static_cast<sf::Glsl::Vec4*>(shader->m_vars["fogColor"].second);
        shader->setVector4("fogColor",fc);
        auto t = static_cast<float*>(shader->m_vars["tiempo"].second);
        *t+=dt;
        shader->setEscalar("tiempo",t);
        auto re = static_cast<float*>(shader->m_vars["ruido_escala"].second);
        shader->setEscalar("ruido_escala",re);
        auto rmm =  static_cast<float *>(shader->m_vars["ruido_min"].second);
        shader->setEscalar("ruido_min",rmm);
        auto rm =  static_cast<float*>(shader->m_vars["ruido_max"].second);
        shader->setEscalar("ruido_max",rm);
        auto ro=  static_cast<float*>(shader->m_vars["opacidad_offset"].second);
        shader->setEscalar("opacidad_offset",ro);


    }
    void EscenaCuadros::onInputs(const CE::Botones& accion)
    {
        //auto p = jugador->getTransformada();
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
                //p->velocidad.y=-200;
            }
            else if(accion.getNombre()=="derecha")
            {
                c->der=true;
                //p->velocidad.x=200;
            }
            else if(accion.getNombre()=="abajo")
            {
                c->abj=true;
                //p->velocidad.y=200;
            }
            else if(accion.getNombre()=="izquierda")
            {
                c->izq=true;
                //p->velocidad.x=-200;
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
                //p->velocidad.y=-200;
            }
            else if(accion.getNombre()=="derecha")
            {
                c->der=false;
                //p->velocidad.x=200;
            }
            else if(accion.getNombre()=="abajo")
            {
                c->abj=false;
                //p->velocidad.y=200;
            }
            else if(accion.getNombre()=="izquierda")
            {
                c->izq=false;
                //p->velocidad.x=-200;
            }
        }
    }

    void EscenaCuadros::onInputs(const CE::MouseButton &accion)
    {
        //no implementado
    }

    void EscenaCuadros::onRender()
    {
        for(auto& b: bg)
            CE::Render::Get().AddToDraw(b);

        for(auto &f: objetos.getPool())
            CE::Render::Get().AddToDraw(*f);
    }
}
