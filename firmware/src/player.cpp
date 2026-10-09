#include "player.h"
#include "pins.h"
#include <AudioFileSourceBuffer.h>
#include <AudioFileSourceSD.h>
#include <AudioGenerator.h>
#include <AudioGeneratorAAC.h>
#include <AudioGeneratorFLAC.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>

enum class AudioFormat {
    MP3,
    AAC,
    FLAC,
    WAV,
    UNKNOWN
};

enum class PlayerState : uint8_t {
    STOPPED,
    STARTING,
    PLAYING,
    PAUSED
};

enum class PlayerCommandType : uint8_t {
    PLAY,
    STOP,
    TOGGLE_PAUSE,
    SET_VOLUME
};

struct PlayerCommand {
    PlayerCommandType type;
    char path[96];
    int value;
};

static QueueHandle_t commandQueue = nullptr;
static TaskHandle_t audioTaskHandle = nullptr;

static AudioOutputI2S* output = nullptr;
static AudioFileSourceSD* source = nullptr;
static AudioFileSourceBuffer* buffer = nullptr;
static AudioGenerator* decoder = nullptr;

static portMUX_TYPE stateMux = portMUX_INITIALIZER_UNLOCKED;
static PlayerState state = PlayerState::STOPPED;
static int volume = 70;
static uint32_t startedAt = 0;
static uint32_t pausedAt = 0;
static uint32_t pausedTotal = 0;

static AudioFormat detectFormat(const char* path) {
    const char* ext = strrchr(path, '.');
    if (!ext) return AudioFormat::UNKNOWN;

    char lower[8];
    strlcpy(lower, ext, sizeof(lower));
    for (size_t i = 0; lower[i]; i++) {
        lower[i] = tolower(lower[i]);
    }

    if (strcmp(lower, ".mp3") == 0) return AudioFormat::MP3;
    if (strcmp(lower, ".aac") == 0) return AudioFormat::AAC;
    if (strcmp(lower, ".flac") == 0) return AudioFormat::FLAC;
    if (strcmp(lower, ".wav") == 0) return AudioFormat::WAV;
    return AudioFormat::UNKNOWN;
}

static const char* formatName(AudioFormat format) {
    switch (format) {
        case AudioFormat::MP3: return "MP3";
        case AudioFormat::AAC: return "AAC";
        case AudioFormat::FLAC: return "FLAC";
        case AudioFormat::WAV: return "WAV";
        default: return "UNKNOWN";
    }
}

static AudioGenerator* createDecoder(AudioFormat format) {
    switch (format) {
        case AudioFormat::MP3: return new AudioGeneratorMP3();
        case AudioFormat::AAC: return new AudioGeneratorAAC();
        case AudioFormat::FLAC: return new AudioGeneratorFLAC();
        case AudioFormat::WAV: return new AudioGeneratorWAV();
        default: return nullptr;
    }
}

static void setState(PlayerState next) {
    portENTER_CRITICAL(&stateMux);
    state = next;
    portEXIT_CRITICAL(&stateMux);
}

static PlayerState getState() {
    portENTER_CRITICAL(&stateMux);
    PlayerState snapshot = state;
    portEXIT_CRITICAL(&stateMux);
    return snapshot;
}

static void setTiming(uint32_t start, uint32_t pauseAt, uint32_t pauseTotal) {
    portENTER_CRITICAL(&stateMux);
    startedAt = start;
    pausedAt = pauseAt;
    pausedTotal = pauseTotal;
    portEXIT_CRITICAL(&stateMux);
}

static void cleanupPlayback() {
    if (decoder) {
        if (decoder->isRunning()) decoder->stop();
        delete decoder;
        decoder = nullptr;
    }
    if (buffer) {
        delete buffer;
        buffer = nullptr;
    }
    if (source) {
        delete source;
        source = nullptr;
    }
}

static bool startPlaybackNow(const char* path) {
    cleanupPlayback();

    AudioFormat format = detectFormat(path);
    decoder = createDecoder(format);
    if (!decoder) {
        Serial.printf("Unsupported audio format: %s\n", path);
        return false;
    }

    source = new AudioFileSourceSD(path);
    if (!source->isOpen()) {
        Serial.printf("Failed to open: %s\n", path);
        cleanupPlayback();
        return false;
    }

    buffer = new AudioFileSourceBuffer(source, 16384);
    if (!decoder->begin(buffer, output)) {
        Serial.printf("Failed to start %s decoder: %s\n", formatName(format), path);
        cleanupPlayback();
        return false;
    }

    setTiming(millis(), 0, 0);
    Serial.printf("Playing %s: %s\n", formatName(format), path);
    return true;
}

