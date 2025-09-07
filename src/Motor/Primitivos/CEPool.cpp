#include "CEPool.hpp"
#include <algorithm>
namespace CE
{
    Pool::Pool(int size)
        :max_size{size}
    {
        objetos.reserve(max_size);
    }
    void Pool::agregarPool(const std::shared_ptr<Objeto>& obj)
    {
        if(objetos.size()>max_size)
        {
            max_size+=max_size;
            objetos.reserve(max_size);
        }
        objetos.emplace_back(obj);
    }

    //lambda para borrar todo objeto marcado como muerto
    void Pool::borrarPool()
    {
        objetos.erase(
                std::remove_if(
                    objetos.begin(),
                    objetos.end(),
                    //lamda
                    [](std::shared_ptr<Objeto> &o)
                    {
                        return !o->estaVivo();
                    }
                ),objetos.end()
        );
    }
    std::shared_ptr<Objeto>& Pool::operator[](std::size_t idx)
    {
        return objetos[idx];
    }
    const std::shared_ptr<Objeto>& Pool::operator[](std::size_t idx) const
    {
        return objetos[idx];
    }
}
