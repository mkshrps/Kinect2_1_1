#pragma once

#include "ofMain.h"

namespace audioStreamer
{
	void init();
	
	class audioDevice;
	class audioStream;
}

class audioStreamer::audioDevice
{
//	friend class ofxNI2::Stream;
	
public:
	
	audioDevice();
    audioDevice(int _bufferSize, int _historySize);

	~audioDevice();

    bool setup();
    bool setup(int device_id);
    void printDeviceList();
    vector<float> getLeftChannel();
    vector<float> getRightChannel();
    vector<float> getHistory();
    ofSoundStream getStream();
    void stop();
    void start();
    float getScaledVol();
    float getSmoothedVol();
    int getBufferCounter();    
    void update();
    void audioIn(ofSoundBuffer & input);

private:

    vector <float> leftChan;
    vector <float> rightChan;
    vector <float> volHistory;
    int     deviceID;
    int 	bufferCounter;
    int 	drawCounter;
    int bufferSize = 256;
    int historySize = 400;

    float smoothedVol;
    float scaledVol;
    
    ofSoundStream soundStream;
    void exit();
    bool init();
    ofSoundStreamSettings settings;
    
    // if you want to set the device id to be different than the default
	
	//void update();
};



