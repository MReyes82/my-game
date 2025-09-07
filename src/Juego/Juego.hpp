#pragma once
#include"../Motor/App/ALayer.hpp"
//#include <vector>
//#include "../Motor/Primitivos/Objetos.hpp"
#include "../Motor/Primitivos/Escena.hpp"

//#include <memory>
namespace IVJ
{
    class Juego: public CE::GameLayer
    {
        public:
            explicit Juego();
            ~Juego()override{};
            void OnInit(void) override;
            void OnInputs(float dt ,std::optional<sf::Event>& eventos) override;
            void OnInputs(float dt ,CE::InputBuffer& buffer) override;
            void OnInputs(float dt) override;
            void OnUpdate(float dt) override;
            void OnRender(float dt) override;
        private:
            //std::vector<std::shared_ptr<CE::Objeto>> objetos;
            //CE::Pool pool{1000};
            CE::Escena* escena_actual;
    };

}

