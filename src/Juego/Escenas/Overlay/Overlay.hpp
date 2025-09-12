#pragma once

#include <SFML/Graphics.hpp>
#include "InfoUI.hpp"
#include "Motor/Componentes/IComponentes.hpp"
#include "Motor/Render/Render.hpp"

namespace  IVJ
{
    /*
     * Abstract class for the overlay system, to be inherited by specific overlay implementations
     */

    class Overlay
    {
    public:
        // TODO: re-analyze if a reference to an object lock would be necessary for some overlays
        //explicit Overlay(const InfoUI& displayElements, const std::shared_ptr<Entidad>& objectLocked);
        explicit Overlay(const InfoUI& displayElements);
        virtual ~Overlay();
        virtual void updateElements(InfoUI elements) = 0;
        virtual void draw(sf::RenderWindow& renderWindow) = 0;
        void setPosition(const CE::Vector2D& pos) const;

    protected:
        InfoUI displayElements;
        std::shared_ptr<CE::ITransform> overlayCenter;
        // TODO: above ^ std::shared_ptr<Entidad> objectLock;

    };

    class OverlayMain : public Overlay
    {
    public:
        explicit OverlayMain(const InfoUI& displayElements, const std::shared_ptr<Entidad>& objectLocked);
        ~OverlayMain() override;

        void updateElements(InfoUI elements) override;
        void draw(sf::RenderWindow& renderWindow) override;
        void Update(const sf::RenderWindow& window, const InfoUI& elements);

    private:
        // init methods for the position of the elements, will be called on the constructor
        void setElementsPosition();
        void setHeartPositions(float x, float y);
        // methods for refreshing dynamic elements
        void drawCurrentHealth(sf::RenderWindow& sceneWindow);
        void updateCrosshairPosition(const sf::RenderWindow& window);
        //void setWeaponSprite(WeaponType weapon);
        //void setUtilitySprite(UtilityType utility);

        std::shared_ptr<Entidad> objectLock;
        sf::Font font;
        sf::Text scoreText; // "score" label
        sf::Text scoreToText; // "actual" score amount
        sf::Text ammoText;
        CE::ISprite crosshair;
        CE::ISprite heartSprite;
        std::vector<sf::Sprite> hearts; // vector of heart sprites to represent
        CE::ISprite weaponSprite;
        CE::ISprite ammoSprite;
        CE::ISprite utilitySprite;
        CE::ISprite weaponCageSprite;
        CE::ISprite ammoCageSprite;
    };
}
