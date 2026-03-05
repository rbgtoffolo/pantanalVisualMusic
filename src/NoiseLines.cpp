#include "NoiseLines.h"


void NoiseLines::setup(int lines){
    nlines.clear();
    timeOffsets.clear();
   for(int i = 0; i < lines; i++) {
        // Sorteia um X fixo dentro da tela (com margem para não bater no canto)
        float xFixo = ofRandom(amplitude, ofGetWidth() - amplitude);
        nlines.push_back(glm::vec2(xFixo, 0)); 
        
        // Offset de tempo para cada linha ser única
        timeOffsets.push_back(ofRandom(1000.0f));
    }
}

void NoiseLines::update(){

}

void NoiseLines::draw(float smoothedAmp, float currentMaxAmp){

    //ofLogNotice() << smoothedAmp;
    ofPushStyle();
    // Habilita explicitamente o alpha blending. O problema era que o VideoManager,
    // ao desenhar o efeito de inversão, chamava ofDisableBlendMode(), e esse estado
    // persistia, fazendo com que o valor de alpha das NoiseLines fosse ignorado.
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    
    float tempo = ofGetElapsedTimef() * 2.0f; // Velocidade da oscilação
    
    for(int i = 0; i < nlines.size(); i++) {
        float noiseInput = tempo + timeOffsets[i];
        float offsetX = ofMap(ofNoise(noiseInput), 0, 1, -smoothedAmp*255, smoothedAmp * 255);
        float alphaNoise = ofNoise(noiseInput * 0.5 + 1000); // Offset diferente para não sincronizar com movimento
        
        float globalAlpha = ofMap(smoothedAmp, 0.0f, currentMaxAmp, 0.0f, 255, true);
        // As linhas são pretas para contrastar com os vídeos.
        ofSetColor(0, 0, 0, ofMap(alphaNoise, 0, 1, 0, globalAlpha));
        
        ofSetLineWidth(ofMap(smoothedAmp, 0.0f, currentMaxAmp, 0.0f, 20.0f, true));

        float xFinal = nlines[i].x + offsetX;
        ofDrawLine(xFinal, 0, xFinal, ofGetHeight());
    }
    ofPopStyle();
}
