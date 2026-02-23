#pragma once

#include <vector>
#include <string>
#include <map>
#include <gd.h>

// Estructura para almacenar un evento de input
struct FrameInput {
    int frame;
    int button;
    bool isPress;
    bool isPlayer1;

    // Para serialización
    void to_json(nlohmann::json& j) const {
        j = nlohmann::json{
            {"frame", frame},
            {"button", button},
            {"isPress", isPress},
            {"isPlayer1", isPlayer1}
        };
    }

    void from_json(const nlohmann::json& j) {
        j.at("frame").get_to(frame);
        j.at("button").get_to(button);
        j.at("isPress").get_to(isPress);
        j.at("isPlayer1").get_to(isPlayer1);
    }
};

// Clase para gestionar las macros
class MacroManager {
public:
    static MacroManager* get();

    void recordInput(int frame, int button, bool isPress, bool isPlayer1);
    void playInput(int frame);
    void clearMacro();

    void startRecording();
    void stopRecording();
    bool isRecording() const { return m_isRecording; }

    void startPlaying();
    void stopPlaying();
    bool isPlaying() const { return m_isPlaying; }

    void setSafeMode(bool safeMode) { m_safeMode = safeMode; }
    bool isSafeMode() const { return m_safeMode; }

    void saveMacro(const std::string& name);
    void loadMacro(const std::string& name);
    std::vector<std::string> getAvailableMacros();

    void setCurrentFrame(int frame) { m_currentFrame = frame; }
    int getCurrentFrame() const { return m_currentFrame; }

private:
    MacroManager();
    static MacroManager* s_instance;

    std::vector<FrameInput> m_currentMacro;
    std::map<int, std::vector<FrameInput>> m_inputsByFrame;
    int m_currentFrame;
    bool m_isRecording;
    bool m_isPlaying;
    bool m_safeMode;

    std::string getMacroFilePath(const std::string& name);
};
