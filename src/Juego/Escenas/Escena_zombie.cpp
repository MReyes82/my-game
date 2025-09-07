#include "Escena_zombie.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../../Motor/Render/Render.hpp"
#include "../objetos/Entidad.hpp"
#include "../../Motor/Camaras/CamarasGestor.hpp"
#include "../Sistemas/Sistemas.hpp"
#include "../Figuras/Figuras.hpp"

namespace IVJ
{
    void Escena_zombie::onInit()
    {
        if (!inicializar)
            return;

        // registrar los botones 
        registrarBotones(sf::Keyboard::Scancode::Escape, "Pausa");

        // objeto dummy para probar el IComponente: IRespawn
        auto dummy = std::make_shared<Entidad>();
        dummy->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("hoja_pink")
            , 68, 91, 1.f));
        dummy->getStats()->hp = 1;
        dummy->setPosicion(-10000000, -10000000);
        objetos.agregarPool(dummy);

        int h = CE::Render::Get().GetVentana().getSize().y;
        int w, sa;
        // crear las entidades enemigas
        for (int i = 0 ; i < 3 ; i++)
        {
            // crear enemigo
            auto p = std::make_shared<IVJ::Entidad>();
            p->getStats()->hp = 255;
            p->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("hoja_yellow"),
            68, 85,
            1.f
            ))
            .addComponente(std::make_shared<CE::INombre>("Enemigo"+std::to_string(i+1)));
            //.addComponente(std::make_shared<CE::IPaths>(600));
            //.addComponente(std::make_shared<CE::IPaths>(600))----;
            /*std::string nombre = p->getComponente<CE::INombre>()->nombre;
            std::cout << nombre << std::endl;*/
            w = p->getComponente<CE::ISprite>()->width;
            sa = p->getComponente<CE::ISprite>()->height;
            int enemy_pos_x = w * 2;
            int enemy_pos_y = h * 0.3334f * (i + 1) - 1.5f * sa;
            //p->setPosicion(w * 2,h * 0.3334f * (i + 1) - 1.5f * sa);
            p->setPosicion(enemy_pos_x, enemy_pos_y);

            // crear carriles de movimiento de los enemigos
            int window_width = CE::Render::Get().GetVentana().getSize().x;
            auto linea = std::make_shared<IVJ::Rectangulo>(
                    window_width, 50, sf::Color{255,255,255,255}, sf::Color{0,255,0,255}
                );
            linea->getStats()->hp = 1;

            // desacoplar las coordenadas de la linea (carril) de la posicion del objeto
            // y poder utilizarla para los puntos de las curvas de IPaths
            float linea_x = w * 3 + window_width / 2;
            float linea_y = h * 0.3334f * (i + 1) - 1.5f * sa;
            linea->setPosicion(linea_x, linea_y);

            // crear puntos de respawn para cada carril
            float respawn_pos_x = window_width - 50;
            float respawn_pos_y = h * 0.3334f * (i + 1) - 1.5f * sa;
            auto respawn = std::make_shared<Rectangulo>(100, 200, sf::Color{255, 0, 0, 255}, sf::Color::Black);
            respawn->getStats()->hp = 1;
            respawn->setPosicion(respawn_pos_x, respawn_pos_y);
            respawn->addComponente(
                std::make_shared<CE::IRespawn>(
                    120 + (i * 60 / 2), 100, 200,
                    dummy.get(), SPAWNS_MAX_ENTITIES));

            auto path = std::make_shared<CE::IPaths>(600);
            // set control points for the paths (pre-defined positions)
            CE::Vector2D spawnInitialPoint = {respawn_pos_x, linea_y};
            CE::Vector2D enemyFinalPoint = {static_cast<float>(enemy_pos_x), linea_y};
            CE::Vector2D midPoint = {(respawn_pos_x + enemy_pos_x) / 2.f, linea_y}; // midpoint between spawn and enemy, same height

            // First curve: from spawn to first intermediate point
            CE::Vector2D firstEnd = {midPoint.x + 100.f, midPoint.y - 100.f}; // Point before midpoint
            path->addCurva(
                spawnInitialPoint,
                CE::Vector2D{spawnInitialPoint.x - 200.f, spawnInitialPoint.y - 150.f},
                firstEnd
            );
            // Second curve: from first intermediate through midpoint to second intermediate
            CE::Vector2D secondEnd = {midPoint.x - 100.f, midPoint.y + 100.f}; // Point after midpoint
            path->addCurva(
                firstEnd,
                midPoint,
                secondEnd
            );
            // Third curve: from second intermediate to enemy
            path->addCurva(
                secondEnd,
                CE::Vector2D{enemyFinalPoint.x + 150.f, enemyFinalPoint.y + 50.f},
                enemyFinalPoint
            );
            prefixed_paths.push_back(*path);
            //objetos.agregarPool(path_test_obj);
            objetos.agregarPool(linea);
            objetos.agregarPool(p);
            objetos.agregarPool(respawn);
        }

        CE::GestorCamaras::Get().setCamaraActiva(1);
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(objetos.getPool()[0]);
        // inicializar las variables

        // detener el sistema de actualizacion
        inicializar = 0;
    }

    void Escena_zombie::onUpdate(float dt)
    {
        SistemaMover(objetos.getPool(), dt);
        //SistemaSpawn(objetos.getPool());
        SistemaSpawn(objetos, prefixed_paths);
        SistemaPaths(objetos.getPool());

        for(auto &f: objetos.getPool())
            f->onUpdate(dt);
        objetos.borrarPool();
    }

    void Escena_zombie::onRender()
    {
        for (auto& obj : objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*obj);
        }
    }

    void Escena_zombie::onFinal()
    {
        
    }

    void Escena_zombie::onInputs(const CE::Botones& accion)
    {
        // procesar los inputs
        // si es necesario, cambiar de escena
    }

    void Escena_zombie::onInputs(const CE::MouseButton &accion)
    {
        //no implementado
    }
}