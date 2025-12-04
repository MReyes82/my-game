#include "QuestSystems.hpp"

#include <cmath>
#include <random>

#include "Motor/Render/Render.hpp"
#include "Motor/Utils/Lerp.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Juego/Sistemas/Sistemas.hpp"

namespace IVJ
{
    // Helper function for adding line breaks
    static std::wstring agregarSaltoLinea(const std::wstring& str, size_t max_len)
    {
        std::wstring resultado;
        size_t pos = 0;

        while (pos < str.size())
        {
            size_t chunk_size = std::min(max_len, str.size() - pos);
            resultado += str.substr(pos, chunk_size) + L'\n';
            pos += chunk_size;

            if (pos < str.size())
            {
                resultado += L"\n";
            }
        }
        return resultado;
    }

    bool checkRayHit(CE::Objeto& npc, CE::Vector2D& p1, CE::Vector2D& p2)
    {
        if (!npc.tieneComponente<CE::IBoundingBox>())
            return false;

        auto midNpc = npc.getComponente<CE::IBoundingBox>()->mitad;
        auto posNpc = npc.getTransformada()->posicion;
        auto line = p2 - p1;

        float dx = std::abs(posNpc.x - p1.x);
        float dy = std::abs(posNpc.y - p1.y);

        float sumMidX = midNpc.x + std::abs(line.x);
        float sumMidY = midNpc.y + std::abs(line.y);

        return (sumMidX - dx > 0) && (sumMidY - dy > 0);
    }

    bool checkDistanceInteraction(CE::Objeto& player, CE::Objeto& npc, float maxDistance)
    {
        auto playerPos = player.getTransformada()->posicion;
        auto npcPos = npc.getTransformada()->posicion;
        float distance = playerPos.distancia(npcPos);
        return distance <= maxDistance;
    }

    // System functions for NPC quest handling
    void SysUpdateQuestNPCs(std::vector<std::shared_ptr<Entidad>>& npcs,
                            std::shared_ptr<Entidad>& player,
                            float dt)
    {
        if (!player || !player->tieneComponente<IRayo>())
            return;

        auto rayo = player->getComponente<IRayo>();
        auto quest = player->tieneComponente<IQuest>() ? player->getComponente<IQuest>() : nullptr;

        for (auto& npc : npcs)
        {
            if (!npc->tieneComponente<IVJ::IDialogo>())
                continue;

            // Check if player is in interaction range
            bool in_range = checkDistanceInteraction(*player, *npc, 100.f);
            bool in_raycast = checkRayHit(*npc, rayo->getP1(), rayo->getP2());

            auto dialogo = npc->getComponente<IVJ::IDialogo>();

            if (in_range && in_raycast)
            {
                // Player is in range and facing NPC
                // Activate dialogue on first interaction (when pressing F)
                if (player->getComponente<CE::IControl>()->NPCinteract && !dialogo->activo)
                {
                    dialogo->activo = true;

                    // Mark that player has talked to NPC for the first time
                    if (quest && !quest->has_talked_to_npc_initially)
                    {
                        quest->has_talked_to_npc_initially = true;
                        CE::printDebug("[QUEST] Player has initiated first NPC interaction");
                    }

                    // Update dialogue progression limits and activate phases based on quest state
                    if (quest)
                    {
                        // Update max_dialogue_index and reset starting index based on quest progress
                        if (quest->phase1_complete && quest->max_dialogue_index < 7)
                        {
                            quest->max_dialogue_index = 7; // Allow dialogues 5-7 after Phase 1
                            dialogo->indice_actual = 5; // Start at dialogue 5 (Phase 2 briefing)
                            SysCargarTextoDesdeID(dialogo, dialogo->id_inicial + 4); // Load dialogue 5 (index 4 from id_inicial)
                            CE::printDebug("[QUEST] Dialogue progression unlocked: now starting at dialogue 5");
                        }

                        if (quest->phase2_complete && quest->max_dialogue_index < 10)
                        {
                            quest->max_dialogue_index = 10; // Allow dialogues 8-10 after Phase 2
                            dialogo->indice_actual = 8; // Start at dialogue 8 (Phase 3 briefing)
                            SysCargarTextoDesdeID(dialogo, dialogo->id_inicial + 7); // Load dialogue 8 (index 7 from id_inicial)
                            CE::printDebug("[QUEST] Dialogue progression unlocked: now starting at dialogue 8");
                        }

                        if (quest->phase3_complete && quest->max_dialogue_index < 13)
                        {
                            quest->max_dialogue_index = 13; // Allow dialogues 11-13 after Phase 3
                            dialogo->indice_actual = 11; // Start at dialogue 11 (Final warning)
                            SysCargarTextoDesdeID(dialogo, dialogo->id_inicial + 10); // Load dialogue 11 (index 10 from id_inicial)
                            CE::printDebug("[QUEST] Dialogue progression unlocked: now starting at dialogue 11");
                        }
                    }
                }

                // Always call onInteractuar to track key state (for edge detection)
                if (dialogo->activo)
                {
                    SysOnInteractuarDialogo(dialogo, *player, player);

                    // Activate phases when player reaches specific dialogue indices
                    if (quest)
                    {
                        // Activate Phase 2 when player reaches dialogue 7 (after completing Phase 1)
                        if (dialogo->indice_actual >= 7 && quest->phase1_complete && !quest->phase2_activated)
                        {
                            quest->phase2_activated = true;
                            CE::printDebug("[QUEST] Phase 2 activated - player has been briefed");
                        }

                        // Activate Phase 3 when player reaches dialogue 10 (after completing Phase 2)
                        if (dialogo->indice_actual >= 10 && quest->phase2_complete && !quest->phase3_activated)
                        {
                            quest->phase3_activated = true;
                            CE::printDebug("[QUEST] Phase 3 activated - player has been briefed");
                        }
                    }
                }
            }
            else if (!in_range)
            {
                // Player walked away - reset dialogue
                dialogo->activo = false;

                // If all dialogues are complete, mark NPC and arrow for deletion
                if (quest && quest->all_dialogues_complete)
                {
                    // Mark NPC for deletion
                    npc->getStats()->hp = 0;

                    // Mark navigation arrow for deletion
                    if (quest->navigation_arrow)
                    {
                        quest->navigation_arrow->getStats()->hp = 0;
                    }

                    CE::printDebug("[QUEST] All dialogues complete and player walked away. NPC and arrow marked for deletion.");
                }
                else
                {
                    // Normal dialogue reset if quest not complete
                    SysResetearDialogo(dialogo);
                }
            }

            npc->onUpdate(dt);
        }
    }

