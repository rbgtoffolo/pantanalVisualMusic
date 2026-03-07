#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    noiseLines.setup(50);
    bufferSize = 256;

    videoManager.setup(ofGetWidth(), ofGetHeight(), 5.0f);
    videoManager.changeVideo(0, 0);

    fft = ofxFft::create(bufferSize, OF_FFT_WINDOW_HANN);
    magnitudes.assign(fft->getBinSize(), 0.0);

    smoothedAmplitude = 0.0;
    ofSoundStreamSettings settings;
   
    //soundStream.printDeviceList();
    settings.setInListener(this);
    settings.sampleRate = 48000;
    settings.numInputChannels = 1;
    settings.numOutputChannels = 1; 
    settings.bufferSize = bufferSize;
    soundStream.setup(settings);

    float frequencyPerBin = (float)settings.sampleRate / fft->getSignalSize();
    startBin = ceil(20.0f / frequencyPerBin);
    endBin = floor(12000.0f / frequencyPerBin);

    endBin = std::min(endBin, (int)magnitudes.size() - 1);
    startBin = std::max(startBin, 0);


    showGui = true; 
    
    	gui.setup("cfg"); 
    	gui.add(maxAmp.set("mainSens", 2.5f, 0.1f, 10.0f));
		gui.add(colorBarSensitivity.set("SensClrBar", 0.2f, 0.01f, 1.0f));
        gui.add(blurAmount.set("Blr", 0.0f, 0.0f, 50.0f));

}

//--------------------------------------------------------------
void ofApp::update(){

    if (showBlur){
    videoManager.setBlurAmount(blurAmount);} else {
        videoManager.setBlurAmount(0.0f);
    }

    // Lógica para o fade in/out da camada de vídeo (tecla 'v')
    // A cada frame, ajustamos a opacidade para chegar ao estado desejado (visível/invisível)
    if (videoLayerFadeDuration <= 0) videoLayerFadeDuration = 0.1f; // Evita divisão por zero
    float fadeSpeed = (1.0f / videoLayerFadeDuration) * ofGetLastFrameTime();
    if (isVideoLayerVisible && videoLayerAlpha < 1.0f) {
        videoLayerAlpha = std::min(1.0f, videoLayerAlpha + fadeSpeed);
    } else if (!isVideoLayerVisible && videoLayerAlpha > 0.0f) {
        videoLayerAlpha = std::max(0.0f, videoLayerAlpha - fadeSpeed);
    }

    videoManager.update();
    videoManager.updateInvert(smoothedAmplitude, maxAmp * 0.1f, maxAmp);
    noiseLines.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
   
    ofBackground(0);
    // Desenha a camada de vídeo, passando o valor de alpha calculado diretamente para o VideoManager.
    // O VideoManager agora é responsável por aplicar esta opacidade final.
    videoManager.draw(0, 0, ofGetWidth(), ofGetHeight(), videoLayerAlpha);

    if (showWebLines){
    auto& lines = videoManager.getWaveLines();
    ofPushStyle(); 
        ofNoFill(); 

        // Cor dos contornos de movimento
        ofColor c;
        c.setHsb(ofMap(smoothedAmplitude, 0.0f, maxAmp, 170, 85, true), 255, 255); // Verde (85) -> Azul (170)
        ofSetColor(c); 
        for (const auto& line : lines) {
            line.draw(); 
        }

        std::vector<glm::vec3> connectionPoints;
        for (const auto& line : lines) {
            for (const auto& pt : line.getVertices()) {
                if (ofRandom(1.0) > 0.98) { 
                    connectionPoints.push_back(pt);
                }
            }
        }

        if (connectionPoints.size() > 1) {
            // Cor para as linhas da teia
             ofColor c;
            c.setHsb(ofMap(smoothedAmplitude, 0.0f, maxAmp, 0, 42, true), 255, 255); // Amarelo (42) -> Vermelho (0)
            ofSetColor(c); 
            for (size_t i = 0; i < connectionPoints.size(); ++i) {
                size_t otherIndex = static_cast<size_t>(ofRandom(connectionPoints.size()));
                if (i != otherIndex) { // Evita conectar um ponto a si mesmo
                    ofDrawLine(connectionPoints[i], connectionPoints[otherIndex]);
                }
            }
        }
    ofPopStyle();
    }


    // Vertical Lines

    if (showNoiseLines) {
    noiseLines.draw(smoothedAmplitude, maxAmp);
    }

    if(showColorBar){
    drawColorBars();
    }

     if (showGui) {
        gui.draw();
    }

}

//--------------------------------------------------------------
void ofApp::exit(){
    soundStream.close();

}


//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'h') {
        showGui = !showGui;
    } else if (key == 'n') {
        showNoiseLines = !showNoiseLines;
    } else if (key == 'v') {
        isVideoLayerVisible = !isVideoLayerVisible;
    } else if (key == 'c') {
        showColorBar = !showColorBar;
    } else if (key == 'w'){
        showWebLines = !showWebLines;
    } else if (key == 'b'){
        showBlur = !showBlur;
    } else if (key == ' ') {
        if (videoManager.getVideoCount() > 1) {
            int newVideoIndex = videoManager.currentIndex;
            // Procura por um novo índice até encontrar um diferente
            while (newVideoIndex == videoManager.currentIndex) {
                newVideoIndex = (int)ofRandom(videoManager.getVideoCount());
            }
            videoManager.changeVideo(newVideoIndex, 5.0f);
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & buffer){

    fft->setSignal(buffer.getBuffer());
    float* curMagnitudes = fft->getAmplitude();
    
    for(int i = 0; i < magnitudes.size(); i++) {
        magnitudes[i] = curMagnitudes[i];
    }

    float totalMagnitudeInRange = 0.0f;
    int numBinsInRange = endBin - startBin + 1;

    if (numBinsInRange > 0) {
        for (int i = startBin; i <= endBin; i++) {
            totalMagnitudeInRange += magnitudes[i];
        }
        float currentAmplitude = (totalMagnitudeInRange / numBinsInRange) * 100.0f;
        smoothedAmplitude = ofLerp(smoothedAmplitude, currentAmplitude, 0.1f);
    } else {
        smoothedAmplitude = ofLerp(smoothedAmplitude, 0.0f, 0.1f); // Fallback
    }
}

//--------------------------------------------------------------

void ofApp::drawColorBars(){
    ofPushStyle();
    ofEnableAlphaBlending();

    int numBars = endBin - startBin + 1;
    if (numBars <= 0) {
        ofPopStyle();
        return;
    }

    float barWidth = (float)ofGetWidth() / numBars;

    for(int i = 0; i < numBars; i++) {
        int binIndex = startBin + i;

        ofColor c;
        
        float intensity = ofMap(magnitudes[binIndex], 0.0f, colorBarSensitivity, 0, 255, true);

        c.setHsb(ofMap(i, 0, numBars, 0, 255), 255, 255);
        
        c.a = intensity;
        
        ofSetColor(c);
        ofDrawRectangle(i * barWidth, 0, barWidth + 1, ofGetHeight());
    }
    ofPopStyle();
}



//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}


//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseScrolled(int x, int y, float scrollX, float scrollY){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    videoManager.onWindowResized(w, h);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