static void handleCommand(const PlayerCommand& cmd) {
    switch (cmd.type) {
        case PlayerCommandType::PLAY:
            setState(PlayerState::STARTING);
            setState(startPlaybackNow(cmd.path) ? PlayerState::PLAYING : PlayerState::STOPPED);
            break;

        case PlayerCommandType::STOP:
            cleanupPlayback();
            setState(PlayerState::STOPPED);
            break;

        case PlayerCommandType::TOGGLE_PAUSE: {
            PlayerState current = getState();
            if (!decoder || current == PlayerState::STOPPED || current == PlayerState::STARTING) return;

            if (current == PlayerState::PAUSED) {
                portENTER_CRITICAL(&stateMux);
                pausedTotal += millis() - pausedAt;
                portEXIT_CRITICAL(&stateMux);
                setState(PlayerState::PLAYING);
            } else {
                portENTER_CRITICAL(&stateMux);
                pausedAt = millis();
                portEXIT_CRITICAL(&stateMux);
                setState(PlayerState::PAUSED);
            }
            break;
        }

        case PlayerCommandType::SET_VOLUME:
            volume = constrain(cmd.value, 0, 100);
            if (output) output->SetGain(volume / 100.0f);
            break;
    }
}

static void audioTask(void*) {
    output = new AudioOutputI2S();
    output->SetPinout(I2S_BCLK, I2S_LRCLK, I2S_DOUT);
    output->SetGain(volume / 100.0f);

    PlayerCommand cmd;
    while (true) {
        while (xQueueReceive(commandQueue, &cmd, 0) == pdTRUE) {
            handleCommand(cmd);
        }

        if (decoder && getState() == PlayerState::PLAYING) {
            if (decoder->isRunning()) {
                if (!decoder->loop()) {
                    Serial.println("Track ended");
                    cleanupPlayback();
                    setState(PlayerState::STOPPED);
                }
            } else {
                cleanupPlayback();
                setState(PlayerState::STOPPED);
            }
            vTaskDelay(1);
        } else {
            vTaskDelay(pdMS_TO_TICKS(2));
        }
    }
}

static bool sendCommand(const PlayerCommand& cmd) {
    if (!commandQueue) return false;
    return xQueueSend(commandQueue, &cmd, pdMS_TO_TICKS(50)) == pdTRUE;
}

void initPlayer() {
    commandQueue = xQueueCreate(8, sizeof(PlayerCommand));
    xTaskCreatePinnedToCore(audioTask, "audio", 8192, nullptr, 3, &audioTaskHandle, 1);
}

bool playTrack(const char* path) {
    PlayerCommand cmd = {};
    cmd.type = PlayerCommandType::PLAY;
    strlcpy(cmd.path, path, sizeof(cmd.path));

    setState(PlayerState::STARTING);
    if (commandQueue) {
        xQueueReset(commandQueue);
    }
    return sendCommand(cmd);
}

void stopPlayback() {
    PlayerCommand cmd = {};
    cmd.type = PlayerCommandType::STOP;
    setState(PlayerState::STOPPED);
    sendCommand(cmd);
}

void togglePause() {
    PlayerCommand cmd = {};
    cmd.type = PlayerCommandType::TOGGLE_PAUSE;
    sendCommand(cmd);
}

void updatePlayer() {
    // Playback is serviced by the audio task.
}

bool playerIsPlaying() {
    PlayerState current = getState();
    return current == PlayerState::STARTING || current == PlayerState::PLAYING;
}

bool playerIsPaused() {
    return getState() == PlayerState::PAUSED;
}

int getElapsedSecs() {
    PlayerState current = getState();
    if (current == PlayerState::STOPPED || current == PlayerState::STARTING) return 0;

    portENTER_CRITICAL(&stateMux);
    uint32_t start = startedAt;
    uint32_t pauseAt = pausedAt;
    uint32_t pauseTotal = pausedTotal;
    portEXIT_CRITICAL(&stateMux);

    uint32_t now = current == PlayerState::PAUSED ? pauseAt : millis();
    return (int)((now - start - pauseTotal) / 1000);
}

void setVolume(int v) {
    PlayerCommand cmd = {};
    cmd.type = PlayerCommandType::SET_VOLUME;
    cmd.value = constrain(v, 0, 100);
    volume = cmd.value;
    sendCommand(cmd);
}

int getVolume() {
    return volume;
}
