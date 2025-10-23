#include "Overlay.hpp"
#include "InfoUI.hpp"

#include <SFML/Graphics.hpp>
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Render/Render.hpp"
#include "Motor/Utils/Utils.hpp"

/*
 * CHANGES FROM PAST VERSION: now since the render handles the object rendering, this class
 *  Won't be pulling a sf::RenderWindow object, instead will be using the CE::Render singleton
 *  and its AddToDraw method to queue the objects to be drawn
 */

namespace  IVJ
{
    Overlay::Overlay(const InfoUI &displayElements)
    {
        this->displayElements = displayElements;
        this->overlayCenter = std::make_shared<CE::ITransform>();
        this->overlayCenter->posicion = CE::Vector2D{0.f, 0.f};
    }

    void Overlay::setPosition(const CE::Vector2D &pos) const
    {
        overlayCenter->posicion = pos;
    }

    void Overlay::updateElements(const InfoUI elements)
    {
        // this method works as a re-constructor as it updates all attributes
        // from this class's InfoUI object
        if (displayElements.getMaxAmmo() != elements.getMaxAmmo())
            displayElements.setMaxAmmo(elements.getMaxAmmo());

        if (displayElements.getCurrentAmmo() != elements.getCurrentAmmo())
            displayElements.setCurrentAmmo(elements.getCurrentAmmo());

        if (displayElements.getHealth() != elements.getHealth())
            displayElements.setHealth(elements.getHealth());

        if (displayElements.getScore() != elements.getScore())
            displayElements.setScore(elements.getScore());

        if (displayElements.getWeapon() != elements.getWeapon())
            displayElements.setWeapon(elements.getWeapon());

        if (displayElements.getUtility() != elements.getUtility())
            displayElements.setUtility(elements.getUtility());
    }
    /*
     * NOTE: this constructor assumes that the sprite textures of the elements are already loaded before call
     * you shall not call this constructor BEFORE loading the textures, as it will make the program crash.
     */
    OverlayMain::OverlayMain(const InfoUI &displayElements, const std::shared_ptr<Entidad> &objectLocked)
        : Overlay{displayElements}, objectLock{objectLocked}, font{}, scoreText{font, ""}, scoreToText{font, ""}, ammoText{font, ""}, reloadingText{font, ""},
          roundText{font, ""},
          crosshair{CE::GestorAssets::Get().getTextura("crosshair"), 15, 17, 1.5f},
          heartSprite{CE::GestorAssets::Get().getTextura("heartSprite"), 8, 8, 1.5f},
          weaponSprite{CE::GestorAssets::Get().getTextura("weaponIconsSprite"), 10, 11, 1.5f},
          ammoSprite{CE::GestorAssets::Get().getTextura("ammoIconSprite"), 8, 8, 2.f},
          utilitySprite{CE::GestorAssets::Get().getTextura("utilityIconSprite"), 14, 10, 1.25f},
          weaponCageSprite{CE::GestorAssets::Get().getTextura("weaponCageSprite"), 18, 18, 1.25f},
          utilityCageSprite{CE::GestorAssets::Get().getTextura("utilityCageSprite"), 18, 18, 1.25f}
    {
        if (!font.openFromFile(ASSETS "/fonts/NotJamSlab14.ttf"))
        {
            CE::printDebug("[OVERLAY] Error loading font");
        }

        // initialize all text values
        scoreText.setFont(font);
        scoreText.setCharacterSize(30);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setString("SCORE");

        scoreToText.setFont(font);
        scoreToText.setCharacterSize(30);
        scoreToText.setFillColor(sf::Color::White);
        scoreToText.setString(std::to_string(displayElements.getScore()));

        ammoText.setFont(font);
        ammoText.setCharacterSize(20);
        ammoText.setFillColor(sf::Color::White);

        reloadingText.setFont(font);
        reloadingText.setCharacterSize(20);
        reloadingText.setFillColor(sf::Color::Yellow);
        reloadingText.setString("RELOADING...");

        roundText.setFont(font);
        roundText.setCharacterSize(15);
        roundText.setFillColor(sf::Color::Red);
        roundText.setString("ROUND OVER, NEW ENEMIES INCOMING");

        const auto centerX = overlayCenter->posicion.x;
        const auto centerY = overlayCenter->posicion.y;
        // patch to make the crosshair fully centered in target object at start
        crosshair.m_sprite.setPosition({
            centerX - (crosshair.m_sprite.getGlobalBounds().size.x / 2),
            centerY - (crosshair.m_sprite.getGlobalBounds().size.y / 2)
        });
        setElementsPosition(CE::Render::Get().GetVentana().getSize().x,
                            CE::Render::Get().GetVentana().getSize().y);
    }

