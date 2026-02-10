#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // videoManager.setup(5.0f);
    // // Exemplo: Iniciar o primeiro vídeo
    // videoManager.changeVideo(0, 0);
    
    personTracker.setup(640, 360);

    

}

//--------------------------------------------------------------
void ofApp::update(){
    // videoManager.update();
       personTracker.update();
    
    // Iterar sobre as pessoas detectadas
    for(int i = 0; i < personTracker.getNumPeople(); i++){
        ofVec2f pos = personTracker.getPersonPosition(i);
        int id = personTracker.getPersonId(i);
        
        ofLog() << "Pessoa ID: " << id << " em " << pos;
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    // videoManager.draw(0, 0, ofGetWidth(), ofGetHeight());
    personTracker.draw(0, 0, 640, 360);
}

//--------------------------------------------------------------
void ofApp::exit(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
