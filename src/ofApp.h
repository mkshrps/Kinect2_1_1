#pragma once
#include "ofMain.h"
#include "ofxNI2.h"
#include "ofxNiTE2.h"
#include "ofxGui.h"
#include "audioStreamer.h"

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

    void drawStartPage();
    void drawPointCloud(bool enableCam);
	void createPointCloud_1();
    bool startRecord(string filename, bool allowLossyCompression, ofxNI2::DepthStream& stream);

    void drawSkeleton();
    //void audioIn(ofSoundBuffer & input);
    void drawSound();

    void resetCamPos();
     // control flags
    bool showGui;
    bool recording;
    bool depthCamView;
    bool skellyPage = false;        // experimental to display tracking skelly
    ofxNI2::ColorStream rgbStream;
    ofxNI2::DepthStream depth;
    ofSpherePrimitive originSphere;
    
    ofxNI2::Device device;              // Live Kinect device
    ofxNI2::Device playbackDevice;       // Playback Device
    
    ofxNiTE2::UserTracker tracker;
    ofPoint head;
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
    ofxFloatSlider cam_heading;

    ofxFloatSlider volume_l;
    ofxFloatSlider volume_r;
    ofxFloatSlider   gain;          // audio vol gain
    ofxFloatSlider   noiseGain;     // visual noise gain
    
    // tracker controls
    ofxFloatSlider   smoothing;
    ofxFloatSlider  overlaycam_x;
    ofxFloatSlider  overlaycam_y;
    ofxFloatSlider  overlaycam_z;


    ofxToggle addNoise;
    ofxToggle addSound;
    ofxToggle drawDepthOnTracker;
    ofxToggle enSkel;


    //ofParameter<float>farclip{"far clip",3000,500,5000}; 

    ofxGuiGroup g_Params;
    ofxGuiGroup recGroup;
    ofxGuiGroup camGroup;
    ofxGuiGroup paramGroup;
    ofxGuiGroup trackerGroup;
    ofEasyCam cam;
    ofCamera overlayCam;

    ofxGuiGroup soundGroup;

    ofParameter<bool> recordStatus;
    //ofParameter<bool> invert;
    ofxToggle invert;
    ofxToggle getFullDepthRange;
    ofxToggle colToDepth;       // force map color range to whole depth image
    ofxToggle trackColor;       // keep both color sliders in sync
    //ofxToggle enableRgbRegistered; // register RGB and Depth streams
    ofxToggle showRGB;          // display RGB feed
  
    ofPoint point0; 
    ofPoint point_max_x  ;
    ofPoint point_max_y  ;
    ofPoint point_max_xy ;
    ofPoint point_half;
    unsigned short mouseDepth;
    
    vector<ofPoint> pt;
    openni::Recorder *oni_recorder;
    int framecount;
    float rgb_w,rgb_h;
    int dfar;

    bool drawSoundEnabled;
    
	ofBoxPrimitive box;
    ofMaterial material;
//    vector <float> left;
//    vector <float> right;
//    vector <float> volHistory;
//    
    int 	bufferCounter;
    int 	drawCounter;
    
//   float smoothedVol;
//    float scaledVol;
    
//    ofSoundStream soundStream;
    float oldTime;
    ofSpherePrimitive sp1;
    //ofMaterial material;
    ofLight l1;
    audioStreamer::audioDevice audioDev;
    int page = 0;
};