    // Update the overlay elements based on the InfoUI object passed, to follow the player as if it were a camera
    // NOTE: offset numbers are hardcoded since the camera has a zoom level so it won't be affected by window size
    void OverlayMain::setElementsPosition(float windowWidth, float windowHeight)
    {
        const auto centerX = overlayCenter->posicion.x;
        const auto centerY = overlayCenter->posicion.y;
        // Position score text at top-right relative to center
        scoreText.setPosition({centerX + 105, centerY - 130});
        scoreToText.setPosition({centerX + 105, centerY - 100});
        // Center-relative positions
        reloadingText.setPosition({centerX - 40.f, centerY});
        roundText.setPosition({centerX - 120.f, centerY - 60.f});

        constexpr int offset = 8;
        weaponCageSprite.m_sprite.setPosition({centerX  + 147, centerY - 3});
        // Center the weapon sprite within the cage
        const auto boxBounds = weaponCageSprite.m_sprite.getGlobalBounds();
        const auto weaponBounds = weaponSprite.m_sprite.getGlobalBounds();
        const float weaponCenterX = boxBounds.position.x + (boxBounds.size.x / 2) - (weaponBounds.size.x / 2);
        const float weaponCenterY = boxBounds.position.y + (boxBounds.size.y / 2) - (weaponBounds.size.y / 2);
        weaponSprite.m_sprite.setPosition({weaponCenterX + offset, weaponCenterY + offset});

        // Ammo sprite relative to center
        ammoSprite.m_sprite.setPosition({centerX + 120, centerY + 20});
        if (displayElements.getWeapon() != CE::WEAPON_TYPE::KNIFE)
        {
            ammoText.setPosition({centerX + 130, centerY + 8});
            ammoText.setString(std::to_string(displayElements.getCurrentAmmo()) + " / " +
                               std::to_string(displayElements.getMaxAmmo()));
        }
        else
        {
            ammoText.setPosition({centerX + 130, centerY + 20});
            ammoText.setString("");
        }

        utilityCageSprite.m_sprite.setPosition({centerX + 147, centerY + 45});
        // Center the utility sprite within the cage
        const auto utilityBoxBounds = utilityCageSprite.m_sprite.getGlobalBounds();
        const auto utilityBounds = utilitySprite.m_sprite.getGlobalBounds();
        const float utilityCenterX = utilityBoxBounds.position.x + (utilityBoxBounds.size.x / 2) - (utilityBounds.size.x / 2);
        const float utilityCenterY = utilityBoxBounds.position.y + (utilityBoxBounds.size.y / 2) - (utilityBounds.size.y / 2);
        utilitySprite.m_sprite.setPosition({utilityCenterX + offset, utilityCenterY + offset});

        // Hearts positioned at bottom-left relative to center
        setHeartPositions(centerX - 180.f, centerY - 120.f);
    }

    // set posiiton of the hearts based on current health, in a contiguous line
    void OverlayMain::setHeartPositions(float xStartPoint, float yStartPoint)
    {
        // clear any existing hearts
        hearts.clear();
        for (int i = 0 ; i < displayElements.getHealth() ; ++i)
        {
            sf::Sprite heart = heartSprite.m_sprite; // create a copy
            heart.setPosition({xStartPoint + i * 14, yStartPoint}); // position with some spacing
            hearts.push_back(heart); // add to the vector
        }
    }
    // util for drawing current health hearts, just a foor loop drawing all hearts in the vector
    void OverlayMain::drawCurrentHealth(CE::Render& render) const
    {
        for (auto& heart : hearts)
        {
            render.AddToDraw(heart);
        }
    }

    // draw method to be called on the scene render method
    void OverlayMain::draw(CE::Render& render)
    {
        render.AddToDraw(crosshair.m_sprite);
        render.AddToDraw(weaponCageSprite.m_sprite);
        render.AddToDraw(weaponSprite.m_sprite);
        render.AddToDraw(utilityCageSprite.m_sprite);
        /*
         ** Do not draw the utility if it's set to NONE
         * By default this sprite is set to the bandage one just because it needs
         * to be initialized with something
         */
        if (displayElements.getUtility() != CE::UTILITY_TYPE::NONE)
            render.AddToDraw(utilitySprite.m_sprite);

        if (displayElements.getWeapon() != CE::WEAPON_TYPE::KNIFE)
            render.AddToDraw(ammoSprite.m_sprite);

        render.AddToDraw(scoreText);
        render.AddToDraw(scoreToText);
        render.AddToDraw(ammoText);

        drawCurrentHealth(render);
    }

