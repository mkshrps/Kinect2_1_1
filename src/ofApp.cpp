#include "ofApp.h"
//#define PLAYBACK
bool liveDevice;

ofEasyCam cam;
//
//--------------------------------------------------------------
// todo put kinect pointcloud stuff in class
//

void ofApp::setup()
{
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(0);
    panel.setup("", "settings.xml", 10, 100);
    mouseDepth = 0;
    pointSize = 3;
    camGroup.setup("Virtual Camera");
    camGroup.add(cam_x);
    camGroup.add(cam_y);
    camGroup.add(cam_z);
    camGroup.add(cam_heading.setup("cam heading",0,0,360));
    soundGroup.setup("Audio");
    soundGroup.add(volume_l.setup("vol L",0,0,10000));
    soundGroup.add(volume_r.setup("vol R",0,0,10000));
    soundGroup.add(gain.setup("Gain",10,0,100));
    soundGroup.add(noiseGain.setup("Gain",0,0,10));
    soundGroup.add(addNoise.setup("Noise",false));
    soundGroup.add(addSound.setup("Sound",false));

    paramGroup.setup("Pointcloud");
    paramGroup.add(pointSize.setup("point size",3,1,10));
    paramGroup.add(nearclip.setup("near clip",50,20,2000));
    paramGroup.add(farclip.setup("far clip",2000,1000,15000));
    paramGroup.add(colMin.setup("color min",0,0,240));
    paramGroup.add(colMax.setup("col max",255,10,255));
    paramGroup.add(colToDepth.setup("color compress" , false));
    paramGroup.add(ghosts.setup("Ghosting",0,0,5)); 
    paramGroup.add(invert.setup("invert" , false));
    paramGroup.add(getFullDepthRange.setup("Max Depth Range" , true));
    paramGroup.add(showRGB.setup("show rgb",false));
   // paramGroup.add(enableRgbRegistered.setup("show registered",false));

    //paramGroup.add(dpHeight.set("dpHeight",480));
    //paramGroup.add(dpWidth.set("dpWidth",640));

    //    mixerGroup.add( kenabled.setup( "kenabled", true ) );
    recGroup.setup("Depth Recording Status");
    recGroup.add( recordStatus.set( "recording", false ) );
    panel.add(&camGroup);
    panel.add(&soundGroup);
    panel.add(&paramGroup);
    panel.add(&recGroup);

    panel.loadFromFile("settings.xml");
//  edit this when playing back a recorded file
    liveDevice = true;
   

    if(liveDevice){
	    device.setLogLevel(OF_LOG_NOTICE);
        device.setup();
        device.setEnableRegistration();
        cout << "registration support" << device.getEnableRegistration() << endl;
        depth.setup(device);      
        depth.setFps(30);
		depth.start();
            
        if (rgbStream.setup(device)) // only for kinect device (no rgbStream on orbbec astra)
        {    
            
            rgb_w = rgbStream.getWidth();
            rgb_h = rgbStream.getHeight();
            rgbStream.setFps(30);
        //    rgbStream.setWidth(depth.getWidth());
        //    rgbStream.setHeight(depth.getHeight());
            rgbStream.start();
        }
    }
    else{
            playbackDevice.setLogLevel(OF_LOG_NOTICE);
            playbackDevice.setup("record2.oni");
            depth.setup(playbackDevice);
            depth.start();
    } 

    /******************************************* */
    // add sound initialistion
    soundStream.printDeviceList();
    int bufferSize = 256;
    left.assign(bufferSize, 0.0);
    right.assign(bufferSize, 0.0);
    volHistory.assign(400, 0.0);
    
    bufferCounter	= 0;
    drawCounter		= 0;
    smoothedVol     = 0.0;
    scaledVol		= 0.0;

    ofSoundStreamSettings settings;
    
    // if you want to set the device id to be different than the default
    auto devices = soundStream.getDeviceList();
    settings.setInDevice(devices[0]);

	// you can also get devices for an specific api
	// auto devices = soundStream.getDevicesByApi(ofSoundDevice::Api::PULSE);
	// settings.device = devices[0];

	// or get the default device for an specific api:
	// settings.api = ofSoundDevice::Api::PULSE;

	// or by name
    //	auto devices = soundStream.getMatchingDevices("default");
    //	if(!devices.empty()){
    //		settings.setInDevice(devices[0]);
    //	}

    settings.setInListener(this);
    settings.sampleRate = 44100;
    #ifdef TARGET_EMSCRIPTEN
        settings.numOutputChannels = 2;
    #else
        settings.numOutputChannels = 0;
    #endif
    settings.numInputChannels = 2;
    settings.bufferSize = bufferSize;
    soundStream.setup(settings);

    /**************************************************** */   


  
    //    tracker.setup(device);
    framecount = 0;
    resetCamPos();

// cam.setGlobalPosition(200,-300,-1500);

//    cam.lookAt(ofVec3f(200,-300,0.0));
    //cam.setTarget(ofVec3f(0.0,0.0,0.0));

    depthCamView = true;

    //farclip = cam.getFarClip();
 

}


