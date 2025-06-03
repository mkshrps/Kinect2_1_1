#include "ofApp.h"
//#define PLAYBACK
bool liveDevice = true;
bool firstRun = true;

//
//--------------------------------------------------------------
// todo put kinect pointcloud stuff in class
//

void ofApp::setup()
{
    enableTracker = true;
    trackerGroup.setBackgroundColor(ofColor::blue);
    //smoothing.setBackgroundColor(ofColor::blue);

    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofBackground(0);
    panel.setup("", "settings.xml", 10, 100);
    mouseDepth = 0;
    pointSize = 1;
    //colControls.addListener(this,colControlSwitch);
    camGroup.setup("Virtual Camera");
    camGroup.add(cam_x);
    camGroup.add(cam_y);
    camGroup.add(cam_z);
    camGroup.add(orientParam);


    //camGroup.add(orientParam);
    camGroup.add(cam_heading.setup("cam heading",0,0,360));
    paramGroup.setup("Pointcloud");
    paramGroup.add(pointSize.setup("point size",1,1,10));
    paramGroup.add(nearclip.setup("near clip",50,20,2000));
    paramGroup.add(farclip.setup("far clip",4000,1000,15000));
    paramGroup.add(colControls.setup("enable color controls" , false));
    paramGroup.add(colMin.setup("color min",0,0,240));
    paramGroup.add(colMax.setup("col max",255,10,255));
    paramGroup.add(ghosts.setup("Ghosting",0,0,5)); 
    paramGroup.add(invert.setup("invert" , false));
    paramGroup.add(getFullDepthRange.setup("Max Depth Range" ,false));
    paramGroup.add(showRGB.setup("show rgb",false));
    paramGroup.add(pcEnableTracking.setup("Enable Tracking",false));
    
    trackerGroup.setup("Tracker");
    trackerGroup.add(headDetected);
    trackerGroup.add(head_x);
    trackerGroup.add(head_y);
    trackerGroup.add(head_z);

    trackerGroup.add(smoothing.setup("smoothing", 0.8,0.5,1.0));
    trackerGroup.add(drawDepthOnTracker.setup("show depth image",false));
    trackerGroup.add(enSkel.setup("draw3D",true));
   // trackerGroup.add(overlaycam_x.setup("overlay cam X",0,-1000,1000));
   // trackerGroup.add(overlaycam_y.setup("overlay cam Y",0,-1000,1000));
   // trackerGroup.add(overlaycam_z.setup("overlay cam Z",0,-1000,1000));

    // paramGroup.add(enableRgbRegistered.setup("show registered",false));
    soundGroup.setup("Audio");
    soundGroup.add(volume_l.setup("vol L",0,0,1000));
    soundGroup.add(volume_r.setup("vol R",0,0,10000));
    soundGroup.add(gain.setup("Gain",10,0,100));
    soundGroup.add(noiseGain.setup("Gain",0,0,10));
    soundGroup.add(addNoise.setup("Noise",false));
    soundGroup.add(addSound.setup("Sound",false));
    paramGroup.minimize();
    camGroup.minimize();
    soundGroup.minimize();
    recGroup.minimize();

    
    //paramGroup.add(dpHeight.set("dpHeight",480));
    //paramGroup.add(dpWidth.set("dpWidth",640));

    //    mixerGroup.add( kenabled.setup( "kenabled", true ) );
    recGroup.setup("Depth Recording Status");
    recGroup.add( recordStatus.set( "recording", false ) );
    panel.add(&paramGroup);
    panel.add(&trackerGroup);
    panel.add(&recGroup);
    panel.add(&camGroup);
    panel.add(&soundGroup);

    panel.loadFromFile("settings.xml");
    panel.setPosition(10,10);

    // force depthrange off for first screen
    getFullDepthRange = false;
//  edit this when playing back a recorded file
    //liveDevice = true;
    //liveDevice = false;

    skellyPage = false;

    if(liveDevice){
	    device.setLogLevel(OF_LOG_NOTICE);
        device.setup();
        device.setEnableRegistration();
        //cout << "registration support" << device.getEnableRegistration() << endl;
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
        
       //openni::VideoStream &v = depth.get();
        oldTime = ofGetElapsedTimef();
        // this next line seems to always return -1
        int frames =  pb->getNumberOfFrames(depth);
        // debug only
        //pb->setSpeed(2.0);
    } 
    
    // start the HI tracker
    if(enableTracker){
        tracker.setup(device);  
        cout << "tracker width" << tracker.getPixelsRef().getWidth() << "tracker Height " << tracker.getPixelsRef().getHeight() << endl;
        
    }

        
    audioDev.printDeviceList();
    int device = 0; 
    audioDev.setup(3);
    framecount = 0;
    resetCamPos();

    // cam.setGlobalPosition(200,-300,-1500);

    //    cam.lookAt(ofVec3f(200,-300,0.0));
    //cam.setTarget(ofVec3f(0.0,0.0,0.0));

    depthCamView = true;
    sp1.set(60,100);

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
    if(colControls){
        colMin.setFillColor(ofColor::gray);
        colMax.setFillColor(ofColor::gray);

    }
    else{
        colMin.setFillColor(ofColor::black);
        colMax.setFillColor(ofColor::black);
    }

    audioDev.update();
    volume_l = audioDev.getSmoothedVol()*500;

    // update the gui camera settings 
    cam_x = cam.getX();
    cam_y = cam.getY();
    cam_z = cam.getZ();
    cam_heading = cam.getHeading();
    ofVec4f vec;
    vec.x = cam.getGlobalOrientation().x;    
    vec.y = cam.getGlobalOrientation().y;    
    vec.z = cam.getGlobalOrientation().z;    
    vec.w = cam.getGlobalOrientation().w;    
    orientParam = vec;


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

            //depthPixels = depth.getPixelsRef(); // real world depth image
        }
        else{ 
            depthPixels = tracker.getPixelsRef(nearclip,farclip,invert); // access the depth data and specify near and far range for grayscale shading
           // depthPixels = depth.getPixelsRef(nearclip,farclip,invert); // access the depth data and specify near and far range for grayscale shading
        }

        depthTexture.loadData(depthPixels);

        //if(showPointCloud){
        //cout << "frame" << endl;
            trackJoint();
            createPointCloud_1();
        //}
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
    // draw sound monitor page for now
    switch (page){
        case 0:
        drawStartPage();
        break;

        case 1:
        drawPointCloud(true);   // enable camera insidde 
        ofSetColor(255,255,255);
        break;

        case 2:
        drawSkeleton();
        break;

        case 3:
        drawSound();
        break;

        default:
        page = 0;

    }
    //    ofPushMatrix();
    //    ofDrawBitmapString("Depth xy "+ofToString(mouseDepth),ofGetMouseX(),ofGetMouseY());
    //    ofPopMatrix();
 
    if(showGui){
        panel.draw();
    }
    
    // do some footer debug stuff
    //ofDrawBitmapString("Tracker FPS: "+ofToString(tracker.getFrameRate()),20,ofGetHeight()-40);
    //ofDrawBitmapString("Depth size: "+ofToString(depth.getPixelsRef().getWidth())+" "+ofToString(depth.getPixelsRef().getHeight()),20,ofGetHeight()-40);
    const ofNode &target = cam.getTarget();
    float dirX = target.getX();
    float dirY = target.getY();
    float dirZ = target.getZ();

    stringstream ss;
    
    //ofDrawBitmapString("users tracked "+ofToString(tracker.getNumUser()),20,ofGetHeight()-100);
    ofDrawBitmapString("Application FPS: "+ofToString(ofGetFrameRate()),20,ofGetHeight()-20);
    if(!depthCamView){
        ofDrawBitmapString("Virtual Cam View",20,ofGetHeight()-40);
    }
    else{
        ofDrawBitmapString("Depth Cam View",20,ofGetHeight()-40);
    }
}

