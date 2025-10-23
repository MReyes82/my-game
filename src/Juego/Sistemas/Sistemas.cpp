#include "Sistemas.hpp"

#include <Motor/GUI/GPropiedades.hpp>
#include <Motor/Render/Render.hpp>
#include <set>

#include "Juego/Figuras/Figuras.hpp"
#include "Juego/objetos/Entidad.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Utils/Lerp.hpp"
#include "Motor/Utils/Utils.hpp"

namespace IVJ
{
    void SistemaControl(CE::Objeto& ente, float dt)
    {
        //no hay necesidad verificar si tiene, se asume que tiene
        //control y transformada
        // NOTE: if you need to get either component from the object
        // you have to use the base class method "get" instead of the getComponente method
        auto p = ente.getTransformada();
        auto c = ente.getComponente<CE::IControl>();
        auto s = ente.getStats();

        if(c->arr)
            p->velocidad.y=-s->maxSpeed;
        if(c->abj)
            p->velocidad.y=s->maxSpeed;
        if(c->der)
            p->velocidad.x=s->maxSpeed;
        if(c->izq)
            p->velocidad.x=-s->maxSpeed;
        if(c->run)
#if DEBUG
            p->velocidad.escala(4.0f);
#else
                p->velocidad.escala(1.5f);
#endif
        /*if (c->atacar)
            //CE::printDebug("Atacando");
        if (c->interactuar)
            //CE::printDebug("Interanctuando");*/
    }
    // for any object that belongs to a pool
    void SistemaMover(const std::vector<std::shared_ptr<CE::Objeto>>& entes, float dt)
    {
        for(auto& ente : entes)
        {
            //todo ente tiene ITransform por lo que no requiere verificación
            auto trans = ente->getTransformada();

            // Check if entity has control component (like player)
            if (ente->tieneComponente<CE::IControl>())
            {
                // For controlled entities, velocity gets reset each frame by SistemaControl
                // So we can directly scale and modify it
                trans->posicion.suma(trans->velocidad.escala(dt));
            }
            else
            {
                // For non-controlled entities (bullets, enemies, etc.)
                // Create a copy to preserve the original velocity
                CE::Vector2D scaledVelocity = trans->velocidad;
                trans->posicion.suma(scaledVelocity.escala(dt));
            }
        }
    }
    // same system as above, but when manipulating Entidad objects directly
    void SistemaMoverEntidad(const std::vector<std::shared_ptr<Entidad>>& entes, float dt)
    {
        for (auto& ente : entes)
        {
            auto trans = ente->getTransformada();

            // Check if entity has control component (like player)
            if (ente->tieneComponente<CE::IControl>())
            {
                // For controlled entities, velocity gets reset each frame by SistemaControl
                trans->posicion.suma(trans->velocidad.escala(dt));
            }
            else
            {
                // For non-controlled entities (bullets, enemies, loot items, etc.)
                // Create a copy to preserve the original velocity
                CE::Vector2D scaledVelocity = trans->velocidad;
                trans->posicion.suma(scaledVelocity.escala(dt));
            }
        }
    }

    bool SistemaColAABB(CE::Objeto& A, CE::Objeto& B, bool resolucion)
    {
        if(!A.tieneComponente<CE::IBoundingBox>() || !B.tieneComponente<CE::IBoundingBox>())
            return false;
        auto bA = A.getComponente<CE::IBoundingBox>()->tam;
        auto mA = A.getComponente<CE::IBoundingBox>()->mitad;
        auto *pa = &A.getTransformada()->posicion;
        auto prevA = A.getTransformada()->pos_prev;
        auto bB = B.getComponente<CE::IBoundingBox>()->tam;
        auto mB = B.getComponente<CE::IBoundingBox>()->mitad;
        auto pb = B.getTransformada()->posicion;
        //calculos
        bool H = pa->y-mA.y < pb.y+bB.y - mB.y && pb.y-mB.y < pa->y+bA.y - mA.y;
        bool V = pa->x-mA.x < pb.x+bB.x - mB.x && pb.x-mB.x < pa->x+bA.x - mA.x;
        bool hay_colision = H && V;
        if(hay_colision && resolucion)
            *pa = prevA;
        return hay_colision;
    }

    bool SistemaColAABBMid(CE::Objeto& A, CE::Objeto& B, bool resolucion)
    {
        if(!A.tieneComponente<CE::IBoundingBox>() || !B.tieneComponente<CE::IBoundingBox>())
            return false;
        auto midA = A.getComponente<CE::IBoundingBox>()->mitad;
        auto *pa = &A.getTransformada()->posicion;
        auto prevA = A.getTransformada()->pos_prev;
        auto midB = B.getComponente<CE::IBoundingBox>()->mitad;
        auto *pb = &B.getTransformada()->posicion;
        //calculos
        float dX = std::abs(pb->x - pa->x);
        float dY = std::abs(pb->y - pa->y);
        float sumMidX = midA.x + midB.x;
        float sumMidY = midA.y + midB.y;
        bool V = sumMidX - dX > 0;
        bool H = sumMidY - dY > 0;
        bool hay_colision = V && H;
        if(resolucion && hay_colision)
            *pa = prevA;
        return hay_colision;
    }

