#pragma once
#include "ofMain.h"
#include "ofxNI2.h"
#include "ofxNiTE2.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp
{
public:
    void setup();
    void exit();
    
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    void drawPointCloud();
	void createPointCloud_1();
    bool startRecord(string filename, bool allowLossyCompression, ofxNI2::DepthStream& stream);
    void drawSkeleton();
    // control flags
    bool showGui;
    bool recording;
    ofxNI2::ColorStream rgbStream;
    ofxNI2::DepthStream depth;
    
    
    ofxNI2::Device device;              // Live Kinect device
    ofxNI2::Device playbackDevice;       // Playback Device
    //openni::Device& dev = playbackDevice.operator openni::Device &(); 
    ofxNiTE2::UserTracker tracker;
    ofPixels_<unsigned short> depthPixels;
    ofTexture rgbTex;
    
    ofTexture depthTexture;
    ofPixelFormat pfmt;
    ofMesh pointCloud;
    bool showPointCloud = false;
    
    // GUI Panel
    ofxPanel panel;
    ofParameter<float>cam_y{"cam y",0,-1000,1000};
    ofParameter<float>cam_x{"cam x",0,-1000,1000};
    ofParameter<float>cam_z{"cam z",0,-1000,1000};
    ofParameter<float>dpHeight{"depth pix height",0,1,32000};
    ofParameter<float>dpWidth{"depth pix height",0,1,32000};

    ofxFloatSlider pointSize; 
    ofxFloatSlider farclip;
    ofxFloatSlider nearclip;
    ofxFloatSlider colMin;
    ofxFloatSlider colMax;
    ofxIntSlider ghosts;
    

    //ofParameter<float>farclip{"far clip",3000,500,5000}; 

    ofxGuiGroup g_Params;
    ofxGuiGroup recGroup;
    ofxGuiGroup camGroup;
    ofxGuiGroup paramGroup;

    ofParameter<bool> recordStatus;
    //ofParameter<bool> invert;
    ofxToggle invert;
    ofxToggle getFullDepthRange;
    ofxToggle colToDepth;       // force map color range to whole depth image
    ofxToggle trackColor;       // keep both color sliders in sync
    ofxToggle showRegistered;
    ofxToggle showRGB;          // display RGB feed

    ofPoint point0; 
    ofPoint point_max_x  ;
    ofPoint point_max_y  ;
    ofPoint point_max_xy ;
    ofPoint point_half;

    vector<ofPoint> pt;
    openni::Recorder *oni_recorder;
    int framecount;
    float rgb_w,rgb_h;
    int dfar;



};
