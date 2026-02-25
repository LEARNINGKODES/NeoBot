#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include "../manager/MacroManager.hpp"
#include "../ui/NeoBotUI.hpp"

using namespace geode::prelude;

// ============================================================
// Hook de GJBaseGameLayer
// Intercepta todos los inputs del jugador (táctiles en Android)
// ============================================================
class $modify(GJBaseGameLayer) {

    // handleButton es el método que GD llama internamente cuando
    // un input es procesado (ya sea táctil, teclado, etc.)
    // En Android, el sistema táctil de Cocos2d-x se traduce aquí.
    void handleButton(bool press, int button, bool player1) {
        auto& mgr = MacroManager::get();

        // Durante la grabación: registrar el input antes de procesarlo
        if (mgr.state == BotState::Recording) {
            bool isPlayer2 = !player1;
            mgr.recordInput(press, isPlayer2, button);
        }

        // Durante la reproducción: bloquear los inputs del usuario
        // para que no interfieran con la macro
        if (mgr.state == BotState::Playing) {
            // No llamamos a la función original; la macro controla los inputs
            return;
        }

        // En cualquier otro estado, procesamos el input normalmente
        GJBaseGameLayer::handleButton(press, button, player1);
    }
};

// ============================================================
// Hook de PlayLayer
// Gestiona el ciclo de vida del nivel y la reproducción frame-perfect
// ============================================================
class $modify(PlayLayer) {

    // --------------------------------------------------------
    // init: Preparar el mod cuando se inicia un nivel
    // --------------------------------------------------------
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto& mgr = MacroManager::get();
        mgr.resetForLevel();

        // Añadir indicador visual de estado (Recording / Playing)
        NeoBotUI::addStateLabel(this);

        return true;
    }

    // --------------------------------------------------------
    // update: El corazón de la reproducción frame-perfect
    // Se llama en cada frame del juego
    // --------------------------------------------------------
    void update(float dt) {
        auto& mgr = MacroManager::get();

        // Incrementar el contador de frames del mod
        // Nota: m_currentFrame es el contador interno de GD
        // Usamos el nuestro para mayor control
        if (mgr.state == BotState::Recording || mgr.state == BotState::Playing) {
            mgr.currentFrame = this->m_currentFrame;
        }

        // Lógica de reproducción: ejecutar inputs del frame actual
        // ANTES de llamar a la función original de update
        if (mgr.state == BotState::Playing) {
            auto inputs = mgr.getInputsForFrame(mgr.currentFrame);
            for (auto& inp : inputs) {
                // Forzar el estado interno del jugador directamente
                // Esto es más confiable que simular eventos táctiles del sistema
                if (!inp.player2) {
                    if (inp.press) {
                        this->m_player1->pushButton(static_cast<PlayerButton>(inp.button));
                    } else {
                        this->m_player1->releaseButton(static_cast<PlayerButton>(inp.button));
                    }
                } else {
                    if (inp.press) {
                        this->m_player2->pushButton(static_cast<PlayerButton>(inp.button));
                    } else {
                        this->m_player2->releaseButton(static_cast<PlayerButton>(inp.button));
                    }
                }
                log::debug("[NeoBot] Reproduciendo: frame={}, press={}, p2={}", inp.frame, inp.press, inp.player2);
            }

            // Detener reproducción si se acabaron los inputs
            if (mgr.playbackIndex >= mgr.currentMacro.inputs.size() && !mgr.currentMacro.inputs.empty()) {
                // Solo detenemos si ya pasamos el último frame grabado
                if (!mgr.currentMacro.inputs.empty() &&
                    mgr.currentFrame > mgr.currentMacro.inputs.back().frame) {
                    mgr.stopPlaying();
                    NeoBotUI::updateStateLabel(this);
                    log::info("[NeoBot] Reproducción completada automáticamente.");
                }
            }
        }

        // Llamar a la función original de update
        PlayLayer::update(dt);

        // Actualizar el indicador visual
        if (mgr.state == BotState::Recording || mgr.state == BotState::Playing) {
            NeoBotUI::updateStateLabel(this);
        }
    }

    // --------------------------------------------------------
    // resetLevel: Reiniciar el estado del mod al reiniciar
    // --------------------------------------------------------
    void resetLevel() {
        auto& mgr = MacroManager::get();

        // Reiniciar el puntero de reproducción y el frame counter
        mgr.currentFrame = 0;
        mgr.playbackIndex = 0;

        // Llamar a la función original
        PlayLayer::resetLevel();

        // Actualizar UI
        NeoBotUI::updateStateLabel(this);
    }

    // --------------------------------------------------------
    // levelComplete: Al completar el nivel
    // --------------------------------------------------------
    void levelComplete() {
        auto& mgr = MacroManager::get();

        // Auto-guardar si estaba grabando
        if (mgr.state == BotState::Recording) {
            std::string levelName = "Unknown";
            if (this->m_level) {
                levelName = this->m_level->m_levelName;
            }
            mgr.stopRecording();
            mgr.tryAutoSave(levelName);
        }

        if (mgr.state == BotState::Playing) {
            mgr.stopPlaying();
        }

        PlayLayer::levelComplete();
    }

    // --------------------------------------------------------
    // onQuit: Al salir del nivel
    // --------------------------------------------------------
    void onQuit() {
        auto& mgr = MacroManager::get();

        // Auto-guardar si estaba grabando al salir
        if (mgr.state == BotState::Recording) {
            std::string levelName = "Unknown";
            if (this->m_level) {
                levelName = this->m_level->m_levelName;
            }
            mgr.stopRecording();
            mgr.tryAutoSave(levelName);
        }

        if (mgr.state == BotState::Playing) {
            mgr.stopPlaying();
        }

        PlayLayer::onQuit();
    }
};