    //void SistemaSpawn(std::vector<std::shared_ptr<CE::Objeto>>& spawns)
    void SistemaSpawn(CE::Pool &spawns, std::vector<CE::IPaths> paths)
    {
        for (auto &s : spawns.getPool())
        {
            auto respawn_data = s->getComponente<CE::IRespawn>();
            // if nullptr
            if (!respawn_data)
                continue;
            // respawnear
            if ((respawn_data->timer_actual >= respawn_data->timer_maximo)
                && (respawn_data->num_objetos < respawn_data->MAX_ENTIDADES))
            {
                // crear objeto
                auto malillo = std::make_shared<Entidad>();
                auto compo = respawn_data->prefab->getComponente<CE::ISprite>();
                malillo->copyComponente<CE::ISprite>(compo);
                malillo->getStats()->hp = 100;
                // asumimos que spawn tiene un cuadro, tomamos el cuadro como el area
                int size_x = respawn_data->width;
                int size_y = respawn_data->height;
                float x0 = s->getTransformada()->posicion.x - size_x / 2.f;
                float y0 = s->getTransformada()->posicion.y - size_y / 2.f;

                float x = x0 + (rand() % size_x);
                float y = y0 + (rand() % size_y);
                // posicionar
                malillo->setPosicion(x, y);
                // agregar al pool
                SistemaAsignarPath(paths, malillo);

                spawns.agregarPool(malillo);
                respawn_data->timer_actual = 0;
                respawn_data->num_objetos++;
            }
            respawn_data->timer_actual++;
        }
    }

    void SistemaPaths(std::vector<std::shared_ptr<CE::Objeto>>& obj)
    {
        // Iterate over all objects
        for (auto &o : obj)
        {
            // Check if the object has a path component
            if (o->tieneComponente<CE::IPaths>())
            {
                auto path = o->getComponente<CE::IPaths>();
                // Each curve is defined by 3 points
                int num_curvas = path->puntos.size() / 3;
                // If there are no points, continue to
                if (path->puntos.size() == 0)
                    continue;

                // If the current curve index exceeds the number of curves, continue to next object
                if (path->id_curva >= num_curvas)
                    continue;

                // Calculate interpolation parameter t for the current frame
                //float t = path->frame_actual_curva / (float) path->frame_total_curva;
                float t = (path->frame_actual_curva % (path->frame_total_curva + 1)) / (float) path->frame_total_curva;

                // Get the 3 control points for the current curve
                CE::Vector2D P0 = path->puntos[0 + path->id_curva * (path->offset - 0)];
                CE::Vector2D P1 = path->puntos[1 + path->id_curva * (path->offset - 0)];
                CE::Vector2D P2 = path->puntos[2 + path->id_curva * (path->offset - 0)];

                // Interpolate position along the curve
                auto posiciones_actuales = CE::lerp2(P0, P1, P2, t);
                // Advance the frame counter for the curve
                path->frame_actual_curva++;
                // Set the object's position to the interpolated value
                o->setPosicion(posiciones_actuales.x, posiciones_actuales.y);

                // If the curve is finished (t == 1), move to the next curve
                if (t == 1.f)
                {
                    path->id_curva++;
                    //path->frame_actual_curva = 0;
                }
            }
        }
    }

    void SistemaAsignarPath(std::vector<CE::IPaths> paths, std::shared_ptr<CE::Objeto> objeto)
    /*
     * Sistema para asignacion de IPath a seguir para el objeto (entidad)
     */
    {
        int i = rand() % 3; // index for selected Ipath
        CE::printDebug("Path seleccionado: " + std::to_string(i));
        const CE::IPaths& p = paths[i]; // obtain IPath at selected index, set as const reference

        objeto->addComponente(std::make_shared<CE::IPaths>(600)); // create new IPath component for the object

        // add curves to the component IPath just created (each curve is a set of 3 points)
        for (auto j = 0; j < p.puntos.size(); j += 3)
        {
            auto IpathCurva1 = p.puntos[j];
            auto IpathCurva2 = p.puntos[j + 1];
            auto IpathCurva3 = p.puntos[j + 2];
            objeto->getComponente<CE::IPaths>()->addCurva(IpathCurva1, IpathCurva2, IpathCurva3);
        }
    }

