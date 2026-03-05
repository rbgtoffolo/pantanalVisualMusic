#pragma once
#include "ofMain.h"

class NoiseLines{

    public:
    void setup(int lines);
    void update();
    void draw(float smoothedAmp, float currentMaxAmp);
    
    private:
    std::vector<glm::vec2> nlines;
    std::vector<float> timeOffsets; 

    float velocidade = 0.5f;
    float amplitude = 50.0f; // Quão longe ela oscila da base


};
 