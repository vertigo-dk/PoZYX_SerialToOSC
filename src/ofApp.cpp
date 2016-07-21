#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	camera.setCursorDrawEnabled(true);
    
    oscsender.setup(HOST, PORT);
// serial setup
    std::vector<ofx::IO::SerialDeviceInfo> devicesInfo = ofx::IO::SerialDeviceUtils::listDevices();
    
    ofLogNotice("ofApp::setup") << "Connected Devices: ";
    
    for (std::size_t i = 0; i < devicesInfo.size(); ++i)
    {
        ofLogNotice("ofApp::setup") << "\t" << devicesInfo[i];
    }
    
    if (!devicesInfo.empty())
    {
        // Connect to the first matching device.
        bool success = device.setup(devicesInfo[0], 115200);
        
        if(success)
        {
            device.registerAllEvents(this);
            
            ofLogNotice("ofApp::setup") << "Successfully setup " << devicesInfo[0];
        }
        else
        {
            ofLogNotice("ofApp::setup") << "Unable to setup " << devicesInfo[0];
        }
    }
    else
    {
        ofLogNotice("ofApp::setup") << "No devices connected.";
    }
    
    tag.position.setFc(0.01);
    tag.position.setType(OFX_BIQUAD_TYPE_LOWPASS);
    tag.position.clear(ofVec3f(0.));

}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    // send osc to Unity
    ofxOscMessage m;
    m.setAddress("/position");
        m.addFloatArg(tag.position.value().x);
        m.addFloatArg(tag.position.value().z);
        m.addFloatArg(tag.position.value().y);
    oscsender.sendMessage(m, false);
        
}

void ofApp::onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args)
{
    // Buffers will show up here when the marker character is found.
    SerialMessage message(args.getBuffer().toString(), "", 500);
    serialMessages.push_back(message);
    
    
    //Parse the data
    // expected string: POS,network_id,posx,posy,posz,errx,erry,errz,errXY,errXZ,errYZ,range1,rss1,...
    // expected string: ANCHOR,network_id,posx,posy,posz
    if(args.getBuffer().toString().length() > 0){
        //        cout << message << endl;
        vector<string> dataStrings = ofSplitString(args.getBuffer().toString(), ",");
        
        if (dataStrings[0] == "ANCHOR"){
            
            ofVec3f pos = ofVec3f( ofToFloat(dataStrings[2])/1000, ofToFloat(dataStrings[3])/1000, ofToFloat(dataStrings[4])/1000);
            string id = dataStrings[1];
            
            cout << dataStrings[0]<< endl;
            cout << id << endl;
            cout << pos << endl;
            
            anchors[dataStrings[1]] = pos;
            
        }
        
        if (dataStrings[0] == "POS"){
            if(ofToFloat(dataStrings[2])!=0 || ofToFloat(dataStrings[3])!=0 || ofToFloat(dataStrings[4])!=0 ){
                
                // !!! switching Z & Y
                tag.id = dataStrings[1];
                tag.rawPosition = ofVec3f( ofToFloat(dataStrings[2])/1000, ofToFloat(dataStrings[3])/1000, ofToFloat(dataStrings[4])/1000);
                tag.position.update(tag.rawPosition);
                tag.error = ofVec3f( ofToFloat(dataStrings[5])/1000, ofToFloat(dataStrings[6])/1000, ofToFloat(dataStrings[7])/1000);
                tag.errXZ = ofToFloat(dataStrings[8])/1000;
                tag.errXY = ofToFloat(dataStrings[9])/1000;
                tag.errYZ = ofToFloat(dataStrings[10])/1000;
                
                // TODO: range and rss
                //                for( int i = 0; i < NUM_ANCHORS; i++){
                //                    anchors[3][i] = ofToInt(dataStrings[11+(i*2)]);
                //                    //println(dataStrings[11+(i*2)]);
                //                    range_data.get(i).setCurVal(ofToFloat(dataStrings[11+(i*2)])/1000.0f);
                //                    rss_data.get(i).setCurVal(ofToFloat(dataStrings[12+(i*2)]));
                //                }
                
            }
            
        }
    }

}

