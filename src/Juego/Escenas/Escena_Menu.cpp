#include "Escena_Menu.hpp"
#include "../../Motor/Camaras/CamarasGestor.hpp"
#include "../../Motor/Primitivos/GestorEscenas.hpp"
#include "../../Motor/Primitivos/GestorAssets.hpp"
#include "../../Motor/Render/Render.hpp"
#include "../objetos/Texto.hpp"

namespace IVJ
{
    void EscenaMenu::setSceneTexts()
    {
        constexpr float leftMargin = 50.f;
        constexpr float baseY = 140.f;
        constexpr float offsetY = 90.f; // Increased offset to avoid overlap
        constexpr  float offsetToTitle = -100.f; // Offset to position title higher

        titleText = std::make_shared<Texto>(CE::GestorAssets::Get().getFont("NotJamSlab14"),"PAUSED");
        titleText->setPosicion(leftMargin, baseY + offsetToTitle); // Position title higher
        titleText->setTextCharacterSize(100); // Make title bigger

        // Move continueText higher on the Y position
        continueText = std::make_shared<Texto>(CE::GestorAssets::Get().getFont("NotJamSlab14"),"continue");
        continueText->setPosicion(leftMargin, baseY + offsetY); // Higher than before
        continueText->setTextCharacterSize(60);

        exitGameText = std::make_shared<Texto>(CE::GestorAssets::Get().getFont("NotJamSlab14"),"exit game");
        exitGameText->setPosicion(leftMargin, baseY + 2 * offsetY);
        exitGameText->setTextCharacterSize(60);

        goToBestiaryText = std::make_shared<Texto>(CE::GestorAssets::Get().getFont("NotJamSlab14"),"go to bestiary");
        goToBestiaryText->setPosicion(leftMargin, baseY + 3 * offsetY);
        goToBestiaryText->setTextCharacterSize(60);

        objetos.agregarPool(titleText);
        objetos.agregarPool(continueText);
        objetos.agregarPool(exitGameText);
        objetos.agregarPool(goToBestiaryText);
    }

    // method to fill the menu options positions array
    // used for the navigation pointer (icon sprite)
    void EscenaMenu::addMenuOptPositions()
    {
        constexpr float yOffset = 50.f;
        menuOptionsPositions[0] = {continueText->getTransformada()->posicion.x + 300.f,
                                        continueText->getTransformada()->posicion.y + yOffset};
        menuOptionsPositions[1] = {exitGameText->getTransformada()->posicion.x + 330.f,
                                        exitGameText->getTransformada()->posicion.y + yOffset};
        menuOptionsPositions[2] = {goToBestiaryText->getTransformada()->posicion.x + 450.f,
                                        goToBestiaryText->getTransformada()->posicion.y + yOffset};
    }

    void EscenaMenu::onInit()
    {
        CE::GestorCamaras::Get().setCamaraActiva(0);
        if(!inicializar) return;
        //agregar el font
        //registramos el esquema de los botones
        registrarBotones(sf::Keyboard::Scancode::W,"arriba");
        registrarBotones(sf::Keyboard::Scancode::Up,"arriba");
        registrarBotones(sf::Keyboard::Scancode::S,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Down,"abajo");
        registrarBotones(sf::Keyboard::Scancode::Escape,"esc");
        registrarBotones(sf::Keyboard::Scancode::Enter, "select");

        setSceneTexts();
        addMenuOptPositions();

        CE::GestorAssets::Get().agregarTextura("iconoGrande", ASSETS "/overlays/icon_lg.png",
                                               CE::Vector2D{0, 0}, CE::Vector2D{160, 150});
        icon = std::make_shared<CE::ISprite>(CE::GestorAssets::Get().getTextura("iconoGrande"),160,150,0.75f);
        //icon->m_sprite.setPosition({70.f, 100.f});

        bgColor = sf::Color::Black;

        inicializar = false;

    }
    void EscenaMenu::onFinal()
    {

    }

    void EscenaMenu::onUpdate(float dt)
    {
        icon->m_sprite.setPosition({menuOptionsPositions[currentSelection].x, menuOptionsPositions[currentSelection].y});

        for(auto &f: objetos.getPool())
            f->onUpdate(dt);
        //objetos.borrarPool();
    }
    void EscenaMenu::onInputs(const CE::Botones& accion)
    {
        if (accion.getTipo() != CE::Botones::TipoAccion::OnPress)
        {
            return;
        }

        if (accion.getNombre() == "esc")
        {
            CE::GestorEscenas::Get().cambiarEscena("Main");
        }
        else if (accion.getNombre() == "abajo")
        {
            // Move down: increment index, wrap to 0 if at last
            currentSelection = (currentSelection + 1) % optionsCount;
        }
        else if (accion.getNombre() == "arriba")
        {
            // Move up: decrement index, wrap to 2 if at first
            currentSelection = (currentSelection - 1 + optionsCount) % optionsCount;
        }
        else if (accion.getNombre() == "select")
        {
            switch (currentSelection)
            {
                case 0: // Continue
                    CE::GestorEscenas::Get().cambiarEscena("Main");
                    break;
                case 1: // Exit Game
                    CE::printDebug("Exiting game from menu...");
                    exit(EXIT_SUCCESS); // or set a flag to exit gracefully
                    break;
                case 2: // Go to Bestiary
                    //CE::GestorEscenas::Get().cambiarEscena("Bestiary");
                    break;
                default:
                    break;
            }
        }
    }

    void EscenaMenu::onInputs(const CE::MouseButton &accion)
    {}

    void EscenaMenu::onRender()
    {
        CE::Render::Get().OnClearColor(bgColor); // paint the background
        CE::Render::Get().AddToDraw(icon->m_sprite); // draw game icon
         // draw all texts in the pool
        for(auto &f: objetos.getPool())
            CE::Render::Get().AddToDraw(*f);
    }
}
