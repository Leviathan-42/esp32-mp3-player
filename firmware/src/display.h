#pragma once
#include <Arduino.h>

void initDisplay();

// Show scrollable file list. selected = highlighted index.
void drawBrowser(const char** files, int count, int selected);

// Show now-playing screen.
// elapsedSec/totalSec: pass totalSec=0 if unknown
void drawPlayer(const char* trackName, int elapsedSec, int totalSec,
                int volume, bool playing);

// Full-screen message (used for errors/loading)
void drawMessage(const char* msg);
