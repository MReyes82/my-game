#pragma once
#include "Objetos.hpp"

namespace CE
{
    class Pool
    {  
        public:
            Pool(int size);
            void agregarPool(const std::shared_ptr<Objeto>& obj); 
            void borrarPool();
            std::shared_ptr<Objeto>& operator[](std::size_t idx);
            const std::shared_ptr<Objeto>& operator[](std::size_t idx) const;
            std::vector<std::shared_ptr<Objeto>>& getPool()
            {
                return objetos;
            }
        public:
            int max_size;
        private:
            std::vector<std::shared_ptr<Objeto>> objetos;
    };

}