    void SysRenderQuestDialogues(std::vector<std::shared_ptr<Entidad>>& npcs)
    {
        for (auto& npc : npcs)
        {
            if (npc->tieneComponente<IVJ::IDialogo>())
            {
                auto dialogo = npc->getComponente<IVJ::IDialogo>();
                if (!dialogo->activo)
                    continue;
                SysOnRenderDialogo(dialogo, *npc);
            }
        }
    }

    // DEBUG
    void debugDrawRay(CE::Vector2D& p1, CE::Vector2D& p2, const sf::Color& color)
    {
        sf::CircleShape cp1 {2.f};
        cp1.setFillColor({0, 0, 0, 255});
        sf::CircleShape cp2 {2.f};
        cp2.setFillColor({0, 0, 0, 255});
        cp1.setPosition({p1.x, p1.y});
        cp2.setPosition({p2.x, p2.y});
        // draw guide lines
        for (float t = 0.f ; t <= 1.f ; t += 0.0015f)
        {
            sf::CircleShape pixel{1};
            pixel.setFillColor(color);
            auto pos = CE::lerp(p1, p2, t);
            pixel.setPosition({pos.x, pos.y});
            CE::Render::Get().AddToDraw(pixel);
        }
        CE::Render::Get().AddToDraw(cp1);
        CE::Render::Get().AddToDraw(cp2);
    }

    // Dialogue-specific system functions
    void SysCargarTextoDesdeID(IDialogo* dialogo, int dialogue_id)
    {
        if (!dialogo)
            return;

        dialogo->id_texto = dialogue_id;
        dialogo->texto = CE::GestorAssets::Get().getDialogue(dialogue_id);
    }

    void SysAvanzarDialogo(IDialogo* dialogo, std::shared_ptr<Entidad>& player)
    {
        if (!dialogo)
            return;

        // Check if player has quest component to enforce dialogue progression
        if (player && player->tieneComponente<IQuest>())
        {
            auto quest = player->getComponente<IQuest>();

            // Check if trying to advance beyond allowed dialogue index
            if (dialogo->indice_actual >= quest->max_dialogue_index)
            {
                CE::printDebug("[QUEST] Cannot advance dialogue - must complete current phase first (max index: " +
                              std::to_string(quest->max_dialogue_index) + ")");
                return; // Don't advance
            }
        }

        if (dialogo->indice_actual < dialogo->max_dialogos)
        {
            dialogo->indice_actual++;
            int next_id = dialogo->id_inicial + dialogo->indice_actual - 1;
            SysCargarTextoDesdeID(dialogo, next_id);

            // Check if player has reached final dialogue (13) - triggers boss activation
            if (player && player->tieneComponente<IQuest>())
            {
                auto quest = player->getComponente<IQuest>();
                if (dialogo->indice_actual >= 13 && !quest->all_dialogues_complete)
                {
                    quest->all_dialogues_complete = true;
                    CE::printDebug("[QUEST] All dialogues complete! Boss activation triggered.");
                }
            }
        }
    }

    void SysResetearDialogo(IDialogo* dialogo)
    {
        if (!dialogo)
            return;

        dialogo->indice_actual = 1;
        SysCargarTextoDesdeID(dialogo, dialogo->id_inicial);
        dialogo->activo = false;
        dialogo->primera_vez = true;
        dialogo->last_interact_state = false;
    }

    void SysOnInteractuarDialogo(IDialogo* dialogo, CE::Objeto& obj, std::shared_ptr<Entidad>& player)
    {
        if (!dialogo || !dialogo->activo)
            return;

        auto control = obj.getComponente<CE::IControl>();
        if (!control)
            return;

        bool current_interact = control->NPCinteract;

        // Detect rising edge: key just pressed (was false, now true)
        bool key_just_pressed = current_interact && !dialogo->last_interact_state;

        if (key_just_pressed)
        {
            // On first press, just show the current dialogue (don't advance)
            if (dialogo->primera_vez)
            {
                dialogo->primera_vez = false; // Next press will advance
            }
            else
            {
                // On subsequent presses, advance to next dialogue
                SysAvanzarDialogo(dialogo, player);
            }
        }

        // Update last state for next frame
        dialogo->last_interact_state = current_interact;
    }

