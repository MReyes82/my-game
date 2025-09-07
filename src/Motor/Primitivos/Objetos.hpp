#pragma once
#include<SFML/Graphics.hpp>
#include "../Componentes/IComponentes.hpp"

namespace CE
{
    class Objeto: public sf::Drawable, public  sf::Transformable
    {
        public:
            explicit Objeto();
            virtual ~Objeto()=default;
            void draw(sf::RenderTarget& target, sf::RenderStates state) const override;
            Objeto& addComponente(const std::shared_ptr<IComponentes>& componnentes);

            template <typename T> Objeto& copyComponente(const T* componnentes)
            {
                // params: // T* componnentes: puntero al componente a copiar
                // componentes: vector de componentes de la entidad (entidad)
                auto copia = std::make_shared<T>(*componnentes);
                componentes.push_back(copia);

                return *this;
            }

            template <typename T> T* getComponente()const
            {
                static_assert(std::is_base_of<IComponentes, T>::value,"Solo derivados de IComponentes");
                for(auto& c: componentes)
                {
                    T* comp = dynamic_cast<T*>(c.get());
                    if(comp)
                        return comp;

                }
                return nullptr;
            };
            template <typename T> bool tieneComponente() const
            {
                static_assert(std::is_base_of<IComponentes, T>::value,"Solo derivados de IComponentes");
                for(auto& c: componentes)
                {
                    T* comp = dynamic_cast<T*>(c.get());
                    if(comp)
                        return true;

                }
                return false;
            };

            static int getNumObjetos()
            {
                return num_objetos;
            }
            std::shared_ptr<INombre> getNombre()
            {
                return nombre;
            }
            std::shared_ptr<ITransform>& getTransformada()
            {
                return transform;
            }
            std::shared_ptr<IStats>& getStats()
            {
                return stats;
            }
            const bool estaVivo()const
            {
                return stats->hp > 0;
            }
            void setPosicion(float x, float y);
            virtual void onUpdate(float dt) = 0;
            virtual std::string toString()
            {
                return nombre->nombre;
            }
        private:
            static int num_objetos;
        protected:
            std::shared_ptr<INombre> nombre;
            std::shared_ptr<ITransform> transform;
            std::shared_ptr<IStats> stats;
            std::vector<std::shared_ptr<IComponentes>> componentes;
    };
}
