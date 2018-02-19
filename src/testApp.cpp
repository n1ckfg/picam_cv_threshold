#include "testApp.h"

using namespace cv;
using namespace ofxCv;

void testApp::setup() {
    width = ofGetWidth();
    height = ofGetHeight();
    fps = 60;
	thresholdValue = 127;
	thresholdKeyCounter = 0;
	thresholdKeyFast = false;
	doDrawInfo = true;
	oscAddress = "blob";
	contourThreshold = 2.0; //127.0;
	contourMin = 1.0; //10.0;
	contourMax = 250.0; //150.0;
	hostname = "RPi";

	file.open(ofToDataPath("hostname.txt"), ofFile::ReadWrite, false);
	if (file) {
		buff = file.readToBuffer();
		hostname = buff.getText();
	} else {
		hostname += "_" + ofGetTimestampString("%y-%m-%d-%H-%M-%S-%i");
		ofStringReplace(hostname, "-", "");
		ofStringReplace(hostname, "\n", "");
		ofStringReplace(hostname, "\r", "");
		buff.set(hostname.c_str(), hostname.size());
		ofBufferToFile("hostname.txt", buff);
	}
	cout << hostname;

    cam.setup(width, height, false); //(w, h, color/gray);
    //cam.setISO(300); // 100 - 800
    cam.setExposureMode((MMAL_PARAM_EXPOSUREMODE_T) 0); // 0 = off, 1 = auto
    //cam.setFrameRate ???

    //ofSetLogLevel(OF_LOG_VERBOSE);
    //ofSetLogLevel("ofThread", OF_LOG_ERROR);
    ofSetVerticalSync(false);    

    //consoleListener.setup(this);

    ofSetFrameRate(fps);

    sender.setup(HOST, PORT);

    contourFinder.setMinAreaRadius(contourMin);
    contourFinder.setMaxAreaRadius(contourMax);
    //contourFinder.setInvert(true); // find black instead of white
    trackingColorMode = TRACK_COLOR_RGB;
}

void testApp::update() {
    frame = cam.grab();

    if(!frame.empty()) {
        //autothreshold(frameProcessed);        
        threshold(frame, frameProcessed, thresholdValue, 255, 0);    
        contourFinder.setThreshold(contourThreshold);
        contourFinder.findContours(frameProcessed);
    }
}

void testApp::draw() {
    ofSetColor(255);
    if (!frame.empty()) {
        drawMat(frameProcessed, 0, 0);

        ofSetLineWidth(2);
        //contourFinder.draw();

        ofNoFill();
        int n = contourFinder.size();
        for (int i = 0; i < n; i++) {
            ofSetColor(cyanPrint);
            float circleRadius;
            ofVec2f circleCenter = toOf(contourFinder.getMinEnclosingCircle(i, circleRadius));
            ofCircle(circleCenter, circleRadius);
            ofCircle(circleCenter, 1);

            sendOsc(i, circleCenter.x, circleCenter.y, circleRadius);
        }
    }

    if (doDrawInfo) {
        stringstream info;
        info << "FPS: " << ofGetFrameRate() << "\n";
        //info << "Camera Resolution: " << cam.width << "x" << cam.height << " @ "<< "xx" <<"FPS"<< "\n";
        ofDrawBitmapStringHighlight(info.str(), 10, 10, ofColor::black, ofColor::yellow);
    }
   
}

void testApp::sendOsc(int index, float x, float y, float z) {
    ofxOscMessage m;
    m.setAddress("/" + oscAddress);
    m.addStringArg(hostName);
    m.addIntArg(index);
    m.addFloatArg(x);
    m.addFloatArg(y);
    //m.addFloatArg(z);
    sender.sendMessage(m);
}

void testApp::keyPressed(int key) {
    thresholdKeyCounter++;
    if (thresholdKeyCounter > 10) thresholdKeyFast = true;

    if (key == OF_KEY_PAGE_DOWN || key == OF_KEY_DOWN) {
        if (thresholdKeyFast) {
            thresholdValue -= 10;
        } else {
            thresholdValue -= 1;
        }
    }

    if (key == OF_KEY_PAGE_UP || OF_KEY_UP) {
       if (thresholdKeyFast) {
           thresholdValue += 10;
       } else {
           thresholdValue += 1;
       }
    }

    if (thresholdValue < 0) thresholdValue = 0;
    if (thresholdValue > 255) thresholdValue = 255;
}

void testApp::keyReleased(int key) {
    thresholdKeyFast = false;
    thresholdKeyCounter = 0;
}

string testApp::getHostName() {
    FILE* stream = popen("hostname", "r");  
    ostringstream output;  

    while(!feof(stream) && !ferror(stream)) {  
        char buf[128];  
        int bytesRead = fread(buf, 1, 128, stream);  
        output.write(buf, bytesRead);  
    }  
    return ofToString(output.str());  
}

//void testApp::onCharacterReceived(KeyListenerEventData& e) {
       //keyPressed((int) e.character);
//}
