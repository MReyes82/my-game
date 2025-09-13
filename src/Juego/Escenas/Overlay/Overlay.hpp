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
        virtual ~Overlay() = default;
        virtual void updateElements(InfoUI elements) = 0;
        virtual void draw(CE::Render& renderWindow) = 0;
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
        ~OverlayMain() override = default;

        void updateElements(InfoUI elements) override;
        void draw(CE::Render& renderWindow) override;
        void Update(CE::Render& window, const InfoUI& elements);
        [[nodiscard]] sf::Text getRoundText() { return roundText; }
        [[nodiscard]] sf::Text getReloadingText() { return reloadingText; }

    private:
        // init methods for the position of the elements, will be called on the constructor
        void setElementsPosition(float windowWidth, float windowHeight);
        void setHeartPositions(float xStartPoint, float yStartPoint);
        // methods for refreshing dynamic elements
        void drawCurrentHealth(CE::Render& sceneWindow) const;
        void updateCrosshairPosition(CE::Render& window);
        //void setWeaponSprite(WeaponType weapon);
        //void setUtilitySprite(UtilityType utility);


        std::shared_ptr<Entidad> objectLock;
        sf::Font font;
        sf::Text scoreText; // "score" label
        sf::Text scoreToText; // "actual" score amount
        sf::Text ammoText;
        sf::Text reloadingText;
        sf::Text roundText;
        CE::ISprite crosshair;
        CE::ISprite heartSprite;
        std::vector<sf::Sprite> hearts; // vector of heart sprites to represent
        CE::ISprite weaponSprite;
        CE::ISprite ammoSprite;
        CE::ISprite utilitySprite;
        CE::ISprite weaponCageSprite;
        CE::ISprite utilityCageSprite;
    };
}
