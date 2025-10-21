#include "EscenaMain.hpp"

#include <algorithm>
#include <Juego/Maquinas/GameStates/EnemyStates.hpp>
#include <Juego/Maquinas/GameStates/PlayerStates.hpp>

#include "Juego/Maquinas/JugadorFSM/IdleFSM.hpp"
#include "Juego/objetos/Entidad.hpp"
#include "Juego/Sistemas/Sistemas.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Primitivos/GestorEscenas.hpp"
#include "Motor/Render/Render.hpp"
#include "Overlay/Overlay.hpp"

#define SECONDS_ 60
#define MINUTES_ 3600
#define PLAYER_BOUNDINGBOX_FACTOR 0.75f
#define MAX_PLAYER_HP 10

namespace IVJ
{
    void EscenaMain::initPlayerPointer()
    {
        CE::GestorAssets::Get().agregarTextura("hojaPlayer", ASSETS "/sprites/player/player_sprite.png",
            CE::Vector2D{0, 0}, CE::Vector2D{128, 640});
        player = std::make_shared<Entidad>(); // note: now the constructor auto-asigns a transform and stats component
        player->setPosicion(540.f, 360.f);
        player->getStats()->hp = MAX_PLAYER_HP;
        player->getStats()->hp_max = MAX_PLAYER_HP;
        player->getStats()->score = 0;
        player->getStats()->damage = 1; // knife damage
        player->getStats()->maxSpeed = 165.f;
        player->damageTimer = std::make_shared<CE::ITimer>(30); // 15 frames of red flash on damage
        player->velocityBoostTimer = std::make_shared<CE::ITimer>(5 * SECONDS_); // 5 seconds of speed boost
        player->reloadTimer = std::make_shared<CE::ITimer>(0); // max will be set when weapon is equipped
        player->fireRateTimer = std::make_shared<CE::ITimer>(0); // max will be set when weapon is equipped

        //! NOTE: YOU HAVE TO ADD THE COMPONENTS IN THIS ORDER, OR ELSE THE COLLISION WON'T WORK PROPERLY
        player->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("hojaPlayer"),
                    32, 32, 1.0f))
                .addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{32.f * PLAYER_BOUNDINGBOX_FACTOR, 32.f * PLAYER_BOUNDINGBOX_FACTOR}))
                .addComponente(std::make_shared<IVJ::IMaquinaEstado>())
                .addComponente(std::make_shared<CE::IControl>());

        auto &fsm_init = player->getComponente<IMaquinaEstado>()->fsm;
        player->setIsEntityFacingRight(player->checkPlayerFacingRight(CE::Render::Get().GetVentana()));
        fsm_init  = std::make_shared<IdleState>(player->getIsEntityFacingRight());
        fsm_init->onEntrar(*player);

        player->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::PLAYER));
        player->addComponente(std::make_shared<CE::ITimer>(180));
        player->addComponente(std::make_shared<CE::IWeapon>(CE::WEAPON_TYPE::KNIFE));
        player->addComponente(std::make_shared<CE::IUtility>(CE::UTILITY_TYPE::NONE));

        objetos.agregarPool(player);
    }

    void EscenaMain::registerButtons()
    {
        registrarBotones(sf::Keyboard::Scancode::W, "arriba");
        registrarBotones(sf::Keyboard::Scancode::Up, "arriba");
        registrarBotones(sf::Keyboard::Scancode::S, "abajo");
        registrarBotones(sf::Keyboard::Scancode::Down, "abajo");
        registrarBotones(sf::Keyboard::Scancode::A, "izquierda");
        registrarBotones(sf::Keyboard::Scancode::Left, "izquierda");
        registrarBotones(sf::Keyboard::Scancode::D, "derecha");
        registrarBotones(sf::Keyboard::Scancode::Right, "derecha");
        registrarBotones(sf::Keyboard::Scancode::Escape, "pausa");
        registrarBotones(sf::Keyboard::Scancode::LShift, "correr");
        registrarBotonesMouse(sf::Mouse::Button::Left, "atacar");
        registrarBotonesMouse(sf::Mouse::Button::Right, "interactuar");
        registrarBotones(sf::Keyboard::Scancode::Escape, "pausa");
    }

    void EscenaMain::summonEnemies(const int maxEnemies)
    {
        // Track used position indices to ensure no duplicates during enemy spawning
        std::set<int> usedPositionIndices;

        for (int i = 0 ; i < maxEnemies ; i++)
        {
            auto enemy = std::make_shared<Entidad>();
            //enemy->setType(Entidad::ENTITY_TYPE::ENEMY);
            const int randNum = rand() % 3;
            std::string enemyType = SystemChooseEnemyType(randNum);
            //CE::printDebug("Enemy type chosen: " + enemyType);
            constexpr float enemyWidth = 32.f;
            constexpr float enemyHeight = 32.f;
            constexpr float enemyScale = 1.f;
            // Use SystemGetRandomPosition to ensure no duplicate spawn positions
            const int randomSpawnIndex = SystemGetRandomPosition(spawnPositions, usedPositionIndices);
            enemy->setPosicion(spawnPositions[randomSpawnIndex].x, spawnPositions[randomSpawnIndex].y);
            SystemAdjustEntityStats(enemy, randNum);

            enemy->addComponente(std::make_shared<CE::ISprite>(
               CE::GestorAssets::Get().getTextura(enemyType), enemyWidth, enemyHeight, enemyScale))
            .addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{(enemyWidth - 8.f) * enemyScale, (enemyHeight - 8.f) * enemyScale}))
            .addComponente(std::make_shared<IVJ::IMaquinaEstado>())
            .addComponente(std::make_shared<CE::IControl>())
            .addComponente(std::make_shared<CE::INombre>("enemy" + std::to_string(i)))
            ;

            auto& fsm_init = enemy->getComponente<IMaquinaEstado>()->fsm;
            fsm_init = std::make_shared<IdleEnemyState>(true);
            fsm_init->onEntrar(*enemy);

            enemy->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::ENEMY));
            enemy->damageTimer = std::make_shared<CE::ITimer>(30); // 15 frames of red flash on damage

            objetos.agregarPool(enemy);
        }
    }

    void EscenaMain::checkRoundEnd()
    {
        if (currentEnemiesInScene == 0)
        {
            currentRound++;
            MAX_ROUND_ENEMIES++;
            currentEnemiesInScene = MAX_ROUND_ENEMIES;
            summonEnemies(currentEnemiesInScene);
            CE::printDebug("New round: " + std::to_string(currentRound));
            shouldShowNewRoundText = true; // todo: implement a UIInfo class to show
            // update player reference to the end of the pool
            //movePlayerPointer();
        }
    }

    // function to move the reference of the player object to the last position of the
    // Pool, this is the only way to fix the collision-resolving issue
    void EscenaMain::movePlayerPointer()
    {
        if (objetos.getPool().empty())
        {
            //
            return;
        }
        auto& pool = objetos.getPool();
        // search for the player object in the pool
        auto it = std::find_if(pool.begin(), pool.end(), // if found, it will be an iterator to the player object
            [&](const std::shared_ptr<CE::Objeto>& obj) { // lambda to compare shared_ptr contents
                return obj == player; // if the object is the player, return true
            });

        // if found, make and in place rotatin to move it to the end of the pool
        if (it != pool.end())
        {
            std::rotate(it, it + 1, pool.end());
        }
    }
    /*
     * Method that adds all the UI assets to the AssetManager
     * to be used by the overlay class
     */
    void EscenaMain::loadUIAssets()
    {
        CE::GestorAssets::Get().agregarTextura("crosshair", ASSETS "/sprites/items/assets UI/UI/Cursor_crosshair02.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{15, 17});
        CE::GestorAssets::Get().agregarTextura("heartSprite", ASSETS "/sprites/items/assets UI/UI/HUD_health01.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{8, 8});
        CE::GestorAssets::Get().agregarTextura("weaponIconsSprite", ASSETS "/sprites/items/weapons/weapon_icons.png",
                                                CE::Vector2D{0, 0}, CE::Vector2D{17.f, 44.f});
        CE::GestorAssets::Get().agregarTextura("ammoIconSprite", ASSETS "/sprites/items/assets UI/UI/HUD_ammo01.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{8.f, 8.f});
        CE::GestorAssets::Get().agregarTextura("utilityIconSprite", ASSETS "/sprites/items/utility_icons.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{14.f, 31.f});
        CE::GestorAssets::Get().agregarTextura("weaponCageSprite", ASSETS "/sprites/items/assets UI/UI/empty_box.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{18.f, 18.f});
        CE::GestorAssets::Get().agregarTextura("utilityCageSprite", ASSETS "/sprites/items/assets UI/UI/empty_box.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{18.f, 18.f});
        CE::GestorAssets::Get().agregarTextura("weaponLootBoxSprite", ASSETS "/sprites/items/PowerUps2.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{16.f, 16.f});
        CE::GestorAssets::Get().agregarTextura("utilityLootBoxSprite", ASSETS "/sprites/items/PowerUps1.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{16.f, 16.f});
        CE::GestorAssets::Get().agregarTextura("bulletSprite", ASSETS "/sprites/items/Bullet1.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{16.f, 16.f});

        // add here the font to the asset manager, however, this is only used for the menu and other scenes.
        // the overlay texts that uses this font load it directly (not from the asset manager)
        CE::GestorAssets::Get().agregarFont("NotJamSlab14",ASSETS "/fonts/NotJamSlab14.ttf");
    }

    // update the InfoUI object attributes based on current game state
    void EscenaMain::updatePlayerUI()
    {
        UIsceneOverlayElements.setHealth(player->getStats()->hp);
        UIsceneOverlayElements.setScore(player->getStats()->score);
        UIsceneOverlayElements.setCurrentAmmo(player->getComponente<CE::IWeapon>()->currentMagBullets);
        UIsceneOverlayElements.setMaxAmmo(player->getComponente<CE::IWeapon>()->maxWeaponBullets);
        UIsceneOverlayElements.setWeapon(player->getComponente<CE::IWeapon>()->type);
        SystemChangePlayerItems(player->shouldChangeWeapon, player->shouldChangeUtility,
                                player,
                                newWeaponType,
                                newUtilityType,
                                UIsceneOverlayElements);
    }

    void EscenaMain::onInit()
    {
        if (!newInstance)
        {
            CE::GestorCamaras::Get().setCamaraActiva(1);
            return;
        }

        loadUIAssets();
        // load background tilemap
        // need to call to the second method that has the pool reference in order to load the collision boxes
        if (!background[0].loadTileMap(ASSETS "/mapas/background_city.txt", objetos))
            exit(EXIT_FAILURE);

        // register all usable buttons
        registerButtons();
        CE::GestorAssets::Get().agregarTextura("hojaErrante", ASSETS "/sprites/enemies/errante_sprite.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{128, 96});
        CE::GestorAssets::Get().agregarTextura("hojaBerserker", ASSETS "/sprites/enemies/berserker_sprite.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{128, 96});
        CE::GestorAssets::Get().agregarTextura("hojaChongus", ASSETS "/sprites/enemies/chongus_sprite.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{128, 96});
        SystemCreateLootItems(lootItems, lootPositions, 60 * SECONDS_, 10); // create 10 loot items with 10 seconds timer
        SystemAddEntitiesToPool(lootItems, objetos);
        initPlayerPointer();

        CE::GestorCamaras::Get().agregarCamara(std::make_shared<CE::CamaraSmoothFollow>(
            CE::Vector2D{540.f, 360.f}, CE::Vector2D{1900.f, 1020.f}));
        CE::GestorCamaras::Get().setCamaraActiva(1);
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(player);

        UIsceneOverlayElements = InfoUI();
        // Initialize UI elements with player's actual weapon type before creating overlay
        //UIsceneOverlayElements.setWeapon(player->getComponente<CE::IWeapon>()->type);
        sceneOverlay = std::make_shared<OverlayMain>(UIsceneOverlayElements, player);
        newRoundTextTimer.max_frame = 5 * SECONDS_;

        newInstance = false;
        gameState = true;
    }

    void EscenaMain::onFinal()
    {}

    void EscenaMain::onUpdate(float dt)
    {
        // First check if the player is alive
        if (!player->estaVivo())
        {
            // Navigate to game over scene
            // TODO: create game over scene and pass score as parameter
            CE::GestorEscenas::Get().cambiarEscena("Menu");
            return;
        }

        if (shouldShowNewRoundText)
        {
            newRoundTextTimer.frame_actual++;
        }

        // Player damage animation is already checked in entity onUpdate method
        // Get control component
        const auto& control = player->getComponente<CE::IControl>();
        bool isAttacking = control->atacar;
        const auto& weapon = player->getComponente<CE::IWeapon>();
        // Handle fire rate for shooting weapons
        if (isAttacking)
        {
            player->fireRateTimer->frame_actual++;

            if (player->hasTimerReachedMax(player->fireRateTimer.get()))
            {
                std::shared_ptr<Entidad> noEnemy = nullptr; // Can't pass nullptr directly to non-const reference
                SystemplayerAttack(true, player, bulletsShot, noEnemy);
                player->resetTimer(player->fireRateTimer.get());
            }
        }
        else
        {
            // Reset the timer when attack button is releasedsd
            player->fireRateTimer->max_frame = static_cast<int>(weapon->fireRate * SECONDS_);
        }
        SystemAddEntitiesToPool(bulletsShot, objetos);

        SistemaControl(*player, dt);
        SistemaMover(objetos.getPool(), dt);
        auto enemies = SystemGetEntityTypeVector(objetos.getPool(), CE::ENTITY_TYPE::ENEMY);

        SystemUpdateBulletsState(bulletsShot, enemies, player, objetos, currentEnemiesInScene, dt);
        SystemCheckLimits(objetos.getPool(), 3840.f, 3840.f);

        checkRoundEnd();
        movePlayerPointer();
        SystemUpdateLootItems(lootItems, player,
            newWeaponType, newUtilityType,
            lootPositions, player->shouldChangeWeapon,
            player->shouldChangeUtility, dt);

        updatePlayerUI();
        sceneOverlay->Update(CE::Render::Get(), UIsceneOverlayElements);

        player->setIsEntityFacingRight(player->checkPlayerFacingRight(CE::Render::Get().GetVentana()));
        player->inputFSM();
        player->handleReload();

        for (auto& obj : objetos.getPool())
        {
            obj->onUpdate(dt);
            if (obj != player)
            {
                SistemaColAABBMid(*player, *obj, true);
            }
        }

        for (auto& e : enemies)
        {
            if (SistemaColAABBMid(*player, *e, true))
            {
                // Enemy collides with player
                e->setCollidedWithAnotherEntity(true);

                // Player attacks enemy with knife if attacking
                SystemplayerAttack(control->atacar, player, bulletsShot, e);

                // Set enemy attack timer
                if (e->tieneComponente<CE::ITimer>())
                {
                    e->getComponente<CE::ITimer>()->max_frame = 60; // frames of attack animation
                }

                // Check if enemy finished attack animation
                if (e->hasTimerReachedMax(e->getComponente<CE::ITimer>()))
                {
                    e->setCollidedWithAnotherEntity(false);

                    // Mark player as hit and apply damage
                    player->hasBeenHit = true;
                    player->checkAndApplyDamage(e->getStats()->damage);

                    /*// Apply knockback to player
                    CE::Vector2D knockbackDir = player->getTransformada()->posicion;
                    knockbackDir.resta(e->getTransformada()->posicion);
                    knockbackDir.normalizacion();
                    player->applyKnockback(knockbackDir, 200.f);*/

                    // Reset enemy timer
                    e->resetTimer(e->getComponente<CE::ITimer>());
                }
            }
            else
            {
                e->setCollidedWithAnotherEntity(false);
                if (e->tieneComponente<CE::ITimer>())
                {
                    e->getComponente<CE::ITimer>()->max_frame = -1;
                }
            }
        }

        // Clean up dead objects from pool
        objetos.borrarPool();
    }

    void EscenaMain::onInputs(const CE::Botones &accion)
    {
        auto playerControl = player->getComponente<CE::IControl>();

        if (accion.getTipo() == CE::Botones::TipoAccion::OnPress)
        {
            if (accion.getNombre() == "correr")
            {
                playerControl->run = true;
            }

            if (accion.getNombre() == "arriba")
            {
                playerControl->arr = true;
            }

            else if (accion.getNombre() == "abajo")
            {
                playerControl->abj = true;
            }

            else if (accion.getNombre() == "derecha")
            {
                playerControl->der = true;
            }

            else if (accion.getNombre() == "izquierda")
            {
                playerControl->izq = true;
            }
            else if (accion.getNombre() == "pausa")
            {
                // detect if game is paused or not
                CE::GestorEscenas::Get().cambiarEscena("Menu");
                gameState = !gameState;
                if (gameState)
                {
                    CE::printDebug("Game paused");
                }
            }
        }

        else if (accion.getTipo() == CE::Botones::TipoAccion::OnRelease)
        {
            if (accion.getNombre() == "correr")
            {
                playerControl->run = false;
            }

            if (accion.getNombre() == "arriba")
            {
                playerControl->arr = false;
            }

            else if (accion.getNombre() == "abajo")
            {
                playerControl->abj = false;
            }

            else if (accion.getNombre() == "derecha")
            {
                playerControl->der = false;
            }

            else if (accion.getNombre() == "izquierda")
            {
                playerControl->izq = false;
            }
            else if (accion.getNombre() == "pausa")
            {
                // do nothing on release
            }
        }
    }

    void EscenaMain::onInputs(const CE::MouseButton &accion)
    {
        auto playerControl = player->getComponente<CE::IControl>();
        auto isPlayerReloading = player->isReloading;

        if (accion.getTipo() == CE::MouseButton::TipoAccion::OnPress)
        {
            if (accion.getNombre() == "atacar" && !isPlayerReloading)
            {
                playerControl->atacar = true;
            }
            else if (accion.getNombre() == "interactuar")
            {
                playerControl->interactuar = true;
                // Consume utility item when K is pressed
                SystemConsumeUtility(player, UIsceneOverlayElements);
            }
        }
        else if (accion.getTipo() == CE::MouseButton::TipoAccion::OnRelease)
        {
            if (accion.getNombre() == "atacar")
            {
                playerControl->atacar = false;
                //CE::printDebug("player stop attacking");
            }
            else if (accion.getNombre() == "interactuar")
            {
                playerControl->interactuar = false;
                //CE::printDebug("player stop interacting");
            }
        }
    }

    void EscenaMain::onRender()
    {
        CE::Render::Get().GetVentana().setMouseCursorVisible(false);
        for (auto& b : background)
            CE::Render::Get().AddToDraw(b);

        for (auto& obj : objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*obj);
        }
        // draw overlay at the end
        sceneOverlay->draw(CE::Render::Get());

        if (newRoundTextTimer.frame_actual <= newRoundTextTimer.max_frame && shouldShowNewRoundText)
        {
            //CE::printDebug("DRAW ROUND TEXT");
            CE::Render::Get().AddToDraw(sceneOverlay->getRoundText());
        }
        else
        {
            shouldShowNewRoundText = false;
            newRoundTextTimer.frame_actual = 0;
        }
        if (player->isReloading)
        {
            //CE::printDebug("DRAW RELOADING TEXT");
            CE::Render::Get().AddToDraw(sceneOverlay->getReloadingText());
        }
    }
} // IVJ
