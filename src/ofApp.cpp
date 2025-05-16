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
    soundGroup.add(volume_l.setup("vol L",0,0,1000));
    soundGroup.add(volume_r.setup("vol R",0,0,10000));
    soundGroup.add(gain.setup("Gain",10,0,100));
    soundGroup.add(noiseGain.setup("Gain",0,0,10));
    soundGroup.add(addNoise.setup("Noise",false));
    soundGroup.add(addSound.setup("Sound",false));

    paramGroup.setup("Pointcloud");
    paramGroup.add(pointSize.setup("point size",3,1,10));
    paramGroup.add(nearclip.setup("near clip",50,20,2000));
    paramGroup.add(farclip.setup("far clip",4000,1000,15000));
    paramGroup.add(colMin.setup("color min",0,0,240));
    paramGroup.add(colMax.setup("col max",255,10,255));
    paramGroup.add(colToDepth.setup("color compress" , false));
    paramGroup.add(ghosts.setup("Ghosting",0,0,5)); 
    paramGroup.add(invert.setup("invert" , false));
    paramGroup.add(getFullDepthRange.setup("Max Depth Range" ,false));
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
    // force depthrange off for first screen
    getFullDepthRange = false;
//  edit this when playing back a recorded file
    liveDevice = true;
    //liveDevice = false;


    if(liveDevice){
	    device.setLogLevel(OF_LOG_NOTICE);
        device.setup();
        device.setEnableRegistration();
        tracker.setup(device);        
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
    // playback from file
    else{
            
        playbackDevice.setLogLevel(OF_LOG_NOTICE);
        playbackDevice.setup("K2File.oni");
        depth.setup(playbackDevice);
        depth.setFps(30);
        depth.start();
        
        
        openni::Device &onidev = playbackDevice.get(); // = playbackDevice.get();
        openni::PlaybackControl *pb = onidev.getPlaybackControl();
                    
        playbackDevice.update();
        
        if (pb->isValid()){
            cout << "PB valid " << endl;
        }
        else{
            cout << "PB not valid " << endl;
        }

        cout << "file y/n" << onidev.isFile() << endl;
        //openni::VideoStream &v = depth.get();
        oldTime = ofGetElapsedTimef();
        // this next line seems to always return -1
        int frames =  pb->getNumberOfFrames(depth);
        // debug only
        //pb->setSpeed(2.0);
        cout << "playback frame count -------------------  " << frames << endl;
        cout << "pb speed --------------------  " << pb->getSpeed() << endl;
    } 

    audioDev.printDeviceList();
    int device = 0; 
    audioDev.setup(0);
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
   tracker.exit();
   device.exit();
   // playbackDevice.exit();
}


