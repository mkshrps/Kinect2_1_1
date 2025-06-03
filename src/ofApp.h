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

    void trackJoint();
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
    ofPoint head , headPosition;
    bool enableTracker;

    ofPixels_<unsigned short> depthPixels;
    ofTexture rgbTex;
    
    ofTexture depthTexture;
    ofPixelFormat pfmt;
    ofMesh pointCloud;
    bool showPointCloud = false;
//    ofQuaternion camOrientation;

    // GUI Panel
    ofxPanel panel;

    // camera debug
    ofParameter<float>cam_y{"cam y",0,-1000,1000};
    ofParameter<float>cam_x{"cam x",0,-1000,1000};
    ofParameter<float>cam_z{"cam z",0,-1000,1000};
    ofParameter<ofVec4f>orientParam{"orientation",ofVec4f(0,0,0,0),ofVec4f(0,0,0,0),ofVec4f(0,0,0,0)};
    ofParameter<ofQuaternion>q{"quaternion",ofQuaternion(0,0,0,0),ofQuaternion(0,0,0,0),ofQuaternion(0,0,0,0)};

    ofxFloatSlider cam_heading;



    ofParameter<float>headDetected{"head Conf",0,0,1000};

    ofParameter<float>head_x{"head x",0,-1000,1000};
    ofParameter<float>head_y{"head y",0,-1000,1000};
    ofParameter<float>head_z{"head z",0,-1000,1000};

    ofParameter<float>dpHeight{"depth pix height",0,1,32000};
    ofParameter<float>dpWidth{"depth pix width",0,1,32000};


    // point cloud page
    ofxFloatSlider pointSize; 
    ofxFloatSlider farclip;
    ofxFloatSlider nearclip;
    ofxFloatSlider colMin;
    ofxFloatSlider colMax;

    ofxIntSlider ghosts;
    ofxToggle   pcEnableTracking;
    ofxFloatSlider volume_l;
    ofxFloatSlider volume_r;
    ofxFloatSlider gain;          // audio vol gain
    ofxFloatSlider noiseGain;     // visual noise gain
    ofxToggle addNoise;
    ofxToggle addSound;
     
    // tracker controls
    ofxFloatSlider  smoothing;
    
    ofxFloatSlider  overlaycam_x;
    ofxFloatSlider  overlaycam_y;
    ofxFloatSlider  overlaycam_z;


   
    // tracker test page
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
    float oldTime,oldTime1;
    ofSpherePrimitive sp1;
    //ofMaterial material;
    ofLight l1;
    audioStreamer::audioDevice audioDev;
    int page = 0;

    ofVec3f headMean;
    ofVec3f wp;
    ofVec3f wpMin;
    ofVec3f wpMax;
    ofVec3f localHead;
    
};
