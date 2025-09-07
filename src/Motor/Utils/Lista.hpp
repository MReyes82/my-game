#pragma once

#include <memory>
#include "CEInterfaces.hpp"
#include "Nodo.hpp"

namespace CE
{
    /*
     * Clase Lista
     * Actua como una pila o cola en eliminación y como una lista
     * enlazada en inserción
     */
    template<typename T> class Lista : public IIterador<T>, public IManipularDatos<T>
    {
        public:
            //constructor template se tiene hacee en el header
            //lo métodos pueden ser en el .cpp
            explicit Lista()
                :m_inicio{nullptr},m_final{nullptr},m_current{nullptr},size{0}
            {};
            //interfaz del patrón iterador
            T* iterInicio(); 
            T* iterFinal(); 
            T* iterActual(); 
            T* iterSiguiente(); 
            bool iterTermino() const;

            //interfaz para agregar y eliminar
            void agregarDato(const T& d);
            T eliminarDato(int id);
            T popDato(); 
            T popFrontDato(); 

            //métodos de la clase, en este caso inline
            unsigned int getSize() const {return size;};

        private:
            std::shared_ptr<Nodo<T>> m_inicio;
            std::shared_ptr<Nodo<T>> m_final;
            std::shared_ptr<Nodo<T>> m_current;
            unsigned int size;

    };
    template<typename T> void Lista<T>::agregarDato(const T& d)
    {
        //como el constructor es protegido no se puede usar
        //make_shared(es un componente) no te lo permite pero
        //lo que vamos hacer es instanciar un shared_ptr con un rawpointer
        auto nuevo = std::shared_ptr<Nodo<T>>(new Nodo<T>(d));
        if(m_inicio == nullptr)
            m_inicio = nuevo;
        else
        {
            m_final->m_next = nuevo;
            nuevo->m_prev = m_final;
        }
        m_final = nuevo;
        size++;
    }
    template<typename T> T Lista<T>::eliminarDato(int id)
    {
        //buscar el dato por posicion
        if(id == 0)
            return popFrontDato();
        else if(id== size-1)
            return popDato();
        //buscar
        auto temp = m_inicio;
        for(int i=0;i<id;++i)
            temp=temp->m_next;

        auto d = temp->dato;
        
        temp->m_prev->m_next = temp->m_next;
        temp->m_next->m_prev = temp->m_prev;
        temp = nullptr;
        size--;
        
        return d;
    }
    template<typename T> T Lista<T>::popDato()
    {
        auto d = m_final->dato;
        size--;
        if(m_final == m_inicio)
        {
            m_final = nullptr;
            m_inicio= nullptr;
            return d;
        }
        m_final = m_final->m_prev;
        m_final->m_next = nullptr;
        return d;
    }
    template<typename T> T Lista<T>::popFrontDato()
    {
        auto d = m_inicio->dato;
        size--;
        if(m_final == m_inicio)
        {
            m_final = nullptr;
            m_inicio= nullptr;
            return d;
        }
        m_inicio = m_inicio->m_next;
        m_inicio->m_prev = nullptr;
        return d;
    }
    template<typename T> T* Lista<T>::iterInicio()
    {
        m_current = m_inicio;
        return &m_inicio->dato;
    }
    template<typename T> T* Lista<T>::iterFinal()
    {
        m_current = m_final;
        return &m_final->dato;
    }
    template<typename T> T* Lista<T>::iterActual()
    {
        return &m_current->dato;
    }
    template<typename T> T* Lista<T>::iterSiguiente()
    {
        if(!m_current) return nullptr;
        m_current = m_current->m_next;
        return &m_current->dato;
    }
    template<typename T> bool Lista<T>::iterTermino() const
    {
        return m_current == nullptr;
    }
}
