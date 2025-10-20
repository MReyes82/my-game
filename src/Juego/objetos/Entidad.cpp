#include "Entidad.hpp"
#include "../../Motor/Componentes/IComponentes.hpp"
#include <memory>

namespace IVJ
{
    void Entidad::onUpdate(float dt) 
    {
        //CE::Objeto::onUpdate(dt);

        //revisar si tiene ciertos componentes
        //para actualizarlos debidamente
        //o hacer una clase hija y ahí revisar
        
        if(tieneComponente<CE::ISprite>())
        {
            auto sprite = getComponente<CE::ISprite>();
            auto pos = getTransformada()->posicion;
            sprite->m_sprite.setPosition({pos.x,pos.y});
        }
        /*if(tieneComponente<CE::ISprite>() && tieneComponente<CE::IBoundingBox>())
        {
            auto sprite = getComponente<CE::ISprite>();
            auto bound = getComponente<CE::IBoundingBox>();
            //sprite->m_sprite.setOrigin({bound->mitad.x,bound->mitad.y});
        }*/

        // si tiene un FSM actualizar el estado
        if(tieneComponente<IVJ::IMaquinaEstado>())
        {
            auto mq = getComponente<IVJ::IMaquinaEstado>();
            if(mq->fsm)
                mq->fsm->onUpdate(*this,dt);
        }
        transform->pos_prev =  transform->posicion;

        // update timer component
        if (tieneComponente<CE::ITimer>())
        {
            const auto timer = getComponente<CE::ITimer>();
            timer->frame_actual++;
        }
    }
    void Entidad::inputFSM()
    {
        // maybe will need to remove the control component check
        // since the enemy should not have this component, but since it's an OR
        if(!getComponente<IMaquinaEstado>() || !getComponente<CE::IControl>() ||
           !getComponente<IMaquinaEstado>()->fsm)
            return;
        auto mq = getComponente<IMaquinaEstado>();
        //crear el nuevo estado , si es nulo no hay nuevo estado aún
        auto control = *getComponente<CE::IControl>();
        auto n_estado = mq->fsm->onInputs(control, *this);
        std::shared_ptr<FSM> estado(n_estado);
        if(estado)
            setFSM(estado);
    }
    
    void Entidad::setFSM(const std::shared_ptr<FSM>& mq)
    {
        auto &estado_actual = getComponente<IMaquinaEstado>()->fsm;
        //transición de salida
        estado_actual->onSalir(*this);
        //transición de entrar a otro estado
        estado_actual = mq;
        estado_actual->onEntrar(*this);
    }


    void Entidad::draw(sf::RenderTarget &target, sf::RenderStates state) const
    {
        state.transform *= getTransform();
        #if DEBUG
        if(tieneComponente<CE::IBoundingBox>())
        {
            auto bound = getComponente<CE::IBoundingBox>();
            sf::RectangleShape box{{bound->tam.x,bound->tam.y}};
            box.setFillColor(sf::Color::Transparent);
            box.setOutlineColor(sf::Color::Red);
            box.setOutlineThickness(1.5f);
            box.setOrigin({bound->mitad.x,bound->mitad.y});
            auto pos = transform->posicion;
            box.setPosition({pos.x,pos.y});
            target.draw(box);
        }
        #endif
        //revisar si tiene ciertos componentes
        //para renderizarlos debidamente
        //o hacer una clase hija y ahí revisar
        if(tieneComponente<CE::ISprite>())
        {
            auto sprite = getComponente<CE::ISprite>();
            //SHADER  NO ME GUSTA ES LENTO
            if(tieneComponente<CE::IShader>())
            {
                target.draw(sprite->m_sprite,&getComponente<CE::IShader>()->m_shader);
            }
            else
                target.draw(sprite->m_sprite);
        }
    }

    bool Entidad::checkPlayerFacingRight(const sf::RenderWindow &window)
    {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        auto windowWidthHalf = window.getSize().x / 2;

        return mousePos.x >= windowWidthHalf;
    }

    // check if the timer has reached its max value (NO RESET)
    bool Entidad::hasTimerReachedMax(CE::ITimer* timer) const
    {
        bool ans = false;
        if (timer == nullptr)
            return ans;

        if (timer->max_frame == -1)
            return ans;

        if (timer->frame_actual >= timer->max_frame)
        {
            ans = true;
        }

        return ans;
    }

    void Entidad::resetTimer(CE::ITimer* timer)
    {
        if (timer == nullptr)
            return;

        timer->frame_actual = 0;
    }

    // apply damage if the entity has been hit NOTE: already tested and working
    void Entidad::checkAndApplyDamage(std::uint8_t damage)
    {
        // assume it has IStats component, since every object has it (declared in constructor)
        if (Entidad::hasBeenHit)
        {
            getStats()->hp -= damage;
            /*
            * Now apply red flash animation to sprite
            * Is set here to begin the flash animation
            */
            auto eSprite = getComponente<CE::ISprite>();
            eSprite->m_sprite.setColor(sf::Color::Red);
            Entidad::isDamageAnimationActive = true;
        }
    }

    void Entidad::applyKnockback(CE::Vector2D direction, float force)
    {
        // assume it has ITransform component, since every object has it (declared in constructor)
        auto eTransform = getTransformada();
        CE::Vector2D normalizedDir = direction.normalizacion();
        direction = normalizedDir;
        eTransform->posicion += direction.escala(force);
    }

    // check current state of damage animation NOTE: already tested and working
    void Entidad::checkDamageAnimation()
    {
        if (isDamageAnimationActive)
        {
            damageTimer->frame_actual++;
            if (hasTimerReachedMax(damageTimer.get()))
            {
                const auto eSprite = getComponente<CE::ISprite>();
                eSprite->m_sprite.setColor(sf::Color::White);
                isDamageAnimationActive = false;
                resetTimer(damageTimer.get());
            }
        }
    }

}
