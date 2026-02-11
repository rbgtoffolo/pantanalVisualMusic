#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // Rotinas de video --------------------------------
    videoManager.setup(5.0f);
    videoManager.changeVideo(0, 0);
    

    //--------------------------------------------------
    // Rotinas de detecção de Pessoas 
    //personTracker.setup(640, 360);



    //--------------------------------------------------
    // Comunicação com Supercollider
    // OSC - Endereço e Porta
    sender.setup("127.0.0.1", 57120);
    ofxOscMessage m;
    m.setAddress("/start");
    sender.sendMessage(m);

    

}

//--------------------------------------------------------------
void ofApp::update(){

    // Rotinas de video --------------------------------
    videoManager.update();


    if (ofGetElapsedTimef() >= nextMessageTime) {
        // Verifica se o índice é válido antes de acessar
        if(videoManager.currentIndex >= 0 && videoManager.currentIndex < videoManager.videos.size()){
            ofPixels pixels = videoManager.videos[videoManager.currentIndex].getPixels();
            currentColors = colorAnalyzer.getDominantColors(pixels, 5);

            // Envia uma mensagem individual para cada cor dominante
            for(int i = 0; i < currentColors.size(); i++) {
                ofxOscMessage m;
                m.setAddress("/color");
                m.addIntArg(i); // Envia o índice (0 é a cor mais dominante)
                m.addFloatArg(currentColors[i].hue);
                m.addFloatArg(currentColors[i].saturation);
                m.addFloatArg(currentColors[i].brightness);
                m.addFloatArg(currentColors[i].percentage);
                sender.sendMessage(m);
            }
        }
        // Atualiza o timer para não enviar a cada frame (controlado por 'interval')
        nextMessageTime = ofGetElapsedTimef() + interval;
    }





    //--------------------------------------------------
    // Rotinas de detecção de Pessoas 
    //    personTracker.update();
    
    // Iterar sobre as pessoas detectadas
    // for(int i = 0; i < personTracker.getNumPeople(); i++){
    //     ofVec2f pos = personTracker.getPersonPosition(i);
    //     int id = personTracker.getPersonId(i);
        
    //     ofLog() << "Pessoa ID: " << id << " em " << pos;
    // }



}

//--------------------------------------------------------------
void ofApp::draw(){

     // Rotinas de video --------------------------------
    videoManager.draw(0, 0, ofGetWidth(), ofGetHeight());




    //--------------------------------------------------
    // Rotinas de detecção de Pessoas 
    // personTracker.draw(0, 0, 640, 360);
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    int newVideo = (int)ofRandom(0, videoManager.getVideoCount());
    videoManager.changeVideo(newVideo, 5.0f);

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

// void ofApp::sendColorInfo(int index, float duration) {
//     ofxOscMessage m;
//     m.setAddress("/frameColors");
//     m.addStringArg("start");
//     m.addIntArg(index);
//     m.addFloatArg(duration);
//     sender.sendMessage(m);
// }


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

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
