#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/GeodeUI.hpp>

#include "MacroManager.hpp"

using namespace geode::prelude;

// UI para el mod
class MacroMenu : public geode::Popup<std::string const&> {
protected:
    bool setup(std::string const& value) override {
        auto winSize = CCDirector::get()->getWinSize();

        this->setTitle("NeoBot Macro");

        auto manager = MacroManager::get();

        // Toggle para grabar
        auto recordToggle = geode::createToggle(
            [manager](bool checked) {
                if (checked) {
                    manager->startRecording();
                } else {
                    manager->stopRecording();
                }
            },
            "Record",
            manager->isRecording()
        );
        recordToggle->setPosition(winSize.width / 2, winSize.height / 2 + 30);
        this->m_mainLayer->addChild(recordToggle);

        // Toggle para reproducir
        auto playToggle = geode::createToggle(
            [manager](bool checked) {
                if (checked) {
                    manager->startPlaying();
                } else {
                    manager->stopPlaying();
                }
            },
            "Play",
            manager->isPlaying()
        );
        playToggle->setPosition(winSize.width / 2, winSize.height / 2);
        this->m_mainLayer->addChild(playToggle);

        // Toggle para Safe Mode
        auto safeModeToggle = geode::createToggle(
            [manager](bool checked) {
                manager->setSafeMode(checked);
            },
            "Safe Mode",
            manager->isSafeMode()
        );
        safeModeToggle->setPosition(winSize.width / 2, winSize.height / 2 - 30);
        this->m_mainLayer->addChild(safeModeToggle);

        // TODO: Añadir botones para guardar/cargar macros, y lista de macros disponibles

        return true;
    }

public:
    static MacroMenu* create(std::string const& value) {
        auto ret = new MacroMenu();
        if (ret->init(300, 200, value)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Hook para PlayLayer
class $modify(MyPlayLayer, PlayLayer) {
    CCLabelBMFont* m_statusLabel = nullptr;

    bool init(GJGameLevel* level) {
        if (!PlayLayer::init(level)) {
            return false;
        }

        // Inicializar el label de estado
        m_statusLabel = CCLabelBMFont::create("", "goldFont.fnt");
        m_statusLabel->setPosition(CCDirector::get()->getWinSize().width / 2, CCDirector::get()->getWinSize().height - 30);
        m_statusLabel->setScale(0.7f);
        this->addChild(m_statusLabel, 100);

        MacroManager::get()->setCurrentFrame(0);

        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        auto manager = MacroManager::get();
        manager->setCurrentFrame(manager->getCurrentFrame() + 1);

        // Actualizar label de estado
        if (manager->isRecording()) {
            m_statusLabel->setString(fmt::format("Recording: {}", manager->getCurrentFrame()).c_str());
        } else if (manager->isPlaying()) {
            m_statusLabel->setString(fmt::format("Playing: {}", manager->getCurrentFrame()).c_str());
        } else {
            m_statusLabel->setString("");
        }

        // Reproducir inputs
        manager->playInput(manager->getCurrentFrame());
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        auto manager = MacroManager::get();
        
        // Capturar input si estamos grabando
        if (manager->isRecording()) {
            manager->recordInput(manager->getCurrentFrame(), button, down, isPlayer1);
        }

        // Si estamos reproduciendo, bloqueamos los inputs físicos para evitar interferencias
        if (manager->isPlaying()) {
            return;
        }

        PlayLayer::handleButton(down, button, isPlayer1);
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        MacroManager::get()->setCurrentFrame(0);
    }

    void onQuit() {
        PlayLayer::onQuit();
        MacroManager::get()->stopRecording();
        MacroManager::get()->stopPlaying();
    }
};

// Hook para PauseLayer para añadir el botón del menú del mod
class $modify(MyPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::get()->getWinSize();

        auto menuBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("geode-logo.png"), // Usar un sprite de Geode como placeholder
            this,
            menu_selector(MyPauseLayer::onMacroMenu)
        );

        auto menu = this->getChildByID("right-button-menu");
        if (menu) {
            menu->addChild(menuBtn);
            menu->updateLayout();
        }
    }

    void onMacroMenu(CCObject* sender) {
        MacroMenu::create("Macro Menu")->show();
    }
};

// Hook para evitar subir records si Safe Mode está activo y se está reproduciendo
class $modify(MyGJGameLevel, GJGameLevel) {
    void levelComplete() {
        auto manager = MacroManager::get();
        if (manager->isPlaying() && manager->isSafeMode()) {
            log::info("Safe Mode: Preventing score submission during macro playback.");
            // No llamar a la función original para evitar el envío del record
            return;
        }
        GJGameLevel::levelComplete();
    }
};
