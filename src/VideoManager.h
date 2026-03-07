#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"

class VideoManager {
public:
    void setup(int w, int h, float loopFadeDuration = 0.0f);
    void update();
    void draw(float x, float y, float w, float h, float alpha = 1.0f);
    void changeVideo(int index, float fadeTime);
    void onWindowResized(int w, int h);
    void setBlurAmount(float amount);
    void setSpeed(float speed);
    void updateInvert(float amplitude, float startThreshold, float fullThreshold);
    int getVideoCount() const;
    const ofPixels& getFramePixels();
    
    const std::vector<ofPolyline>& getWaveLines() const { return waveLines; }

    std::vector<ofVideoPlayer> videos;
    std::vector<ofVideoPlayer> loopVideos;
    int currentIndex = -1;
    int nextIndex = -1;
    bool isFading = false;
    float fadeTimer = 0.0f;
    float fadeDuration = 1.0f;
    float loopCrossfadeDuration = 0.0f;
    bool isLoopFading = false;
    ofPixels blendedPixels;

private:
    // Recursos do OpenCV
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;
    ofxCvColorImage blurImg; // Para o efeito de blur
    ofxCvContourFinder contourFinder;

    // FBOs para efeitos de pós-processamento como a inversão gradual
    ofFbo fboOriginal;
    ofFbo fboInverted;

    std::vector<ofPolyline> waveLines;
    
    int width, height;
    float thresholdValue = 30.0;
    bool bLearnBackground = true;
    bool bCvInitialized = false; 
    float invertAmount = 0.0f; 
    float blurAmount = 0.0f;   
};