void ofApp::onSerialError(const ofx::IO::SerialBufferErrorEventArgs& args)
{
    // Errors and their corresponding buffer (if any) will show up here.
    SerialMessage message(args.getBuffer().toString(),
                          args.getException().displayText(),
                          500);
    serialMessages.push_back(message);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);


    camera.begin();
    ofBackground(0);

    ofPushStyle();
    ofSetColor(155,100,100);
    ofDrawGrid(1.0f, 10.0f, true, false, false, true);
    ofDrawAxis(10);
    ofPopStyle();
    
    // Stage
    ofPushStyle();
    ofSetColor(ofColor::deepSkyBlue, 100);
    
    ofPushMatrix();
    ofTranslate(1.22,1.22,0);
    ofDrawPlane(-0.61, -0.61, 1.21, 1.21);
    ofPopMatrix();
    
    ofPushMatrix();
    ofTranslate(1.22,1.22,1.22);
    ofRotateX(90);
    ofDrawPlane(-0.61, -0.61, 1.21, 1.21);
    ofPopMatrix();

    ofPushMatrix();
    ofTranslate(1.22,1.22,0);
    ofRotateY(90);
    ofDrawPlane(-0.61, -0.61, 1.21, 1.21);
    ofPopMatrix();
    
    ofPopStyle();
    
    // DRAW ANCORS
    
    for( map<string, ofVec3f>::iterator ii=anchors.begin(); ii!=anchors.end(); ++ii)
    {
        ofVec3f pos = (*ii).second;
        ofPushStyle();
        ofSetColor(ofColor::orangeRed);
        ofDrawBox(pos, 0.10);
        ofSetColor(255);
        
        std::stringstream sst;
        sst << "ID : " << (*ii).first << std::endl;
        sst << "pos: " << setprecision (2) << fixed << pos;
        ofDrawBitmapString(sst.str(), pos.x+0.1, pos.y-0.15, pos.z);
        ofPopStyle();
        
    }
    
    // draw TAG
    ofPushStyle();
    ofSetColor(ofColor::yellow);
    ofDrawSphere(tag.position.value(), 0.10);
    ofSetColor(ofColor::orange);
    ofDrawSphere(tag.rawPosition, 0.05);

    ofSetColor(255);
    std::stringstream sst;
    sst << "ID : " << tag.id << std::endl;
    sst << "pos: " << setprecision (2) << fixed << tag.position.value();
    ofDrawBitmapString(sst.str(), tag.position.value().x+0.1, tag.position.value().y-0.15, tag.position.value().z);
    ofPopStyle();
    
    camera.end();
    
    
    ofSetColor(255,100);
    
    std::stringstream ss;
    
    ss << "         FPS: " << ofGetFrameRate() << std::endl;
    ss << "Connected to: " << device.getPortName();
    
    ofDrawBitmapString(ss.str(), ofVec2f(20, 20));
    
    int x = 20;
    int y = 50;
    int height = 20;
    
    auto iter = serialMessages.begin();
    
    // Cycle through each of our messages and delete those that have expired.
    while (iter != serialMessages.end())
    {
        iter->fade -= 1;
        
        if (iter->fade < 0)
        {
            iter = serialMessages.erase(iter);
        }
        else
        {
            ofSetColor(255, ofClamp(iter->fade, 0, 50));
            ofDrawBitmapString(iter->message, ofVec2f(x, y));
            
            y += height;
            
            if (!iter->exception.empty())
            {
                ofSetColor(255, 0, 0, ofClamp(iter->fade, 0, 100));
                ofDrawBitmapString(iter->exception, ofVec2f(x + height, y));
                y += height;
            }
            
            ++iter;
        }
    }
    
    


}
//--------------------------------------------------------------
void ofApp::exit()
{
    device.unregisterAllEvents(this);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
