#pragma once

#include "ofMain.h"
#include "VideoManager.h"
#include "PersonTracker.h"
#include "NoiseLines.h"
#include "ofxFft.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

	public:
		void setup() override;
		void update() override;
		void draw() override;
		void exit() override;

		void keyPressed(int key) override;
		void keyReleased(int key) override;
		void mouseMoved(int x, int y ) override;
		void mouseDragged(int x, int y, int button) override;
		void mousePressed(int x, int y, int button) override;
		void mouseReleased(int x, int y, int button) override;
		void mouseScrolled(int x, int y, float scrollX, float scrollY) override;
		void mouseEntered(int x, int y) override;
		void mouseExited(int x, int y) override;
		void windowResized(int w, int h) override;
		void dragEvent(ofDragInfo dragInfo) override;
		void gotMessage(ofMessage msg) override;
		void drawColorBars();

		// GUI
		ofxPanel gui;
        ofParameter<float> maxAmp;
        ofParameter<float> colorBarSensitivity;
		ofParameter<float> blurAmount;
        bool showGui; 
		
		// Audio
		void audioIn(ofSoundBuffer & buffer) override;

		// Video
		VideoManager videoManager;
		PersonTracker personTracker; 
		bool isVideoLayerVisible = true;
		float videoLayerAlpha = 1.0f;
		float videoLayerFadeDuration = 5.0f; // Duração do fade da camada de vídeo em segundos
		bool showWebLines = true; 
		bool showBlur = false;

		// Audio
		ofSoundStream soundStream;
		float smoothedAmplitude;
		float convertedAmp;

		ofxFft* fft;
    	std::vector<float> magnitudes;
		int bufferSize;
		int startBin; 
		int endBin;  

		NoiseLines noiseLines;
		bool showNoiseLines = true;
		
		bool showColorBar = true;
	private:
		ofSoundBuffer lastInputBuffer;
		ofMutex audioMutex;
};
