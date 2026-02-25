#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "../manager/MacroManager.hpp"

using namespace geode::prelude;

// ============================================================
// SaveMacroLayer
// Popup para guardar la macro actual con un nombre personalizado
// ============================================================
class SaveMacroLayer : public Popup<> {
protected:
    TextInput* m_nameInput = nullptr;

    bool setup() override {
        this->setTitle("Guardar Macro");

        auto& mgr = MacroManager::get();
        auto contentSize = this->m_mainLayer->getContentSize();
        float cx = contentSize.width / 2;
        float cy = contentSize.height / 2;

        // Información de la macro actual
        auto infoLabel = CCLabelBMFont::create(
            fmt::format("Inputs grabados: {}", mgr.currentMacro.inputs.size()).c_str(),
            "bigFont.fnt"
        );
        infoLabel->setScale(0.4f);
        infoLabel->setPosition(ccp(cx, cy + 50.f));
        infoLabel->setColor(ccc3(200, 200, 200));
        this->m_mainLayer->addChild(infoLabel);

        // Label de instrucción
        auto nameLabel = CCLabelBMFont::create("Nombre del archivo:", "bigFont.fnt");
        nameLabel->setScale(0.4f);
        nameLabel->setPosition(ccp(cx, cy + 20.f));
        this->m_mainLayer->addChild(nameLabel);

        // Campo de texto para el nombre
        m_nameInput = TextInput::create(240.f, "mi_macro");
        m_nameInput->setPosition(ccp(cx, cy - 15.f));
        m_nameInput->setString(mgr.currentMacro.name);
        this->m_mainLayer->addChild(m_nameInput);

        // Botón Guardar
        auto saveSprite = ButtonSprite::create("Guardar", "bigFont.fnt", "GJ_button_01.png");
        auto saveBtn = CCMenuItemSpriteExtra::create(
            saveSprite, this,
            menu_selector(SaveMacroLayer::onSave)
        );

        // Botón Cancelar
        auto cancelSprite = ButtonSprite::create("Cancelar", "bigFont.fnt", "GJ_button_06.png");
        auto cancelBtn = CCMenuItemSpriteExtra::create(
            cancelSprite, this,
            menu_selector(SaveMacroLayer::onClose)
        );

        auto btnMenu = CCMenu::create();
        btnMenu->setPosition(ccp(cx, cy - 55.f));
        saveBtn->setPosition(ccp(-60.f, 0.f));
        cancelBtn->setPosition(ccp(60.f, 0.f));
        btnMenu->addChild(saveBtn);
        btnMenu->addChild(cancelBtn);
        this->m_mainLayer->addChild(btnMenu);

        // Botón de cerrar (X)
        auto closeBtn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
            this,
            menu_selector(SaveMacroLayer::onClose)
        );
        auto closeMenu = CCMenu::create();
        closeMenu->setPosition(ccp(cx, cy));
        closeBtn->setPosition(ccp(-cx + 15.f, cy - 15.f));
        closeMenu->addChild(closeBtn);
        this->m_mainLayer->addChild(closeMenu);

        return true;
    }

    void onSave(CCObject*) {
        auto& mgr = MacroManager::get();

        if (mgr.currentMacro.inputs.empty()) {
            Notification::create(
                "No hay inputs grabados para guardar",
                NotificationIcon::Warning,
                2.0f
            )->show();
            return;
        }

        std::string name = m_nameInput->getString();
        if (name.empty()) name = "mi_macro";

        // Actualizar el nombre en la macro
        mgr.currentMacro.name = name;

        // Crear directorio si no existe
        auto saveDir = Mod::get()->getSaveDir() / "macros";
        if (!std::filesystem::exists(saveDir)) {
            std::filesystem::create_directories(saveDir);
        }

        // Sanitizar el nombre del archivo
        std::string filename = name + ".json";
        for (char& c : filename) {
            if (c == '/' || c == '\\' || c == ':' || c == '*' ||
                c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
                c = '_';
        }

        auto savePath = saveDir / filename;
        if (mgr.saveMacro(savePath)) {
            Notification::create(
                fmt::format("Macro '{}' guardada", name),
                NotificationIcon::Success,
                2.0f
            )->show();
        } else {
            Notification::create(
                "Error al guardar la macro",
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
    static SaveMacroLayer* create() {
        auto ret = new SaveMacroLayer();
        if (ret->initAnchored(300.f, 200.f)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
        return nullptr;
    }

    static void open() {
        auto layer = SaveMacroLayer::create();
        if (layer) {
            auto scene = CCDirector::sharedDirector()->getRunningScene();
            scene->addChild(layer, 200);
        }
    }
};
