#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "../ui/NeoBotMenuLayer.hpp"

using namespace geode::prelude;

// ============================================================
// Hook de PauseLayer
// Añade el botón de NeoBot al menú de pausa
// ============================================================
class $modify(PauseLayer) {

    void customSetup() {
        PauseLayer::customSetup();

        // Crear el sprite del botón usando un sprite de GD existente
        // Usamos GJ_playBtn2_001.png como base (igual que xdBot)
        auto sprite = CCSprite::createWithSpriteFrameName("GJ_playBtn2_001.png");
        if (!sprite) {
            // Fallback: usar un sprite básico si el anterior no existe
            sprite = CCSprite::create();
        }
        sprite->setScale(0.35f);

        // Crear el botón del menú
        auto btn = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(PauseLayer::onNeoBotMenu)
        );
        btn->setID("neobot-menu-btn"_spr);

        // Intentar añadir al menú derecho de la pausa
        if (auto menu = this->getChildByID("right-button-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        } else {
            // Fallback: crear un menú propio
            auto fallbackMenu = CCMenu::create();
            fallbackMenu->setID("neobot-menu"_spr);
            fallbackMenu->setPosition(ccp(214, 88));
            btn->setPosition(ccp(0, 0));
            fallbackMenu->addChild(btn);
            this->addChild(fallbackMenu, 10);
        }
    }

    // Callback para abrir el menú de NeoBot
    void onNeoBotMenu(CCObject*) {
        NeoBotMenuLayer::open();
    }
};
