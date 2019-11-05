#include "ofApp.h"
#include "mathutil.h"


//--------------------------------------------------------------
void ofApp::setup(){
    frame.set(0, 0, 1000, 1000 * ofGetScreenHeight() / ofGetScreenWidth());
    
    ofLog(OF_LOG_NOTICE, "width X height = %f X %f", frame.getWidth(), frame.getHeight());
    
	kinect.open();
	kinect.initDepthSource();

	auto depthWidth = kinect.getDepthSource()->getWidth();
	auto depthHeight = kinect.getDepthSource()->getHeight();
    grayImage.allocate(depthWidth, depthHeight);
    grayThreshNear.allocate(depthWidth, depthHeight);
    grayThreshFar.allocate(depthWidth, depthHeight);
    
    nearThreshold = 100;
    farThreshold = 2048;
        
    ofSetFrameRate(60);
    
    // zero the tilt on startup
    angle = 10;
    
    directLight.setDirectional();
    directLight.setPosition(0, 0, -150);
    directLight.lookAt({0, 0, 0});
    directLight.setAmbientColor(ofFloatColor(.15, .15, .15));
    directLight.setDiffuseColor( ofFloatColor(1.0f, 1.0f, 1.0f) );
    directLight.setSpecularColor( ofFloatColor(1.f, 1.f, 1.f));
    
    
    testBox.set( ofGetWidth()*0.25 );
    testBox.setPosition(0, 0, 0);
    {
        auto disk = make_unique<Disk >(glm::vec3(-260, 180, 0), 60, 80, 120, 5);
        disk->setRotationSpeed(TAU/10.0f);
        diskArr.push_back(std::move(disk));
    }
    {
        auto disk = make_unique<Disk >(glm::vec3(-120, 40, 0), 90, 120, 120, 8);
        disk->setRotationSpeed(TAU/10.0f);
        diskArr.push_back(std::move(disk));
    }
    {
        auto disk = make_unique<Disk >(glm::vec3(180, 120, 0), 80, 100, 120, 6);
        disk->setRotationSpeed(TAU/10.0f);
        diskArr.push_back(std::move(disk));
    }
    {
        auto disk = make_unique<Disk >(glm::vec3(60, 190, 0), 60, 80, 120, 5);
        disk->setRotationSpeed(TAU/10.0f);
        diskArr.push_back(std::move(disk));
    }
    {
        auto disk = make_unique<Disk >(glm::vec3(-180, -120, 0), 80, 100, 120, 7);
        disk->setRotationSpeed(TAU/10.0f);
        diskArr.push_back(std::move(disk));
    }
    {
        auto disk = make_unique<Disk >(glm::vec3(40, -20, 0), 80, 100, 120, 7);
        disk->setRotationSpeed(TAU/10.0f);
        diskArr.push_back(std::move(disk));
    }
    {
        auto disk = make_unique<Disk >(glm::vec3(170, -160, 0), 100, 120, 120, 5);
        disk->setRotationSpeed(TAU/10.0f);
        diskArr.push_back(std::move(disk));
    }

	drawDepth = false;
    drawBoundary = true;
    fullscreen = false;
    cameraScale = 1.0f;
}

float ofApp::getGrayDistance(glm::vec3 position) {
    
    if(!kinect.isOpen())
        return 1.0f;
    
    auto pixels = grayImage.getPixels();
    
    auto imgX = (position.x + frame.getWidth()/2) * pixels.getWidth() / frame.getWidth();
    auto imgY = (position.y + frame.getHeight()/2) * pixels.getHeight() / frame.getHeight();
    
    for(int y = imgY - 4; y <= imgY + 4; ++y) {
        for(int x = imgX - 4; x <= imgX + 4; ++x) {
            if(x < 0 || x >= pixels.getWidth() || y < 0 || y >= pixels.getHeight())
                continue;
            auto pix = pixels.getColor(imgX, pixels.getHeight() - imgY - 1);
            
            if(pix == 255)
                return 0.0f;
        }
    }

    return 1.0f;
}

//--------------------------------------------------------------
void ofApp::exit() {
    kinect.close();
}

//--------------------------------------------------------------
void ofApp::update(){
    
    kinect.update();

	auto depthSource = kinect.getDepthSource();
    
    // there is a new frame and we are connected
    if(kinect.isFrameNew()) {
		ofPixels grayPixels;
		auto devicePixels = depthSource->getPixels();
		grayPixels.allocate(devicePixels.getWidth(), devicePixels.getHeight(), OF_IMAGE_GRAYSCALE);

		auto src = devicePixels.begin();
		auto dst = grayPixels.begin();
		while (src != devicePixels.end() || dst != grayPixels.end()) {
			if (*src > nearThreshold && *src < farThreshold)
				*dst = 255;
			else
				*dst = 0;
			++src;
			++dst;
		}

        grayImage.setFromPixels(grayPixels);
        if(drawBoundary) {
            contourFinder.findContours(grayImage, 200, grayImage.getWidth() * grayImage.getHeight() / 2, 20, false, true);
        }
    }
    
    if(!fullscreen) {
        std::stringstream strm;
		strm << ofGetScreenWidth() << " X " << ofGetScreenHeight();
		strm << " fps: " << ofGetFrameRate();
        ofSetWindowTitle(strm.str());
        ofShowCursor();
    }
    else {
        ofHideCursor();
    }
}