    // system for enemies to follow the player
    // TODO: improve the following logic, currently using old version
    void SystemFollowPlayer(std::vector<std::shared_ptr<CE::Objeto>>& chasers, CE::Objeto &target, float dt)
    {
        // TODO: implement a following logic where it calculates how much it has to move based on the distance to the player
        // (calculating the vector2D)
        // only changing the velocidad->x and y.
    }
    /*constexpr float followDistance = 12.f;
    constexpr float separationDistance = 50.f;
    const float separationStrength = 1.5f;
    const float seekWeight = 1.0f;
    const float separationWeight = 1.5f;
    const float cohesionWeight = 0.0f;

    for (auto& chaser : chasers)
    {
        if (!chaser->tieneComponente<CE::IEntityType>())
            continue;

        if (chaser->getComponente<CE::IEntityType>()->type != CE::ENTITY_TYPE::ENEMY)
            continue;

        auto& chaserTransform = *chaser->getTransformada();
        auto& targetTransform = *target.getTransformada();
        CE::Vector2D directionToTarget = targetTransform.posicion - chaserTransform.posicion;
        float distanceToTarget = directionToTarget.magnitud();
        const float speed = chaser->getStats()->maxSpeed;

        CE::Vector2D seekVelocity{0.f, 0.f};
        if (distanceToTarget < followDistance)
        {
            seekVelocity = directionToTarget.normalizacion().escala(speed);
        }
        else
        {
            seekVelocity = CE::Vector2D{0.f, 0.f}; // stop moving within follow distance
        }

        CE::Vector2D separationForce{0.f, 0.f};
        for (const auto& otherChaser : chasers)
        {
            if (!otherChaser->tieneComponente<CE::IEntityType>())
                continue;

            if (otherChaser->getComponente<CE::IEntityType>()->type != CE::ENTITY_TYPE::ENEMY)
                continue;

            if (chaser != otherChaser)
            {
                auto& otherTransform = *otherChaser->getTransformada();
                CE::Vector2D diff = chaserTransform.posicion - otherTransform.posicion;
                float distance = diff.magnitud();
                if (distance < separationDistance && distance > 0) // avoid division by zero
                    separationForce += diff.normalizacion().escala((separationDistance - distance) / separationDistance);
            }
        }

        separationForce = separationForce.escala(separationStrength);
        CE::Vector2D cohesionForce{0.f, 0.f}; // not implemented yet
        CE::Vector2D combinedForce = {seekVelocity.escala(seekWeight) + separationForce.escala(separationWeight) + cohesionForce.escala(cohesionWeight)};
        CE::Vector2D finalVelocity = combinedForce.normalizacion().escala(speed);
        chaserTransform.velocidad = lerp(chaserTransform.posicion, finalVelocity, 0.5f * dt);
        // check if the enemy arrived to the player
        // cast to Entidad to access getCollidedWithAnotherEntity method
        auto& chaserCast = dynamic_cast<Entidad&>(*chaser);
        if (chaserCast.getCollidedWithAnotherEntity())
            chaserTransform.velocidad = {0.f, 0.f};
    }*/

    // System for choosing enemy type based on a rand number (0 <= NUM <= 3)
    std::string SystemChooseEnemyType(const int num)
    {
        std::string choice;

        switch (num)
        {
        case 0:
            choice = "hojaErrante";
            break;

        case 1:
            choice = "hojaBerserker";
            break;

        case 2:
            choice = "hojaChongus";
            break;

        default:
            CE::printDebug("[SISTEMAS] Error: enemy type not recognized");
            choice = "";
            break;
        }

        return choice;
    }

    // system to choose a random weapon loot item, returns the type based on the integer value generated
    CE::WEAPON_TYPE SystemChooseRandWeapon()
    {
        const int randNum = rand() % 4;
        switch (randNum)
        {
        case 0:
            return CE::WEAPON_TYPE::REVOLVER;

        case 1:
            return CE::WEAPON_TYPE::SHOTGUN;

        case 2:
            return CE::WEAPON_TYPE::SMG;

        case 3:
            return CE::WEAPON_TYPE::RIFLE;

        default:
            return CE::WEAPON_TYPE::NONE;
        }
    }

    CE::UTILITY_TYPE SystemChooseRandUtility()
    {
        const int randNum = rand() % 3;
        switch (randNum)
        {
        case 0:
            return CE::UTILITY_TYPE::BANDAGE;

        case 1:
            return CE::UTILITY_TYPE::MEDKIT;

        case 2:
            return CE::UTILITY_TYPE::ENERGY_DRINK;

        default:
            return CE::UTILITY_TYPE::NONE;
        }
    }


    // system that will choose a random position from the array passed as parameter
    // and ensures that the position hasn't been used before by checking the usedIndices set
    int SystemGetRandomPosition(std::array<CE::Vector2D, 20> positionsArr, std::set<int>& usedIndices)
    {

        // if all positions are used, clear the set to start over
        if (usedIndices.size() >= positionsArr.size())
        {
            CE::printDebug("[SYSTEM] All positions used, clearing used indices set");
            usedIndices.clear();
        }

        int randIndex;
        int attempts = 0;
        // keep generating random indices until we find one that hasn't been used
        do
        {
            randIndex = std::rand() % positionsArr.size();
            attempts++;
            if (attempts > 100)
            {
                break;
            }
        } while (usedIndices.find(randIndex) != usedIndices.end());

        // mark this index as used
        usedIndices.insert(randIndex);
        return randIndex;
    }

    // system to adjust entity stats based on enemy type
    void SystemAdjustEntityStats(std::shared_ptr<CE::Objeto> entity, const int type)
    {
        float baseSpeed = 100.f;
        const auto stats = entity->getStats();
        switch (type)
        {
        case 0: // errante
            stats->hp = 10;
            stats->hp_max = 10;
            stats->damage = 1;
            stats->maxSpeed = baseSpeed;
            break;

        case 1: // berserker
            stats->hp = 20;
            stats->hp_max = 20;
            stats->damage = 3;
            stats->maxSpeed = baseSpeed * 2.f;
            break;

        case 2: // chongus
            stats->hp = 50;
            stats->hp_max = 50;
            stats->damage = 5;
            stats->maxSpeed = baseSpeed * 0.75f;;
            break;

        default:
            CE::printDebug("[SISTEMAS] Error: enemy type not recognized");
            break;
        }
    }