void ofApp::drawStartPage(){

        if(liveDevice){
                rgbStream.draw(640,0,rgbStream.getWidth(),rgbStream.getHeight());
        }

        // depth texture is allocated automagically when the first Depth frame is updated
        if(depthTexture.isAllocated()){
                depthTexture.draw(0,0,640,480);
        }
 
}


void ofApp::trackJoint(){
    
    tracker.setSkeletonSmoothingFactor(smoothing);
    // track the users
    for (int i = 0; i < tracker.getNumUser(); i++)
        {
            ofxNiTE2::User::Ref user = tracker.getUser(i);
            
            const ofxNiTE2::Joint &headJoint = user->getJoint(nite::JOINT_HEAD);
            const ofxNiTE2::Joint &joint_l = user->getJoint(nite::JOINT_LEFT_HAND);
            const ofxNiTE2::Joint &joint_r = user->getJoint(nite::JOINT_RIGHT_HAND);
            //joint.getPosition();
            
            headDetected = headJoint.getPositionConfidence(); // not returning a value ??
            headDetected = 1.0;
            
            headPosition = headJoint.getGlobalPosition();
            // corect for openGL Z direction
            headPosition.z *= -1;
            // display coordinates for debugging
            head_x = headPosition.x;
            head_y = headPosition.y;
            head_z = headPosition.z;

//            glm::vec3 scale(1.0,1.0,-1.0);

        }
}

