#pragma once
#include "GLayer.hpp"
#include "../Utils/Lista.hpp"
#include <utility> //pair
#include <imgui.h>
namespace CE
{
    class GLogger : public GLayer
    {
        public:
        enum class Niveles
        {
            LOG=0,
            LOG_DEBUG,
            LOG_ERROR,
            LOG_SEVERO
        };

        public:
            static GLogger& Get(void);
            ~GLogger() override{};

            //de la clase abstracta
            void OnInit(const MotorConfig& des) override;
            void OnUpdate(float dt) override;
            void OnRender(void) override;

            //del logger
            void agregarLog(const std::string& log,Niveles n);
        private:
            ImVec4 nivelColores(Niveles n);
            std::string nivelToString(Niveles n);
        private:
            explicit GLogger();
            Lista<std::pair<std::string,Niveles>> queue;
            static inline GLogger *instancia = nullptr;
            std::string m_sfps;
    };
}
