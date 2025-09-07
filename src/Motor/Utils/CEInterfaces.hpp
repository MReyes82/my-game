#pragma once

namespace CE
{
    template<typename T> class IIterador
    {
        public:
            virtual T* iterInicio()=0; 
            virtual T* iterFinal()=0; 
            virtual T* iterActual()=0; 
            virtual T* iterSiguiente()=0; 
            virtual bool iterTermino() const = 0;
    };

    template<typename T> class IManipularDatos
    {
        public:
            virtual void agregarDato(const T& d)=0;
            virtual T eliminarDato(int id)=0;
            virtual T popDato()=0; 
            virtual T popFrontDato()=0; 
    };
}