void ofApp::drawSkeleton(){
        // tracker.draw();
        // draw 3D skeleton in 2D
        //ofPushView();
        //tracker.getOverlayCamera().begin(ofRectangle(0, 0, depthTexture.getWidth(), depthTexture.getHeight()));
    
    //    tracker.getOverlayCamera().begin(ofRectangle(0,0,cam.screenToWorld(depthTexture.getWidth()));
    //    ofDrawAxis(100);
    //    tracker.draw3D();
    //    tracker.getOverlayCamera().end();
        //ofPopView();

        // draw in 3D
        //cam.begin();
        ofPushMatrix();
        

        //depthTexture.draw(0,0); 

        if(depthTexture.isAllocated()){
            //cout << "teture allocated" << depthTexture.getWidth() << endl;
            if(drawDepthOnTracker){
                depthTexture.draw((ofGetWidth()/2)-320,(ofGetHeight()/2)-240,640,480);
            }
        }
        
        ofPushMatrix();
        ofEnableDepthTest();
        ofSetColor(ofColor::white);
        ofScale(1,-1,1);
        
               
        tracker.getOverlayCamera().begin(ofRectangle((ofGetWidth()/2)-320,(ofGetHeight()/2)-240, depthTexture.getWidth(), depthTexture.getHeight()));
        
        glm::vec3 campos = tracker.getOverlayCamera().getGlobalPosition(); 
        overlaycam_x = campos.x;
        overlaycam_y = campos.y;
        overlaycam_z = campos.z;
        //cam.begin();
        //drawPointCloud(false);
        if(enSkel){
            tracker.draw3D();     // draws the skeleton
        }
        //tracker.draw();
        tracker.setSkeletonSmoothingFactor(smoothing);
        // draw box
        
        //ofNoFill();
        ofDrawAxis(100);

        ofSetColor(255, 0, 0);

        for (int i = 0; i < tracker.getNumUser(); i++)
        {
            ofxNiTE2::User::Ref user = tracker.getUser(i);
            
            const ofxNiTE2::Joint &joint = user->getJoint(nite::JOINT_HEAD);
            const ofxNiTE2::Joint &joint_l = user->getJoint(nite::JOINT_LEFT_HAND);
            const ofxNiTE2::Joint &joint_r = user->getJoint(nite::JOINT_RIGHT_HAND);

            //joint.getPosition();
            head = joint.getGlobalPosition();
            head.z *= -1;
            
            //cam.worldToCamera(head);
            //cam.lookAt(head);
            
            joint.transformGL();
            ofDrawBox(100);
            //cam.lookAt(joint.getGlobalPosition());
            joint.restoreTransformGL();
            joint_l.transformGL();
            ofDrawBox(30);
            joint_l.restoreTransformGL();
            joint_r.transformGL();
            ofDrawBox(30);
            joint_r.restoreTransformGL();


        }
        ofSetColor(ofColor::white);
        //cam.end();
        tracker.getOverlayCamera().end();
        ofDisableDepthTest();
        ofPopMatrix();

        ofPopMatrix();

       // cam.end();
        // display coordinates
        stringstream ss;
        ss << "Head Coordinates x,y,x " << head.x << "," << head.y << "," << head.z  << endl; 
        ofDrawBitmapString(ss.str(),20,ofGetWidth() -20);

        
    } 


