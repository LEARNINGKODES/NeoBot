/**
 * NeoBot - Mod de macros frame-perfect para Geometry Dash 2.2081
 * Plataforma: Android (ARM) con Geode
 *
 * ============================================================
 * CÓMO FUNCIONA EL SISTEMA INTERNAMENTE
 * ============================================================
 *
 * 1. GRABACIÓN:
 *    El hook en GJBaseGameLayer::handleButton intercepta todos los
 *    inputs del jugador (táctiles en Android, procesados por Cocos2d-x).
 *    Cada vez que el jugador presiona o suelta la pantalla, se registra
 *    el evento con el número de frame actual de PlayLayer (m_currentFrame).
 *    Estos eventos se almacenan en un vector de TouchInput dentro de
 *    MacroManager.
 *
 * 2. REPRODUCCIÓN:
 *    El hook en PlayLayer::update se ejecuta en cada frame del juego.
 *    Antes de llamar al update original, se consulta MacroManager para
 *    obtener los inputs que corresponden al frame actual. Si hay inputs,
 *    se llaman directamente a PlayerObject::pushButton y releaseButton,
 *    modificando el estado interno del jugador sin simular eventos del
 *    sistema Android. Esto garantiza reproducción frame-perfect.
 *    Los inputs del usuario se bloquean durante la reproducción para
 *    evitar interferencias.
 *
 * 3. SERIALIZACIÓN:
 *    Las macros se guardan en formato JSON usando matjson (incluido en
 *    Geode). Los archivos se almacenan en el directorio de guardado del
 *    mod (Mod::get()->getSaveDir() / "macros").
 *
 * 4. SAFE MODE:
 *    Una bandera (usedPlayback) se activa cuando se inicia la reproducción.
 *    Si Safe Mode está habilitado y esta bandera está activa, se bloquea
 *    el envío de scores al leaderboard y se notifica al usuario.
 *
 * ============================================================
 * ESTRUCTURA DEL PROYECTO
 * ============================================================
 *
 *  NeoBot/
 *  ├── CMakeLists.txt
 *  ├── mod.json
 *  ├── src/
 *  │   ├── main.cpp                    <- Este archivo
 *  │   ├── manager/
 *  │   │   └── MacroManager.hpp        <- Lógica central del bot
 *  │   ├── hooks/
 *  │   │   ├── PlayLayerHook.hpp       <- Hooks de PlayLayer y GJBaseGameLayer
 *  │   │   ├── PauseLayerHook.hpp      <- Botón en el menú de pausa
 *  │   │   └── SafeModeHook.hpp        <- Bloqueo de leaderboard
 *  │   └── ui/
 *  │       ├── NeoBotUI.hpp            <- Indicador visual en PlayLayer
 *  │       ├── NeoBotMenuLayer.hpp     <- Menú principal del mod
 *  │       ├── SaveMacroLayer.hpp      <- Popup para guardar macros
 *  │       └── LoadMacroLayer.hpp      <- Popup para cargar macros
 *  └── resources/
 *      └── (vacío - no se requieren recursos externos)
 *
 * ============================================================
 * CANAL DE YOUTUBE: https://youtube.com/@neomc12
 * ============================================================
 */

#include <Geode/Geode.hpp>

// Incluir el gestor central
#include "manager/MacroManager.hpp"

// Incluir todos los hooks
#include "hooks/PlayLayerHook.hpp"
#include "hooks/PauseLayerHook.hpp"
#include "hooks/SafeModeHook.hpp"

using namespace geode::prelude;

// ============================================================
// Inicialización del mod
// ============================================================
$on_mod(Loaded) {
    auto& mgr = MacroManager::get();

    // Cargar configuración guardada
    mgr.safeMode = Mod::get()->getSavedValue<bool>("safe_mode", true);
    mgr.autoSave = Mod::get()->getSavedValue<bool>("auto_save", true);

    // Asegurar que el directorio de macros existe
    auto macrosDir = Mod::get()->getSaveDir() / "macros";
    if (!std::filesystem::exists(macrosDir)) {
        std::filesystem::create_directories(macrosDir);
    }

    log::info("NeoBot cargado correctamente.");
    log::info("Directorio de macros: {}", macrosDir.string());
    log::info("Safe Mode: {}", mgr.safeMode ? "Activado" : "Desactivado");
    log::info("Auto-Guardar: {}", mgr.autoSave ? "Activado" : "Desactivado");
    log::info("Canal de YouTube: https://youtube.com/@neomc12");
}
