#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    // O parâmetro maxAmp foi removido do setup, pois será passado no draw()
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

    // Calcula os bins (faixas de frequência) que correspondem ao intervalo de 20-16000Hz
    float frequencyPerBin = (float)settings.sampleRate / fft->getSignalSize();
    startBin = ceil(20.0f / frequencyPerBin);
    endBin = floor(12000.0f / frequencyPerBin);
    // Garante que os bins estejam dentro dos limites do vetor de magnitudes
    endBin = std::min(endBin, (int)magnitudes.size() - 1);
    startBin = std::max(startBin, 0);


    showGui = true; 
    
    	gui.setup("cfg"); // Nome do painel
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

    videoManager.update();
    noiseLines.update();
 
    // Usa o maxAmp da GUI como o limite máximo para o efeito de inversão
    // O limiar inicial (startThreshold) foi alterado de um valor fixo (0.05f)
    // para uma porcentagem de maxAmp, tornando o início do efeito mais adaptável.
    videoManager.updateInvert(smoothedAmplitude, maxAmp * 0.1f, maxAmp);
}

//--------------------------------------------------------------
void ofApp::draw(){
   

    ofBackground(0);
    if(showVideo){
    videoManager.draw(0, 0, ofGetWidth(), ofGetHeight());
}
    if (showWebLines){
    auto& lines = videoManager.getWaveLines();
    ofPushStyle(); 
        ofNoFill(); // Garante que as linhas não sejam preenchidas

        // Cor dos contornos de movimento
        ofColor c;
        c.setHsb(ofMap(smoothedAmplitude, 0.0f, maxAmp, 170, 85, true), 255, 255); // Verde (85) -> Azul (170)
        ofSetColor(c); 
        for (const auto& line : lines) {
            line.draw(); 
        }

        // 2. Coleta pontos aleatórios nas ondas para criar as teias
        std::vector<glm::vec3> connectionPoints;
        for (const auto& line : lines) {
            for (const auto& pt : line.getVertices()) {
                // A mesma chance de 2% seleciona pontos para a teia
                if (ofRandom(1.0) > 0.98) { 
                    connectionPoints.push_back(pt);
                }
            }
        }

        // 3. Desenha as teias conectando os pontos coletados
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
    // Passa o maxAmp da GUI a cada frame para garantir que a sensibilidade seja atualizada

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
    if (key != 'h'){
    int newVideo = (int)ofRandom(0, videoManager.getVideoCount());
    videoManager.changeVideo(newVideo, 5.0f);
    }

    if (key == 'h') {
        showGui = !showGui;
    }

    if (key == 'n') {
        showNoiseLines = !showNoiseLines;
    }

    if (key == 'v') {
        showVideo = !showVideo;

    }

    if (key == 'c') {
        showColorBar = !showColorBar;
    }

    if (key == 'w'){
        showWebLines = !showWebLines;
    }

    if (key == 'b'){
        showBlur = !showBlur;
    }
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & buffer){

    fft->setSignal(buffer.getBuffer());
    
    // 2. Recuperamos as magnitudes calculadas
    float* curMagnitudes = fft->getAmplitude();
    
    // Copia todas as magnitudes do resultado do FFT para o nosso vetor
    for(int i = 0; i < magnitudes.size(); i++) {
        magnitudes[i] = curMagnitudes[i];
    }

    // Calcula a amplitude média (volume geral) usando apenas as bandas dentro do nosso intervalo de interesse
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

        // O Hue agora é mapeado para o número de barras visíveis, para manter o arco-íris completo
        c.setHsb(ofMap(i, 0, numBars, 0, 255), 255, 255);
        
        c.a = intensity;
        
        ofSetColor(c);
        // Desenha a barra na sua nova posição e largura. O +1 ajuda a evitar falhas visuais entre as barras.
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
