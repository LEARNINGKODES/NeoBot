#pragma once

#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include <filesystem>

using namespace geode::prelude;

// ============================================================
// Estructuras de datos del macro
// ============================================================

// Representa un único evento de input táctil grabado
struct TouchInput {
    int frame;      // Frame exacto desde el inicio del nivel
    bool press;     // true = presionar, false = soltar
    bool player2;   // true = jugador 2
    int button;     // 1 = jump (por defecto en Android)
};

// Contiene toda la información de una macro grabada
struct Macro {
    std::string name        = "Sin nombre";
    std::string author      = "NeoMC";
    float gameVersion       = 2.2081f;
    float framerate         = 240.0f;
    std::vector<TouchInput> inputs;

    void clear() {
        inputs.clear();
        name = "Sin nombre";
    }
};

// Estados posibles del mod
enum class BotState {
    None,       // Inactivo
    Recording,  // Grabando
    Playing     // Reproduciendo
};

// ============================================================
// Clase principal MacroManager (Singleton)
// ============================================================
class MacroManager {
public:
    // Singleton
    static MacroManager& get() {
        static MacroManager instance;
        return instance;
    }

    // Estado actual del bot
    BotState state = BotState::None;

    // La macro actualmente cargada / grabándose
    Macro currentMacro;

    // Índice de reproducción (qué input se debe reproducir a continuación)
    size_t playbackIndex = 0;

    // Indica si se ha usado reproducción en el intento actual (para Safe Mode)
    bool usedPlayback = false;

    // Safe Mode: evita subir records si se usó reproducción
    bool safeMode = true;

    // Auto-guardado habilitado
    bool autoSave = true;

    // Frame actual del nivel (sincronizado con PlayLayer)
    int currentFrame = 0;

    // ============================================================
    // Control de estado
    // ============================================================

    void startRecording() {
        currentMacro.clear();
        currentFrame = 0;
        state = BotState::Recording;
        log::info("[NeoBot] Grabación iniciada.");
    }

    void stopRecording() {
        state = BotState::None;
        log::info("[NeoBot] Grabación detenida. Inputs grabados: {}", currentMacro.inputs.size());
    }

    void startPlaying() {
        playbackIndex = 0;
        currentFrame = 0;
        usedPlayback = true;
        state = BotState::Playing;
        log::info("[NeoBot] Reproducción iniciada. Inputs a reproducir: {}", currentMacro.inputs.size());
    }

    void stopPlaying() {
        state = BotState::None;
        playbackIndex = 0;
        log::info("[NeoBot] Reproducción detenida.");
    }

    void resetState() {
        state = BotState::None;
        playbackIndex = 0;
        currentFrame = 0;
    }

    void resetForLevel() {
        currentFrame = 0;
        playbackIndex = 0;
        usedPlayback = false;
    }

    // ============================================================
    // Grabación de inputs
    // ============================================================

    // Llamado desde el hook de GJBaseGameLayer::handleButton
    void recordInput(bool press, bool player2, int button = 1) {
        if (state != BotState::Recording) return;
        TouchInput input;
        input.frame   = currentFrame;
        input.press   = press;
        input.player2 = player2;
        input.button  = button;
        currentMacro.inputs.push_back(input);
        log::debug("[NeoBot] Input grabado: frame={}, press={}, p2={}", currentFrame, press, player2);
    }

    // ============================================================
    // Reproducción de inputs
    // ============================================================

    // Llamado desde el hook de PlayLayer::update en cada frame
    // Devuelve los inputs que deben ejecutarse en el frame actual
    std::vector<TouchInput> getInputsForFrame(int frame) {
        std::vector<TouchInput> result;
        if (state != BotState::Playing) return result;

        while (playbackIndex < currentMacro.inputs.size() &&
               currentMacro.inputs[playbackIndex].frame == frame) {
            result.push_back(currentMacro.inputs[playbackIndex]);
            playbackIndex++;
        }
        return result;
    }

    // ============================================================
    // Serialización (Guardar / Cargar)
    // ============================================================

