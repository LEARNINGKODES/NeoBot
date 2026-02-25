#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../manager/MacroManager.hpp"
#include "SaveMacroLayer.hpp"
#include "LoadMacroLayer.hpp"
#include "../ui/NeoBotUI.hpp"

using namespace geode::prelude;

// ============================================================
// NeoBotMenuLayer
// Menú principal del mod, accesible desde el PauseLayer
// Contiene toggles para Record, Play, Safe Mode y botones
// para guardar/cargar macros.
// ============================================================
class NeoBotMenuLayer : public Popup<> {
protected:
    // Referencias a los toggles para actualizar su estado visual
    CCMenuItemToggler* m_recordToggle  = nullptr;
    CCMenuItemToggler* m_playToggle    = nullptr;
    CCMenuItemToggler* m_safeModeToggle = nullptr;
    CCMenuItemToggler* m_autoSaveToggle = nullptr;

    // Label de información de la macro actual
    CCLabelBMFont* m_macroInfoLabel = nullptr;

    bool setup() override {
        this->setTitle("NeoBot");

        auto& mgr = MacroManager::get();
        auto contentSize = this->m_mainLayer->getContentSize();
        float cx = contentSize.width / 2;
        float cy = contentSize.height / 2;

        // ---- Fondo del menú ----
        auto bg = CCScale9Sprite::create("GJ_square01.png");
        bg->setContentSize(contentSize);
        bg->setPosition(ccp(cx, cy));
        bg->setOpacity(200);
        this->m_mainLayer->addChild(bg, -1);

        // ---- Información de la macro actual ----
        std::string infoText = fmt::format(
            "Inputs: {}  |  Estado: {}",
            mgr.currentMacro.inputs.size(),
            mgr.state == BotState::Recording ? "Grabando" :
            mgr.state == BotState::Playing   ? "Reproduciendo" : "Inactivo"
        );
        m_macroInfoLabel = CCLabelBMFont::create(infoText.c_str(), "bigFont.fnt");
        m_macroInfoLabel->setScale(0.3f);
        m_macroInfoLabel->setPosition(ccp(cx, cy + 105.f));
        m_macroInfoLabel->setColor(ccc3(200, 230, 255));
        this->m_mainLayer->addChild(m_macroInfoLabel);

        // ---- Nombre de la macro ----
        auto nameLabel = CCLabelBMFont::create(
            fmt::format("Macro: {}", mgr.currentMacro.name).c_str(),
            "bigFont.fnt"
        );
        nameLabel->setScale(0.35f);
        nameLabel->setPosition(ccp(cx, cy + 88.f));
        nameLabel->setColor(ccc3(255, 255, 200));
        this->m_mainLayer->addChild(nameLabel);

        // ---- Sección de Toggles ----
        float toggleStartY = cy + 60.f;
        float toggleStep   = 28.f;

        // Toggle: Grabar
        this->addToggleRow(
            "Grabar",
            mgr.state == BotState::Recording,
            ccp(cx, toggleStartY),
            menu_selector(NeoBotMenuLayer::onToggleRecord),
            m_recordToggle
        );

        // Toggle: Reproducir
        this->addToggleRow(
            "Reproducir",
            mgr.state == BotState::Playing,
            ccp(cx, toggleStartY - toggleStep),
            menu_selector(NeoBotMenuLayer::onTogglePlay),
            m_playToggle
        );

        // Toggle: Safe Mode
        this->addToggleRow(
            "Safe Mode",
            mgr.safeMode,
            ccp(cx, toggleStartY - toggleStep * 2),
            menu_selector(NeoBotMenuLayer::onToggleSafeMode),
            m_safeModeToggle
        );

        // Toggle: Auto-Guardar
        this->addToggleRow(
            "Auto-Guardar",
            mgr.autoSave,
            ccp(cx, toggleStartY - toggleStep * 3),
            menu_selector(NeoBotMenuLayer::onToggleAutoSave),
            m_autoSaveToggle
        );

        // ---- Botones de acción ----
        auto actionMenu = CCMenu::create();
        actionMenu->setPosition(ccp(cx, cy - 55.f));

        // Botón: Guardar Macro
        auto saveSpr = ButtonSprite::create("Guardar", "bigFont.fnt", "GJ_button_01.png");
        saveSpr->setScale(0.7f);
        auto saveBtn = CCMenuItemSpriteExtra::create(
            saveSpr, this,
            menu_selector(NeoBotMenuLayer::onSaveMacro)
        );
        saveBtn->setPosition(ccp(-70.f, 0.f));
        actionMenu->addChild(saveBtn);

        // Botón: Cargar Macro
        auto loadSpr = ButtonSprite::create("Cargar", "bigFont.fnt", "GJ_button_02.png");
        loadSpr->setScale(0.7f);
        auto loadBtn = CCMenuItemSpriteExtra::create(
            loadSpr, this,
            menu_selector(NeoBotMenuLayer::onLoadMacro)
        );
        loadBtn->setPosition(ccp(0.f, 0.f));
        actionMenu->addChild(loadBtn);

        // Botón: Limpiar Macro
        auto clearSpr = ButtonSprite::create("Limpiar", "bigFont.fnt", "GJ_button_06.png");
        clearSpr->setScale(0.7f);
        auto clearBtn = CCMenuItemSpriteExtra::create(
            clearSpr, this,
            menu_selector(NeoBotMenuLayer::onClearMacro)
        );
        clearBtn->setPosition(ccp(70.f, 0.f));
        actionMenu->addChild(clearBtn);

        this->m_mainLayer->addChild(actionMenu);

        // ---- Link de YouTube ----
        auto ytLabel = CCLabelBMFont::create(
            "youtube.com/@neomc12",
            "chatFont.fnt"
        );
        ytLabel->setScale(0.45f);
        ytLabel->setPosition(ccp(cx, cy - 90.f));
        ytLabel->setColor(ccc3(255, 100, 100));
        this->m_mainLayer->addChild(ytLabel);

        // ---- Botón de cerrar ----
        auto closeBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
            this,
            menu_selector(NeoBotMenuLayer::onClose)
        );
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition(ccp(cx, cy));
        closeBtn->setPosition(ccp(-cx + 15.f, cy - 15.f));
        closeMenu->addChild(closeBtn);
        this->m_mainLayer->addChild(closeMenu);

