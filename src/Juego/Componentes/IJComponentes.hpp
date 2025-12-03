#pragma once
#include "../Maquinas/FSM.hpp"
#include "Motor/Componentes/IComponentes.hpp"
#include "Motor/Primitivos/Objetos.hpp"

namespace IVJ
{
    class FSM; //refefencia circular
    //Maquina estado componente
    class IMaquinaEstado : public CE::IComponentes
    {
        public:
            explicit IMaquinaEstado();
            ~IMaquinaEstado(){};
        public:
            std::shared_ptr<FSM> fsm;
    };

    class IRayo : public CE::IComponentes
    {
    public:
        explicit IRayo(CE::Vector2D& pos, float& sig, float magnitud = 1.f);
        ~IRayo() override{};

        CE::Vector2D& getP1() const;
        CE::Vector2D& getP2();
        float getMagnitud() const { return magnitud; }

    private:
        float magnitud;
        float& dir;
        CE::Vector2D& lp1;
        CE::Vector2D lp2;
    };

    class IInteractable : public CE::IComponentes
    {
    public:
        IInteractable() = default;
        ~IInteractable() override {};
        virtual void onInteractuar(CE::Objeto& obj) = 0;

        bool interactuado {false};
    };

    class IDialogo : public IInteractable
    {
    public:
        IDialogo();
        explicit IDialogo(int dialogue_id);
        explicit IDialogo(int initial_dialogue_id, int max_dialogue_count);
        ~IDialogo() override {};
        void onInteractuar(CE::Objeto &obj) override; // Keep for interface compliance, but will call system function

        std::wstring texto;
        int id_texto;
        int id_inicial;
        int indice_actual;
        int max_dialogos;
        bool activo {false};
        bool primera_vez {true}; // Flag to track if this is the first interaction
        bool last_interact_state {false}; // Track previous frame's interact state
    };

    // Quest phases enum
    enum class QUEST_PHASE
    {
        NOT_STARTED = 0,
        PHASE_1_TRAP_LESSON = 1,
        PHASE_2_RANGED_LESSON = 2,
        PHASE_3_TELEPORT_LESSON = 3,
        PHASE_COMPLETE = 4
    };

    // Component to track quest state (attached to player)
    class IQuest : public CE::IComponentes
    {
    public:
        IQuest() : current_phase(QUEST_PHASE::NOT_STARTED), last_phase(QUEST_PHASE::NOT_STARTED),
                   phase1_complete(false), phase2_complete(false), phase3_complete(false),
                   phase2_activated(false), phase3_activated(false),
                   has_talked_to_npc_initially(false), // Track if player has had first NPC interaction
                   max_dialogue_index(4), // Start with dialogues 1-4 accessible
                   navigation_target(nullptr), // Navigation arrow target
                   navigation_arrow(nullptr) {} // Navigation arrow entity
        ~IQuest() override {}

        QUEST_PHASE current_phase;
        QUEST_PHASE last_phase; // Track last phase to prevent duplicate debug messages
        bool phase1_complete;
        bool phase2_complete;
        bool phase3_complete;

        // Phase activation flags - phases don't start until player talks to NPC after previous phase
        bool phase2_activated; // Set to true when player talks to NPC after Phase 1 completion
        bool phase3_activated; // Set to true when player talks to NPC after Phase 2 completion
        bool has_talked_to_npc_initially; // Set to true after first NPC interaction

        // Dialogue progression control
        int max_dialogue_index; // Maximum dialogue index the player can currently access

        // Navigation arrow - points to next quest objective
        std::shared_ptr<Entidad> navigation_target; // Current target entity (NPC or Signal Jammer)
        std::shared_ptr<Entidad> navigation_arrow; // Arrow entity that follows player and points to target
    };

    // Component for signal jammer entities
    class ISignalJammer : public IInteractable
    {
    public:
        explicit ISignalJammer(int phase_number)
            : phase_number(phase_number), stabilized(false), trap_spawned(false), projectile_spawned(false),
              currentBurstCount(0), currentProjectilesInBurst(0), isShootingBurst(false),
              teleport_initialized(false), current_position_index(0),
              is_being_stabilized(false) {} // Stabilization state
        ~ISignalJammer() override {}

        void onInteractuar(CE::Objeto& obj) override; // Will call system function

        int phase_number; // Which phase this jammer represents (1, 2, or 3)
        bool stabilized; // Has the player completed this jammer?
        bool trap_spawned; // For phase 1: have traps been spawned around this jammer?
        bool projectile_spawned; // For phase 2: have projectiles been spawned around this jammer?
        std::vector<std::shared_ptr<Entidad>> traps; // Store trap entities for phase 1
        std::vector<std::shared_ptr<Entidad>> projectiles; // Store projectile entities for phase 2

        // Stabilization mechanics (hold-press for 6 seconds)
        std::shared_ptr<CE::ITimer> stabilizationTimer; // Timer for hold-press stabilization (6 seconds)
        bool is_being_stabilized; // Is player currently holding the interact key to stabilize?

        // Phase 2: Ranged attack burst mechanics
        std::shared_ptr<CE::ITimer> rangedAttackTimer;  // 3 second timer between burst sequences
        std::shared_ptr<CE::ITimer> projectileBurstTimer;  // Timer for burst cadence (30 frames between projectiles)
        int currentBurstCount;  // Current burst number (0-2, for 3 bursts)
        int currentProjectilesInBurst;  // Current projectile in burst (0-4, for 5 projectiles)
        bool isShootingBurst;  // Is currently shooting a burst of projectiles

        // Phase 3: Teleportation mechanics
        std::shared_ptr<CE::ITimer> teleportTimer;  // Timer for teleportation (2 or 5 seconds)
        std::vector<CE::Vector2D> teleport_positions;  // Predefined positions for teleportation (10 positions in a circle)
        int current_position_index;  // Current position in the teleport positions list
        bool teleport_initialized;  // Have teleport positions been initialized?
    };
}