    void SysOnRenderDialogo(IDialogo* dialogo, CE::Objeto& npc)
    {
        if (!dialogo)
            return;

        // Get NPC position to position dialogue box above NPC
        auto npcPos = npc.getTransformada()->posicion;

        // Calculate NPC sprite bounds to position above head
        float npcHeight = 85.f * 0.5f; // NPC sprite height * scale

        sf::Font font = CE::GestorAssets::Get().getFont("PressStart");
        // Reduce max line length for smaller box
        std::wstring text_with_linebreaks = agregarSaltoLinea(dialogo->texto, 40);
        sf::Text renderedText {font, text_with_linebreaks, 8}; // Font size 8 for PressStart2P

        // Get text bounds to size background properly
        auto textBounds = renderedText.getLocalBounds();
        float padding = 12.f; // Increased padding for larger background box

        sf::RectangleShape background;
        // Size background to fit text with padding
        background.setSize(sf::Vector2f{textBounds.size.x + padding * 2, textBounds.size.y + padding * 2});
        background.setFillColor({0, 0, 0, 180});
        // Position centered above NPC head
        background.setPosition(sf::Vector2f{
            npcPos.x - (textBounds.size.x + padding * 2) / 2.f,
            npcPos.y - npcHeight - 65.f
        });

        // Position text inside background with padding
        renderedText.setPosition(sf::Vector2f{
            background.getPosition().x + padding,
            background.getPosition().y + padding
        });

        // add to render queue
        CE::Render::Get().AddToDraw(background);
        CE::Render::Get().AddToDraw(renderedText);
    }

    // ============================================================================
    // QUEST SYSTEM FUNCTIONS
    // ============================================================================

    void SysOnInteractuarSignalJammer(ISignalJammer* jammer, CE::Objeto& jammerEntity)
    {
        if (!jammer || jammer->stabilized || !jammer->stabilizationTimer)
            return;

        // Mark that player is currently stabilizing the jammer
        jammer->is_being_stabilized = true;

        // Update stabilization timer
        jammer->stabilizationTimer->frame_actual++;

        // Check if stabilization is complete (6 seconds = 360 frames at 60 FPS)
        if (jammer->stabilizationTimer->frame_actual >= jammer->stabilizationTimer->max_frame)
        {
            jammer->stabilized = true;
            jammer->is_being_stabilized = false;
            CE::printDebug("[QUEST] Signal Jammer Phase " + std::to_string(jammer->phase_number) + " stabilized!");
        }
    }

    void SysResetJammerStabilization(std::vector<std::shared_ptr<Entidad>>& jammers)
    {
        // Reset stabilization progress for all jammers that are being stabilized but key was released
        for (auto& jammer : jammers)
        {
            if (!jammer->tieneComponente<ISignalJammer>())
                continue;

            auto jammerComp = jammer->getComponente<ISignalJammer>();
            if (jammerComp->is_being_stabilized && !jammerComp->stabilized)
            {
                // Player released the key, reset stabilization progress
                jammerComp->stabilizationTimer->frame_actual = 0;
                jammerComp->is_being_stabilized = false;
                CE::printDebug("[QUEST] Jammer Phase " + std::to_string(jammerComp->phase_number) + " stabilization reset - player released key");
            }
        }
    }

    void SysRenderStabilizationText(std::vector<std::shared_ptr<Entidad>>& jammers)
    {
        // Render "Estabilizando..." text for any jammer currently being stabilized
        for (auto& jammer : jammers)
        {
            if (!jammer->tieneComponente<ISignalJammer>())
                continue;

            auto jammerComp = jammer->getComponente<ISignalJammer>();

            // Only render if jammer is actively being stabilized
            if (!jammerComp->is_being_stabilized || jammerComp->stabilized)
                continue;

            // Get jammer position to position text above it
            auto jammerPos = jammer->getTransformada()->posicion;

            // Load PressStart2P font
            sf::Font font = CE::GestorAssets::Get().getFont("PressStart");

            // Create text with progress indicator
            float progress = static_cast<float>(jammerComp->stabilizationTimer->frame_actual) /
                           static_cast<float>(jammerComp->stabilizationTimer->max_frame);
            int percentage = static_cast<int>(progress * 100.f);

            std::string textString = "Estabilizando... " + std::to_string(percentage) + "%";
            sf::Text renderedText {font, textString, 10}; // Font size 10 for visibility
            renderedText.setFillColor(sf::Color::White);

            // Get text bounds to center it properly
            auto textBounds = renderedText.getLocalBounds();
            float padding = 8.f;

            // Create background rectangle
            sf::RectangleShape background;
            background.setSize(sf::Vector2f{textBounds.size.x + padding * 2, textBounds.size.y + padding * 2});
            background.setFillColor({0, 0, 0, 200}); // Darker background for better visibility
            background.setOutlineColor(sf::Color::Yellow); // Yellow outline to indicate active process
            background.setOutlineThickness(2.f);

            // Position centered above jammer
            background.setPosition(sf::Vector2f{
                jammerPos.x - (textBounds.size.x + padding * 2) / 2.f,
                jammerPos.y - 50.f // Position above the jammer
            });

            // Position text inside background with padding
            renderedText.setPosition(sf::Vector2f{
                background.getPosition().x + padding,
                background.getPosition().y + padding
            });

            // Add to render queue
            CE::Render::Get().AddToDraw(background);
            CE::Render::Get().AddToDraw(renderedText);
        }
    }

