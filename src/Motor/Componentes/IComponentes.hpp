#pragma once
#include <memory>
#include <string>
#include "../Utils/Vector2D.hpp"
#include<map>
// include Juego/objetos/Entidad.hpp <--- NO INCLUIR AQUI, SE GENERA UNA AUTOREFERENCIA Y NO SE PUEDE COMPILAR
namespace IVJ
{
    class Entidad; //referencia circular
}

namespace CE
{
    /*
     * Weapon type enum, values are the offset for the player spritesheet
     * on the FSM, so when updating, we take the values from here.
     */
    enum class WEAPON_TYPE
    {
        NONE = -1,
        KNIFE = 0,
        REVOLVER =  128,
        SHOTGUN = 256,
        SMG = 384,
        RIFLE = 512,
    };

    enum class UTILITY_TYPE
    {
        NONE = -1,
        BANDAGE = 0,
        MEDKIT = 1,
        ENERGY_DRINK = 2,
    };

    enum class ENTITY_TYPE
    {
        PLAYER = 0xAAA,
        ENEMY,
        NPC,
        LOOT_WEAPON,
        LOOT_UTILITY,
        PROJECTILE,
        STATIC,
        DYNAMIC,
    };

    class IComponentes
    {
        public:
            virtual ~IComponentes()=default;
    };

    class INombre : public IComponentes
    {
        public:
            explicit INombre(const std::string& nom);
            ~INombre() override{};
        public:
            std::string nombre;
    };

    class ITransform: public IComponentes
    {
        public:
            ITransform();
            ITransform(const Vector2D& pos, const Vector2D& vel, float ang);
            ~ITransform() override{};
        public:
            Vector2D posicion;
            Vector2D pos_prev;
            Vector2D velocidad;
            float angulo;
    };

    class ITimer : public IComponentes
    {
        public:
            ITimer();
            ITimer(int max_f);
            ~ITimer() override{};

            int frame_actual;
            int max_frame;
    };
    class IStats : public IComponentes
    {
        public:
            ~IStats()override{};
            // floar players and enemies
            //8bits = 255 maximo
            int hp;
            int hp_max;
            int damage;
            unsigned int score;// for player
            float maxSpeed; // for enemies
    };

    class ITexto : public IComponentes
    {
        public:
            ITexto(const sf::Font& font, const std::string& texto);
            ~ITexto()override{};

            sf::Text m_texto;
    };

    class ISprite : public IComponentes
    {
        public:
            ISprite(const sf::Texture& textura,float escala);
            ISprite(const sf::Texture& textura,int w,int h,float escala);
            ~ISprite() override{};
        public:
            sf::Sprite m_sprite;
            int width;
            int height;
            float escala;
        private:
            sf::Texture m_textura;
    };

    class IShader : public IComponentes
    {
        public:
            enum class ShaderVars
            {
                FLOAT,
                VEC2F,
                VEC3F,
                VEC4F,
                MAT3,
                MAT4,
                TEX
            };
        public:
            explicit IShader(const std::string& vert, const std::string& frag);
            ~IShader() override{};
            void setEscalar(const std::string& key, float* valor);
            void setVector2(const std::string& key, sf::Vector2f* valor);
            void setVector3(const std::string& key, sf::Vector3f* valor);
            void setVector4(const std::string& key, sf::Glsl::Vec4* valor);
            void setMat3(const std::string& key, sf::Glsl::Mat3* valor);
            void setMat4(const std::string& key, sf::Glsl::Mat4* valor);
            void setTextura(const std::string& key, sf::Texture* valor);
        public:
            sf::Shader m_shader;
            std::string m_fragshaderFile;
            std::string m_vertshaderFile;
            //al sacarlo hay que castear a un tipo de dato
            //m_vars["tiempo"] = &tiempo; (float*)m_vars["tiempo"];
            //m_vars["textura"] = &sprite->getTexture(); (sf::Texture*)m_vars["textura"];
            std::map<std::string,std::pair<IShader::ShaderVars,void*>> m_vars;
    };

//COntrol
    class IControl : public IComponentes
    {
        public:
            explicit IControl();
            ~IControl() override{};
        public:
            //si tiene muchos botones usar un
            //arreglo o un entero de 32bits para
            //32 botones y con mascaras
            bool arr;
            bool abj;
            bool izq;
            bool der;
            bool run;
            bool jmp;
            bool atacar;
            bool interactuar;
            bool NPCinteract;
            bool swapWeapon; // swap between melee and any other weapon
    };

    class IBoundingBox: public IComponentes
    {
        public:
            explicit IBoundingBox(const Vector2D& dim);
            Vector2D tam;
            Vector2D mitad;
    };

    class IRespawn : public IComponentes
    {
    public:
        IRespawn(std::vector<std::shared_ptr<IComponentes>>& lista_comp, int max); // constructor por defecto
        IRespawn(int max, int w, int h, IVJ::Entidad* pre, int max_entidades); // constructor 2

        int timer_actual;
        int timer_maximo;
        int num_objetos;
        int MAX_ENTIDADES;
        // area
        IVJ::Entidad* prefab;
        int width;
        int height;
        std::vector<std::shared_ptr<IComponentes>> componentes;
    };

    class IPaths : public IComponentes
    {
        public:
            explicit IPaths(int total_frames);
            void addCurva(Vector2D p1, Vector2D p2, Vector2D p3);

        public:
            std::vector<Vector2D> puntos;
            int offset;
            int frame_total_curva;
            int frame_actual_curva;
            int id_curva;
    };

    class IEntityType : public IComponentes
    {
        public:

        public:
            explicit IEntityType(ENTITY_TYPE t) : type{t} {}
            ~IEntityType() override = default;

            ENTITY_TYPE type {};
    };

    class IWeapon : public IComponentes
    {
        public:
            explicit IWeapon(WEAPON_TYPE t);
            ~IWeapon() override = default;

            WEAPON_TYPE type; // current weapon type
            int currentMagBullets; // bullets in the magazine at runtime
            int maxWeaponBullets; // max bullets the weapon can hold
            int magSize; // size of each magazine
            float fireRate; // bullets per second
            float reloadTime; // time to reload in seconds
            bool isReloading {false};
            bool isKnife {false}; // flag to identify if the weapon is a knife (melee), to simplify checks
    };
    class IUtility : public IComponentes
    {
        public:
            explicit IUtility(UTILITY_TYPE t);
            ~IUtility() override = default;

            UTILITY_TYPE type;// current utility item type
            int quantity; // quantity of the utility item (e.g., number of bandages), so it can be stacked when picking up the same item
    };
}


