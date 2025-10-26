#include "SistemasBosses.h"
#include "../Sistemas.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include <random>

#include "Motor/Utils/Lerp.hpp"

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

                stats->hp = 10;
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
        std::set<int> usedIndices;
        /*int posIndex = SystemGetRandomPosition(positionsArr, usedIndices);
        CE::Vector2D spawnPos = positionsArr[posIndex];*/
        const CE::Vector2D spawnPos = {300.f, 300.f}; // fixed spawn for testing
        boss->setPosicion(spawnPos.x, spawnPos.y);
        boss->addComponente(std::make_shared<IBossBhvrMirage>());
        BSysAdjustBossStats(boss, BOSS_TYPE::MIRAGE);
        boss->addComponente(std::make_shared<CE::ISprite>(
            CE::GestorAssets::Get().getTextura("MirageSprite"),
            64, 64, 1.f))
        .addComponente(std::make_shared<CE::IBoundingBox>(
            CE::Vector2D{64.f, 64.f}))
        // this is just a patch so the boss is recognized as an enemy for collision systems, bullets systemsetc.
        .addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::ENEMY));

        // Initialize damage timer for damage animation
        boss->damageTimer = std::make_shared<CE::ITimer>(30); // 30 frames of red flash on damage

        // Set boss to melee phase for testing
        auto behavior = boss->getComponente<IBossBhvrMirage>();
        behavior->currentAttackPhase = IBossBhvrMirage::ATTACK_PHASE::MELEE;

        CE::printDebug("[BOSS INIT] Mirage boss initialized at position (" +
                      std::to_string(spawnPos.x) + ", " + std::to_string(spawnPos.y) + ") in MELEE phase");
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
     * Directly sets velocity like enemy movement (no IControl)
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
        // If winding up an attack
        if (behavior->isWindingUp)
        {
            if (behavior->currentMeleeAttack == IBossBhvrMirage::MELEE_ATTACK_TYPE::SIMPLE)
            {
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
            if (behavior->currentMeleeAttack == IBossBhvrMirage::MELEE_ATTACK_TYPE::QUICK)
            {
                // Quick attack always stops movement and progresses timer during windup
                BSysStopMovement(boss);
                boss->getComponente<CE::ISprite>()->m_sprite.setColor(sf::Color::Green);
                behavior->quickMeleeWindupTimer->frame_actual++;

                // Attack lands
                if (boss->hasTimerReachedMax(behavior->quickMeleeWindupTimer.get()) && !behavior->hasLandedAttack)
                {
                    // Check if player is still in range
                    if (distanceToPlayer <= behavior->meleeAttackRange)
                    {
                        // Deal damage to player
                        player->hasBeenHit = true;
                        player->checkAndApplyDamage(behavior->quickMeleeAttackDamage);
                        CE::printDebug("[BOSS] Quick melee attack landed! Dealt " +
                                     std::to_string(behavior->quickMeleeAttackDamage) + " damage.");
                        boss->getComponente<CE::ISprite>()->m_sprite.setColor(sf::Color::White);
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
            return true; // Attack is active
        }

        // Check if we can start a new attack (cooldown complete)
        if (boss->hasTimerReachedMax(behavior->meleeAttackCooldownTimer.get()))
        {
            // Randomly choose between simple and quick attack
            // 80% chance for simple attack, 20% chance for quick attack
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<> dis(0, 9);
            int attackChoice = dis(gen);
            std::cout << "[BOSS] Melee attack choice roll: " << attackChoice << std::endl;

            // Simple attack - only if in range
            if (attackChoice < 5)// && distanceToPlayer <= behavior->meleeAttackRange)
            {
                behavior->currentMeleeAttack = IBossBhvrMirage::MELEE_ATTACK_TYPE::SIMPLE;
                boss->resetTimer(behavior->simpleMeleeWindupTimer.get());
                CE::printDebug("[BOSS] Starting SIMPLE melee attack (windup: " +
                             std::to_string(behavior->simpleMeleeWindupTimer->max_frame) + " frames)");

                behavior->isWindingUp = true;
                behavior->hasLandedAttack = false;
                return true; // Attack is now active
            }
            // Quick attack
            if (attackChoice >= 5)
            {
                // Define reasonable teleport distance (e.g., between 100 and 400 units)
                const float minTeleportDistance = 100.f;
                const float maxTeleportDistance = 400.f;

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
                    behavior->didTeleport = true; // Mark that this attack involved a teleport
                    return true; // Attack is now active
                }
                if (distanceToPlayer <= behavior->meleeAttackRange)
                {
                    // Too close for teleport, just do quick attack from current position
                    behavior->currentMeleeAttack = IBossBhvrMirage::MELEE_ATTACK_TYPE::QUICK;
                    boss->resetTimer(behavior->quickMeleeWindupTimer.get());
                    CE::printDebug("[BOSS] Starting QUICK melee attack (windup: " +
                                 std::to_string(behavior->quickMeleeWindupTimer->max_frame) + " frames)");

                    behavior->isWindingUp = true;
                    behavior->hasLandedAttack = false;
                    behavior->didTeleport = false; // No teleport for this attack
                    return true; // Attack is now active
                }
            }
        }

        return false; // No attack active
    }

    /*
     * Main system to handle the movement behavior of the Mirage boss
     */
    void BSysMrgMovement(std::shared_ptr<Entidad>& boss, std::shared_ptr<Entidad>& player, float dt)
    {
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
            // TODO: Implement ranged behavior
            // For now, maintain distance from player
        }
    }
}