    void SysUpdateQuestState(std::shared_ptr<Entidad>& player, int currentRound)
    {
        if (!player || !player->tieneComponente<IQuest>())
            return;

        auto quest = player->getComponente<IQuest>();
        QUEST_PHASE new_phase = quest->current_phase;

        // Determine new phase based on completed phases AND activation flags
        // Quest can only start after currentRound >= 3
        if (!quest->phase1_complete && quest->current_phase == QUEST_PHASE::NOT_STARTED && currentRound >= 3)
        {
            new_phase = QUEST_PHASE::PHASE_1_TRAP_LESSON;
        }
        else if (quest->phase1_complete && !quest->phase2_complete && quest->phase2_activated)
        {
            // Phase 2 only starts after player talks to NPC (reaches dialogue 7)
            new_phase = QUEST_PHASE::PHASE_2_RANGED_LESSON;
        }
        else if (quest->phase2_complete && !quest->phase3_complete && quest->phase3_activated)
        {
            // Phase 3 only starts after player talks to NPC (reaches dialogue 10)
            new_phase = QUEST_PHASE::PHASE_3_TELEPORT_LESSON;
        }
        else if (quest->phase3_complete)
        {
            new_phase = QUEST_PHASE::PHASE_COMPLETE;
        }

        // Only update and print debug message if phase actually changed
        if (new_phase != quest->last_phase)
        {
            quest->current_phase = new_phase;
            quest->last_phase = new_phase;

            // Print debug message for phase transition
            switch (new_phase)
            {
                case QUEST_PHASE::PHASE_1_TRAP_LESSON:
                    CE::printDebug("[QUEST] Starting Phase 1: Trap Lesson (Round " + std::to_string(currentRound) + ")");
                    break;
                case QUEST_PHASE::PHASE_2_RANGED_LESSON:
                    CE::printDebug("[QUEST] Starting Phase 2: Ranged Attack Lesson");
                    break;
                case QUEST_PHASE::PHASE_3_TELEPORT_LESSON:
                    CE::printDebug("[QUEST] Starting Phase 3: Teleport Lesson");
                    break;
                case QUEST_PHASE::PHASE_COMPLETE:
                    CE::printDebug("[QUEST] All phases complete!");
                    break;
                default:
                    break;
            }
        }
    }

    void SysUpdateSignalJammers(std::vector<std::shared_ptr<Entidad>>& jammers,
                                std::shared_ptr<Entidad>& player,
                                CE::Pool& pool,
                                float dt)
    {
        if (!player || !player->tieneComponente<IQuest>())
            return;

        auto quest = player->getComponente<IQuest>();

        for (auto& jammer : jammers)
        {
            if (!jammer->tieneComponente<ISignalJammer>())
                continue;

            auto jammerComp = jammer->getComponente<ISignalJammer>();

            // Phase 1: Trap Lesson
            if (jammerComp->phase_number == 1)
            {
                // Check if jammer was stabilized and phase needs to complete
                // Do this FIRST before phase check, so it runs even after phase changes
                if (jammerComp->stabilized && !quest->phase1_complete)
                {
                    SysCheckPhase1Complete(jammerComp, quest, *jammer);
                }

                // Only spawn and update traps if phase is active
                if (quest->current_phase == QUEST_PHASE::PHASE_1_TRAP_LESSON)
                {
                    // Spawn traps around jammer if not already spawned
                    if (!jammerComp->trap_spawned)
                    {
                        SysSpawnTrapsAroundJammer(jammerComp, *jammer, pool);
                    }

                    // Update traps and check player collision
                    SysUpdateQuestTraps(jammerComp->traps, player, dt);
                }
            }
            // Phase 2: Ranged Attack Lesson
            else if (jammerComp->phase_number == 2)
            {
                // Check if jammer was stabilized and phase needs to complete
                if (jammerComp->stabilized && !quest->phase2_complete)
                {
                    SysCheckPhase2Complete(jammerComp, quest, *jammer);
                }

                // Only shoot projectiles if phase is active, jammer not stabilized, AND not being stabilized
                if (quest->current_phase == QUEST_PHASE::PHASE_2_RANGED_LESSON && !jammerComp->stabilized && !jammerComp->is_being_stabilized)
                {
                    // Initialize timers on first frame if needed
                    if (!jammerComp->projectile_spawned)
                    {
                        jammerComp->projectile_spawned = true;
                        CE::printDebug("[QUEST] Phase 2: Starting ranged attack burst sequences");
                    }

                    // Handle burst shooting mechanics
                    SysJammerShootProjectileBursts(jammerComp, *jammer, player, pool, dt);
                }

                // Update existing projectiles
                SysUpdateQuestProjectiles(jammerComp->projectiles, player, dt);
            }
            // Phase 3: Teleport Lesson
            else if (jammerComp->phase_number == 3)
            {
                // Check if jammer was stabilized and phase needs to complete
                if (jammerComp->stabilized && !quest->phase3_complete)
                {
                    SysCheckPhase3Complete(jammerComp, quest, *jammer);
                }

                // Only teleport if phase is active, jammer not stabilized, AND not being stabilized
                if (quest->current_phase == QUEST_PHASE::PHASE_3_TELEPORT_LESSON && !jammerComp->stabilized && !jammerComp->is_being_stabilized)
                {
                    // Initialize teleport positions on first frame
                    if (!jammerComp->teleport_initialized)
                    {
                        SysInitializeTeleportPositions(jammerComp, *jammer);
                    }

                    // Handle teleportation mechanics
                    SysJammerTeleport(jammerComp, *jammer, dt);
                }
            }

            jammer->onUpdate(dt);
        }
    }

    // ============================================================================
    // PHASE 1: TRAP LESSON FUNCTIONS (Reusing Mirage trap mechanics)
    // ============================================================================

