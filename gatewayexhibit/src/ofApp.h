#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
//#include "ofxKinect.h"
#include "ofxKinectForWindows2.h"
#include "disk.h"
#include <vector>



class ofApp : public ofBaseApp {

public:
    void setup();
    void exit();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
private:
    ofRectangle frame;
    
    void changeWindow();
    
    void drawBlobs();
    
    float getGrayDistance(glm::vec3 position);
    float getScaledDistance(glm::vec3 position, float range, int clipNear, int clipFar);
    
    int angle;
	ofxKFW2::Device kinect;

    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
    ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
    
    int nearThreshold;
    int farThreshold;
    
    ofxCvContourFinder contourFinder;
    
    ofCamera cam;
    ofLight directLight;

    ofBoxPrimitive testBox;
    
    vector<unique_ptr<Disk>> diskArr;
    
    bool drawDepth;
    bool drawBoundary;
    bool fullscreen;
    float cameraScale;
    
    glm::vec3 startCenter;
};