void ofApp::update()
{

    audioDev.update();
    volume_l = audioDev.getSmoothedVol()*500;

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
        openni::Device &onidev = playbackDevice.get(); // = playbackDevice.get();
        openni::PlaybackControl *pb = onidev.getPlaybackControl();

        playbackDevice.update();
        if(depth.isFrameNew()){
            oldTime = ofGetElapsedTimef();
            framecount++;
           //openni::Device & dev = playbackDevice.get();
            //cout << "frames" << pb->getNumberOfFrames(depth) << "framecount" << framecount << endl;
            /*
            if(framecount == 100){
            //if(framecount > onidev.getPlaybackControl()->getNumberOfFrames(depth) - 2){
           }
        */
            }
            else{
                // no frames for over 1 second
                if(ofGetElapsedTimef() - oldTime > 1.0){
                // reset to start of file
                onidev.getPlaybackControl()->seek(depth,1);
                cout << "reset framecount" << framecount << endl;
                framecount = 0;
 
                }
            }

    }
    // process depth frame same for live or recorded device
    if(depth.isFrameNew()){

        //depthPixels = depth.getPixelsRef(20,6000,false); // access the depth data and specify near and far range for grayscale shading
        // default depth.getPixelsRef() returns depth in mm between 0.5 and 4.5m
        // passing near clip and far clip returns raw depth data i.e. unsigned int value between 0 - 65535
        if(getFullDepthRange){
            depthPixels = tracker.getPixelsRef();
       //     depthPixels = depth.getPixelsRef(); // real world depth image
        }
        else{ 
            depthPixels = tracker.getPixelsRef(nearclip,farclip,invert); // access the depth data and specify near and far range for grayscale shading
           // depthPixels = depth.getPixelsRef(nearclip,farclip,invert); // access the depth data and specify near and far range for grayscale shading
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
			for (unsigned int i = 0; i < audioDev.getLeftChannel().size(); i++){
				ofVertex(i*2, 100 - audioDev.getLeftChannel()[i]*180.0f);
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
			for (unsigned int i = 0; i < audioDev.getRightChannel().size(); i++){
				ofVertex(i*2, 100 -audioDev.getRightChannel()[i]*180.0f);
			}
			ofEndShape(false);
			
		ofPopMatrix();
	ofPopStyle();
	
	// draw the average volume:
	ofPushStyle();
		ofPushMatrix();
		ofTranslate(565, 170, 0);
			
		ofSetColor(225);
		ofDrawBitmapString("Scaled average vol (0-100): " + ofToString(audioDev.getScaledVol() * 100.0, 0), 4, 18);
		ofDrawRectangle(0, 0, 400, 400);
		
		ofSetColor(245, 58, 135);
		ofFill();		
		ofDrawCircle(200, 200, audioDev.getScaledVol() * 190.0f);
		
		//lets draw the volume history as a graph
		ofBeginShape();
		for (unsigned int i = 0; i < audioDev.getHistory().size(); i++){
			if( i == 0 ) ofVertex(i, 400);

			ofVertex(i, 400 - audioDev.getHistory()[i] * 70);
			
			if( i == audioDev.getHistory().size() -1 ) ofVertex(i, 400);
		}
		ofEndShape(false);		
			
		ofPopMatrix();
	ofPopStyle();
	
	drawCounter++;
	
	ofSetColor(225);
	string reportString = "ScaledVol: "+ofToString(audioDev.getScaledVol())+"\nsmoothedVol: "+ofToString(audioDev.getSmoothedVol())+"\nticks: " + ofToString(audioDev.getStream().getTickCount());
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
        //tracker.draw();
    // draw 3D skeleton in 2D
        ofPushView();
        tracker.getOverlayCamera().begin(ofRectangle(0, 0, depthTexture.getWidth(), depthTexture.getHeight()));
        ofDrawAxis(100);
        tracker.draw3D();
        tracker.getOverlayCamera().end();
        ofPopView();

        // draw in 3D
        cam.begin();
        ofDrawAxis(100);
        tracker.draw3D();

        // draw box
        ofNoFill();
        ofSetColor(255, 0, 0);
        for (int i = 0; i < tracker.getNumUser(); i++)
        {
            ofxNiTE2::User::Ref user = tracker.getUser(i);
            const ofxNiTE2::Joint &joint = user->getJoint(nite::JOINT_HEAD);
    
            joint.transformGL();
            ofDrawBox(300);
            joint.restoreTransformGL();
        }

        cam.end();
    
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
    noise = (ofNoise(ofRandom(10)) -0.5) * noiseGain;
    //cout << "Noise value" << noise;
    
    float dx,dy;
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
            dy = y;

            if(addNoise){
                dx += noise;
                dy += noise;
            } 

            ofPoint point = ofPoint(dx,dy,dval); 
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
                        col.setHsb(ofMap(dval,nearclip,farclip,0,255) ,255,255);
                    }
                    else{
                        col.setHsb(ofMap(dval,nearclip,farclip,colMin,colMax),255,255);
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
        getFullDepthRange = true;
        if(showPointCloud){
            getFullDepthRange = false;
        }
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

    if(key == 'f'){
            openni::Device &onidev = playbackDevice.get();
            openni::PlaybackControl pb = *onidev.getPlaybackControl();
            //openni::Device &dev = onidev();


            cout << "file y/n" << (onidev.isFile()? "Y" : "n") << endl;

            cout << "playback frame count -------------------" << pb.getNumberOfFrames(depth) << endl;

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
		
        audioDev.stop();
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