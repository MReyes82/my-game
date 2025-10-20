#include "Sistemas.hpp"

#include <Motor/GUI/GPropiedades.hpp>
#include <set>

#include "Juego/Figuras/Figuras.hpp"
#include "Juego/objetos/Entidad.hpp"
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
        if (c->atacar)
            CE::printDebug("Atacando");
        if (c->interactuar)
            CE::printDebug("Interanctuando");

    }
    // for any object that belongs to a pool
    void SistemaMover(const std::vector<std::shared_ptr<CE::Objeto>>& entes, float dt)
    {
        for(auto& ente : entes)
        {
            //todo ente tiene ITransform por lo que no requiere verificación
            auto trans = ente->getTransformada();
            trans->posicion.suma(trans->velocidad.escala(dt));
        }
    }
    // same system as above, but when manipulating Entidad objects directly
    void SistemaMoverEntidad(const std::vector<std::shared_ptr<Entidad>>& entes, float dt)
    {
        for (auto& ente : entes)
        {
            auto trans = ente->getTransformada();
            trans->posicion.suma(trans->velocidad.escala(dt));
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
            return CE::WEAPON_TYPE::KNIFE;

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
            stats->damage = 2;
            weapon->currentMagBullets = 6;
            weapon->magSize = 6;
            weapon->maxWeaponBullets = 36; // 6 mags
            weapon->reloadTime = 0.5f;
            weapon->fireRate = 0.8f; // 0.8 seconds between shots
            break;

        case CE::WEAPON_TYPE::SHOTGUN:
            stats->damage = 10;
            weapon->currentMagBullets = 5;
            weapon->magSize = 5;
            weapon->maxWeaponBullets = 25; // 5 mags
            weapon->reloadTime = 1.0f;
            weapon->fireRate = 1.2f; // 1.2 seconds between
            break;

        case CE::WEAPON_TYPE::SMG:
            stats->damage = 3;
            weapon->currentMagBullets = 25;
            weapon->magSize = 25;
            weapon->maxWeaponBullets = 100; // 4 mags
            weapon->reloadTime = 1.5f;
            weapon->fireRate = 0.0833f ; // 12 bullets per second
            break;

        case CE::WEAPON_TYPE::RIFLE:
            stats->damage = 5;
            weapon->currentMagBullets = 30;
            weapon->magSize = 30;
            weapon->maxWeaponBullets = 90; // 3 mags
            weapon->reloadTime = 2.0f;
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
        SystemUpdateEnergyDrink(player->isVelocityBoostActive, player, dt); // always true since the energy drink effect is handled inside the method
    }

    // system to handle the energy drink effect on the player
    void SystemUpdateEnergyDrink(bool& isEnergyDrinkActive, std::shared_ptr<Entidad>& player, float dt)
    {
        if (!isEnergyDrinkActive)
        // do nothing if the energy drink effect is not active
            return;

        auto ed_timer = player->velocityBoostTimer; // get timer for the energy drink effect
        ed_timer->frame_actual += dt;
        // check if the effect timer has reached max
        if (player->hasTimerReachedMax(ed_timer.get()))
        {
            // reset player speed to normal
            player->getStats()->maxSpeed /= 1.5f; // assuming the boost was 1.5x
            isEnergyDrinkActive = false;
            ed_timer->frame_actual = 0.f; // reset timer
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
}

