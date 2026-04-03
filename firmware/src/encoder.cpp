#include "encoder.h"
#include "pins.h"
#include <ESP32Encoder.h>
#include <Arduino.h>

static ESP32Encoder enc;
static long         lastCount   = 0;
static EncoderEvent pendingEvt  = ENC_NONE;

// Button state
static bool     btnWasPressed   = false;
static uint32_t btnPressedAt    = 0;
static bool     holdFired       = false;
static const uint32_t HOLD_MS   = 600;

void initEncoder() {
    ESP32Encoder::useInternalWeakPullResistors = puType::up;
    enc.attachHalfQuad(ENC_A, ENC_B);
    enc.setCount(0);
    lastCount = 0;

    pinMode(ENC_BTN, INPUT_PULLUP);
}

void updateEncoder() {
    // --- rotation ---
    long count = enc.getCount();
    long delta = count - lastCount;
    if (delta >= 2) {
        pendingEvt = ENC_CW;
        lastCount  = count;
    } else if (delta <= -2) {
        pendingEvt = ENC_CCW;
        lastCount  = count;
    }

    // --- button ---
    bool pressed = (digitalRead(ENC_BTN) == LOW);

    if (pressed && !btnWasPressed) {
        btnPressedAt  = millis();
        holdFired     = false;
    }

    if (pressed && !holdFired && (millis() - btnPressedAt >= HOLD_MS)) {
        pendingEvt = ENC_HOLD;
        holdFired  = true;
    }

    if (!pressed && btnWasPressed && !holdFired) {
        pendingEvt = ENC_CLICK;
    }

    btnWasPressed = pressed;
}

EncoderEvent getEncoderEvent() {
    EncoderEvent e = pendingEvt;
    pendingEvt = ENC_NONE;
    return e;
}
