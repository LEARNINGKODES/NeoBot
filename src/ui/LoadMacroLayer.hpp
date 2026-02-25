#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../manager/MacroManager.hpp"

using namespace geode::prelude;

// ============================================================
// LoadMacroLayer
// Popup para seleccionar y cargar una macro guardada
// ============================================================
class LoadMacroLayer : public Popup<> {
protected:
    CCMenu* m_listMenu = nullptr;
    CCScrollLayerExt* m_scrollLayer = nullptr;

    bool setup() override {
        this->setTitle("Cargar Macro");

        auto winSize = CCDirector::sharedDirector()->getWinSize();
        auto& mgr = MacroManager::get();
        auto macros = mgr.getSavedMacros();

        // Área de scroll para la lista de macros
        auto scrollSize = CCSize(300.f, 180.f);
        auto scrollNode = CCNode::create();
        scrollNode->setContentSize(scrollSize);
        scrollNode->setPosition(ccp(
            this->m_mainLayer->getContentSize().width / 2 - scrollSize.width / 2,
            this->m_mainLayer->getContentSize().height / 2 - scrollSize.height / 2 - 10.f
        ));

        m_listMenu = CCMenu::create();
        m_listMenu->setPosition(ccp(0, 0));
        m_listMenu->setContentSize(scrollSize);

        if (macros.empty()) {
            auto noMacrosLabel = CCLabelBMFont::create(
                "No hay macros guardadas",
                "bigFont.fnt"
            );
            noMacrosLabel->setScale(0.4f);
            noMacrosLabel->setPosition(ccp(scrollSize.width / 2, scrollSize.height / 2));
            noMacrosLabel->setColor(ccc3(200, 200, 200));
            scrollNode->addChild(noMacrosLabel);
        } else {
            float yPos = scrollSize.height - 20.f;
            for (auto& macroPath : macros) {
                std::string filename = macroPath.filename().string();
                // Quitar la extensión .json del nombre mostrado
                if (filename.size() > 5) filename = filename.substr(0, filename.size() - 5);

                auto bg = CCScale9Sprite::create("square02_001.png");
                bg->setContentSize(CCSize(280.f, 30.f));
                bg->setOpacity(100);

                auto nameLabel = CCLabelBMFont::create(filename.c_str(), "bigFont.fnt");
                nameLabel->setScale(0.35f);
                nameLabel->setAnchorPoint(ccp(0.f, 0.5f));
                nameLabel->setPosition(ccp(10.f, 15.f));
                nameLabel->setColor(ccc3(255, 255, 255));
                bg->addChild(nameLabel);

                // Botón de carga
                auto loadBtn = CCMenuItemSpriteExtra::create(
                    bg, this,
                    menu_selector(LoadMacroLayer::onLoadMacro)
                );
                loadBtn->setPosition(ccp(scrollSize.width / 2, yPos));
                loadBtn->setUserObject(CCString::create(macroPath.string()));
                m_listMenu->addChild(loadBtn);

                yPos -= 35.f;
            }
        }

        scrollNode->addChild(m_listMenu);
        this->m_mainLayer->addChild(scrollNode);

        // Botón de cerrar
        auto closeBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
            this,
            menu_selector(LoadMacroLayer::onClose)
        );
        closeBtn->setPosition(ccp(
            -this->m_mainLayer->getContentSize().width / 2 + 15.f,
             this->m_mainLayer->getContentSize().height / 2 - 15.f
        ));
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition(ccp(
            this->m_mainLayer->getContentSize().width / 2,
            this->m_mainLayer->getContentSize().height / 2
        ));
        closeMenu->addChild(closeBtn);
        this->m_mainLayer->addChild(closeMenu);

        return true;
    }

    void onLoadMacro(CCObject* sender) {
        auto btn = static_cast<CCMenuItemSpriteExtra*>(sender);
        auto pathStr = static_cast<CCString*>(btn->getUserObject())->getCString();
        std::filesystem::path macroPath(pathStr);

        auto& mgr = MacroManager::get();
        if (mgr.loadMacro(macroPath)) {
            Notification::create(
                fmt::format("Macro '{}' cargada", macroPath.filename().string()),
                NotificationIcon::Success,
                2.0f
            )->show();
        } else {
            Notification::create(
                "Error al cargar la macro",
                NotificationIcon::Error,
                2.0f
            )->show();
        }
        this->onClose(nullptr);
    }

    void onClose(CCObject*) {
        this->setKeypadEnabled(false);
        this->setTouchEnabled(false);
        this->removeFromParentAndCleanup(true);
    }

public:
    static LoadMacroLayer* create() {
        auto ret = new LoadMacroLayer();
        if (ret->initAnchored(340.f, 260.f)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void open() {
        auto layer = LoadMacroLayer::create();
        if (layer) {
            auto scene = CCDirector::sharedDirector()->getRunningScene();
            scene->addChild(layer, 200);
        }
    }
};