void ofApp::exit()
{
   // tracker.exit();
   // device.exit();
    playbackDevice.exit();
}

//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){
	
	float curVol = 0.0;
	
	// samples are "interleaved"
	int numCounted = 0;	

	//lets go through each sample and calculate the root mean square which is a rough way to calculate volume	
	for (size_t i = 0; i < input.getNumFrames(); i++){
		left[i]		= input[i*2]*0.5;
		right[i]	= input[i*2+1]*0.5;

		curVol += left[i] * left[i];
		curVol += right[i] * right[i];
		numCounted+=2;
	}
	
	//this is how we get the mean of rms :) 
	curVol /= (float)numCounted;
	
	// this is how we get the root of rms :) 
	curVol = sqrt( curVol );
	
	smoothedVol *= 0.93;
	smoothedVol += 0.07 * curVol;
    volume_l = smoothedVol * 100;
    
	bufferCounter++;
	
}

//--------------------------------------------------------------
void ofApp::update()
{
	//lets scale the vol up to a 0-1 range 
	scaledVol = ofMap(smoothedVol, 0.0, 0.17, 0.0, 1.0, true);
    volume_r = scaledVol;

	//lets record the volume into an array
	volHistory.push_back( scaledVol );
	
	//if we are bigger the the size we want to record - lets drop the oldest value
	if( volHistory.size() >= 400 ){
		volHistory.erase(volHistory.begin(), volHistory.begin()+1);
	}


    // update the gui camera settings 
    cam_x = cam.getX();
    cam_y = cam.getY();
    cam_z = cam.getZ();
    cam_heading = cam.getHeading();

    if(liveDevice) {
        device.update();
        if(rgbStream.isFrameNew()){
            rgbTex.loadData(rgbStream.getPixelsRef());
        }
    } 
    else {
        framecount++;
        openni::Device & dev = playbackDevice.get();
        if(framecount > dev.getPlaybackControl()->getNumberOfFrames(depth) - 10){
        dev.getPlaybackControl()->seek(depth,1);
        cout << "reset framecount" << framecount << endl;
        framecount = 0;
        playbackDevice.update();
        }
    }
    // process depth frame same for live or recorded device
    if(depth.isFrameNew()){

        //depthPixels = depth.getPixelsRef(20,6000,false); // access the depth data and specify near and far range for grayscale shading
        // default depth.getPixelsRef() returns depth in mm between 0.5 and 4.5m
        // passing near clip and far clip returns raw depth data i.e. unsigned int value between 0 - 65535
        if(getFullDepthRange){
            depthPixels = depth.getPixelsRef(); // real world depth image
        }
        else{ 
            depthPixels = depth.getPixelsRef(nearclip,farclip,invert); // access the depth data and specify near and far range for grayscale shading
        }

        depthTexture.loadData(depthPixels);

        if(showPointCloud){
        //cout << "frame" << endl;
            createPointCloud_1();
        }
            //tracker.getPixelsRef()
    }
    
//    }

}

