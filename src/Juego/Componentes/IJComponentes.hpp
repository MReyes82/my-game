#pragma once
#include "../Maquinas/FSM.hpp"
#include "Motor/Componentes/IComponentes.hpp"
#include "Motor/Primitivos/Objetos.hpp"

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
        explicit IRayo(CE::Vector2D& pos, float& sig, float magnitud = 1.f);
        ~IRayo() override{};

        CE::Vector2D& getP1() const;
        CE::Vector2D& getP2();
        float getMagnitud() const { return magnitud; }

    private:
        float magnitud;
        float& dir;
        CE::Vector2D& lp1;
        CE::Vector2D lp2;
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
        explicit IDialogo(int dialogue_id);
        explicit IDialogo(int initial_dialogue_id, int max_dialogue_count);
        ~IDialogo() override {};
        void onInteractuar(CE::Objeto &obj) override; // Keep for interface compliance, but will call system function

        std::wstring texto;
        int id_texto;
        int id_inicial;
        int indice_actual;
        int max_dialogos;
        bool activo {false};
        bool primera_vez {true}; // Flag to track if this is the first interaction
        bool last_interact_state {false}; // Track previous frame's interact state
    };
}