    // system to get a vector of entities references of the type passed on the parameter entityType, making the cast to Entidad from Objeto
    std::vector<std::shared_ptr<Entidad>>& SystemGetEntityTypeVector(std::vector<std::shared_ptr<CE::Objeto>>& entities, CE::ENTITY_TYPE entityType)
    {
        auto result = new std::vector<std::shared_ptr<Entidad>>();
        for (auto& obj : entities)
        {
            if (!obj->tieneComponente<CE::IEntityType>())
                continue;

            if (obj->getComponente<CE::IEntityType>()->type == entityType)
            {
                auto entCast = std::dynamic_pointer_cast<Entidad>(obj);
                if (entCast)  // Check if the cast was successful
                    result->push_back(entCast);
            }
        }

        return *result;
    }

    // system to check limits of entities in the scene, if they reach the limit they will invert their velocity
    void SystemCheckLimits(std::vector<std::shared_ptr<CE::Objeto>>& entes, const unsigned int dimensionX, const unsigned int dimensionY)
    {
        for (auto& ente : entes)
        {
            if (!ente->tieneComponente<CE::IEntityType>())
                continue;
            if (ente->getComponente<CE::IEntityType>()->type != CE::ENTITY_TYPE::ENEMY)
                continue;

            const auto& tran = *ente->getTransformada();
            const CE::Vector2D pos = tran.posicion;
            CE::Vector2D vel = tran.velocidad;

            if (pos.x <= 0 || pos.x >= dimensionX)
                vel.x = -vel.x;
            if (pos.y <= 0 || pos.y >= dimensionY)
                vel.y = -vel.y;
        }
    }

    // system to update player weapon stats based on weapon type passed as parameter
    void SystemUpdatePlayerWeaponStats(const CE::WEAPON_TYPE weaponType, const std::shared_ptr<Entidad>& player)
    {
        auto stats = player->getStats();
        auto weapon = player->getComponente<CE::IWeapon>();

        switch (weaponType)
        {
        case CE::WEAPON_TYPE::NONE:
            CE::printDebug("[SISTEMAS] Warning: player has no weapon equipped");
            break;

        case CE::WEAPON_TYPE::KNIFE:
            stats->damage = 5;
            // -1 value for any of the ammo values since it does not have
            weapon->currentMagBullets = -1;
            weapon->magSize = -1;
            weapon->maxWeaponBullets = -1;
            weapon->reloadTime = -1;
            weapon->fireRate = 0.1f;
            break;

        case CE::WEAPON_TYPE::REVOLVER:
            stats->damage = 4;
            weapon->currentMagBullets = 6;
            weapon->magSize = 6;
            weapon->maxWeaponBullets = 36; // 6 mags
            weapon->reloadTime = 3.f;
            weapon->fireRate = 0.8f; // 0.8 seconds between shots
            break;

        case CE::WEAPON_TYPE::SHOTGUN:
            stats->damage = 10;
            weapon->currentMagBullets = 5;
            weapon->magSize = 5;
            weapon->maxWeaponBullets = 25; // 5 mags
            weapon->reloadTime = 5.0f;
            weapon->fireRate = 1.2f; // 1.2 seconds between
            break;

        case CE::WEAPON_TYPE::SMG:
            stats->damage = 5;
            weapon->currentMagBullets = 25;
            weapon->magSize = 25;
            weapon->maxWeaponBullets = 100; // 4 mags
            weapon->reloadTime = 2.f;
            weapon->fireRate = 0.0833f ; // 12 bullets per second
            break;

        case CE::WEAPON_TYPE::RIFLE:
            stats->damage = 8;
            weapon->currentMagBullets = 30;
            weapon->magSize = 30;
            weapon->maxWeaponBullets = 90; // 3 mags
            weapon->reloadTime = 2.5f;
            weapon->fireRate = 0.1f; // 10 bullets per second
            break;

        default:
            CE::printDebug("[SISTEMAS] Error: weapon type not recognized");
            break;
        }
    }

