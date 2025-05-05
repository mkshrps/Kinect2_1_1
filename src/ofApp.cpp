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
    
    pointSize = 3;
    camGroup.setup("Virtual Camera");
    camGroup.add(cam_x);
    camGroup.add(cam_y);
    camGroup.add(cam_z);
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
    paramGroup.add(showRegistered.setup("show registered",false));

    //paramGroup.add(dpHeight.set("dpHeight",480));
    //paramGroup.add(dpWidth.set("dpWidth",640));

    //    mixerGroup.add( kenabled.setup( "kenabled", true ) );
    recGroup.setup("Depth Recording Status");
    recGroup.add( recordStatus.set( "recording", false ) );
    panel.add(&camGroup);
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
            rgbStream.start();
        }
    }
    else{
        playbackDevice.setLogLevel(OF_LOG_NOTICE);
        playbackDevice.setup("record2.oni");
        depth.setup(playbackDevice);
		depth.start();
        //device.setDepthColorSyncEnabled(true);
    //    openni::Device& dev = playbackDevice.operator openni::Device &(); 
    
    //    dev.getPlaybackControl();

    //    cout << "number of frames" << dev.getPlaybackControl()->getNumberOfFrames(depth) << endl;
    //    bool err = dev.getPlaybackControl()->setRepeatEnabled(TRUE);
    //    cout << "set repeat enabled function returned " << err << endl;
    //    cout << "repeat enabled flag" << dev.getPlaybackControl()->getRepeatEnabled() << endl;
        //depth.
        //device.setDepthColorSyncEnabled(true);
    }  
//    tracker.setup(device);
    framecount = 0;
    cam.move(200,-300,2000);
    //farclip = cam.getFarClip();
    

}

void ofApp::exit()
{
   // tracker.exit();
   // device.exit();
    playbackDevice.exit();
}

//--------------------------------------------------------------
void ofApp::update()
{
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
        if(getFullDepthRange){
            depthPixels = depth.getPixelsRef(); // thhe whole depth image
        }
        else{ 
            depthPixels = depth.getPixelsRef(nearclip,farclip,false); // access the depth data and specify near and far range for grayscale shading
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
void ofApp::draw()
{
    //depthPixels = tracker.getPixelsRef(1000, 4000);
//    depthPixels = depth.getPixelsRef(300,10000,false);
//    depthTexture.loadData(depthPixels,GL_RGBA);

    if(showPointCloud){
        if(showRGB){
            rgbStream.draw(0,0,ofGetWidth(),ofGetHeight());
        }
        
        drawPointCloud();
        
    } 
    else
    {
        if(liveDevice){
            rgbStream.draw(640,0,640,480);
            depthTexture.draw(0,0,depthTexture.getWidth(),depthTexture.getHeight());
        }

        // depth texture is allocated automagically when the first Depth frame is updated
        if(depthTexture.isAllocated()){
            depthTexture.draw(0,0,640,480);
            
        }
    }
    if(showGui){
        panel.draw();
    }
    
    //ofDrawBitmapString("Tracker FPS: "+ofToString(tracker.getFrameRate()),20,ofGetHeight()-40);
    //ofDrawBitmapString("Depth size: "+ofToString(depth.getPixelsRef().getWidth())+" "+ofToString(depth.getPixelsRef().getHeight()),20,ofGetHeight()-40);
    ofDrawBitmapString("Depth size: "+ofToString(depthPixels.getWidth())+" "+ofToString(depthPixels.getHeight()),20,ofGetHeight()-40);
    ofDrawBitmapString("Application FPS: "+ofToString(ofGetFrameRate()),20,ofGetHeight()-20);

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
            ofPoint point = ofPoint(x,y,dval); 
            
            //if (x%200 == 0){
            //cout << "point" << point.x << point.y << point.z << endl;
            //}
            // set limits on the depth that we want to see in the pointcloud
            if(dval >= nearclip && dval <= farclip){
                pointCloud.addVertex(point);
                //pointCloud.addColor(ofColor(0,255,0));
                ofColor col;
                if(colToDepth){
                    col.setHsb(ofMap(dval,nearclip,farclip,0,255),255,255);
                }
                else{
                  col.setHsb(ofMap(dval,nearclip,farclip,colMin,colMax),255,255);
                  
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
    

    cam_x = cam.getX();
    cam_y = cam.getY();
    cam_z = cam.getZ();
    ofScale(1, 1, 1);
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

    if (key == 'x'){
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
    }
    if(key=='c'){
        showRGB = !showRGB;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){}
void ofApp::mouseMoved(int x, int y){}

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