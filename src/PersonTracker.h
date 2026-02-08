#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"

struct TrackedPerson {
    int id;
    ofVec2f pos;
    int dyingCount; // Frames sem detecção antes de remover
};

class PersonTracker {
public:
    void setup(int w, int h);
    void update();
    void draw(float x, float y, float w, float h);

    // Getters solicitados
    int getNumPeople() const;
    ofVec2f getPersonPosition(int index) const;
    int getPersonId(int index) const;

    // Controles para ajuste da detecção
    void setThreshold(int value);
    void resetBackground();

private:
    ofVideoGrabber vidGrabber;

    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;

    ofxCvContourFinder contourFinder;

    int threshold;
    bool bLearnBackground;

    vector<TrackedPerson> trackedPeople;
    int idCounter;
};