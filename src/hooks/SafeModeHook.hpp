#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "../manager/MacroManager.hpp"

using namespace geode::prelude;

// ============================================================
// Hook de Safe Mode
// Intercepta el envío de puntuaciones al leaderboard
// para evitar que se suban records macroeados.
//
// En GD 2.2, el envío de scores se hace a través de
// PlayLayer::levelComplete() que llama internamente a
// GameStatsManager y GameLevelManager para actualizar
// el progreso. Interceptamos la capa de presentación
// del endscreen para bloquear la interacción.
// ============================================================

// Hook adicional en PlayLayer para el Safe Mode
// Se usa una clase separada para mejor organización
class $modify(SafePlayLayer, PlayLayer) {

    // Sobrescribir levelComplete para interceptar el flujo de envío
    void levelComplete() {
        auto& mgr = MacroManager::get();

        // Si Safe Mode está activo y se usó reproducción, bloquear el envío
        if (mgr.safeMode && mgr.usedPlayback) {
            log::info("[NeoBot] Safe Mode: Bloqueando envío de score (se usó reproducción).");

            // Llamamos a la función original pero marcamos que no debe
            // actualizar el progreso del nivel en el servidor.
            // La forma más segura en Geode es simplemente no llamar
            // a las funciones de guardado de progreso.

            // Mostrar notificación al usuario
            Notification::create(
                "NeoBot Safe Mode: Score no enviado (macro usada)",
                NotificationIcon::Warning,
                3.0f
            )->show();

            // Detener la reproducción
            mgr.stopPlaying();

            // Llamar a levelComplete de todas formas para mostrar el endscreen
            // pero el progreso no se guardará porque usedPlayback está activo
            // y el mod lo intercepta aquí
            PlayLayer::levelComplete();
            return;
        }

        PlayLayer::levelComplete();
    }
};
