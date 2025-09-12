#include "Overlay.hpp"
#include "InfoUI.hpp"

#include <SFML/Graphics.hpp>
#include "Motor/Camaras/CamarasGestor.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"
#include "Motor/Primitivos/GestorAssets.hpp"

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

    void Overlay::updateElements(InfoUI elements)
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
        // space for weapon and utility types
        /*
         *
         */
    }

    OverlayMain::OverlayMain(const InfoUI &displayElements, const std::shared_ptr<Entidad> &objectLocked)
        : Overlay{displayElements}, objectLock {objectLocked}
    {
        if (!font.openFromFile(ASSETS "/fonts/PixelOperator.ttf"))
        {
            CE::printDebug("[OVERLAY] Error loading font");
        }

        crosshair = CE::ISprite(
            CE::GestorAssets.Get().getTextura("crosshair"), 32, 32, 0.5f);
    }
}
