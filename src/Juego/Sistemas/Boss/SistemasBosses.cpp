#include "SistemasBosses.h"
#include "../Sistemas.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include <random>

#include "Juego/Maquinas/Boss/MirageStates.h"

namespace IVJ
{
    // system to adjust the stats of the boss based on its type
    void BSysAdjustBossStats(std::shared_ptr<Entidad>& boss, BOSS_TYPE bossTyp)
    {
        float baseSpeed = 80.f;
        const auto stats = boss->getStats();

        switch (bossTyp)
        {
            case BOSS_TYPE::MIRAGE:
                const auto bossBehaviorComp = boss->getComponente<IBossBhvrMirage>();
                if (!bossBehaviorComp)
                {
                    CE::printDebug("[SISTEMAS BOSS] Error: boss behavior component not found for Mirage boss");
                    return;
                }
                stats->hp = 100;
                stats->hp_max = stats->hp;
                stats->damage = bossBehaviorComp->rangedAttackDamage; // projectile phase is default
                stats->maxSpeed = baseSpeed * 1.2f;
                break;
        }
    }
    // constructor for the Mirage boss
    // this function assumes that the boss pointer passed as parameter it's already created
    void MirageInit(std::shared_ptr<Entidad>& boss, std::array<CE::Vector2D, 20>& positionsArr)
    {
        // Load projectile texture
        CE::GestorAssets::Get().agregarTextura("MirageProjectile", ASSETS "/sprites/bosses/mirage_projectile.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{16.f, 16.f});

        // Load trap texture
        CE::GestorAssets::Get().agregarTextura("MirageTrap", ASSETS "/sprites/bosses/mirage_trap.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{16.f, 16.f});

        std::set<int> usedIndices;
        /*int posIndex = SystemGetRandomPosition(positionsArr, usedIndices);
        CE::Vector2D spawnPos = positionsArr[posIndex];*/
        const CE::Vector2D spawnPos = {1000.f, 1000.f}; // fixed spawn for testing
        boss->setPosicion(spawnPos.x, spawnPos.y);
        boss->addComponente(std::make_shared<IBossBhvrMirage>());
        BSysAdjustBossStats(boss, BOSS_TYPE::MIRAGE);
        boss->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("MirageSprite"),
            64, 64, 1.f))
        .addComponente(std::make_shared<CE::IBoundingBox>(
            CE::Vector2D{64.f, 64.f}))
        // this is just a patch so the boss is recognized as an enemy for collision systems, bullets systems etc.
        .addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::ENEMY))
        .addComponente(std::make_shared<IMaquinaEstado>()) // componentes needed for FSM, same as enemies
        .addComponente(std::make_shared<CE::IControl>());

        auto& fsm_init = boss->getComponente<IMaquinaEstado>()->fsm;
        fsm_init = std::make_shared<MrgIdleState>(true);
        fsm_init->onEntrar(*boss);

        // Initialize damage timer for damage animation
        boss->damageTimer = std::make_shared<CE::ITimer>(30); // 30 frames of red flash on damage
        // Set boss to ranged phase for testing
        auto behavior = boss->getComponente<IBossBhvrMirage>();
        behavior->currentAttackPhase = IBossBhvrMirage::ATTACK_PHASE::RANGED;
        boss->setIsEntityFacingRight(true);

        CE::printDebug("[BOSS INIT] Mirage boss initialized at position (" +
                      std::to_string(spawnPos.x) + ", " + std::to_string(spawnPos.y) + ") in RANGED phase");
    }
    /*
     * Helper function to calculate distance between boss and player
     */
    float BSysGetDistanceToPlayer(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player)
    {
        const auto& bossPos = boss->getTransformada()->posicion;
        const auto& playerPos = player->getTransformada()->posicion;
        CE::Vector2D diff = playerPos - bossPos;
        return diff.magnitud();
    }
    /*
     * Helper function to move boss towards player
     * Directly sets velocity like enemy movement
     */
    void BSysMoveTowardsPlayer(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player)
    {
        auto& bossTransform = *boss->getTransformada();
        const auto& playerPos = player->getTransformada()->posicion;

        CE::Vector2D directionToPlayer = playerPos - bossTransform.posicion;
        directionToPlayer = directionToPlayer.normalizacion();

        // Get boss speed and set velocity directly
        const float speed = boss->getStats()->maxSpeed;
        bossTransform.velocidad = directionToPlayer.escala(speed);
        // Update facing direction
        if (directionToPlayer.x != 0)
        {
            boss->setIsEntityFacingRight(directionToPlayer.x > 0);
        }
    }
    /*
     * Helper function to stop boss movement
     */
    void BSysStopMovement(std::shared_ptr<Entidad>& boss)
    {
        boss->getTransformada()->velocidad = {0.0f, 0.0f};
    }
    /*
     * Helper: Handle simple melee attack windup logic
     * Returns true if attack is active and boss should not move
     */
    bool BSysMrgHandleSimpleAttackWindup(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player, float distanceToPlayer)
    {
        auto behavior = boss->getComponente<IBossBhvrMirage>();
        boss->getComponente<CE::ISprite>()->m_sprite.setColor(sf::Color::Yellow);

        // Only progress windup timer and stop movement if boss is close enough to the player
        if (distanceToPlayer <= behavior->meleeAttackRange)
        {
            BSysStopMovement(boss);
            behavior->simpleMeleeWindupTimer->frame_actual++;

            // Attack lands
            if (boss->hasTimerReachedMax(behavior->simpleMeleeWindupTimer.get()) && !behavior->hasLandedAttack)
            {
                // Deal damage to player
                player->hasBeenHit = true;
                player->checkAndApplyDamage(behavior->meleeAttackDamage);
                CE::printDebug("[BOSS] Simple melee attack landed! Dealt " +
                             std::to_string(behavior->meleeAttackDamage) + " damage.");
                boss->getComponente<CE::ISprite>()->m_sprite.setColor(sf::Color::White);

                behavior->hasLandedAttack = true;
                behavior->isWindingUp = false;
                behavior->currentMeleeAttack = IBossBhvrMirage::MELEE_ATTACK_TYPE::NONE;
                boss->resetTimer(behavior->meleeAttackCooldownTimer.get());
            }

            return true; // Attack is actively winding up and boss is in range
        }

        return false; // Let movement system handle positioning
    }

    /*
     * Helper: Handle quick melee attack windup logic
     * Returns true (quick attack always commits, boss stops moving)
     */
    bool BSysMrgHandleQuickAttackWindup(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player, float distanceToPlayer)
    {
        auto behavior = boss->getComponente<IBossBhvrMirage>();

        // Quick attack always stops movement and progresses timer during windup
        BSysStopMovement(boss);
        boss->getComponente<CE::ISprite>()->m_sprite.setColor(sf::Color::Green);
        behavior->quickMeleeWindupTimer->frame_actual++;

        // Attack lands
        if (boss->hasTimerReachedMax(behavior->quickMeleeWindupTimer.get()) && !behavior->hasLandedAttack)
        {
            boss->getComponente<CE::ISprite>()->m_sprite.setColor(sf::Color::White);

            // Check if player is still in range
            if (distanceToPlayer <= behavior->meleeAttackRange)
            {
                // Deal damage to player
                player->hasBeenHit = true;
                player->checkAndApplyDamage(behavior->quickMeleeAttackDamage);
                CE::printDebug("[BOSS] Quick melee attack landed! Dealt " +
                             std::to_string(behavior->quickMeleeAttackDamage) + " damage.");
            }
            else
            {
                // Attack missed
                CE::printDebug("[BOSS] Quick melee attack MISSED! Player out of range.");
            }

            behavior->hasLandedAttack = true;
            behavior->isWindingUp = false;
            behavior->currentMeleeAttack = IBossBhvrMirage::MELEE_ATTACK_TYPE::NONE;
            boss->resetTimer(behavior->meleeAttackCooldownTimer.get());
        }

        return true; // Attack is active
    }

    /*
     * Helper: Try to start a simple melee attack
     * Returns true if attack was initiated
     */
    bool BSysMrgTryStartSimpleAttack(std::shared_ptr<Entidad>& boss)
    {
        auto behavior = boss->getComponente<IBossBhvrMirage>();

        behavior->currentMeleeAttack = IBossBhvrMirage::MELEE_ATTACK_TYPE::SIMPLE;
        boss->resetTimer(behavior->simpleMeleeWindupTimer.get());
        CE::printDebug("[BOSS] Starting SIMPLE melee attack (windup: " +
                     std::to_string(behavior->simpleMeleeWindupTimer->max_frame) + " frames)");

        behavior->isWindingUp = true;
        behavior->hasLandedAttack = false;
        return true;
    }

    /*
     * Helper: Try to start a quick melee attack (with teleport logic)
     * Returns true if attack was initiated
     */
    bool BSysMrgTryStartQuickAttack(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player, float distanceToPlayer)
    {
        auto behavior = boss->getComponente<IBossBhvrMirage>();

        // Define reasonable teleport distance
        const float minTeleportDistance = 100.f;
        const float maxTeleportDistance = 400.f;

        // Teleport attack if in medium range
        if (distanceToPlayer >= minTeleportDistance && distanceToPlayer <= maxTeleportDistance)
        {
            // Teleport to player position (with slight offset to be in attack range)
            const auto& playerPos = player->getTransformada()->posicion;
            CE::Vector2D directionToPlayer = playerPos - boss->getTransformada()->posicion;
            directionToPlayer = directionToPlayer.normalizacion();

            // Position boss slightly away from player (at 70% of attack range)
            CE::Vector2D teleportOffset = directionToPlayer.escala(-behavior->meleeAttackRange * 0.7f);
            boss->getTransformada()->posicion = playerPos + teleportOffset;

            behavior->currentMeleeAttack = IBossBhvrMirage::MELEE_ATTACK_TYPE::QUICK;
            boss->resetTimer(behavior->quickMeleeWindupTimer.get());
            CE::printDebug("[BOSS] TELEPORTED to player and starting QUICK melee attack (windup: " +
                         std::to_string(behavior->quickMeleeWindupTimer->max_frame) + " frames)");

            behavior->isWindingUp = true;
            behavior->hasLandedAttack = false;
            behavior->didTeleport = true;
            return true;
        }

        // Quick attack without teleport if in close range
        if (distanceToPlayer <= behavior->meleeAttackRange)
        {
            behavior->currentMeleeAttack = IBossBhvrMirage::MELEE_ATTACK_TYPE::QUICK;
            boss->resetTimer(behavior->quickMeleeWindupTimer.get());
            CE::printDebug("[BOSS] Starting QUICK melee attack (windup: " +
                         std::to_string(behavior->quickMeleeWindupTimer->max_frame) + " frames)");

            behavior->isWindingUp = true;
            behavior->hasLandedAttack = false;
            behavior->didTeleport = false;
            return true;
        }

        return false; // Cannot start quick attack from current distance
    }

    /*
     * System to handle melee attack execution
     * Returns true if an attack is currently active (winding up or executing)
     */
    bool BSysMrgMeleeAttack(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player, float dt)
    {
        auto behavior = boss->getComponente<IBossBhvrMirage>();
        float distanceToPlayer = BSysGetDistanceToPlayer(boss, player);

        // Update attack cooldown timer
        if (!boss->hasTimerReachedMax(behavior->meleeAttackCooldownTimer.get()))
        {
            behavior->meleeAttackCooldownTimer->frame_actual++;
        }

        // Handle ongoing attack windup
        if (behavior->isWindingUp)
        {
            if (behavior->currentMeleeAttack == IBossBhvrMirage::MELEE_ATTACK_TYPE::SIMPLE)
            {
                boss->getStats()->maxSpeed = 140;
                return BSysMrgHandleSimpleAttackWindup(boss, player, distanceToPlayer);
            }
            if (behavior->currentMeleeAttack == IBossBhvrMirage::MELEE_ATTACK_TYPE::QUICK)
            {
                boss->getStats()->maxSpeed = 110;
                return BSysMrgHandleQuickAttackWindup(boss, player, distanceToPlayer);
            }

            return true; // Attack is active
        }

        // Try to start a new attack if cooldown is complete
        if (boss->hasTimerReachedMax(behavior->meleeAttackCooldownTimer.get()))
        {
            // Randomly choose between simple and quick attack (50/50 split)
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(0, 9);
            int attackChoice = dis(gen);

            if (attackChoice < 5)
            {
                return BSysMrgTryStartSimpleAttack(boss);
            }
            else
            {
                return BSysMrgTryStartQuickAttack(boss, player, distanceToPlayer);
            }
        }

        return false; // No attack active
    }

    /*
     * Helper function to maintain distance from player (for ranged mode)
     */
    void BSysMrgMaintainDistance(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player,
                                 float distance, float worldWidth, float worldHeight)
    {
        auto& bossTransform = *boss->getTransformada();
        const auto& playerPos = player->getTransformada()->posicion;
        auto behavior = boss->getComponente<IBossBhvrMirage>();

        CE::Vector2D directionToPlayer = playerPos - bossTransform.posicion;
        float currentDistance = directionToPlayer.magnitud();
        directionToPlayer = directionToPlayer.normalizacion();

        const float speed = boss->getStats()->maxSpeed;
        const float keepDistance = behavior->rangedKeepDistanceRange;
        const float toleranceRange = 30.f; // tolerance zone to prevent jitter
        const float boundaryMargin = 100.f; // Stay at least 100 units from world edges

        CE::Vector2D velocity{0.f, 0.f};

        // Calculate desired velocity based on distance to player
        if (currentDistance < keepDistance - toleranceRange)
        {
            // Too close - move away from player
            velocity = directionToPlayer.escala(-speed);

            // Update facing direction
            if (directionToPlayer.x != 0)
            {
                boss->setIsEntityFacingRight(directionToPlayer.x < 0); // face away from player
            }
        }
        else if (currentDistance > keepDistance + toleranceRange)
        {
            // Too far - move towards player
            velocity = directionToPlayer.escala(speed);

            // Update facing direction
            if (directionToPlayer.x != 0)
            {
                boss->setIsEntityFacingRight(directionToPlayer.x > 0); // face towards player
            }
        }
        else
        {
            // Within tolerance range, stop moving
            BSysStopMovement(boss);
            return;
        }

        // Clamp velocity to prevent moving outside world boundaries
        // Only prevent movement that would take boss further out of bounds
        if (bossTransform.posicion.x < boundaryMargin && velocity.x < 0)
        {
            velocity.x = 0.f; // Don't move further left
        }
        else if (bossTransform.posicion.x > worldWidth - boundaryMargin && velocity.x > 0)
        {
            velocity.x = 0.f; // Don't move further right
        }

        if (bossTransform.posicion.y < boundaryMargin && velocity.y < 0)
        {
            velocity.y = 0.f; // Don't move further up
        }
        else if (bossTransform.posicion.y > worldHeight - boundaryMargin && velocity.y > 0)
        {
            velocity.y = 0.f; // Don't move further down
        }

        bossTransform.velocidad = velocity;
    }

    /*
     * Helper function to create a trap at a specified position (or boss position if not specified)
     */
    void BSysMrgCreateTrap(std::shared_ptr<Entidad>& boss, std::vector<std::shared_ptr<Entidad>>& traps,
                           CE::Pool& pool, const CE::Vector2D* customPosition = nullptr)
    {
        auto behavior = boss->getComponente<IBossBhvrMirage>();

        // Check if max traps deployed
        if (behavior->currentTrapsDeployed >= behavior->maxTrapsAllowed)
        {
            CE::printDebug("[BOSS] Cannot deploy trap: max traps reached (" +
                          std::to_string(behavior->maxTrapsAllowed) + ")");
            return;
        }

        // Get trap placement position (custom position or boss's current position)
        const CE::Vector2D trapPos = customPosition ? *customPosition : boss->getTransformada()->posicion;

        // Check if new trap would be too close to existing traps (prevent clustering)
        const float minTrapDistance = 60.f;
        for (const auto& existingTrap : traps)
        {
            const auto& existingTrapPos = existingTrap->getTransformada()->posicion;
            CE::Vector2D diff = trapPos - existingTrapPos;
            float distance = diff.magnitud();

            if (distance < minTrapDistance)
            {
                CE::printDebug("[BOSS] Cannot deploy trap: too close to existing trap (distance: " +
                              std::to_string(distance) + ")");
                return;
            }
        }

        // Create trap entity at specified position
        auto trap = std::make_shared<Entidad>();
        trap->setPosicion(trapPos.x, trapPos.y);

        // Add sprite component using the trap texture
        trap->addComponente(std::make_shared<CE::ISprite>(
                    CE::GestorAssets::Get().getTextura("MirageTrap"),
                    16, 16, 1.f))
            .addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{16.f, 16.f}))
            .addComponente(std::make_shared<CE::ITimer>(600)); // 10 seconds lifetime at 60 FPS

        // Initialize trap stats
        trap->getStats()->hp = 1; // keep it alive
        trap->getStats()->damage = behavior->rangedAttackDamage;

        // Add entity type to identify as trap/enemy for collision
        // * Removed entity component so collisions are not handled and the player can
        // step on the trap freely (trap logic will handle damage application)
        //trap->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::ENEMY));
        trap->damageTimer = std::make_shared<CE::ITimer>(30); //

        traps.push_back(trap);
        pool.agregarPool(trap); // Add to pool for rendering and collision
        behavior->currentTrapsDeployed++;

        CE::printDebug("[BOSS] Trap deployed at (" + std::to_string(trapPos.x) + ", " +
                      std::to_string(trapPos.y) + "). Total traps: " +
                      std::to_string(behavior->currentTrapsDeployed));
    }

    /*
     * System to handle ranged attack (projectile bursts)
     * Returns true if an attack is currently active
     */
    void BSysMrgRangedAttack(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player,
                             std::vector<std::shared_ptr<Entidad>>& bossProjectiles,
                             CE::Pool& pool, float dt)
    {
        auto behavior = boss->getComponente<IBossBhvrMirage>();

        // Update ranged attack timer
        if (!boss->hasTimerReachedMax(behavior->rangedAttackTimer.get()))
        {
            behavior->rangedAttackTimer->frame_actual++;
        }

        // If we're currently shooting a burst
        if (behavior->isShootingBurst)
        {
            // Update burst cadence timer
            behavior->projectileBurstTimer->frame_actual++;

            // Time to shoot next projectile in burst
            if (boss->hasTimerReachedMax(behavior->projectileBurstTimer.get()))
            {
                // Shoot projectile towards player
                const auto& bossPos = boss->getTransformada()->posicion;
                const auto& playerPos = player->getTransformada()->posicion;

                CE::Vector2D directionToPlayer = playerPos - bossPos;
                directionToPlayer = directionToPlayer.normalizacion();
                CE::Vector2D projectileVel = directionToPlayer.escala(150.f); // projectile speed

                // Create projectile (similar to bullets - not marked as ENEMY)
                auto projectile = std::make_shared<Entidad>();
                projectile->setPosicion(bossPos.x, bossPos.y);
                projectile->getTransformada()->velocidad = projectileVel;

                projectile->addComponente(std::make_shared<CE::ISprite>(
                                 CE::GestorAssets::Get().getTextura("MirageProjectile"),
                                 16, 16, 1.f))
                          .addComponente(std::make_shared<CE::IBoundingBox>(
                                 CE::Vector2D{16.f, 16.f}))
                          .addComponente(std::make_shared<CE::ITimer>(180)); // 3 seconds lifetime
                projectile->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::PROJECTILE));

                projectile->getStats()->hp = 1;
                projectile->getStats()->damage = behavior->rangedAttackDamage;
                // No entity type component - projectiles are just projectiles, not enemies

                bossProjectiles.push_back(projectile);
                pool.agregarPool(projectile); // Add to pool for rendering

                behavior->currentProjectilesInBurst++;
                CE::printDebug("[BOSS] Fired projectile " + std::to_string(behavior->currentProjectilesInBurst) +
                              " of burst " + std::to_string(behavior->currentBurstCount + 1));

                // Check if burst is complete (5 projectiles)
                if (behavior->currentProjectilesInBurst >= 5)
                {
                    behavior->currentProjectilesInBurst = 0;
                    behavior->currentBurstCount++;

                    // Check if all bursts are complete (3 bursts)
                    if (behavior->currentBurstCount >= 3)
                    {
                        behavior->isShootingBurst = false;
                        behavior->currentBurstCount = 0;
                        boss->resetTimer(behavior->rangedAttackTimer.get());
                        CE::printDebug("[BOSS] Completed all 3 bursts. Resetting ranged attack timer.");
                    }
                }

                // Reset burst timer for next projectile
                boss->resetTimer(behavior->projectileBurstTimer.get());
            }
        }
        // Try to start a new burst sequence if timer is ready
        else if (boss->hasTimerReachedMax(behavior->rangedAttackTimer.get()))
        {
            behavior->isShootingBurst = true;
            behavior->currentProjectilesInBurst = 0;
            behavior->currentBurstCount = 0;
            boss->resetTimer(behavior->projectileBurstTimer.get());
            CE::printDebug("[BOSS] Starting new projectile burst sequence (3 bursts of 5 projectiles)");
        }
    }

    /*
     * System to update boss projectiles (collision, lifetime)
     */
    void BSysUpdateProjectiles(std::vector<std::shared_ptr<Entidad>>& bossProjectiles,
                               std::shared_ptr<Entidad>& boss,
                               std::vector<std::shared_ptr<Entidad>>& bossTraps,
                               std::shared_ptr<Entidad>& player,
                               CE::Pool& pool, float dt)
    {
        for (auto it = bossProjectiles.begin(); it != bossProjectiles.end(); )
        {
            auto& projectile = *it;
            bool shouldRemove = false;
            bool shouldConvertToTrap = false;

            projectile->onUpdate(dt);

            // Check collision with player
            if (SistemaColAABBMid(*projectile, *player, true))
            {
                player->hasBeenHit = true;
                player->checkAndApplyDamage(projectile->getStats()->damage);
                shouldRemove = true;
                CE::printDebug("[BOSS] Projectile hit player!");
            }

            // Check if projectile lifetime expired
            if (projectile->tieneComponente<CE::ITimer>() &&
                projectile->hasTimerReachedMax(projectile->getComponente<CE::ITimer>()))
            {
                // 50% chance to convert expired projectile into a trap
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_int_distribution<> dis(0, 99);

                if (dis(gen) < 50)
                {
                    shouldConvertToTrap = true;
                    CE::printDebug("[BOSS] Projectile expired - converting to trap!");
                }
                else
                {
                    CE::printDebug("[BOSS] Projectile expired - dissipating");
                }

                shouldRemove = true;
            }

            if (shouldRemove)
            {
                // Convert to trap if needed
                if (shouldConvertToTrap)
                {
                    const auto& projectilePos = projectile->getTransformada()->posicion;
                    CE::printDebug("[BOSS] Attempting to convert projectile at (" +
                                  std::to_string(projectilePos.x) + ", " +
                                  std::to_string(projectilePos.y) + ") to trap");

                    // Use the existing trap creation function with custom position
                    BSysMrgCreateTrap(boss, bossTraps, pool, &projectilePos);
                }

                projectile->getStats()->hp = 0; // Mark for deletion by pool cleanup
                it = bossProjectiles.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    /*
     * System to update boss traps (collision, lifetime)
     */
    void BSysUpdateTraps(std::vector<std::shared_ptr<Entidad>>& bossTraps,
                         std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player, float dt)
    {
        for (auto it = bossTraps.begin(); it != bossTraps.end(); )
        {
            auto& trap = *it;
            bool shouldRemove = false;

            trap->onUpdate(dt);

            // Check collision with player
            if (SistemaColAABBMid(*trap, *player, true))
            {
                player->hasBeenHit = true;
                player->checkAndApplyDamage(trap->getStats()->damage);
                shouldRemove = true;

                // Decrease trap counter
                auto behavior = boss->getComponente<IBossBhvrMirage>();
                behavior->currentTrapsDeployed--;
                CE::printDebug("[BOSS] Player triggered trap!");
            }

            // Check if trap lifetime expired
            if (trap->tieneComponente<CE::ITimer>() &&
                trap->hasTimerReachedMax(trap->getComponente<CE::ITimer>()))
            {
                shouldRemove = true;

                // Decrease trap counter
                if (boss->estaVivo())
                {
                    auto behavior = boss->getComponente<IBossBhvrMirage>();
                    behavior->currentTrapsDeployed--;
                }

                CE::printDebug("[BOSS] Trap expired!");
            }

            if (shouldRemove)
            {
                trap->getStats()->hp = 0; // Mark for deletion by pool cleanup
                it = bossTraps.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    /*
     * System to check and handle attack mode switching based on HP thresholds or timer
     * Returns true if mode was changed
     */
    bool BSysMrgCheckModeSwitch(std::shared_ptr<Entidad>& boss)
    {
        auto behavior = boss->getComponente<IBossBhvrMirage>();
        const auto stats = boss->getStats();

        // Calculate current HP percentage
        float hpPercentage = (static_cast<float>(stats->hp) / static_cast<float>(stats->hp_max)) * 100.0f;

        bool shouldSwitch = false;
        std::string switchReason;

        // Calculate which threshold the boss is currently at (rounded down to nearest interval)
        int currentThreshold = static_cast<int>(hpPercentage / static_cast<float>(behavior->hpThresholdInterval)) * behavior->hpThresholdInterval;

        // Check if we've crossed a new HP threshold (going downward)
        if (currentThreshold < behavior->lastHpThresholdCrossed)
        {
            behavior->lastHpThresholdCrossed = currentThreshold;
            shouldSwitch = true;
            switchReason = "HP dropped to " + std::to_string(currentThreshold) + "%";
        }

        // Update attack mode timer
        if (!boss->hasTimerReachedMax(behavior->attackModeTimer.get()))
        {
            behavior->attackModeTimer->frame_actual++;
        }

        // Check if timer has reached 3 minutes
        if (boss->hasTimerReachedMax(behavior->attackModeTimer.get()) && !shouldSwitch)
        {
            shouldSwitch = true;
            switchReason = "3 minute timer expired";
        }

        // If mode should switch, toggle between MELEE and RANGED
        if (shouldSwitch)
        {
            auto oldPhase = behavior->currentAttackPhase;

            if (behavior->currentAttackPhase == IBossBhvrMirage::ATTACK_PHASE::MELEE)
            {
                behavior->currentAttackPhase = IBossBhvrMirage::ATTACK_PHASE::RANGED;
                boss->getStats()->maxSpeed = boss->getStats()->maxSpeed = 95; // set to normal speed if ranged
            }
            else
            {
                behavior->currentAttackPhase = IBossBhvrMirage::ATTACK_PHASE::MELEE;
                boss->getStats()->maxSpeed = boss->getStats()->maxSpeed = 105; // set to faster speed if melee
            }

            // Reset attack mode timer
            boss->resetTimer(behavior->attackModeTimer.get());

            // Reset attack states for clean transition
            behavior->isWindingUp = false;
            behavior->hasLandedAttack = false;
            behavior->currentMeleeAttack = IBossBhvrMirage::MELEE_ATTACK_TYPE::NONE;
            behavior->isShootingBurst = false;
            behavior->currentBurstCount = 0;
            behavior->currentProjectilesInBurst = 0;

            CE::printDebug("[BOSS MODE SWITCH] Reason: " + switchReason +
                          " | Old: " + std::string(oldPhase == IBossBhvrMirage::ATTACK_PHASE::MELEE ? "MELEE" : "RANGED") +
                          " -> New: " + std::string(behavior->currentAttackPhase == IBossBhvrMirage::ATTACK_PHASE::MELEE ? "MELEE" : "RANGED") +
                          " | Current HP: " + std::to_string(stats->hp) + "/" + std::to_string(stats->hp_max) +
                          " (" + std::to_string(static_cast<int>(hpPercentage)) + "%)");

            return true;
        }

        return false;
    }

    /*
     * Main system to handle the movement behavior of the Mirage boss
     */
    void BSysMrgMovement(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player,
                         std::vector<std::shared_ptr<Entidad>>& bossProjectiles,
                         std::vector<std::shared_ptr<Entidad>>& traps,
                         CE::Pool& pool, float worldWidth, float worldHeight, float dt)
    {
        // Check if boss should switch attack modes (based on HP thresholds or timer)
        BSysMrgCheckModeSwitch(boss);

        auto behavior = boss->getComponente<IBossBhvrMirage>();
        auto currentAttackPhase = behavior->currentAttackPhase;

        // Debug: Log once every 60 frames (~1 second)
        static int debugFrameCounter = 0;
        if (debugFrameCounter++ % 60 == 0)
        {
            float distance = BSysGetDistanceToPlayer(boss, player);
            CE::printDebug("[BOSS MOVEMENT] Phase: " +
                          std::string(currentAttackPhase == IBossBhvrMirage::ATTACK_PHASE::MELEE ? "MELEE" : "RANGED") +
                          " | Distance: " + std::to_string(distance) +
                          " | Position: (" + std::to_string(boss->getTransformada()->posicion.x) +
                          ", " + std::to_string(boss->getTransformada()->posicion.y) + ")");
        }
        // check in which phase the boss is to apply the corresponding movement behavior
        // if it's on melee, it will try to approach the player,
        if (currentAttackPhase == IBossBhvrMirage::ATTACK_PHASE::MELEE)
        {
            // Try to execute melee attack if in range
            bool isAttacking = BSysMrgMeleeAttack(boss, player, dt);
            // If not attacking, move towards the player
            if (!isAttacking)
            {
                float distanceToPlayer = BSysGetDistanceToPlayer(boss, player);

                // If outside attack range, move towards player
                if (distanceToPlayer > behavior->meleeAttackRange * 0.8f) // Start approaching at 80% of attack range
                {
                    BSysMoveTowardsPlayer(boss, player);
                }
                else
                {
                    // Within range, stop and wait for attack cooldown
                    BSysStopMovement(boss);
                }
            }
        }
        // if it's on ranged, it will try to maintain distance from the player
        // and rely on projectiles to damage the player
        else if (currentAttackPhase == IBossBhvrMirage::ATTACK_PHASE::RANGED)
        {
            float distanceToPlayer = BSysGetDistanceToPlayer(boss, player);

            // Maintain desired distance from player with world boundary checking
            BSysMrgMaintainDistance(boss, player, distanceToPlayer, worldWidth, worldHeight);

            // Execute ranged attack (projectile bursts)
            BSysMrgRangedAttack(boss, player, bossProjectiles, pool, dt);

            // Randomly set traps (10% chance per second, controlled by frame counter)
            static int trapChanceCounter = 0;
            trapChanceCounter++;

            // Check every 60 frames (1 second) for trap deployment
            if (trapChanceCounter >= 60)
            {
                trapChanceCounter = 0;

                // 30% chance to deploy trap
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_int_distribution<> dis(0, 99);

                if (dis(gen) < 30)
                {
                    BSysMrgCreateTrap(boss, traps, pool, nullptr);
                }
            }
        }
    }
}