        return true;
    }

    // Función auxiliar para crear una fila de toggle con label
    void addToggleRow(
        const char* labelText,
        bool initialState,
        CCPoint pos,
        SEL_MenuHandler callback,
        CCMenuItemToggler*& toggleRef
    ) {
        auto menu = CCMenu::create();
        menu->setPosition(pos);

        // Label del toggle
        auto label = CCLabelBMFont::create(labelText, "bigFont.fnt");
        label->setScale(0.4f);
        label->setAnchorPoint(ccp(1.f, 0.5f));
        label->setPosition(ccp(-10.f, 0.f));
        menu->addChild(label);

        // Toggle
        auto toggleOff = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
        auto toggleOn  = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
        auto toggle = CCMenuItemToggler::create(toggleOff, toggleOn, this, callback);
        toggle->setPosition(ccp(20.f, 0.f));
        toggle->toggle(initialState);
        toggleRef = toggle;
        menu->addChild(toggle);

        this->m_mainLayer->addChild(menu);
    }

    // ---- Callbacks de toggles ----

    void onToggleRecord(CCObject*) {
        auto& mgr = MacroManager::get();

        if (mgr.state == BotState::Recording) {
            // Detener grabación
            std::string levelName = "Unknown";
            if (auto pl = PlayLayer::get()) {
                if (pl->m_level) levelName = pl->m_level->m_levelName;
            }
            mgr.stopRecording();
            mgr.tryAutoSave(levelName);
            Notification::create("Grabación detenida", NotificationIcon::Info, 1.5f)->show();
        } else {
            // Iniciar grabación (detener reproducción si estaba activa)
            if (mgr.state == BotState::Playing) {
                mgr.stopPlaying();
                if (m_playToggle) m_playToggle->toggle(false);
            }
            mgr.startRecording();
            Notification::create("Grabación iniciada", NotificationIcon::Info, 1.5f)->show();
        }

        updateInfoLabel();
        updateStateInGame();
    }

    void onTogglePlay(CCObject*) {
        auto& mgr = MacroManager::get();

        if (mgr.state == BotState::Playing) {
            // Detener reproducción
            mgr.stopPlaying();
            Notification::create("Reproducción detenida", NotificationIcon::Info, 1.5f)->show();
        } else {
            if (mgr.currentMacro.inputs.empty()) {
                Notification::create(
                    "No hay macro cargada para reproducir",
                    NotificationIcon::Warning,
                    2.0f
                )->show();
                // Revertir el toggle
                if (m_playToggle) m_playToggle->toggle(false);
                return;
            }
            // Detener grabación si estaba activa
            if (mgr.state == BotState::Recording) {
                mgr.stopRecording();
                if (m_recordToggle) m_recordToggle->toggle(false);
            }
            mgr.startPlaying();
            Notification::create("Reproducción iniciada", NotificationIcon::Info, 1.5f)->show();
        }

        updateInfoLabel();
        updateStateInGame();
    }

    void onToggleSafeMode(CCObject*) {
        auto& mgr = MacroManager::get();
        mgr.safeMode = !mgr.safeMode;
        Mod::get()->setSavedValue("safe_mode", mgr.safeMode);
        Notification::create(
            fmt::format("Safe Mode: {}", mgr.safeMode ? "Activado" : "Desactivado"),
            NotificationIcon::Info, 1.5f
        )->show();
    }

    void onToggleAutoSave(CCObject*) {
        auto& mgr = MacroManager::get();
        mgr.autoSave = !mgr.autoSave;
        Mod::get()->setSavedValue("auto_save", mgr.autoSave);
        Notification::create(
            fmt::format("Auto-Guardar: {}", mgr.autoSave ? "Activado" : "Desactivado"),
            NotificationIcon::Info, 1.5f
        )->show();
    }

    // ---- Callbacks de botones ----

    void onSaveMacro(CCObject*) {
        auto& mgr = MacroManager::get();
        if (mgr.currentMacro.inputs.empty()) {
            Notification::create(
                "No hay inputs grabados para guardar",
                NotificationIcon::Warning, 2.0f
            )->show();
            return;
        }
        this->onClose(nullptr);
        SaveMacroLayer::open();
    }

    void onLoadMacro(CCObject*) {
        this->onClose(nullptr);
        LoadMacroLayer::open();
    }

    void onClearMacro(CCObject*) {
        auto& mgr = MacroManager::get();
        if (mgr.state != BotState::None) {
            Notification::create(
                "Detén la grabación/reproducción antes de limpiar",
                NotificationIcon::Warning, 2.0f
            )->show();
            return;
        }
        mgr.currentMacro.clear();
        Notification::create("Macro limpiada", NotificationIcon::Info, 1.5f)->show();
        updateInfoLabel();
    }

    void onClose(CCObject*) {
        this->setKeypadEnabled(false);
        this->setTouchEnabled(false);
        this->removeFromParentAndCleanup(true);
    }

    // ---- Utilidades ----

    void updateInfoLabel() {
        if (!m_macroInfoLabel) return;
        auto& mgr = MacroManager::get();
        std::string infoText = fmt::format(
            "Inputs: {}  |  Estado: {}",
            mgr.currentMacro.inputs.size(),
            mgr.state == BotState::Recording ? "Grabando" :
            mgr.state == BotState::Playing   ? "Reproduciendo" : "Inactivo"
        );
        m_macroInfoLabel->setString(infoText.c_str());
    }

    void updateStateInGame() {
        if (auto pl = PlayLayer::get()) {
            NeoBotUI::updateStateLabel(pl);
        }
    }

public:
    static NeoBotMenuLayer* create() {
        auto ret = new NeoBotMenuLayer();
        if (ret->initAnchored(340.f, 280.f)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void open() {
        // Cerrar si ya está abierto
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (auto existing = scene->getChildByID("neobot-menu-layer")) {
            existing->removeFromParent();
        }

        auto layer = NeoBotMenuLayer::create();
        if (layer) {
            layer->setID("neobot-menu-layer");
            scene->addChild(layer, 200);
        }
    }
};
