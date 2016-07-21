#pragma once

#include "ofMain.h"
#include "ofxSerial.h"
#include "ofxOsc.h"
#include "ofxGrabCam.h"
#include "ofxBiquadFilter.h"

// OSC
#define HOST "localhost"
#define PORT 8000

class SerialMessage
{
public:
    SerialMessage(): fade(0)
    {
    }
    
    SerialMessage(const std::string& _message,
                  const std::string& _exception,
                  int _fade):
    message(_message),
    exception(_exception),
    fade(_fade)
    {
    }
    
    std::string message;
    std::string exception;
    int fade;
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
        void exit();
    
        void serialToPosition(string message);

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
    
    // 3d visualizer
    ofxGrabCam camera;
		
    
    // OSC
    ofxOscSender oscsender;
    
    // PoZYX
    void onSerialBuffer(const ofx::IO::SerialBufferEventArgs& args);
    void onSerialError(const ofx::IO::SerialBufferErrorEventArgs& args);
    
    ofx::IO::BufferedSerialDevice device;
    
    std::vector<SerialMessage> serialMessages;

    
    map<string, ofVec3f> anchors;
    
    struct Tag{
        //POS,network_id,posx,posy,posz,errx,erry,errz,errXY,errXZ,errYZ,range1,rss1,...
        string id;
        ofVec3f rawPosition;
        ofxBiquadFilter3f position;

        ofVec3f error;
        float errXY;
        float errXZ;
        float errYZ;
    };
    
    Tag tag;
    
};
