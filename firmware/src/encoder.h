#pragma once

enum EncoderEvent {
    ENC_NONE,
    ENC_CW,     // clockwise turn
    ENC_CCW,    // counter-clockwise turn
    ENC_CLICK,  // short press
    ENC_HOLD,   // held for 600ms+ (used to go back to browser)
};

void     initEncoder();
void     updateEncoder();
EncoderEvent getEncoderEvent();  // returns event and clears it
