#include "EscenaMain.hpp"

#include <algorithm>
#include <Juego/Maquinas/GameStates/EnemyStates.hpp>
#include <Juego/Maquinas/GameStates/PlayerStates.hpp>
#include "Juego/objetos/Entidad.hpp"
#include "Juego/Sistemas/Sistemas.hpp"
#include "Juego/Sistemas/Quest/QuestSystems.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Primitivos/GestorEscenas.hpp"
#include "Motor/Render/Render.hpp"
#include "Overlay/Overlay.hpp"
#include "Juego/Sistemas/Boss/SistemasBosses.hpp"

#define SECONDS_ 60
#define MINUTES_ 3600
#define PLAYER_BOUNDINGBOX_FACTOR 0.75f
#define MAX_PLAYER_HP 10

namespace IVJ
{
    std::shared_ptr<Entidad> boss; // global variable for testing boss systems

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

        // Add IRayo component for NPC interaction detection
        player->addComponente(std::make_shared<IRayo>(
            player->getTransformada()->posicion,
            player->getTransformada()->velocidad.x,
            100.f
        ));

        player->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::PLAYER));
        player->addComponente(std::make_shared<CE::ITimer>(180));
        player->addComponente(std::make_shared<CE::IWeapon>(CE::WEAPON_TYPE::KNIFE));
        player->addComponente(std::make_shared<CE::IUtility>(CE::UTILITY_TYPE::NONE));

        // Add quest component for quest tracking
        player->addComponente(std::make_shared<IVJ::IQuest>());

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
        registrarBotones(sf::Keyboard::Scancode::P, "pauseboss");
        registrarBotones(sf::Keyboard::Scancode::F, "npcinteract"); // For NPC interaction
#if DEBUG
        registrarBotones(sf::Keyboard::Scancode::R, "debuground"); // Debug: increment round
