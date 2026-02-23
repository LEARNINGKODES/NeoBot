#include "MacroManager.hpp"
#include <Geode/Geode.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace geode::prelude;

MacroManager* MacroManager::s_instance = nullptr;

MacroManager* MacroManager::get() {
    if (!s_instance) {
        s_instance = new MacroManager();
    }
    return s_instance;
}

MacroManager::MacroManager()
    : m_currentFrame(0),
      m_isRecording(false),
      m_isPlaying(false),
      m_safeMode(true) {}

void MacroManager::recordInput(int frame, int button, bool isPress, bool isPlayer1) {
    if (m_isRecording) {
        m_currentMacro.push_back({frame, button, isPress, isPlayer1});
    }
}

void MacroManager::playInput(int frame) {
    if (m_isPlaying) {
        if (m_inputsByFrame.count(frame)) {
            for (const auto& input : m_inputsByFrame[frame]) {
                // Forzar el estado interno del botón de salto
                // Esto es un placeholder, la implementación real dependerá de cómo GD maneja los inputs
                // y si podemos acceder directamente a PlayerObject o necesitamos hookear GJBaseGameLayer::handleButton
                auto playLayer = PlayLayer::get();
                if (playLayer) {
                    auto player1 = playLayer->m_player1;
                    auto player2 = playLayer->m_player2;

                    PlayerObject* targetPlayer = nullptr;
                    if (input.isPlayer1) {
                        targetPlayer = player1;
                    } else {
                        targetPlayer = player2;
                    }

                    if (targetPlayer) {
                        if (input.isPress) {
                            targetPlayer->pushButton(static_cast<PlayerButton>(input.button));
                        } else {
                            targetPlayer->releaseButton(static_cast<PlayerButton>(input.button));
                        }
                    }
                }
            }
        }
    }
}

void MacroManager::clearMacro() {
    m_currentMacro.clear();
    m_inputsByFrame.clear();
}

void MacroManager::startRecording() {
    clearMacro();
    m_isRecording = true;
    m_isPlaying = false;
    m_currentFrame = 0;
    log::info("Recording started.");
}

void MacroManager::stopRecording() {
    m_isRecording = false;
    // Organizar inputs por frame para una reproducción eficiente
    for (const auto& input : m_currentMacro) {
        m_inputsByFrame[input.frame].push_back(input);
    }
    log::info("Recording stopped. Recorded {} inputs.", m_currentMacro.size());
}

void MacroManager::startPlaying() {
    m_isPlaying = true;
    m_isRecording = false;
    m_currentFrame = 0;
    log::info("Playing started.");
}

void MacroManager::stopPlaying() {
    m_isPlaying = false;
    log::info("Playing stopped.");
}

std::string MacroManager::getMacroFilePath(const std::string& name) {
    auto configDir = Mod::get()->getConfigDir();
    return configDir.string() + "/macros/" + name + ".json";
}

void MacroManager::saveMacro(const std::string& name) {
    if (m_currentMacro.empty()) {
        log::warn("Cannot save empty macro.");
        return;
    }

    nlohmann::json j;
    for (const auto& input : m_currentMacro) {
        nlohmann::json input_json;
        input.to_json(input_json);
        j["inputs"].push_back(input_json);
    }

    auto filePath = getMacroFilePath(name);
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
    std::ofstream o(filePath);
    o << std::setw(4) << j << std::endl;
    log::info("Macro saved to {}.", filePath);
}

void MacroManager::loadMacro(const std::string& name) {
    auto filePath = getMacroFilePath(name);
    std::ifstream i(filePath);
    if (!i.is_open()) {
        log::error("Failed to open macro file: {}.", filePath);
        return;
    }

    nlohmann::json j;
    i >> j;

    clearMacro();
    for (const auto& input_json : j["inputs"]) {
        FrameInput input;
        input.from_json(input_json);
        m_currentMacro.push_back(input);
        m_inputsByFrame[input.frame].push_back(input);
    }
    log::info("Macro loaded from {}. Loaded {} inputs.", filePath, m_currentMacro.size());
}

std::vector<std::string> MacroManager::getAvailableMacros() {
    std::vector<std::string> macroNames;
    auto configDir = Mod::get()->getConfigDir();
    auto macrosPath = configDir.string() + "/macros/";

    if (std::filesystem::exists(macrosPath) && std::filesystem::is_directory(macrosPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(macrosPath)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                macroNames.push_back(entry.path().stem().string());
            }
        }
    }
    return macroNames;
}
