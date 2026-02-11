#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

struct ColorProfile {
    ofColor color;      // Cor RGB original
    float hue;          // Matiz (0-255)
    float saturation;   // Saturação (0-255)
    float brightness;   // Brilho (0-255)
    float percentage;   // Dominância (0.0 a 1.0)
};

class ColorAnalyzer {
public:
    // Analisa o frame e retorna as 'k' cores dominantes usando K-Means.
    // O frame é redimensionado internamente para performance.
    vector<ColorProfile> getDominantColors(const ofPixels& pixels, int k);
};