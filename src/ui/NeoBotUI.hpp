#pragma once

#include <Geode/Geode.hpp>
#include "../manager/MacroManager.hpp"

using namespace geode::prelude;

// ============================================================
// NeoBotUI
// Clase de utilidades estáticas para gestionar los elementos
// visuales del mod dentro de PlayLayer
// ============================================================
class NeoBotUI {
public:

    // ID del label de estado dentro de PlayLayer
    static constexpr const char* STATE_LABEL_ID = "neobot-state-label";

    // Añade el indicador visual de estado a PlayLayer
    static void addStateLabel(PlayLayer* pl) {
        if (!pl) return;

        // Eliminar si ya existe (por si acaso)
        if (auto existing = pl->getChildByID(STATE_LABEL_ID)) {
            existing->removeFromParent();
        }

        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        label->setID(STATE_LABEL_ID);
        label->setScale(0.5f);
        label->setOpacity(200);

        // Posicionar en la esquina superior izquierda
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        label->setPosition(ccp(winSize.width * 0.12f, winSize.height * 0.92f));
        label->setAnchorPoint(ccp(0.5f, 0.5f));
        label->setZOrder(100);

        pl->addChild(label);
        updateStateLabel(pl);
    }

    // Actualiza el texto del indicador visual según el estado actual
    static void updateStateLabel(PlayLayer* pl) {
        if (!pl) return;

        auto label = typeinfo_cast<CCLabelBMFont*>(pl->getChildByID(STATE_LABEL_ID));
        if (!label) return;

        auto& mgr = MacroManager::get();

        switch (mgr.state) {
            case BotState::Recording:
                label->setString("REC");
                label->setColor(ccc3(255, 50, 50)); // Rojo para grabación
                label->setVisible(true);
                break;
            case BotState::Playing:
                label->setString("PLAY");
                label->setColor(ccc3(50, 255, 50)); // Verde para reproducción
                label->setVisible(true);
                break;
            case BotState::None:
            default:
                label->setString("");
                label->setVisible(false);
                break;
        }
    }
};
