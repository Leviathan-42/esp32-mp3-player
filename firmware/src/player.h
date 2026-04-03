#pragma once
#include <Arduino.h>

void initPlayer();
void playTrack(const char* path);
void stopPlayback();
void togglePause();
void updatePlayer();       // call every loop iteration
bool playerIsPlaying();
bool playerIsPaused();
int  getElapsedSecs();
void setVolume(int v);     // 0-100
int  getVolume();