//--------------------------------------------------------------
void ofApp::drawSound(){
	
	ofSetColor(225);
	ofDrawBitmapString("AUDIO INPUT EXAMPLE", 32, 32);
	ofDrawBitmapString("press 's' to unpause the audio\n'e' to pause the audio", 31, 92);
	
	ofNoFill();
	
	// draw the left channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 170, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Left Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 512, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (unsigned int i = 0; i < left.size(); i++){
				ofVertex(i*2, 100 -left[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();

	// draw the right channel:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(32, 370, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Right Channel", 4, 18);
		
		ofSetLineWidth(1);	
		ofDrawRectangle(0, 0, 512, 200);

		ofSetColor(245, 58, 135);
		ofSetLineWidth(3);
					
			ofBeginShape();
			for (unsigned int i = 0; i < right.size(); i++){
				ofVertex(i*2, 100 -right[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();
	
	// draw the average volume:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(565, 170, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Scaled average vol (0-100): " + ofToString(scaledVol * 100.0, 0), 4, 18);
		ofDrawRectangle(0, 0, 400, 400);
		
		ofSetColor(245, 58, 135);
		ofFill();		
		ofDrawCircle(200, 200, scaledVol * 190.0f);
		
		//lets draw the volume history as a graph
		ofBeginShape();
		for (unsigned int i = 0; i < volHistory.size(); i++){
			if( i == 0 ) ofVertex(i, 400);

			ofVertex(i, 400 - volHistory[i] * 70);
			
			if( i == volHistory.size() -1 ) ofVertex(i, 400);
		}
		ofEndShape(false);		
			
		ofPopMatrix();
	ofPopStyle();
	
	drawCounter++;
	
	ofSetColor(225);
	string reportString = "buffers received: "+ofToString(bufferCounter)+"\ndraw routines called: "+ofToString(drawCounter)+"\nticks: " + ofToString(soundStream.getTickCount());
	ofDrawBitmapString(reportString, 32, 589);
		
}



//--------------------------------------------------------------
void ofApp::draw()
{
    //depthPixels = tracker.getPixelsRef(1000, 4000);
//    depthPixels = depth.getPixelsRef(300,10000,false);
//    depthTexture.loadData(depthPixels,GL_RGBA);
    if(drawSoundEnabled){
        drawSound();
    }

    else{
    if(showPointCloud){
       
    
        drawPointCloud();

        ofSetColor(255,255,255);
        cam.begin();
        ofEnableDepthTest();
        
        originSphere.setPosition(0,0,0);
        originSphere.setRadius(2);
        originSphere.draw();    
        ofDisableDepthTest();
        cam.end();
    } 
    else
    {
        if(liveDevice){
            
            rgbStream.draw(640,0,rgbStream.getWidth(),rgbStream.getHeight());
//            depthTexture.draw(0,0,640,480);
        }

        // depth texture is allocated automagically when the first Depth frame is updated
        if(depthTexture.isAllocated()){
            depthTexture.draw(0,0,640,480);
            
        }
    }
        ofSetColor(255,255,255);
    //    ofPushMatrix();
        ofDrawBitmapString("Depth xy "+ofToString(mouseDepth),ofGetMouseX(),ofGetMouseY());
    //    ofPopMatrix();
 
    if(showGui){
        panel.draw();
    }
    
    //ofDrawBitmapString("Tracker FPS: "+ofToString(tracker.getFrameRate()),20,ofGetHeight()-40);
    //ofDrawBitmapString("Depth size: "+ofToString(depth.getPixelsRef().getWidth())+" "+ofToString(depth.getPixelsRef().getHeight()),20,ofGetHeight()-40);
    const ofNode &target = cam.getTarget();
    float dirX = target.getX();
    float dirY = target.getY();
    float dirZ = target.getZ();

    stringstream ss;
    ofDrawBitmapString("cam heading "+ofToString(cam.getHeading())+"Target X,Y,Z "+ofToString(dirX)+" "+ofToString(dirY)+" "+ofToString(dirZ),20,ofGetHeight()-60);
    ofDrawBitmapString("Depth size: "+ofToString(depthPixels.getWidth())+" "+ofToString(depthPixels.getHeight()),20,ofGetHeight()-40);
    ofDrawBitmapString("Application FPS: "+ofToString(ofGetFrameRate()),20,ofGetHeight()-20);
    if(!depthCamView){
        ofDrawBitmapString("Virtual Cam View",20,ofGetHeight()-80);
    }
    else{
        ofDrawBitmapString("Depth Cam View",20,ofGetHeight()-80);
    }
}
}

void testDepthValues(){

}


void ofApp::createPointCloud_1(){

    pointCloud.clear();
    pointCloud.setMode(OF_PRIMITIVE_POINTS);
    //unsigned short depthPixel;
    // I originally had this assignment inside the loop which slowed it down a lot
    // if set max depth is true then get whole depth image
    // else limit the range of values using near and far clip
    
    // epthpixels is grabbed during update from depth cam feed
    /*
    if(getFullDepthRange){
        depthPixels = depth.getPixelsRef(); // thhe whole depth image
    }
    else{
        depthPixels = depth.getPixelsRef(nearclip,farclip,invert);
    }
        */
    //dpWidth = depthPixels.getWidth();
    //dpHeight = depthPixels.getHeight();
    //depthPixels = depth.getPixelsRef(100,1000,true);

    //glPointSize(pointSize.get());
    //float noisy_x, noisy_y;
    //float t = (ofGetElapsedTimeMillis()/10.0 ) ;
    float noise ;
    noise = ofNoise(ofRandom(10)) * noiseGain;
    //cout << "Noise value" << noise;
    
    float dx;
    glPointSize(pointSize);
    for (std::size_t y = 0; y < depthPixels.getHeight(); y++)
    {
        for (std::size_t x = 0; x < depthPixels.getWidth(); x++)
        {
            ///depthPixels.getPixelFormat();
            //unsigned short* pixPtr;

            // work out the current depth value from x,y coords 
            int idx = depthPixels.getPixelIndex(x,y);

            //unsigned short dval = depth.getPixelsRef()[idx];
            unsigned short dval = depthPixels[idx];

            //if (y > 200 && y < 250){
              //  cout << "value" << dval << endl;
            //}
            // create a point in the x,y,z space
            dx = x;
            if(addNoise){
                dx += noise;
            } 

            ofPoint point = ofPoint(dx,y,dval); 
        //    ofPoint point = ofPoint(x,y,dval); 
            
            //if (x%200 == 0){
            //cout << "point" << point.x << point.y << point.z << endl;
            //}
            // set limits on the depth that we want to see in the pointcloud
            if(dval >= nearclip && dval <= farclip){
                pointCloud.addVertex(point);
                //pointCloud.addColor(ofColor(0,255,0));
                ofColor col;
                if(!showRGB){
                    if(colToDepth){
                        col.setHsb(ofMap(dval,nearclip,farclip,0,255) + noise,255,255);
                    }
                    else{
                        col.setHsb(ofMap(dval,nearclip,farclip,colMin,colMax)+ noise,255,255);
                    }

                }
                else{
                    
                    col = rgbStream.getPixelsRef().getColor(x,y);
                    col.setBrightness(ofRandom(255));
                }
                    pointCloud.addColor(col);
            }
        }
    }
}


void ofApp::drawPointCloud(){
  //  rgbStream.draw( ofGetWidth()/2.0-rgb_w/8,ofGetHeight()/2.0 - rgb_h/8 , rgb_w/4, rgb_h/4);
        
    ofEnableDepthTest();
    cam.begin();
    ofPushMatrix();
    


    //ofScale(1, 1, 1);
    ofScale(1, -1, 1);
    pointCloud.drawVertices();
    int blur  = ghosts;

    for(int i =0; i<blur; i++){

    
    ofPushMatrix();
    ofTranslate(10 + i*i,10+i*i,0);
        pointCloud.drawVertices();
    ofPopMatrix();
    }

    ofPopMatrix();
    cam.end();
    ofDisableDepthTest();

}

void ofApp::resetCamPos(){
        cam.setGlobalPosition(200,-300,-500);
        cam.lookAt(ofVec3f(200,-300,0.0));
        depthCamView = true;
 }

// file handling , recording, key handling etc

bool ofApp::startRecord(string filename, bool allowLossyCompression, ofxNI2::DepthStream& stream)
{
	//if (recorder) return false;

	if (filename == "")
		filename = ofToString(time(0)) + ".oni";

	ofLogVerbose("ofxNI2") << "recording started: " << filename;
	
	filename = ofToDataPath(filename);
	
	oni_recorder = new openni::Recorder;
	oni_recorder->create(filename.c_str());
	
//		ofxNI2::Stream &s = *streams[i];
		oni_recorder->attach(stream.get(), allowLossyCompression);
	
	oni_recorder->start();
	return oni_recorder->isValid();
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
    if (key == 'p')
    {
        showPointCloud = !showPointCloud;
    }
    if (key == 'd')
    {
        cout << "bytes per pixel" <<  depth.getPixelsRef().getBytesPerPixel() << endl;
    }
    if (key == 'r'){
        if(!recordStatus){
        string str = "K2File_" + ofGetTimestampString() + ".oni" ;
        startRecord(str,false,depth);
        }
        else device.stopRecord();
        recordStatus = !recordStatus;
    }

    if (key == 'h'){
        device.stopRecord();
    }

    if (key == 'm'){
        openni::Device & dev = playbackDevice.get();
        dev.getPlaybackControl()->seek(depth,1);
    }
    if(key == 'z') showGui = !showGui;
    
    if(key == OF_KEY_RETURN) ofSaveScreen("screenshot" + ofToString(ofRandom(0,1000),0) + ".png");
    
    if(key == 's') {
        ofFileDialogResult res;
        res = ofSystemSaveDialog("preset.xml", "Saving Preset");
        if(res.bSuccess) {
            panel.saveToFile(res.filePath);
        }

    }
    if(key == 'l') {

        ofFileDialogResult res;
        res = ofSystemLoadDialog("Loading Preset");
        if(res.bSuccess) panel.loadFromFile(res.filePath);
        // set the camera to the preset position
        // otherwise the gui settings will be updated during the next update
        cam.setGlobalPosition(cam_x,cam_y,cam_z);
        int flipx = 1;
        if(cam_z < 0){
            flipx = -1;
        }
        cam.lookAt(ofVec3f(200,-300 * flipx,0.0));
 
        //cam.set
        //cam.lookAt(ofVec3f(0.0,0.0,0.0));


    }

    if(key=='x'){
       // cam.reset();
        //cam.setGlobalPosition(200,-300,-1000);
        resetCamPos();
    }

    if(key=='c'){
        showRGB = !showRGB;
    }

    // these are just for messing with at the moment
    if(key=='1'){
           // cam.reset();
           cam.setGlobalPosition(300,-240,1500);
           cam.lookAt(ofVec3f(300.0,-240.0,0.0));
           depthCamView = false;
    }

    if(key=='2'){
           // cam.reset();
           cam.setGlobalPosition(300,-240,-500);
           cam.lookAt(ofVec3f(300.0,-240.0,0.0));
           
           depthCamView = true; 
    }

    if( key == 'a' ){
		//soundStream.start();
        drawSoundEnabled = !drawSoundEnabled;
	}
	
	if( key == 'e' ){
		soundStream.stop();
        drawSoundEnabled = false;
	}     
}





//--------------------------------------------------------------
void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y){
    if(!showPointCloud){
        if(depthPixels.isAllocated()){
        }
    }
}

void ofApp::mouseDragged(int x, int y, int button)
{

        
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
    
}
void ofApp::drawSkeleton(){
    //ofVec3f rhPos,lhPos;
    // draw in 2D
    //ofSetColor(0);
    ////depthTexture.draw(0, 0);
    //tracker.draw();


    
    // draw 3D skeleton in 2D
    /*
    ofPushView();
    tracker.getOverlayCamera().begin(ofRectangle(0, 0, depthTexture.getWidth(), depthTexture.getHeight()));
    ofDrawAxis(100);
    tracker.draw3D();
    tracker.getOverlayCamera().end();
    ofPopView();
*/
    // draw in 3D
    /*
    cam.begin();
    ofDrawAxis(200);
    tracker.draw3D();

    // draw box
    ofNoFill();
    ofSetColor(255, 0, 0);
    for (int i = 0; i < tracker.getNumUser(); i++)
    {
        ofxNiTE2::User::Ref user = tracker.getUser(i);
        const ofxNiTE2::Joint &head = user->getJoint(nite::JOINT_HEAD);
       // const ofxNiTE2::Joint &left_hand = user->getJoint(nite::JOINT_RIGHT_HAND);
       // const ofxNiTE2::Joint &right_hand = user->getJoint(nite::JOINT_LEFT_HAND);
        //ofxNiTE2::Joint joint;
        //joint.transformGL();
        ofxNITE2::
        ofSetColor(255,255,255);
        head.transformGL();
        ofDrawSphere(100);
        head.restoreTransformGL();



        //ofDrawBox(300);
        //ofDrawSphere(200);

//        joint.restoreTransformGL();
    }

    cam.end();
*/
    }