    // Guarda la macro actual en un archivo JSON
    bool saveMacro(const std::filesystem::path& path) {
        try {
            auto j = matjson::Value::object();
            j["name"]        = currentMacro.name;
            j["author"]      = currentMacro.author;
            j["gameVersion"] = (double)currentMacro.gameVersion;
            j["framerate"]   = (double)currentMacro.framerate;

            auto inputsArr = matjson::Value::array();
            for (auto& inp : currentMacro.inputs) {
                auto obj = matjson::Value::object();
                obj["frame"]   = inp.frame;
                obj["press"]   = inp.press;
                obj["player2"] = inp.player2;
                obj["button"]  = inp.button;
                inputsArr.push(obj);
            }
            j["inputs"] = inputsArr;

            std::ofstream file(path);
            if (!file.is_open()) {
                log::error("[NeoBot] No se pudo abrir el archivo para guardar: {}", path.string());
                return false;
            }
            file << j.dump(4);
            file.close();
            log::info("[NeoBot] Macro guardada en: {}", path.string());
            return true;
        } catch (const std::exception& e) {
            log::error("[NeoBot] Error al guardar macro: {}", e.what());
            return false;
        }
    }

    // Carga una macro desde un archivo JSON
    bool loadMacro(const std::filesystem::path& path) {
        try {
            std::ifstream file(path);
            if (!file.is_open()) {
                log::error("[NeoBot] No se pudo abrir el archivo: {}", path.string());
                return false;
            }
            std::string content((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
            file.close();

            auto parseResult = matjson::parse(content);
            if (!parseResult) {
                log::error("[NeoBot] Error al parsear JSON.");
                return false;
            }
            auto& j = parseResult.value();

            currentMacro.clear();
            if (j.contains("name"))        currentMacro.name        = j["name"].asString().value_or("Sin nombre");
            if (j.contains("author"))      currentMacro.author      = j["author"].asString().value_or("NeoMC");
            if (j.contains("gameVersion")) currentMacro.gameVersion = (float)j["gameVersion"].asDouble().value_or(2.2081);
            if (j.contains("framerate"))   currentMacro.framerate   = (float)j["framerate"].asDouble().value_or(240.0);

            if (j.contains("inputs") && j["inputs"].isArray()) {
                for (auto& inp : j["inputs"].asArray().value()) {
                    TouchInput ti;
                    ti.frame   = inp["frame"].asInt().value_or(0);
                    ti.press   = inp["press"].asBool().value_or(true);
                    ti.player2 = inp["player2"].asBool().value_or(false);
                    ti.button  = inp["button"].asInt().value_or(1);
                    currentMacro.inputs.push_back(ti);
                }
            }
            log::info("[NeoBot] Macro cargada: {} inputs desde {}", currentMacro.inputs.size(), path.string());
            return true;
        } catch (const std::exception& e) {
            log::error("[NeoBot] Error al cargar macro: {}", e.what());
            return false;
        }
    }

    // Auto-guardado con nombre automático basado en el nivel
    void tryAutoSave(const std::string& levelName) {
        if (!autoSave) return;
        if (currentMacro.inputs.empty()) return;

        auto saveDir = Mod::get()->getSaveDir() / "macros";
        if (!std::filesystem::exists(saveDir)) {
            std::filesystem::create_directories(saveDir);
        }
        std::string filename = levelName + "_autosave.json";
        // Reemplazar caracteres inválidos en el nombre de archivo
        for (char& c : filename) {
            if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|')
                c = '_';
        }
        saveMacro(saveDir / filename);
    }

    // Obtiene la lista de macros guardadas
    std::vector<std::filesystem::path> getSavedMacros() {
        std::vector<std::filesystem::path> result;
        auto saveDir = Mod::get()->getSaveDir() / "macros";
        if (!std::filesystem::exists(saveDir)) return result;
        for (auto& entry : std::filesystem::directory_iterator(saveDir)) {
            if (entry.path().extension() == ".json") {
                result.push_back(entry.path());
            }
        }
        return result;
    }

private:
    MacroManager() = default;
    MacroManager(const MacroManager&) = delete;
    MacroManager& operator=(const MacroManager&) = delete;
};