    // Update all loot items in the scene
    void SystemUpdateLootItems(std::vector<std::shared_ptr<Entidad>>& lootItems, std::shared_ptr<Entidad>& player,
        CE::WEAPON_TYPE& tempRefWpn, CE::UTILITY_TYPE& tempRefUtil, std::array<CE::Vector2D, 20> positionsArr,
        bool& shouldChangeWeaponFlag, bool& shouldChangeUtilFlag, float dt)
    {
        // Track used position indices to ensure no duplicates
        static std::set<int> usedPositionIndices;

        for (auto& item : lootItems)
        {
            // assume the item in the vector has IEntityType
            // check first for the weapon loot items
            if (item->getComponente<CE::IEntityType>()->type == CE::ENTITY_TYPE::LOOT_WEAPON)
            {
                if (SistemaColAABBMid(*player, *item, true))
                {
                    CE::printDebug("[SystemUpdateLootItems] Player picked up weapon loot");
                    // choose a new random weapon sprite and update the temp reference for the scene
                    const auto randWeaopn = SystemChooseRandWeapon();
                    tempRefWpn = randWeaopn;
                    // update the weapon loot position and timer (reset) for the one just picked
                    int newPosIndex = SystemGetRandomPosition(positionsArr, usedPositionIndices);
                    CE::Vector2D newPos = positionsArr[newPosIndex];
                    item->setPosicion(newPos.x, newPos.y);
                    item->getComponente<CE::ITimer>()->frame_actual = 0;
                    shouldChangeWeaponFlag = true;
                }

                // check if the timer needs reset,  for the weapon loot item if player hasn't collided (picked it up)
                else if (item->hasTimerReachedMax(item->getComponente<CE::ITimer>()))
                {
                    const auto randWeapon = SystemChooseRandWeapon();
                    tempRefWpn = randWeapon;
                    int newPosIndex = SystemGetRandomPosition(positionsArr, usedPositionIndices);
                    CE::Vector2D newPos = positionsArr[newPosIndex];
                    item->setPosicion(newPos.x, newPos.y);
                    item->getComponente<CE::ITimer>()->frame_actual = 0;
                    //shouldChangeWeaponFlag = true;
                }
                continue;
            }
            // now check for utility loot items
            if (item->getComponente<CE::IEntityType>()->type == CE::ENTITY_TYPE::LOOT_UTILITY)
            {
                if (SistemaColAABBMid(*player, *item, true))
                {
                    CE::printDebug("[SystemUpdateLootItems] Player picked up utility loot");
                    // choose a new random utility sprite and update the temp reference for the scene
                    const auto randUtility = SystemChooseRandUtility();
                    tempRefUtil = randUtility;
                    // update the utility loot position and timer (reset) for the one just picked
                    int newPosIndex = SystemGetRandomPosition(positionsArr, usedPositionIndices);
                    CE::Vector2D newPos = positionsArr[newPosIndex];
                    item->setPosicion(newPos.x, newPos.y);
                    item->getComponente<CE::ITimer>()->frame_actual = 0;
                    shouldChangeUtilFlag = true;
                }
                // check if the timer needs reset,  for the utility loot item if player hasn't collided
                else if (item->hasTimerReachedMax(item->getComponente<CE::ITimer>()))
                {
                    const auto randUtility = SystemChooseRandUtility();
                    tempRefUtil = randUtility;
                    int newPosIndex = SystemGetRandomPosition(positionsArr, usedPositionIndices);
                    CE::Vector2D newPos = positionsArr[newPosIndex];
                    item->setPosicion(newPos.x, newPos.y);
                    item->getComponente<CE::ITimer>()->frame_actual = 0;
                    //shouldChangeUtilFlag = true;
                }
                continue;
            }
        }
        // call to the movement system for the loot items so the positions are updated
        SistemaMoverEntidad(lootItems, dt);
        SystemUpdateEnergyDrink(player); // always true since the energy drink effect is handled inside the method
    }

    /*
     * system to handle the energy drink effect on the player
     * this system is implemented for the player but used the abstraction of the velocity boost
     * methods and attributes
    */
    void SystemUpdateEnergyDrink(std::shared_ptr<Entidad>& player)
    {
        if (!player->isVelocityBoostActive)
        // do nothing if the energy drink effect is not active
            return;

        auto ed_timer = player->velocityBoostTimer; // get timer for the energy drink effect
        ed_timer->frame_actual++;
        // check if the effect timer has reached max
        if (player->hasTimerReachedMax(ed_timer.get()))
        {
            // reset player speed to normal
            player->resetSpeed(2.f);
        }
    }
    // System to update the UI sprite for the loot items and change the player items when picked from loot
    void SystemChangePlayerItems(bool& shouldChangeWeaponFlag, bool& shouldChangeUtilFlag,
        std::shared_ptr<Entidad>& player, CE::WEAPON_TYPE& tempRefWpn, CE::UTILITY_TYPE& tempRefUtil, InfoUI& sceneOverlayElements)
    {
        if (shouldChangeWeaponFlag)
        {
            // update the player's component weapon type based on the new weapon type picked
            player->getComponente<CE::IWeapon>()->type = tempRefWpn;
            sceneOverlayElements.setWeapon(player->getComponente<CE::IWeapon>()->type);
            // call to the system to adjust the IStats component of the player
            SystemUpdatePlayerWeaponStats(player->getComponente<CE::IWeapon>()->type, player);
            tempRefWpn = CE::WEAPON_TYPE::NONE;
            shouldChangeWeaponFlag = false;
        }

        if (shouldChangeUtilFlag)
        {
            player->getComponente<CE::IUtility>()->type = tempRefUtil;
            // update overlay type for utility
            sceneOverlayElements.setUtility(player->getComponente<CE::IUtility>()->type);
            tempRefUtil = CE::UTILITY_TYPE::NONE;
            shouldChangeUtilFlag = false;
        }
    }

