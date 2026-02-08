#pragma once
#include "ofMain.h"

class VideoManager {
public:
    void setup(float loopFadeDuration = 0.0f);
    void update();
    void draw(float x, float y, float w, float h);
    void changeVideo(int index, float fadeTime);

    std::vector<ofVideoPlayer> videos;
    std::vector<ofVideoPlayer> loopVideos;
    int currentIndex = -1;
    int nextIndex = -1;
    bool isFading = false;
    float fadeTimer = 0.0f;
    float fadeDuration = 1.0f;
    
    float loopCrossfadeDuration = 0.0f;
    bool isLoopFading = false;
};