void ofApp::drawBlobs() {
    ofPushStyle();
    ofPushMatrix();
    
    float scaleX = frame.getWidth() / float(grayImage.getWidth());
    float scaleY = frame.getHeight() / float(grayImage.getHeight());

    ofTranslate(-frame.getWidth()/2, frame.getHeight()/2);
    ofScale(scaleX, -scaleY);
    
    ofNoFill();
    ofSetColor(255, 0, 0);

    for(auto blob : contourFinder.blobs) {
        ofBeginShape();
        for( int j = 0; j < blob.nPts; j++ ) {
            ofVertex( blob.pts[j].x, blob.pts[j].y , 2.0f);
        }
        ofEndShape();
    }
    
    ofPopMatrix();
    ofPopStyle();
}


//--------------------------------------------------------------
void ofApp::draw(){
    cam.setGlobalPosition({ 0, 0, cam.getImagePlaneDistance(frame) });
    cam.setScale(cameraScale);
    cam.enableOrtho();
    cam.begin();

    ofBackground(135, 206, 235);
    
    for(auto& disk : diskArr) {
    
        auto dist = getGrayDistance(disk->getCenter());
    
        disk->update(dist);
    }
    if(drawDepth) {
        ofDisableDepthTest();
        grayImage.draw(-frame.getWidth()/2, -frame.getHeight()/2, frame.getWidth(), frame.getHeight());
    }
    if(drawBoundary) {
        drawBlobs();
    }
    ofEnableDepthTest();

    ofEnableLighting();
    directLight.enable();

    for(auto& disk : diskArr) {

        disk->draw();
    }
    
    cam.end();
}

void ofApp::changeWindow() {
    if(!fullscreen){
        ofSetWindowShape(frame.getWidth(), frame.getHeight());
        ofSetFullscreen(false);
        // figure out how to put the window in the center:
        int screenW = ofGetScreenWidth();
        int screenH = ofGetScreenHeight();
        ofSetWindowPosition(screenW/2-300/2, screenH/2-300/2);
    }
    else {
        ofSetFullscreen(true);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key) {
        case 'd':
            drawDepth = !drawDepth;
            break;
        case 'b':
            drawBoundary = !drawBoundary;
            break;
        case 'f':
            fullscreen = !fullscreen;
            changeWindow();
            break;
        case '>':
        case '.':
            farThreshold += 20;
            if (farThreshold > 4096) farThreshold = 4096;
            ofLog(OF_LOG_NOTICE, "farThreshold = %d", farThreshold);
            break;
            
        case '<':
        case ',':
            farThreshold -= 20;
            if (farThreshold < 0) farThreshold = 0;
            ofLog(OF_LOG_NOTICE, "farThreshold = %d", farThreshold);
            break;
            
        case '+':
        case '=':
            nearThreshold += 20;
            if (nearThreshold > farThreshold) nearThreshold = farThreshold;
            ofLog(OF_LOG_NOTICE, "nearThreshold = %d", nearThreshold);
            break;
            
        case '-':
            nearThreshold -= 20;
            if (nearThreshold < 0) nearThreshold = 0;
            ofLog(OF_LOG_NOTICE, "nearThreshold = %d", nearThreshold);
            break;
        case OF_KEY_LEFT:
            cameraScale -= 0.1f;
            if(cameraScale < 0.2)
                cameraScale = 0.2;
            break;
        case OF_KEY_RIGHT:
            cameraScale += 0.1f;
            if(cameraScale > 2.0)
                cameraScale = 2.0;
            break;

    }
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
    startCenter = glm::vec3(x - ofGetWidth()/2, ofGetHeight()/2 - y, 0);
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

    auto endPos = glm::vec3(x - ofGetWidth()/2, ofGetHeight()/2 - y, 0);
    auto radius = glm::distance(startCenter, endPos);
    //auto pos = cam.screenToWorld(glm::vec3(x, y, 0));
    auto disk = make_unique<Disk >(startCenter, radius - 10, radius + 10, 120, 5);
    disk->setRotationSpeed(TAU/10.0f);
    diskArr.push_back(std::move(disk));
    ofLog(OF_LOG_NOTICE, "auto disk = make_unique<Disk >(glm::vec3(%f, %f, 0), %f, %f, 120, 5);", startCenter.x, startCenter.y, radius - 10, radius + 10);
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

	cameraScale = frame.getWidth() / w;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