    // System to consume the utility item that the player has picked up
    void SystemConsumeUtility(std::shared_ptr<Entidad>& player, InfoUI& sceneOverlayElements)
    {
        const auto stats = player->getStats();
        const auto utility = player->getComponente<CE::IUtility>();

        switch (utility->type)
        {
            case CE::UTILITY_TYPE::MEDKIT:
                // Fully restore player health
                stats->hp = stats->hp_max;
                break;

            case CE::UTILITY_TYPE::BANDAGE:
                // Restore half of max health
                stats->hp += stats->hp_max / 2;
                // Clamp to max health
                if (stats->hp > stats->hp_max)
                    stats->hp = stats->hp_max;
                break;

            case CE::UTILITY_TYPE::ENERGY_DRINK:
                // Apply speed boost if not already active
                if (!player->isVelocityBoostActive)
                {
                    stats->maxSpeed *= 2.f;
                    player->isVelocityBoostActive = true;
                }
                break;

            default:
                // No utility to consume or NONE type
                return;
        }

        // Consume the utility - set to NONE and update UI
        utility->type = CE::UTILITY_TYPE::NONE;
        sceneOverlayElements.setUtility(CE::UTILITY_TYPE::NONE);
    }
    // System to create the loot items in the scene at the start of the main scene, called only once
    // receives the maxFrames parameter as the maximum time before the loot item needs to change it's position
    // and the maximum of loot items to create
    void SystemCreateLootItems(std::vector<std::shared_ptr<Entidad>>& lootItems, const std::array<CE::Vector2D, 20>& positionsArr, int maxFrames, const int maxLootItems)
    {
        // Track used position indices to ensure no duplicates during initial creation
        std::set<int> usedPositionIndices;

        // it is choosen that half of the loot items will be weapons and the other half utilities
        for (int i = 0 ; i < maxLootItems ; i++)
        {
            if (i % 2 == 0) // even index, create weapon loot
            {
                auto lootWeapon = std::make_shared<Entidad>();
                // set random position ensuring no duplicates
                int newPosIndex = SystemGetRandomPosition(positionsArr, usedPositionIndices);
                CE::Vector2D newPos = positionsArr[newPosIndex];
                lootWeapon->setPosicion(newPos.x, newPos.y);
                lootWeapon->getStats()->hp = 1;

                lootWeapon->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("weaponLootBoxSprite"),
                    16, 16, 1.5f));
                lootWeapon->addComponente(std::make_shared<CE::IBoundingBox>(
                CE::Vector2D{16.f * 1.2f, 16.f * 1.2f}));
                lootWeapon->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::LOOT_WEAPON));
                lootWeapon->addComponente(std::make_shared<CE::ITimer>(maxFrames));
                // add to the loot items vector
                lootItems.push_back(lootWeapon);
            }
            else // odd index, create utility loot
            {
                auto lootUtility = std::make_shared<Entidad>();
                // set random position ensuring no duplicates
                int newPosIndex = SystemGetRandomPosition(positionsArr, usedPositionIndices);
                CE::Vector2D newPos = positionsArr[newPosIndex];
                lootUtility->setPosicion(newPos.x, newPos.y);
                lootUtility->getStats()->hp = 1;

                lootUtility->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("utilityLootBoxSprite"),
                    16, 16, 1.5f));
                lootUtility->addComponente(std::make_shared<CE::IBoundingBox>(
                    CE::Vector2D{16.f * 1.2f, 16.f * 1.2f}));
                lootUtility->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::LOOT_UTILITY));
                lootUtility->addComponente(std::make_shared<CE::ITimer>(maxFrames));
                // add to the loot items vector
                lootItems.push_back(lootUtility);
            }
        }
    }

    // Helper function to calculate projectile velocity based on mouse position and projectile position
    CE::Vector2D calculateProjectileVel(const sf::Vector2i& mousePos, const CE::Vector2D& projectilePos, float speed)
    {
        // Map pixel coordinates to world coordinates using the render window AND the active camera's view
        const sf::Vector2f mousePosOnWorld = CE::Render::Get().GetVentana().mapPixelToCoords(
            mousePos,
            CE::GestorCamaras::Get().getCamaraActiva().getView()
        );
        //std::cout << "[calculateProjectileVel] Mouse pos on world: " << mousePosOnWorld.x << " " << mousePosOnWorld.y << std::endl;
        // Calculate direction vector from projectile position to mouse position
        CE::Vector2D direction = CE::Vector2D{mousePosOnWorld.x, mousePosOnWorld.y} - projectilePos;
        // Normalize the direction vector
        direction.normalizacion();
        // Scale the normalized direction by the desired speed
        const CE::Vector2D finalVelocity = direction.escala(speed);

        return finalVelocity;
    }

    // System to generate bullets when the player is attacking
    void SystemGenerateBullets(const bool isAttacking, std::shared_ptr<Entidad>& player, std::vector<std::shared_ptr<Entidad>>& bulletsShot)
    {
        // If the player is not attacking this frame, do nothing
        if (!isAttacking)
            return;

        // 1. Get the current mouse position in window coordinates
        const sf::Vector2i currentMousePosition = sf::Mouse::getPosition(CE::Render::Get().GetVentana());
        // 2. Calculate a normalized velocity vector from the player to the mouse,
        //    scaled by the desired bullet speed (400 units/sec)
        const CE::Vector2D bulletTarget = calculateProjectileVel(currentMousePosition,
                                                                   player->getTransformada()->posicion,
                                                                   400.f);
        // 3. Capture the player's current position for the bullet spawn point
        const auto pos = player->getTransformada()->posicion;
        // 4. Create a new projectile entity
        auto bullet = std::make_shared<Entidad>();
        // 5. Attach components to the bullet:
        //    a) Transform: initial position and calculated velocity
        //    b) Sprite: texture, sub‐rect, scale
        //    c) BoundingBox: half‐width/height for collisions
        //    d) Stats: holds damage and maxSpeed
        //    e) Timer: lifespan in frames (120 ≈ 2 seconds at 60 FPS)
        bullet->setPosicion(pos.x, pos.y);
        bullet->getTransformada()->velocidad = CE::Vector2D{bulletTarget};
        // assume texture is already loaded in main scene (as for every other entity with a sprite)
        bullet->addComponente(std::make_shared<CE::ISprite>(
                             CE::GestorAssets::Get().getTextura("bulletSprite"),
                             16, 16, 0.5f))
              .addComponente(std::make_shared<CE::IBoundingBox>(
                             CE::Vector2D{8.f, 8.f}))
              .addComponente(std::make_shared<CE::ITimer>(120));
        // 6. Initialize bullet-specific stats:
        //    - damage matches the player's current weapon damage
        //    - maxSpeed is a fixed projectile speed
        bullet->getStats()->hp = 1; // so the pool does not delete it immediately
        bullet->getStats()->damage = player->getStats()->damage;
        bullet->getStats()->maxSpeed = 20.f;
        // 7. Add entity type component to identify as projectile
        bullet->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::PROJECTILE));
        // 8. Store the new bullet in the active list for updates & rendering
        bulletsShot.push_back(bullet);
        //std::cout << "[SystemGenerateBullets] Bullet created at position: " << pos.x << ", " << pos.y << std::endl;
    }

    // System to add entities from a vector to the main Pool
    // This transfers ownership to share it with the Pool as well
    // TODO: add a checking mechanism to avoid adding duplicates
    void SystemAddEntitiesToPool(std::vector<std::shared_ptr<Entidad>>& entities, CE::Pool& pool)
    {
        if (entities.empty())
            return;

        //std::cout << "[SystemAddEntitiesToPool] Adding " << entities.size() << " entities to pool" << std::endl;
        for (auto& entity : entities)
        {
            // Add entity to the main pool
            pool.agregarPool(entity);
        }
    }

    // System to update bullet states each frame
    // Handles collision detection, damage application, and bullet removal
    void SystemUpdateBulletsState(std::vector<std::shared_ptr<Entidad>>& bulletsShot,
        std::vector<std::shared_ptr<Entidad>>& enemies,
        std::shared_ptr<Entidad>& player,
        CE::Pool& collisionPool,
        int& currentEnemiesInScene,
        float dt)
    {
        /*
         * Update state of all active bullets each frame:
         * 1. Advance each bullet's internal timer and components.
         * 2. Check collisions with living enemies.
         * 3. Apply damage using checkAndApplyDamage (ensures damage applied only once).
         * 4. Check collisions with static objects (walls, obstacles).
         * 5. Remove bullets that expired or collided.
         */

        // Iterate with an iterator since bullets may be erased mid-loop
        for (auto it = bulletsShot.begin(); it != bulletsShot.end(); )
        {
            auto& bullet = *it;
            bool bulletRemoved = false;

            // 1. Advance bullet's lifetime and any attached components
            bullet->onUpdate(dt);

            // Track if bullet hit anything this frame
            bool bulletHitSomething = false;

            // 2. Check collision against each enemy
            for (auto& enemy : enemies)
            {
                /*// Skip if enemy doesn't have entity type component
                if (!enemy->tieneComponente<CE::IEntityType>())
                    continue;

                // Skip non-enemy entities
                if (enemy->getComponente<CE::IEntityType>()->type != CE::ENTITY_TYPE::ENEMY)
                    continue;*/

                // Skip enemies with no HP left
                if (enemy->getStats()->hp <= 0)
                    continue;

                // If bullet overlaps an enemy's bounding box
                if (SistemaColAABBMid(*bullet, *enemy, true))
                {
                    // Mark enemy as hit so checkAndApplyDamage can process it
                    enemy->hasBeenHit = true;

                    // Apply damage using the Entidad method (handles red flash animation)
                    enemy->checkAndApplyDamage(bullet->getStats()->damage);
                    // Mark bullet as hit
                    bulletHitSomething = true;
                    // 3. If enemy HP is depleted, update score and enemy count
                    if (enemy->getStats()->hp <= 0)
                    {
                        player->getStats()->score += 1;  // award point
                        currentEnemiesInScene--;         // track remaining enemies
                        // delete object reference from the enemies vector
                        // std::cout << "[SystemUpdateBulletsState] Enemy killed! Score: "
                        //           << player->getStats()->score
                        //           << ", Remaining enemies: " << currentEnemiesInScene << std::endl;
                    }
                    // Bullet can only hit one enemy per frame, so break
                    break;
                }
            }

            // 4. Check collision with static objects (walls, obstacles) from the pool
            if (!bulletHitSomething)
            {
                for (auto& obj : collisionPool.getPool())
                {
                    // Skip if object doesn't have entity type
                    if (!obj->tieneComponente<CE::IEntityType>())
                        continue;

                    // Only check collision with static/dynamic objects (not enemies, not player, not loot)
                    auto entityType = obj->getComponente<CE::IEntityType>()->type;
                    if (entityType != CE::ENTITY_TYPE::STATIC && entityType != CE::ENTITY_TYPE::DYNAMIC)
                        continue;

                    // Check collision with obstacle
                    if (SistemaColAABBMid(*bullet, *obj, false))
                    {
                        bulletHitSomething = true;
                        //std::cout << "[SystemUpdateBulletsState] Bullet hit obstacle!" << std::endl;
                        break;
                    }
                }
            }
            // 5. Remove bullet if it expired (timer reached max) or hit something
            if (bullet->hasTimerReachedMax(bullet->getComponente<CE::ITimer>()) || bulletHitSomething)
            {
                bullet->getStats()->hp = 0;  // Mark as dead for pool cleanup
                it = bulletsShot.erase(it);  // erase returns next iterator
                bulletRemoved = true;
                // std::cout << "[SystemUpdateBulletsState] Bullet removed. Active bullets: "
                //           << bulletsShot.size() << std::endl;
            }
            // Only advance iterator if we didn't erase
            if (!bulletRemoved)
                ++it;
        }
    }

    // system that handles the player melee attack logic
    // returns true if an enemy was hit
    // the function assumes it's called after checking the player's current
    // weapon is the knife
    bool SystemPlayerMeleeAttack(std::shared_ptr<Entidad>& player,  std::shared_ptr<Entidad>& enemyToAttack)
    {
        // flag to return if the enemy passed as parameter was hit
        // so the loop on the caller scope can break
        bool didHitEnemy = false;
        if (enemyToAttack == nullptr)
            return didHitEnemy;

        const CE::Vector2D playerPos = player->getTransformada()->posicion;
        CE::Vector2D enemyPos = enemyToAttack->getTransformada()->posicion;

        const CE::Vector2D diff = enemyPos.resta(playerPos);
        const float distance = diff.magnitud();
        // if the player is close enough to attack the enemy
        if (distance <= 35.f)
        {
            player->attackWithKnife(true, enemyToAttack);
            didHitEnemy = true;
        }
        else
            didHitEnemy = false;

        return didHitEnemy;
    }
    // System that handles the player shooting logic
    // the function assumes it's called after checking the player's
    // current weapon is different from the knife
    void SystemPlayerShoot(std::shared_ptr<Entidad>& player, std::vector<std::shared_ptr<Entidad>>& bulletShotVector)
    {
        // check if the player is reloading, if so exit
        if (player->isReloading)
            return;

        auto playerCtrl = player->getComponente<CE::IControl>();
        // if player is attacking and has ammo, bullets are generated
        if (playerCtrl->atacar && !player->weaponIsEmpty())
        {
            SystemGenerateBullets(playerCtrl->atacar, player, bulletShotVector);
            // decrease the ammo count in the player's weapon component
            player->getComponente<CE::IWeapon>()->currentMagBullets--;
            player->updateReloadStatus();
        }
    }

    // Processes player shooting input, handles fire rate for ranged weapons, generates bullets, and adds them to the pool
    void SystemProcessPlayerShooting(
        std::shared_ptr<Entidad>& player,
        std::vector<std::shared_ptr<Entidad>>& bulletsShot,
        CE::Pool& pool)
    {
        // Constants for frame conversion
        constexpr int FRAMES_PER_SECOND = 60;
        // set component variables for easier access
        const auto& control = player->getComponente<CE::IControl>();
        bool isAttacking = control->atacar;
        const auto& weapon = player->getComponente<CE::IWeapon>();

        if (isAttacking && weapon->type != CE::WEAPON_TYPE::KNIFE)
        {
            player->fireRateTimer->frame_actual++;
            if (player->hasTimerReachedMax(player->fireRateTimer.get()))
            {
                SystemPlayerShoot(player, bulletsShot);
                player->resetTimer(player->fireRateTimer.get());
            }
            SystemAddEntitiesToPool(bulletsShot, pool);
        }
        else
        {
            // Reset the timer when attack button is released
            player->fireRateTimer->max_frame = static_cast<int>(weapon->fireRate * FRAMES_PER_SECOND);
        }
    }

    // Handles enemies colliding with and attacking the player
    void SystemHandleEnemyAttacks(
        std::shared_ptr<Entidad>& player,
        std::vector<std::shared_ptr<Entidad>>& enemies)
    {
        for (auto& e : enemies)
        {
            if (SistemaColAABBMid(*player, *e, true))
            {
                e->setCollidedWithAnotherEntity(true);
                if (e->tieneComponente<CE::ITimer>())
                    e->getComponente<CE::ITimer>()->max_frame = 60;

                if (e->hasTimerReachedMax(e->getComponente<CE::ITimer>()))
                {
                    player->hasBeenHit = true;
                    player->checkAndApplyDamage(e->getStats()->damage);
                    CE::Vector2D knockbackDir = player->getTransformada()->posicion;
                    knockbackDir.resta(e->getTransformada()->posicion);
                    knockbackDir.normalizacion();
                    player->applyKnockback(knockbackDir, 200.f);
                    e->resetTimer(e->getComponente<CE::ITimer>());
                }
            }
            else
            {
                e->setCollidedWithAnotherEntity(false);
                if (e->tieneComponente<CE::ITimer>())
                    e->getComponente<CE::ITimer>()->max_frame = -1;
            }
        }
    }
}