void testDepthValues(){

}


void ofApp::createPointCloud_1(){

    pointCloud.clear();
    pointCloud.setMode(OF_PRIMITIVE_POINTS);
    
 
    //glPointSize(pointSize.get());
    //float noisy_x, noisy_y;
    //float t = (ofGetElapsedTimeMillis()/10.0 ) ;
    float noise ;
    noise = (ofNoise(ofRandom(10)) -0.5) * noiseGain;
    //cout << "Noise value" << noise;
   //headMean = ofVec3f(0,0,0);
    wpMin = ofVec3f(1000,1000,10000);
    wpMax = ofVec3f(-500,0,0);
    ofVec2f xMax, yMax = ofVec2f(0,0);
    ofVec2f xMin = ofVec2f(depth.getWidth(),0);
    ofVec2f yMin = ofVec2f(0,depth.getHeight());
    float zMax = 0;
    float zMin = 15000;
    unsigned short dval;
    ofPoint point;
    float dx,dy;
    glPointSize(pointSize);
    float sv = audioDev.getScaledVol();
    float vol = ofMap(sv,0,1,0,10);
    int iVol = vol;
    ghosts = vol;

    for (std::size_t y = 0; y < depthPixels.getHeight(); y++)
    {
        for (std::size_t x = 0; x < depthPixels.getWidth(); x++)
        {
            ///depthPixels.getPixelFormat();
            //unsigned short* pixPtr;
            wp = depth.getWorldCoordinateAt(x,y);
            
            // work out the current depth value from x,y coords 
            int idx = depthPixels.getPixelIndex(x,y);

            //unsigned short dval = depth.getPixelsRef()[idx];
            dval = depthPixels[idx];

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
            // just save the lowest point
            point = ofPoint(dx,dy,dval); 
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
                    if(colControls){
                        // map colors to controls
                        col.setHsb(ofMap(dval,nearclip,farclip,colMin,colMax),255,255);
                    }
                    else{
                        // map colors to depth
                        col.setHsb(ofMap(dval,nearclip,farclip,0,255) ,255,255);
                    }

                }
                else{
                    
                    col = rgbStream.getPixelsRef().getColor(x,y);
                    col.setBrightness(ofRandom(255));
                }
                // ignore normal display if using tracker
                if(pcEnableTracking){
                    // check if we are in the head region
                    if(headDetected > 0.1){
                        // detect the whole head region for drawing 
                    
                        if(abs(headPosition.x - wp.x) < 100 && abs(headPosition.y - wp.y) < 100 && abs(headPosition.z - wp.z ) < 500 ){
                        // you can set a different col etc here if required 
                        // or just leave color as defined by the normal gui controls 
                                                      // now test if we are in a close region of head centre
                            if(abs(headPosition.x - wp.x) < 10 && abs(headPosition.y - wp.y) < 10 ){
                            // calculate head centre


                                if (x < xMin.x) {
                                    xMin.x = x;
                                    xMin.y = y;
                                }
                                if (y < yMin.y){
                                    yMin.y = y;
                                    yMin.x = x;
                                }
                                if (dval < zMin) zMin = dval;
                                if (x > xMax.x){
                                    xMax.x = x;
                                    xMax.y = y;
                                }
                                if (y > yMax.y){
                                    yMax.y = y; 
                                    yMax.x = x;
                                }
                                if (dval > zMax) zMax = dval;

                            }

                            col.setHsb(ofMap(abs(headPosition.x - wp.x),0,100,colMin,colMax),255,255);

                            //ofQuaternion q;

                            //cout << "wp.x , wpMax.x " <<  wp.x << wpMax.x << endl;
                            //cout << "wp.x , wpMin.x " <<  wp.x << wpMin.x << endl;

                        //cout << "head pos " << headPosition.x << "," << headPosition.y << "," << headPosition.z << " world Pos " << wp.x << "," << wp.y << "," << wp.z << endl; 
                        // col = ofColor::white;
                        }
                        else{
                            col = ofColor::black;
                        }
                    }
                }
                pointCloud.addColor(col);
               
            }
        }
    }
    
        // finished scan
        if(pcEnableTracking){
            firstRun = false;
        }
   
        // calculate centre of head

        headMean.x = (wpMax.x - wpMin.x)/2 + wpMin.x;
        headMean.y = (wpMax.y - wpMin.y)/2 + wpMin.y;
        headMean.z = (wpMax.z - wpMin.z)/2 + wpMin.z;
        localHead.x = (xMax.x - xMin.x)/2 + xMin.x;
        localHead.y = (yMax.y - yMin.y)/2 + yMin.y;
        localHead.z = (zMax - zMin)/2 + zMin;
    
 }