#endif
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
            enemy->addComponente(std::make_shared<CE::ITimer>(SECONDS_ * 1)); // default timer, used for the attacking animation timing
            enemy->damageTimer = std::make_shared<CE::ITimer>(30); // 30 frames of red flash on damage

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
            shouldShowNewRoundText = true;
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
        CE::GestorAssets::Get().agregarTextura("iconoGrande", ASSETS "/overlays/icon_lg.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{160, 150});
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
                                               CE::Vector2D{0,0}, CE::Vector2D{16,16});
        CE::GestorAssets::Get().agregarTextura("bulletSprite", ASSETS "/sprites/items/Bullet1.png",
                                               CE::Vector2D{0,0}, CE::Vector2D{16,16});
        CE::GestorAssets::Get().agregarTextura("navigation_e", ASSETS "/sprites/items/assets UI/Icons/navigation_e.png",
                                               CE::Vector2D{0,0}, CE::Vector2D{32,32});

        CE::GestorAssets::Get().agregarTextura("MirageSprite", ASSETS "/sprites/bosses/mirageSprite.png",
                                                      CE::Vector2D{0, 0}, CE::Vector2D{264.f, 192.f});

        // add here the font to the asset manager, however, this is only used for the menu and other scenes.
        // the overlay texts that uses this font load it directly (not from the asset manager)
        CE::GestorAssets::Get().agregarFont("NotJamSlab14",ASSETS "/fonts/NotJamSlab14.ttf");
        CE::GestorAssets::Get().agregarFont("PressStart", ASSETS "/fonts/PressStart2P-Regular.ttf");

        // Load quest dialogues
        CE::GestorAssets::Get().cargarDialogues(ASSETS "/dialogues/quest_dialogues.csv");
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

        // Create quest NPC
        CE::GestorAssets::Get().agregarTextura("hoja_yellow", ASSETS "/sprites/aliens/alienYellow.png",
                                         CE::Vector2D{0, 0}, CE::Vector2D{256, 512});
        auto npc = std::make_shared<Entidad>();
        npc->getStats()->hp = 100;
        // Position NPC far away from playable area until round 3
        npc->setPosicion(10000.f, 10000.f);
        npc->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("hoja_yellow"),
            68, 85, 0.5f)); // Scale 0.5f for proper size with 0.3f zoom camera
        npc->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{68 * 0.5f, 85 * 0.5f})); // Match bounding box to scaled size
        npc->addComponente(std::make_shared<CE::IControl>());
        npc->addComponente(std::make_shared<IMaquinaEstado>());
        npc->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::NPC)); // Mark as NPC type
        // Add dialogue component with ID 1 as start, and 13 total dialogues (all quest dialogues)
        npc->addComponente(std::make_shared<IVJ::IDialogo>(1, 13));
        objetos.agregarPool(npc);

        // Create Signal Jammer entities for quest phases
        // Using weapon loot box sprite as placeholder for jammers
        // Spread them across different areas of the map to make navigation arrow useful

        // Phase 1 Jammer: Trap Lesson - Northeast area (positioned far away until round 3)
        auto jammer1 = std::make_shared<Entidad>();
        jammer1->setPosicion(10000.f, 10000.f); // Far away until round 3
        jammer1->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("weaponLootBoxSprite"),
            16, 16, 2.0f)); // Larger scale to make it visible
        jammer1->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{32.f, 32.f}));
        jammer1->addComponente(std::make_shared<CE::IControl>());
        jammer1->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::SIGNAL_JAMMER));
        jammer1->addComponente(std::make_shared<IVJ::ISignalJammer>(1)); // Phase 1
        // Initialize stabilization timer (6 seconds at 60 FPS = 360 frames)
        auto jammer1Comp = jammer1->getComponente<IVJ::ISignalJammer>();
        jammer1Comp->stabilizationTimer = std::make_shared<CE::ITimer>(360);
        jammer1->getStats()->hp = 100; // Keep alive
        objetos.agregarPool(jammer1);

        // Phase 2 Jammer: Ranged Attack Lesson - Southwest area (positioned far away until round 3)
        auto jammer2 = std::make_shared<Entidad>();
        jammer2->setPosicion(10000.f, 10000.f); // Far away until round 3
        jammer2->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("weaponLootBoxSprite"),
            16, 16, 2.0f));
        jammer2->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{32.f, 32.f}));
        jammer2->addComponente(std::make_shared<CE::IControl>());
        jammer2->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::SIGNAL_JAMMER));
        jammer2->addComponente(std::make_shared<IVJ::ISignalJammer>(2)); // Phase 2
        // Initialize Phase 2 burst shooting timers
        auto jammer2Comp = jammer2->getComponente<IVJ::ISignalJammer>();
        jammer2Comp->rangedAttackTimer = std::make_shared<CE::ITimer>(180); // 3 seconds at 60 FPS
        jammer2Comp->projectileBurstTimer = std::make_shared<CE::ITimer>(30); // 30 frames between projectiles
        jammer2Comp->stabilizationTimer = std::make_shared<CE::ITimer>(360); // 6 seconds for stabilization
        jammer2->getStats()->hp = 100;
        objetos.agregarPool(jammer2);

        // Phase 3 Jammer: Teleport Lesson - Southeast corner (positioned far away until round 3)
        auto jammer3 = std::make_shared<Entidad>();
        jammer3->setPosicion(10000.f, 10000.f); // Far away until round 3
        jammer3->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("weaponLootBoxSprite"),
            16, 16, 2.0f));
        jammer3->addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{32.f, 32.f}));
        jammer3->addComponente(std::make_shared<CE::IControl>());
        jammer3->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::SIGNAL_JAMMER));
        jammer3->addComponente(std::make_shared<IVJ::ISignalJammer>(3)); // Phase 3
        // Initialize Phase 3 teleport timer (starts at 2 seconds, can change to 5 seconds)
        auto jammer3Comp = jammer3->getComponente<IVJ::ISignalJammer>();
        jammer3Comp->teleportTimer = std::make_shared<CE::ITimer>(120); // 2 seconds at 60 FPS
        jammer3Comp->stabilizationTimer = std::make_shared<CE::ITimer>(360); // 6 seconds for stabilization
        jammer3->getStats()->hp = 100;
        objetos.agregarPool(jammer3);

        // Initialize navigation arrow for quest guidance
        SysInitNavigationArrow(player, objetos);

        CE::GestorCamaras::Get().agregarCamara(std::make_shared<CE::CamaraSmoothFollow>(
            CE::Vector2D{540.f, 360.f}, CE::Vector2D{1900.f, 1020.f}));
        CE::GestorCamaras::Get().setCamaraActiva(1);
        CE::GestorCamaras::Get().getCamaraActiva().lockEnObjeto(player);

        UIsceneOverlayElements = InfoUI();
        // Initialize UI elements with player's actual weapon type before creating overlay
        //UIsceneOverlayElements.setWeapon(player->getComponente<CE::IWeapon>()->type);
        sceneOverlay = std::make_shared<OverlayMain>(UIsceneOverlayElements, player);
        newRoundTextTimer.max_frame = 5 * SECONDS_;

        boss = std::make_shared<Entidad>();
        MirageInit(boss, spawnPositions);
        // Position boss far away from playable area until quest is complete
        boss->setPosicion(10000.f, 10000.f);
        boss->getComponente<IBossBhvrMirage>()->hpText->setPosicion(10000.f, 10000.f - 50.f);
        objetos.agregarPool(boss);
        objetos.agregarPool(boss->getComponente<IBossBhvrMirage>()->hpText);

        newInstance = false;
        gameState = true;
    }

    void EscenaMain::onFinal()
    {}

    void EscenaMain::processPlayerCollisions(float dt)
    {
        for (auto& obj : objetos.getPool())
        {
            obj->onUpdate(dt);
            auto entityTypeComp = obj->getComponente<CE::IEntityType>();
            if (!entityTypeComp)
                continue;
            if (obj != player)
            {
                SistemaColAABBMid(*player, *obj, true);
            }
        }
    }

    void EscenaMain::processPlayerKnifeAttack(std::vector<std::shared_ptr<Entidad>> enemies)
    {
        const bool isPlayerAttacking = player->getComponente<CE::IControl>()->atacar;
        if (player->getComponente<CE::IWeapon>()->type == CE::WEAPON_TYPE::KNIFE
            && isPlayerAttacking)
        {
            // start loop to check knife collision with enemies
            for (auto& e : enemies)
                // check if the player already landed one hit, so we don't hit multiple enemies in one attack
                if (SystemPlayerMeleeAttack(player, e))
                {
                    // if the player killed the enemy after the attack, update the score and enemy count
                    if (!e->estaVivo())
                    {
                        currentEnemiesInScene--;
                        player->getStats()->score += 1; // add score for enemy killed
                    }
                    break;
                }

        }
    }

    void EscenaMain::checkAndRefreshFSMOnWeaponChange(CE::WEAPON_TYPE weaponBeforeUpdate)
    {
        // Check if weapon changed after UI update, and force FSM refresh if it did
        auto weaponAfterUpdate = player->getComponente<CE::IWeapon>()->type;
        if (weaponBeforeUpdate != weaponAfterUpdate)
        {
            // Force FSM state re-entry to refresh sprite animation frames with new weapon
            auto currentFSM = player->getComponente<IMaquinaEstado>()->fsm;
            if (currentFSM)
            {
                currentFSM->onSalir(*player);
                currentFSM->onEntrar(*player);
            }
        }
    }

    void EscenaMain::stopEnemyMovement(std::vector<std::shared_ptr<Entidad>>& enemies)
    {
        // Set all enemy velocities to 0 when dialogue is active
        for (auto& enemy : enemies)
        {
            if (enemy && enemy->getTransformada())
            {
                enemy->getTransformada()->velocidad.x = 0.f;
                enemy->getTransformada()->velocidad.y = 0.f;
            }
        }
    }

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

        // Replace shooting logic with system call
        SystemProcessPlayerShooting(player, bulletsShot, objetos);
        SistemaControl(*player, dt);
        SistemaMover(objetos.getPool(), dt);
        auto enemies = SystemGetEntityTypeVector(objetos.getPool(), CE::ENTITY_TYPE::ENEMY);

        // Check if any dialogue is active before allowing enemies to follow player
        bool isDialogueActive = false;
        auto npcs = SystemGetEntityTypeVector(objetos.getPool(), CE::ENTITY_TYPE::NPC);
        for (auto& npc : npcs)
        {
            if (npc->tieneComponente<IVJ::IDialogo>())
            {
                auto dialogo = npc->getComponente<IVJ::IDialogo>();
                if (dialogo->activo)
                {
                    isDialogueActive = true;
                    break;
                }
            }
        }

        // Only allow enemies to follow player if no dialogue is active
        if (!isDialogueActive && boss->estaVivo())
        {
            SystemFollowPlayer(enemies, player, dt);
        }
        else
        {
            // Stop enemy movement by setting velocities to 0
            stopEnemyMovement(enemies);
        }

        SystemUpdateBulletsState(bulletsShot, enemies, player, objetos, currentEnemiesInScene, dt);
        SystemCheckLimits(objetos.getPool(), 3840.f, 3840.f);
        checkRoundEnd();
        movePlayerPointer();
        SystemUpdateLootItems(lootItems, player,
            newWeaponType, newUtilityType,
            lootPositions, player->shouldChangeWeapon,
            player->shouldChangeUtility, dt);

        // Store weapon type before UI update to detect changes
        auto weaponBeforeUpdate = player->getComponente<CE::IWeapon>()->type;
        updatePlayerUI();
        sceneOverlay->Update(CE::Render::Get(), UIsceneOverlayElements);
        checkAndRefreshFSMOnWeaponChange(weaponBeforeUpdate);

        player->setIsEntityFacingRight(player->checkPlayerFacingRight(CE::Render::Get().GetVentana()));
        player->inputFSM();
        player->handleReload();
        // handle the player collisions with other objects in the pool and update every entity state
        processPlayerCollisions(dt);
        processPlayerKnifeAttack(enemies);
        // Handle enemy attacks on the player via system
        SystemHandleEnemyAttacks(player, enemies);

        // NPCs were already retrieved earlier for dialogue check, use them here
        auto jammers = SystemGetEntityTypeVector(objetos.getPool(), CE::ENTITY_TYPE::SIGNAL_JAMMER);

        // Move NPC and jammers to playable area when round 3 is reached
        if (currentRound >= 3)
        {
            // Move NPC to center of map
            if (!npcs.empty())
            {
                for (auto& npc : npcs)
                {
                    // Check if NPC is still at distant position
                    if (npc->getTransformada()->posicion.x > 9000.f)
                    {
                        npc->setPosicion(1920.f, 1920.f);
                        CE::printDebug("[QUEST] Round 3 reached! NPC moved to playable area at (1920, 1920)");
                    }
                }
            }

            // Move jammers to their proper positions
            if (!jammers.empty())
            {
                for (auto& jammer : jammers)
                {
                    if (!jammer->tieneComponente<IVJ::ISignalJammer>())
                        continue;

                    // Check if jammer is still at distant position
                    if (jammer->getTransformada()->posicion.x > 9000.f)
                    {
                        auto jammerComp = jammer->getComponente<IVJ::ISignalJammer>();

                        // Position each jammer based on its phase number
                        if (jammerComp->phase_number == 1)
                        {
                            jammer->setPosicion(3200.f, 800.f); // Northeast area
                            CE::printDebug("[QUEST] Jammer 1 (Phase 1) moved to (3200, 800)");
                        }
                        else if (jammerComp->phase_number == 2)
                        {
                            jammer->setPosicion(800.f, 3000.f); // Southwest area
                            CE::printDebug("[QUEST] Jammer 2 (Phase 2) moved to (800, 3000)");
                        }
                        else if (jammerComp->phase_number == 3)
                        {
                            jammer->setPosicion(3000.f, 3200.f); // Southeast corner
                            CE::printDebug("[QUEST] Jammer 3 (Phase 3) moved to (3000, 3200)");
                        }
                    }
                }
            }
        }

        SysUpdateQuestNPCs(npcs, player, dt);

        // Update quest state and signal jammers (pass currentRound for quest availability check)
        SysUpdateQuestState(player, currentRound);
        SysUpdateSignalJammers(jammers, player, objetos, dt);

        // Update navigation arrow target based on quest progress
        SysUpdateQuestNavigationTarget(player, npcs, jammers);
        // Update arrow position and rotation to point at target (hide if dialogue is active)
        SysUpdateNavigationArrow(player, npcs);

        // Handle player interaction with signal jammers
        if (player->getComponente<CE::IControl>()->NPCinteract && player->tieneComponente<IRayo>())
        {
            auto rayo = player->getComponente<IRayo>();
            for (auto& jammer : jammers)
            {
                if (!jammer->tieneComponente<IVJ::ISignalJammer>())
                    continue;

                bool in_range = checkDistanceInteraction(*player, *jammer, 100.f);
                bool in_raycast = checkRayHit(*jammer, rayo->getP1(), rayo->getP2());

                if (in_range && in_raycast)
                {
                    auto jammerComp = jammer->getComponente<IVJ::ISignalJammer>();
                    jammerComp->onInteractuar(*jammer); // Pass jammer entity, not player
                }
            }
        }
        else
        {
            // Key released, reset stabilization progress for all jammers
            SysResetJammerStabilization(jammers);
        }
        // Update systems related to bosses (boss uses direct velocity control like enemies)
        // Check if quest is complete and boss needs to be activated
        // IMPORTANT: Boss activates when player finishes all dialogues (dialogue 13), not just phase 3
        auto quest = player->getComponente<IQuest>();
        if (quest && quest->all_dialogues_complete)
        {
            // Check if boss is still at distant position (one-time activation)
            if (boss->getTransformada()->posicion.x > 9000.f)
            {
                // All dialogues complete, move boss to playable area
                // Use one of the spawn positions for the boss
                std::set<int> usedIndices;
                int bossSpawnIndex = SystemGetRandomPosition(spawnPositions, usedIndices);
                CE::Vector2D bossSpawnPos = spawnPositions[bossSpawnIndex];

                boss->setPosicion(bossSpawnPos.x, bossSpawnPos.y);
                boss->getComponente<IBossBhvrMirage>()->hpText->setPosicion(bossSpawnPos.x, bossSpawnPos.y - 50.f);
                isBossPaused = false; // Activate boss

                CE::printDebug("[QUEST] All dialogues complete! Boss activated at position (" +
                              std::to_string(bossSpawnPos.x) + ", " + std::to_string(bossSpawnPos.y) + ")");
            }
        }

        if (boss->estaVivo() && !isBossPaused)
        {
            boss->inputFSM();
            BSysMrgMovement(boss, player, bossProjectiles, bossTraps, objetos, 3840.f, 3840.f, dt);
            BSysUpdateProjectiles(bossProjectiles, boss, bossTraps, player, objetos, dt);
            BSysUpdateTraps(bossTraps, boss, player, dt);
        }
        // Update HP text display (handles both alive and dead states)
        BSysUpdateHPDisplay(boss);

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

            else if (accion.getNombre() == "arriba")
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
            else if (accion.getNombre() == "pauseboss")
            {
                isBossPaused = !isBossPaused;
            }
            else if (accion.getNombre() == "npcinteract")
            {
                playerControl->NPCinteract = true;
            }