    // Update for the crosshair,  to be called on this class Update method
    void OverlayMain::updateCrosshairPosition(CE::Render& render)
    {
        // get current mouse position relative to the window
        sf::RenderWindow& window = render.GetVentana();
        const sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        // convert to world coordinates using the camera's view
        const sf::Vector2f worldPos = window.mapPixelToCoords(mousePos, CE::GestorCamaras::Get().getCamaraActiva().getView());
        // update crosshair position, no need to adjust for the sprite globalbounds anymore (on SFML 3.0)
        crosshair.m_sprite.setPosition({worldPos.x, worldPos.y});
    }

    // update of this class InfoUI object and sprites on the overlay
    void OverlayMain::updateElements(InfoUI elements)
    {
        // retrieve current weapon before the update
        const auto currentWeapon = displayElements.getWeapon();
        const auto currentUtility = displayElements.getUtility();

        Overlay::updateElements(elements);
        // update the weapon and utility sprites if they have changed
         if (currentWeapon != displayElements.getWeapon())
             // if the weapon is different, update the sprite
             setWeaponSprite(displayElements.getWeapon());
         if (currentUtility != displayElements.getUtility())
             // if the utility is different, update the sprite
             setUtilitySprite(displayElements.getUtility());

    }

    // main update method of the class, to be called on the scene update method
    void OverlayMain::Update(CE::Render& render, const InfoUI& elements)
    {
        updateElements(elements);
        scoreToText.setString(std::to_string(displayElements.getScore()));
        updateCrosshairPosition(render);
        // invoke the camera pos in this scope so the overlay follows the camera
        // and actually gets changed. Since when calling from the scene it won't reflect the changes.
        // then call it in the scene's Update method
        const auto& cameraPos = CE::GestorCamaras::Get().getCamaraActiva().getTransformada().posicion;
        setPosition(cameraPos);
        setElementsPosition(render.GetVentana().getSize().x, render.GetVentana().getSize().y);
    }

    // update the spritesheet TextureRect based on the current utility type
    void OverlayMain::setUtilitySprite(CE::UTILITY_TYPE utility)
    {
        switch (utility)
        {
        case CE::UTILITY_TYPE::NONE:
            CE::printDebug("[OVERLAY] Warning: trying to set utility sprite to NONE type");
            break;

        case CE::UTILITY_TYPE::BANDAGE:
            utilitySprite.m_sprite.setTextureRect(sf::IntRect{{0, 0}, {14, 10}});
            break;

        case CE::UTILITY_TYPE::MEDKIT:
            utilitySprite.m_sprite.setTextureRect(sf::IntRect{{0, 11}, {14, 10}});
            break;

        case CE::UTILITY_TYPE::ENERGY_DRINK:
            utilitySprite.m_sprite.setTextureRect(sf::IntRect{{0, 22}, {8, 9}});
            break;

        default:
            CE::printDebug("[OVERLAY] Warning: default case reached in setUtilitySprite");
            break;
        }
    }

    // update the spritesheet TextureRect based on the current weapon type
    void OverlayMain::setWeaponSprite(CE::WEAPON_TYPE weapon)
    {
        switch (weapon)
        {
        case CE::WEAPON_TYPE::NONE:
            CE::printDebug("[OVERLAY] Warning: trying to set weapon sprite to NONE type");
            break;

        case CE::WEAPON_TYPE::KNIFE:
            weaponSprite.m_sprite.setTextureRect(sf::IntRect{{0, 0}, {10, 11}});
            break;

        case CE::WEAPON_TYPE::REVOLVER:
            weaponSprite.m_sprite.setTextureRect(sf::IntRect{{0, 12}, {11, 7}});
            break;

        case CE::WEAPON_TYPE::SHOTGUN:
            weaponSprite.m_sprite.setTextureRect(sf::IntRect{{0, 20}, {15, 6}});
            weaponSprite.m_sprite.setScale({1.25f, 1.25f}); // update also the scale so it doesn't look too small
            break;

        case CE::WEAPON_TYPE::SMG:
            weaponSprite.m_sprite.setTextureRect(sf::IntRect{{0, 27}, {16, 10}});
            weaponSprite.m_sprite.setScale({1.f, 1.f}); // update also the scale so it doesn't look too small
            break;

        case CE::WEAPON_TYPE::RIFLE:
            weaponSprite.m_sprite.setTextureRect(sf::IntRect{{0, 38}, {17, 6}});
            break;

        default:
            CE::printDebug("[OVERLAY] Warning: default case reached in setWeaponSprite");
            break;
        }
    }

}