    void SysUpdateQuestTraps(std::vector<std::shared_ptr<Entidad>>& traps,
                             std::shared_ptr<Entidad>& player, float dt)
    {
        for (auto& trap : traps)
        {
            trap->onUpdate(dt);

            // Check collision with player using AABB collision
            // Traps damage player but do NOT disappear on contact
            if (SistemaColAABBMid(*trap, *player, true))
            {
                // Use trap's ITimer component for damage cooldown
                auto trapTimer = trap->getComponente<CE::ITimer>();
                if (trapTimer && trap->hasTimerReachedMax(trapTimer))
                {
                    // Apply damage and knockback to player
                    player->hasBeenHit = true;
                    player->checkAndApplyDamage(trap->getStats()->damage);

                    // Apply knockback away from trap
                    CE::Vector2D knockbackDir = player->getTransformada()->posicion - trap->getTransformada()->posicion;
                    knockbackDir = knockbackDir.normalizacion();
                    player->applyKnockback(knockbackDir, 50.f); // Knockback force

                    // Reset trap timer for next damage cycle
                    trap->resetTimer(trapTimer);

                    CE::printDebug("[QUEST] Player triggered trap!");
                }
            }
        }
        // Note: Traps do not expire and only disappear when jammer is stabilized
    }

    void SysSpawnTrapsAroundJammer(ISignalJammer* jammer, CE::Objeto& jammerEntity, CE::Pool& pool)
    {
        if (!jammer || jammer->trap_spawned)
            return;

        CE::printDebug("[QUEST] Spawning traps around Signal Jammer Phase 1");

        const auto& jammerPos = jammerEntity.getTransformada()->posicion;
        const int numTraps = 8; // Spawn 8 traps in a circle around jammer
        const float radius = 80.f; // Distance from jammer center

        // Trap texture (MirageTrap) is loaded elsewhere in the codebase

        for (int i = 0; i < numTraps; ++i)
        {
            // Calculate position in circle around jammer
            float angle = (2.f * 3.14159f * i) / numTraps;
            float xOffset = radius * cos(angle);
            float yOffset = radius * sin(angle);

            CE::Vector2D trapPos = jammerPos + CE::Vector2D{xOffset, yOffset};

            // Create trap entity (duplicated from BSysMrgCreateTrap)
            auto trap = std::make_shared<IVJ::Entidad>();
            trap->setPosicion(trapPos.x, trapPos.y);

            // Add sprite component using the trap texture
            trap->addComponente(std::make_shared<CE::ISprite>(
                        CE::GestorAssets::Get().getTextura("MirageTrap"),
                        16, 16, 1.f))
                .addComponente(std::make_shared<CE::IBoundingBox>(CE::Vector2D{16.f, 16.f}))
                .addComponente(std::make_shared<CE::ITimer>(60)); // Damage cooldown timer (1 second at 60 FPS)

            // Initialize trap stats
            trap->getStats()->hp = 1; // keep it alive
            trap->getStats()->damage = 1; // damage dealt to player


            jammer->traps.push_back(trap);
            pool.agregarPool(trap); // Add to pool for rendering
        }

        jammer->trap_spawned = true;
        CE::printDebug("[QUEST] Phase 1: " + std::to_string(numTraps) + " traps spawned around jammer");
    }

    void SysCheckPhase1Complete(ISignalJammer* jammer, IQuest* quest, CE::Objeto& jammerEntity)
    {
        if (!jammer || !quest || quest->phase1_complete)
            return;

        if (jammer->stabilized && jammer->phase_number == 1)
        {
            quest->phase1_complete = true;
            CE::printDebug("[QUEST] Phase 1 Complete! Moving to Phase 2...");

            // Clean up traps
            CE::printDebug("[QUEST] Cleaning up " + std::to_string(jammer->traps.size()) + " traps");
            for (auto& trap : jammer->traps)
            {
                trap->getStats()->hp = 0; // Mark for deletion
            }
            jammer->traps.clear();

            // Remove the jammer from the scene
            jammerEntity.getStats()->hp = 0;
        }
    }

    // ============================================================================
    // PHASE 2: RANGED ATTACK LESSON FUNCTIONS (Reusing Mirage projectile mechanics)
    // ============================================================================

