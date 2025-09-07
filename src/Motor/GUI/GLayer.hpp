#pragma once
#include "../Utils/Utils.hpp"

namespace CE
{
    class GLayer
    {
        public:
            virtual ~GLayer(){};
            virtual void OnInit(const MotorConfig& des)=0;
            virtual void OnUpdate(float dt)=0;
            virtual void OnRender(void)=0;
    };
}