#if DEBUG
            else if (accion.getNombre() == "debuground")
            {
                // Debug: Manually increment round
                currentRound++;
                CE::printDebug("[DEBUG] Round manually incremented to: " + std::to_string(currentRound));
            }
#endif
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
            else if (accion.getNombre() == "izquierda")
            {
                playerControl->izq = false;
            }
            else if (accion.getNombre() == "npcinteract")
            {
                playerControl->NPCinteract = false;
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
#if DEBUG
        CE::Render::Get().GetVentana().setMouseCursorVisible(true);
#elif !DEBUG // if release build (not debug, I don't have a flag for release build)
        CE::Render::Get().GetVentana().setMouseCursorVisible(false);
#endif

        for (auto& b : background)
            CE::Render::Get().AddToDraw(b);

        for (auto& obj : objetos.getPool())
        {
            CE::Render::Get().AddToDraw(*obj);
        }
        // draw overlay at the end
        sceneOverlay->draw(CE::Render::Get());

        if (newRoundTextTimer.frame_actual <= newRoundTextTimer.max_frame && shouldShowNewRoundText
            && currentRound > 1)
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

        // Render quest dialogues
        auto npcs = SystemGetEntityTypeVector(objetos.getPool(), CE::ENTITY_TYPE::NPC);
        SysRenderQuestDialogues(npcs);

        // Render navigation arrow pointing to next quest objective
        SysRenderNavigationArrow(player);

        // Render stabilization progress text for jammers being stabilized
        auto jammers = SystemGetEntityTypeVector(objetos.getPool(), CE::ENTITY_TYPE::SIGNAL_JAMMER);
        SysRenderStabilizationText(jammers);

#if DEBUG
        // DEBUG: Draw ray for NPC interaction
        if (player->getComponente<CE::IControl>()->NPCinteract)
        {
            if (player->tieneComponente<IRayo>())
            {
                auto rayo = player->getComponente<IRayo>();
                debugDrawRay(rayo->getP1(), rayo->getP2(), sf::Color::Red);
            }
        }
#endif
    }
} // IVJ
