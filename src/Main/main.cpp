#include<iostream>
#include "../Motor/Motor.hpp"
#include "../Motor/Utils/Utils.hpp"
#include "../Juego/Juego.hpp"
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[])
{
    std::srand(std::time({}));
    //Crear objeto del motor
    CE::MotorConfig config;
    config.vW=1900;
    config.vH=1020;
    config.titulo="CimaEngine";
    std::cout<<"Iniciando "<<config.titulo<<"\n";

    CE::Motor motor{config,std::make_unique<IVJ::Juego>()};
    //ejecutar

    return motor.OnRun();
}
