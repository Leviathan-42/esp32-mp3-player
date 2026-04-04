#include "encoder.h"
#include "pins.h"
#include <Arduino.h>

static EncoderEvent pendingEvt  = ENC_NONE;
static int          lastA       = HIGH;

// Button state
static bool     btnWasPressed   = false;
static uint32_t btnPressedAt    = 0;
static bool     holdFired       = false;
static const uint32_t HOLD_MS   = 600;

void initEncoder() {
    pinMode(ENC_A, INPUT_PULLUP);
    pinMode(ENC_B, INPUT_PULLUP);
    pinMode(ENC_BTN, INPUT_PULLUP);
    lastA = digitalRead(ENC_A);
}

void updateEncoder() {
    // --- rotation (simple quadrature read) ---
    int a = digitalRead(ENC_A);
    if (a != lastA && a == LOW) {
        int b = digitalRead(ENC_B);
        pendingEvt = (b == HIGH) ? ENC_CW : ENC_CCW;
    }
    lastA = a;

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
