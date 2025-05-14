#include "audioStreamer.h"

namespace audioStreamer
{

    audioDevice::audioDevice(){
        bufferSize = 256;
        historySize = 400;
    }
    audioDevice::audioDevice(int _bufferSize,int _historySize){
        bufferSize = _bufferSize;
        historySize = _historySize;
        
    }

    audioDevice::~audioDevice(){
        exit();
    }


    bool audioDevice::init(){    
        leftChan.assign(bufferSize, 0.0);
        rightChan.assign(bufferSize, 0.0);
        volHistory.assign(historySize, 0.0);
        
        bufferCounter	= 0;
        drawCounter		= 0;
        smoothedVol     = 0.0;
        scaledVol		= 0.0;
        auto devices = soundStream.getDeviceList();
        
        settings.setInDevice(devices[deviceID]);
        settings.setInListener(this);
        settings.sampleRate = 44100;
        settings.numOutputChannels = devices[deviceID].outputChannels;
        settings.numInputChannels = devices[deviceID].inputChannels;
        settings.bufferSize = bufferSize;
        return soundStream.setup(settings);
    }
    
    void audioDevice::exit(){
        soundStream.close();
    }

    void audioDevice::start(){
        soundStream.start();
    }
    
    void audioDevice::stop(){
        soundStream.stop();
    }
    
    vector<float> audioDevice::getHistory(){
        return volHistory;
    } 
    
    vector<float> audioDevice::getLeftChannel(){
        return leftChan;
    };
    
    vector<float>  audioDevice::getRightChannel(){
        return rightChan;
    };
    
    float audioDevice::getScaledVol(){
        return scaledVol;
    }

    float audioDevice::getSmoothedVol(){
        return smoothedVol;
    }

    int audioDevice::getBufferCounter(){
        return bufferCounter;
    }

    bool audioDevice::setup(){
        deviceID = 0; // setup default dev ID
        return init();
    }

    bool audioDevice::setup(int _deviceID){
        deviceID = _deviceID;
        return init();
    }

    void audioDevice::printDeviceList(){
        soundStream.printDeviceList();
    }
    
    void audioDevice::audioIn(ofSoundBuffer & input){
	
        float curVol = 0.0;
        
        // samples are "interleaved"
        int numCounted = 0;	
    
        //lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
        for (size_t i = 0; i < input.getNumFrames(); i++){
            leftChan[i]		= input[i*2]*0.5;
            rightChan[i]	= input[i*2+1]*0.5;
    
            curVol += leftChan[i] * leftChan[i];
            curVol += rightChan[i] * rightChan[i];
            numCounted+=2;
        }
        
        //this is how we get the mean of rms :) 
        curVol /= (float)numCounted;
        
        // this is how we get the root of rms :) 
        curVol = sqrt( curVol );
        
        smoothedVol *= 0.93;
        smoothedVol += 0.07 * curVol;
       // volume_l = smoothedVol * 100;
        cout << "smoothed" << smoothedVol << endl;
        bufferCounter++;
        
    }

    void audioDevice::update(){
        //lets scale the vol up to a 0-1 range 
        scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
        //volume_r = scaledVol; // set the gui indicator

        //lets record the volume into an array
        volHistory.push_back( scaledVol );
        
        //if we are bigger the the size we want to record - lets drop the oldest value
        if( volHistory.size() >= historySize ){
            volHistory.erase(volHistory.begin(), volHistory.begin()+1);
        }

    }
    
    ofSoundStream audioDevice::getStream(){
        return soundStream;
    }
    

}