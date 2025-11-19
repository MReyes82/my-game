#pragma once
#include "../Maquinas/FSM.hpp"
//#include "../../Motor/Componentes/IComponentes.hpp"
#include "Motor/Componentes/IComponentes.hpp"

namespace IVJ
{
    class FSM; //refefencia circular
    //Maquina estado componente
    class IMaquinaEstado : public CE::IComponentes
    {
        public:
            explicit IMaquinaEstado();
            ~IMaquinaEstado(){};
        public:
            std::shared_ptr<FSM> fsm;
    };

    class IRayo : public CE::IComponentes
    {
    public:
        explicit IRayo(CE::Vector2D& pos, float& sig, const float magnitud = 1.f);
        ~IRayo() override{};

        CE::Vector2D& getP1() const;
        CE::Vector2D getP2();
        float getMagnitud() const { return magnitud; }

    private:
        float magnitud;
        float& dir;
        CE::Vector2D& lp1;
        CE::Vector2D& lp2;
    };

    class IInteractable : public CE::IComponentes
    {
    public:
        IInteractable() = default;
        ~IInteractable() override {};
        virtual void onInteractuar(CE::Objeto& obj) = 0;

        bool interactuado {false};
    };

    class IDialogo : public IInteractable
    {
    public:
        IDialogo();
        ~IDialogo() override {};
        void onInteractuar(CE::Objeto &obj) override;
        void onRender();

    private:
        std::wstring texto;
        int id_texto;
        bool activo {false};

        std::wstring agregarSaltoLinea(const std::wstring& str, size_t max_len);
    };
}
