#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxGui.h"

static ofGLWindowSettings settingsFullScreen;

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw(ofEventArgs& e);
    void drawGUI(ofEventArgs& e);
    void audioIn(ofSoundBuffer & input);

    ofApp(){
        settingsFullScreen.setSize(1024, 768);
        settingsFullScreen.windowMode = OF_WINDOW;
        this->guiWindow = ofCreateWindow(settingsFullScreen);
    }

    // GUI components
    ofxPanel gui;
    ofxFloatSlider rectThicknessSlider;
    ofxFloatSlider pointsSlider;
    ofxFloatSlider thresholdSlider;
    ofxFloatSlider soundThresholdSlider;
    ofxFloatSlider interlineaSlider;
    ofxButton neonButton;
    ofxButton foxShadowButton;
    ofxButton sphereButton;      // Pulsante per attivare modalità sfera
    ofxButton backgroundButton;  // Pulsante per attivare modalità sfondo
    ofxFloatSlider glowTransparencySlider;
    ofxFloatSlider glowThicknessSlider;

    ofxLabel backgroundLabel;
    ofxButton whiteBgButton;
    ofxButton grayBgButton;
    ofxButton blackBgButton;

    ofxLabel movementLabel;
    ofxButton redButton;
    ofxButton yellowButton;
    ofxButton purpleButton;

    ofxLabel filterLabel;
    ofxColorSlider filterColorSlider;

    // Event handlers for GUI
    void setBackgroundWhite();
    void setBackgroundGray();
    void setBackgroundBlack();
    void setColorRed();
    void setColorYellow();
    void setColorPurple();
    void foxShadowButtonPressed();
    void neonButtonPressed();
    void sphereButtonPressed();
    void backgroundButtonPressed();

    // Sound stream settings
    ofSoundStream soundStream;
    float smoothedVol;
    float scaledVol;

    // Colors
    ofColor drawColor;
    ofColor backgroundColor;
    ofColor filterColor;
    ofColor originalDrawColor;
    ofColor originalBackgroundColor;

    // Flags
    bool isFoxShadowMode;
    bool isNeonEnabled;
    bool isSphereMode;  // Attiva/disattiva modalità sfera

    // GUI Window
    shared_ptr<ofAppBaseWindow> guiWindow;
    // CANVA Window
    shared_ptr<ofAppBaseWindow> canvaWindow;

    // Camera and images
    ofVideoGrabber cam;
    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage prevGrayImage;
    ofxCvGrayscaleImage diffImage;
    ofxCvContourFinder contourFinder;

    // Variables for movement detection
    int movingPixelCount;
    float minDistance;
    vector<ofPoint> randomPoints;
    
    ofxButton blackAndWhiteButton; // Pulsante per attivare/disattivare la modalità bianco e nero
    bool isBlackAndWhite;          // Stato della modalità bianco e nero
    void blackAndWhiteButtonPressed(); // Handler per il pulsante

    // Utility functions
    void drawNeonEffect(const ofPoint& point1, const ofPoint& point2, const ofColor& color, float transparency, float thickness);
    void drawNeonRectangle(const ofRectangle& rect, const ofColor& color, float transparency, float thickness);
};
