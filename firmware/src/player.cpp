#include "player.h"
#include "pins.h"
#include <AudioFileSourceSD.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <SdFat.h>

static AudioFileSourceSD* source   = nullptr;
static AudioGeneratorMP3* mp3      = nullptr;
static AudioOutputI2S*    output   = nullptr;

static int      volume      = 70;
static bool     paused      = false;
static uint32_t startedAt   = 0;
static uint32_t pausedAt    = 0;
static uint32_t pausedTotal = 0;  // accumulated paused time

void initPlayer() {
    output = new AudioOutputI2S();
    output->SetPinout(I2S_BCLK, I2S_LRCLK, I2S_DOUT);
    output->SetGain(volume / 100.0f);
}

void playTrack(const char* path) {
    stopPlayback();

    source = new AudioFileSourceSD(path);
    if (!source->isOpen()) {
        Serial.printf("Failed to open: %s\n", path);
        delete source;
        source = nullptr;
        return;
    }

    mp3 = new AudioGeneratorMP3();
    mp3->begin(source, output);

    paused      = false;
    startedAt   = millis();
    pausedTotal = 0;
    Serial.printf("Playing: %s\n", path);
}

void stopPlayback() {
    if (mp3) {
        if (mp3->isRunning()) mp3->stop();
        delete mp3;
        mp3 = nullptr;
    }
    if (source) {
        delete source;
        source = nullptr;
    }
    paused = false;
}

void togglePause() {
    if (!mp3) return;

    if (!paused) {
        // Pause: stop the generator but keep source alive
        if (mp3->isRunning()) mp3->stop();
        paused    = true;
        pausedAt  = millis();
    } else {
        // Resume: restart from current source position
        pausedTotal += millis() - pausedAt;
        mp3->begin(source, output);
        paused = false;
    }
}

void updatePlayer() {
    if (!mp3 || paused) return;
    if (mp3->isRunning()) {
        if (!mp3->loop()) {
            // Track finished naturally
            Serial.println("Track ended");
            stopPlayback();
        }
    }
}

bool playerIsPlaying() {
    return mp3 && mp3->isRunning() && !paused;
}

bool playerIsPaused() {
    return mp3 && paused;
}

int getElapsedSecs() {
    if (!mp3) return 0;
    uint32_t now = paused ? pausedAt : millis();
    return (int)((now - startedAt - pausedTotal) / 1000);
}

void setVolume(int v) {
    volume = constrain(v, 0, 100);
    if (output) output->SetGain(volume / 100.0f);
}

int getVolume() { return volume; }