void ofApp::drawPointCloud(bool enableCam){
  //  rgbStream.draw( ofGetWidth()/2.0-rgb_w/8,ofGetHeight()/2.0 - rgb_h/8 , rgb_w/4, rgb_h/4);
   ofNode headIndicatorNode;
        
   
    //l1.enable();
    //l1.lookAt(sp1);
    //headIndicatorNode.setGlobalPosition(headPosition);

     if(enableCam){
//        cam.setPosition(ofVec3f(headMean.x,headMean.y,headMean.z + 1000));
        cam.begin();
    }
    ofEnableDepthTest();

    ofPushMatrix();
    ofScale(1, -1, 1);
    pointCloud.drawVertices();
    //ofVec3f head = headMean;
    //head.y *= -1;
    
    headIndicatorNode.setGlobalPosition(localHead);
    
    headIndicatorNode.transformGL();
        ofSetColor(ofColor::green);
        ofDrawBox(5);
        ofSetColor(ofColor::white);
    headIndicatorNode.restoreTransformGL();
    //localHead.z = 1000;
    //cam.setTarget(localHead);
    //ofDrawAxis(200);

    int blur  = ghosts;

    for(int i =0; i<blur; i++){
        ofPushMatrix();
            ofTranslate(10 + i*i,10+i*i,0);
            pointCloud.drawVertices();
        ofPopMatrix();
    }
    //sp1.setPosition(230,300,2000);
    material.setAmbientColor(ofColor::aquamarine);
    material.setMetallic(0.5);
    material.begin();
    //sp1.draw();
    material.end();
    ofVec3f mouse3DCoords;
    mouse3DCoords = cam.screenToWorld(ofVec3f(ofGetMouseX(),ofGetMouseY(),0),ofGetCurrentViewport());
    ofPopMatrix();
    
    ofDisableDepthTest();
   
    if(enableCam){
        cam.end();
    }
   
    stringstream ss;
//    stringstream ss1, ss2,ss3,ms;

    ss << "head Node " << headIndicatorNode.getX() << "," <<  headIndicatorNode.getY() << "," << headIndicatorNode.getZ() << endl; 
//    ss1 << "WP Mean " << headMean.x << "," <<  headMean.y << "," << headMean.z << endl;
//    ss2 << "head Min x,y,z " << wpMin.x << ", " << wpMin.y << ", " << wpMin.z <<  endl; 
//    ss3 << "head Max x,y,z " << wpMax.x << ", " << wpMax.y << ", " << wpMax.z << endl; 
    //ms << "Mouse xyz" << mouse3DCoords.x << "," << mouse3DCoords.y << ", " << mouse3DCoords.z << endl;

    ofDrawBitmapString(ss.str(),20,ofGetHeight() -80); 
//    ofDrawBitmapString(ss1.str(),20,ofGetHeight() -100); 
//    ofDrawBitmapString(ss2.str(),20,ofGetHeight() -120); 
//    ofDrawBitmapString(ss3.str(),20,ofGetHeight() -140); 
//    ofDrawBitmapString(ms.str(),20,ofGetHeight() -160); 

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
    // a - audio on
    if( key == 'a' ){
		//soundStream.start();
        page = 0;
        drawSoundEnabled = !drawSoundEnabled;
        if (drawSoundEnabled){
            page = 3;
        }

	}
	// e- audio off
	if( key == 'e' ){
		
        audioDev.stop();
//        drawSoundEnabled = false;
        
	}     
    
    if(key=='c'){
        showRGB = !showRGB;
    }
    
    // only used for debug
    if(key == 'f'){
            openni::Device &onidev = playbackDevice.get();
            openni::PlaybackControl pb = *onidev.getPlaybackControl();
            //openni::Device &dev = onidev();


            cout << "file y/n" << (onidev.isFile()? "Y" : "n") << endl;

            cout << "playback frame count -------------------" << pb.getNumberOfFrames(depth) << endl;

    }


    if (key == 'h'){
        device.stopRecord();
    }

    if (key == 'k'){
        page = 0;
        skellyPage = !skellyPage;
        if(skellyPage){
            page = 2;
        }
    }
    
    if(key == 'l') {

        ofFileDialogResult res;
        res = ofSystemLoadDialog("Loading Preset");
        if(res.bSuccess) panel.loadFromFile(res.filePath);

        // set the camera to the preset position
        // otherwise the gui settings will be updated during the next update
        cam.setGlobalPosition(cam_x,cam_y,cam_z);
        glm::quat q;
        ofVec4f vec;
        vec = orientParam;
        q.x = vec.x;
        q.y = vec.y;
        q.z = vec.z;
        q.w = vec.w;

        cam.setGlobalOrientation(q);
       /* 
        int flipx = 1;
        if(cam_z < 0){
            flipx = -1;
        }
        cam.lookAt(ofVec3f(200,-300 * flipx,0.0));
 
        //cam.set
        //cam.lookAt(ofVec3f(0.0,0.0,0.0));
        */

    }

 
    if (key == 'm'){
        openni::Device & dev = playbackDevice.get();
        dev.getPlaybackControl()->seek(depth,1);
    }
    // p - full depth range
     if (key == 'p')
    {
        page = 0;
        showPointCloud = !showPointCloud;
        getFullDepthRange = false;
        if(showPointCloud){
            page = 1;
            getFullDepthRange = true;
        }
            
    }

    if (key == 'r'){
        if(!recordStatus){
        string str = "K2File_" + ofGetTimestampString() + ".oni" ;
        startRecord(str,false,depth);
        }
        else device.stopRecord();
        recordStatus = !recordStatus;
    }

    if(key == 's') {
        ofFileDialogResult res;
        res = ofSystemSaveDialog("preset.xml", "Saving Preset");
        if(res.bSuccess) {
            panel.saveToFile(res.filePath);
        }

    }
    if(key == 't'){
        pcEnableTracking = true;
    }

    if(key == 'u' ){
        // trigger world coord output
        firstRun = true;
    }    
    if(key=='x'){
       // cam.reset();
        //cam.setGlobalPosition(200,-300,-1000);
        resetCamPos();
    }

    if(key == 'y'){
        tracker.clear();
    }

    if(key == 'z') showGui = !showGui;
    
    if(key == OF_KEY_RETURN) ofSaveScreen("screenshot" + ofToString(ofRandom(0,1000),0) + ".png");
    
    // these are just for messing with at the moment
    if(key=='1'){
           // cam.reset();
           cam.setGlobalPosition(300,-240,3000);
           cam.lookAt(ofVec3f(300.0,-240.0,0.0));
           depthCamView = true;
    }

    if(key=='2'){
           // cam.reset();
           cam.setGlobalPosition(300,-240,-2200);
           cam.lookAt(ofVec3f(300.0,-240.0,0.0));
           depthCamView = false; 
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

/*void ofApp::drawSkeleton(){
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

    }
*/    
