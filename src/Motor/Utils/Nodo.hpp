#pragma once
#include <memory>
namespace CE
{
    //definición de la clase para poder 
    //asignarla como amigable antes de definirla
    template<typename T> class Lista;

    template<typename T> class Nodo
    {
        //nadie puede instanciar la clase Nodo solo clases amigables
        friend class Lista<T>;
        //para poder usar el constructor en la clase amigables
        protected:
        Nodo(T d)
            :dato{d}
        {
            m_next = nullptr;
            m_prev = nullptr;
        }
        public:
        T dato;
        std::shared_ptr<Nodo<T>> m_prev; 
        std::shared_ptr<Nodo<T>> m_next; 
        unsigned int size;      
    };
}