    void SysUpdateQuestProjectiles(std::vector<std::shared_ptr<Entidad>>& projectiles,
                                    std::shared_ptr<Entidad>& player, float dt)
    {
        for (auto it = projectiles.begin(); it != projectiles.end(); )
        {
            auto& projectile = *it;
            bool shouldRemove = false;

            projectile->onUpdate(dt);

            // Check collision with player
            if (SistemaColAABBMid(*projectile, *player, true))
            {
                player->hasBeenHit = true;
                player->checkAndApplyDamage(projectile->getStats()->damage);

                // Apply knockback away from projectile
                CE::Vector2D knockbackDir = player->getTransformada()->posicion - projectile->getTransformada()->posicion;
                knockbackDir = knockbackDir.normalizacion();
                player->applyKnockback(knockbackDir, 50.f);

                shouldRemove = true;
                CE::printDebug("[QUEST] Player hit by projectile!");
            }

            // Check if projectile lifetime expired
            if (projectile->tieneComponente<CE::ITimer>() &&
                projectile->hasTimerReachedMax(projectile->getComponente<CE::ITimer>()))
            {
                shouldRemove = true;
            }

            if (shouldRemove)
            {
                projectile->getStats()->hp = 0; // Mark for deletion
                it = projectiles.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void SysJammerShootProjectileBursts(ISignalJammer* jammer, CE::Objeto& jammerEntity,
                                        std::shared_ptr<Entidad>& player,
                                        CE::Pool& pool, float dt)
    {
        if (!jammer || !jammer->rangedAttackTimer || !jammer->projectileBurstTimer)
            return;

        // Update the ranged attack timer
        jammer->rangedAttackTimer->frame_actual++;

        // If we're currently shooting a burst
        if (jammer->isShootingBurst)
        {
            // Update burst cadence timer
            jammer->projectileBurstTimer->frame_actual++;

            // Time to shoot next projectile in burst
            if (jammer->projectileBurstTimer->frame_actual >= jammer->projectileBurstTimer->max_frame)
            {
                // Shoot projectile towards player
                const auto& jammerPos = jammerEntity.getTransformada()->posicion;
                const auto& playerPos = player->getTransformada()->posicion;

                CE::Vector2D directionToPlayer = playerPos - jammerPos;
                directionToPlayer = directionToPlayer.normalizacion();
                CE::Vector2D projectileVel = directionToPlayer.escala(150.f); // projectile speed

                // Create projectile (similar to boss projectiles)
                auto projectile = std::make_shared<Entidad>();
                projectile->setPosicion(jammerPos.x, jammerPos.y);
                projectile->getTransformada()->velocidad = projectileVel;

                projectile->addComponente(std::make_shared<CE::ISprite>(
                                 CE::GestorAssets::Get().getTextura("MirageProjectile"),
                                 16, 16, 1.f))
                          .addComponente(std::make_shared<CE::IBoundingBox>(
                                 CE::Vector2D{16.f, 16.f}))
                          .addComponente(std::make_shared<CE::ITimer>(240)); // 4 seconds lifetime
                projectile->addComponente(std::make_shared<CE::IEntityType>(CE::ENTITY_TYPE::PROJECTILE));

                projectile->getStats()->hp = 1;
                projectile->getStats()->damage = 1;

                jammer->projectiles.push_back(projectile);
                pool.agregarPool(projectile); // Add to pool for rendering

                jammer->currentProjectilesInBurst++;
                CE::printDebug("[QUEST] Jammer fired projectile " + std::to_string(jammer->currentProjectilesInBurst) +
                              " of burst " + std::to_string(jammer->currentBurstCount + 1));

                // Check if burst is complete (5 projectiles)
                if (jammer->currentProjectilesInBurst >= 5)
                {
                    jammer->currentProjectilesInBurst = 0;
                    jammer->currentBurstCount++;

                    // Check if all bursts are complete (3 bursts)
                    if (jammer->currentBurstCount >= 3)
                    {
                        jammer->isShootingBurst = false;
                        jammer->currentBurstCount = 0;
                        jammer->rangedAttackTimer->frame_actual = 0;
                        CE::printDebug("[QUEST] Jammer completed all 3 bursts. Resetting ranged attack timer.");
                    }
                }

                // Reset burst timer for next projectile
                jammer->projectileBurstTimer->frame_actual = 0;
            }
        }
        // Try to start a new burst sequence if timer is ready
        else if (jammer->rangedAttackTimer->frame_actual >= jammer->rangedAttackTimer->max_frame)
        {
            jammer->isShootingBurst = true;
            jammer->currentProjectilesInBurst = 0;
            jammer->currentBurstCount = 0;
            jammer->projectileBurstTimer->frame_actual = 0;
            CE::printDebug("[QUEST] Jammer starting new projectile burst sequence (3 bursts of 5 projectiles)");
        }
    }

    void SysCheckPhase2Complete(ISignalJammer* jammer, IQuest* quest, CE::Objeto& jammerEntity)
    {
        if (!jammer || !quest || quest->phase2_complete)
            return;

        if (jammer->stabilized && jammer->phase_number == 2)
        {
            quest->phase2_complete = true;
            CE::printDebug("[QUEST] Phase 2 Complete! Moving to Phase 3...");

            // Clean up projectiles
            CE::printDebug("[QUEST] Cleaning up " + std::to_string(jammer->projectiles.size()) + " projectiles");
            for (auto& projectile : jammer->projectiles)
            {
                projectile->getStats()->hp = 0; // Mark for deletion
            }
            jammer->projectiles.clear();

            // Remove the jammer from the scene
            jammerEntity.getStats()->hp = 0;
        }
    }

    // ============================================================================
    // PHASE 3: TELEPORT LESSON FUNCTIONS
    // ============================================================================

    void SysInitializeTeleportPositions(ISignalJammer* jammer, CE::Objeto& jammerEntity)
    {
        if (!jammer || jammer->teleport_initialized)
            return;

        CE::printDebug("[QUEST] Initializing teleport positions for Signal Jammer Phase 3");

        // Get the initial jammer position as the center of the circle
        const auto& centerPos = jammerEntity.getTransformada()->posicion;
        const int numPositions = 10; // 10 predefined positions
        const float radius = 100.f; // Distance from center

        // Generate 10 positions in a circle around the initial position
        for (int i = 0; i < numPositions; ++i)
        {
            float angle = (2.f * 3.14159f * i) / numPositions;
            float xOffset = radius * cos(angle);
            float yOffset = radius * sin(angle);

            CE::Vector2D position = centerPos + CE::Vector2D{xOffset, yOffset};
            jammer->teleport_positions.push_back(position);
        }

        // Set initial position to a random position from the list
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> posDis(0, numPositions - 1);

        jammer->current_position_index = posDis(gen);
        jammerEntity.setPosicion(
            jammer->teleport_positions[jammer->current_position_index].x,
            jammer->teleport_positions[jammer->current_position_index].y
        );

        jammer->teleport_initialized = true;
        CE::printDebug("[QUEST] Phase 3: 10 teleport positions initialized, starting at index " +
                      std::to_string(jammer->current_position_index));
    }

    void SysJammerTeleport(ISignalJammer* jammer, CE::Objeto& jammerEntity, float dt)
    {
        if (!jammer || !jammer->teleportTimer)
            return;

        // Update teleport timer
        jammer->teleportTimer->frame_actual++;

        // Check if it's time to teleport
        if (jammer->teleportTimer->frame_actual >= jammer->teleportTimer->max_frame)
        {
            // Teleport to a random position from the list
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> posDis(0, jammer->teleport_positions.size() - 1);

            // Select a different position than the current one
            int newIndex;
            do {
                newIndex = posDis(gen);
            } while (newIndex == jammer->current_position_index && jammer->teleport_positions.size() > 1);

            jammer->current_position_index = newIndex;

            // Teleport the jammer to the new position
            const auto& newPos = jammer->teleport_positions[newIndex];
            jammerEntity.setPosicion(newPos.x, newPos.y);

            CE::printDebug("[QUEST] Jammer teleported to position index " + std::to_string(newIndex));

            // Determine next teleport delay: 33% chance of 5 seconds, 67% chance of 2 seconds
            std::uniform_int_distribution<> chanceDis(0, 99);
            int chance = chanceDis(gen);

            if (chance < 33) // 33% chance
            {
                jammer->teleportTimer->max_frame = 300; // 5 seconds at 60 FPS
                CE::printDebug("[QUEST] Next teleport in 5 seconds (slow window for player to stabilize)");
            }
            else // 67% chance
            {
                jammer->teleportTimer->max_frame = 60; // 1 seconds at 60 FPS
                CE::printDebug("[QUEST] Next teleport in 2 seconds");
            }

            // Reset timer
            jammer->teleportTimer->frame_actual = 0;
        }
    }

    void SysCheckPhase3Complete(ISignalJammer* jammer, IQuest* quest, CE::Objeto& jammerEntity)
    {
        if (!jammer || !quest || quest->phase3_complete)
            return;

        if (jammer->stabilized && jammer->phase_number == 3)
        {
            quest->phase3_complete = true;
            CE::printDebug("[QUEST] Phase 3 Complete! Quest finished!");

            // Remove the jammer from the scene
            jammerEntity.getStats()->hp = 0;
        }
    }

    // ============================================================================
    // NAVIGATION ARROW SYSTEM
    // ============================================================================

    void SysUpdateQuestNavigationTarget(std::shared_ptr<Entidad>& player,
                                        std::vector<std::shared_ptr<Entidad>>& npcs,
                                        std::vector<std::shared_ptr<Entidad>>& jammers)
    {
        if (!player || !player->tieneComponente<IQuest>())
            return;

        auto quest = player->getComponente<IQuest>();

        // Determine navigation target based on quest state
        // Priority order: NPC (for briefings) -> Active Jammer -> NPC (for debriefings)

        // Case 0: Player hasn't talked to NPC for the first time - always point to NPC
        if (!quest->has_talked_to_npc_initially)
        {
            for (auto& npc : npcs)
            {
                if (npc->tieneComponente<IDialogo>())
                {
                    quest->navigation_target = npc;
                    return;
                }
            }
        }

        // Case 1: Need to return to NPC for briefing after completing a phase
        if ((quest->phase1_complete && !quest->phase2_activated) ||
            (quest->phase2_complete && !quest->phase3_activated))
        {
            // Point to NPC
            for (auto& npc : npcs)
            {
                if (npc->tieneComponente<IDialogo>())
                {
                    quest->navigation_target = npc;
                    return;
                }
            }
        }

        // Case 2: Phase 1 active - point to jammer 1
        else if (quest->current_phase == QUEST_PHASE::PHASE_1_TRAP_LESSON && !quest->phase1_complete)
        {
            for (auto& jammer : jammers)
            {
                if (jammer->tieneComponente<ISignalJammer>())
                {
                    auto jammerComp = jammer->getComponente<ISignalJammer>();
                    if (jammerComp->phase_number == 1 && !jammerComp->stabilized)
                    {
                        quest->navigation_target = jammer;
                        return;
                    }
                }
            }
        }

        // Case 3: Phase 2 active - point to jammer 2
        else if (quest->current_phase == QUEST_PHASE::PHASE_2_RANGED_LESSON && !quest->phase2_complete)
        {
            for (auto& jammer : jammers)
            {
                if (jammer->tieneComponente<ISignalJammer>())
                {
                    auto jammerComp = jammer->getComponente<ISignalJammer>();
                    if (jammerComp->phase_number == 2 && !jammerComp->stabilized)
                    {
                        quest->navigation_target = jammer;
                        return;
                    }
                }
            }
        }

        // Case 4: Phase 3 active - point to jammer 3
        else if (quest->current_phase == QUEST_PHASE::PHASE_3_TELEPORT_LESSON && !quest->phase3_complete)
        {
            for (auto& jammer : jammers)
            {
                if (jammer->tieneComponente<ISignalJammer>())
                {
                    auto jammerComp = jammer->getComponente<ISignalJammer>();
                    if (jammerComp->phase_number == 3 && !jammerComp->stabilized)
                    {
                        quest->navigation_target = jammer;
                        return;
                    }
                }
            }
        }

        // Case 5: Phase 3 complete - point to NPC for final dialogues (11-13)
        else if (quest->phase3_complete)
        {
            // Check if player hasn't reached the final dialogue yet
            for (auto& npc : npcs)
            {
                if (npc->tieneComponente<IDialogo>())
                {
                    auto dialogo = npc->getComponente<IDialogo>();
                    // If player hasn't reached dialogue 13 yet, point to NPC
                    if (dialogo->indice_actual < 13)
                    {
                        quest->navigation_target = npc;
                        return;
                    }
                }
            }
        }

        // If no valid target found, clear navigation target
        quest->navigation_target = nullptr;
    }

    void SysInitNavigationArrow(std::shared_ptr<Entidad>& player, CE::Pool& pool)
    {
        if (!player || !player->tieneComponente<IQuest>())
            return;

        auto quest = player->getComponente<IQuest>();

        // Create arrow entity if it doesn't exist
        if (!quest->navigation_arrow)
        {
            auto arrow = std::make_shared<Entidad>();
            // Position arrow far away until round 3 (will be updated in SysUpdateNavigationArrow)
            arrow->setPosicion(-10000.f, -10000.f);
            arrow->addComponente(std::make_shared<CE::ISprite>(
                CE::GestorAssets::Get().getTextura("navigation_e"),
                32, 32, 0.3f));
            arrow->getStats()->hp = 1; // Keep alive

            // Store reference in quest component
            quest->navigation_arrow = arrow;

            // Add to pool
            pool.agregarPool(arrow);

            CE::printDebug("[QUEST] Navigation arrow entity created (hidden until round 3)");
        }
    }

    void SysUpdateNavigationArrow(std::shared_ptr<Entidad>& player,
                                  std::vector<std::shared_ptr<Entidad>>& npcs)
    {
        if (!player || !player->tieneComponente<IQuest>())
            return;

        auto quest = player->getComponente<IQuest>();

        // Don't update if arrow doesn't exist
        if (!quest->navigation_arrow)
            return;

        // Check if quest is completely done (phase 3 complete AND all dialogues read)
        bool allDialoguesRead = false;
        if (quest->phase3_complete)
        {
            for (auto& npc : npcs)
            {
                if (npc->tieneComponente<IDialogo>())
                {
                    auto dialogo = npc->getComponente<IDialogo>();
                    // If player has reached dialogue 13 or beyond, all dialogues are read
                    if (dialogo->indice_actual >= 13)
                    {
                        allDialoguesRead = true;
                        break;
                    }
                }
            }
        }

        // Hide arrow if quest hasn't started yet (before round 3) OR if all dialogues are read
        if (quest->current_phase == QUEST_PHASE::NOT_STARTED || allDialoguesRead)
        {
            quest->navigation_arrow->setPosicion(-10000.f, -10000.f);
            return;
        }

        // Check if any dialogue is currently active - hide arrow during dialogues
        for (auto& npc : npcs)
        {
            if (npc->tieneComponente<IDialogo>())
            {
                auto dialogo = npc->getComponente<IDialogo>();
                if (dialogo->activo)
                {
                    // Hide arrow when dialogue is active
                    quest->navigation_arrow->setPosicion(-10000.f, -10000.f);
                    return;
                }
            }
        }

        // Don't show if no target
        if (!quest->navigation_target)
        {
            // Move arrow off-screen
            quest->navigation_arrow->setPosicion(-10000.f, -10000.f);
            return;
        }

        // Get player and target positions
        auto playerPos = player->getTransformada()->posicion;
        auto targetPos = quest->navigation_target->getTransformada()->posicion;

        // Calculate direction to target
        CE::Vector2D directionToTarget = targetPos - playerPos;
        float distance = directionToTarget.magnitud();

        // Don't show arrow if player is very close to target (within 80 units)
        if (distance < 80.f)
        {
            // Move arrow off-screen
            quest->navigation_arrow->setPosicion(-10000.f, -10000.f);
            return;
        }

        directionToTarget = directionToTarget.normalizacion();

        // Calculate angle to rotate arrow sprite
        float angle = std::atan2(directionToTarget.y, directionToTarget.x) * 180.f / 3.14159f;

        // Position arrow offset from player (floating above/around player)
        float arrowDistance = 60.f; // Distance from player center
        CE::Vector2D arrowOffset = directionToTarget.escala(arrowDistance);
        CE::Vector2D arrowPos = playerPos + arrowOffset;

        // Update arrow position and rotation
        quest->navigation_arrow->setPosicion(arrowPos.x, arrowPos.y);

        // Update sprite rotation and add pulsing effect
        if (quest->navigation_arrow->tieneComponente<CE::ISprite>())
        {
            auto sprite = quest->navigation_arrow->getComponente<CE::ISprite>();
            sprite->m_sprite.setRotation(sf::degrees(angle));

            // Add pulsing effect based on time
            static float pulseTime = 0.f;
            pulseTime += 0.05f;
            float pulse = 0.8f + 0.2f * std::sin(pulseTime);
            sprite->m_sprite.setScale(sf::Vector2f(1.5f * pulse, 1.5f * pulse));
        }
    }

    void SysRenderNavigationArrow(std::shared_ptr<Entidad>& player)
    {
        if (!player || !player->tieneComponente<IQuest>())
            return;

        auto quest = player->getComponente<IQuest>();

        // Render arrow entity if it exists and is alive
        if (quest->navigation_arrow && quest->navigation_arrow->estaVivo())
        {
            CE::Render::Get().AddToDraw(*quest->navigation_arrow);
        }
    }
}
