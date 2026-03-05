#pragma once
#include "ofMain.h"

class NoiseLines{

    public:
    void setup(int lines, float maxAmp);
    void update();
    void draw(float transparency);
    
    private:
    std::vector<glm::vec2> nlines;
    std::vector<float> timeOffsets; 

    float maxAmpli;
    float velocidade = 0.5f;
    float amplitude = 50.0f; // Quão longe ela oscila da base


};
 