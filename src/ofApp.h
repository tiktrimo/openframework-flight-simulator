#pragma once

#include <cmath>
#include "ofMain.h"
#include "Aircraft.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

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
		ofColor colorSet;

		// value used for blinking
		int flipper = 0;
		// vertical/horizontal line's start position
		float gridStartX = 0;
		float groundStartX = 0;
		float gridStartY = 0;

		// simulated aircraft
		Aircraft aircraft = Aircraft(27);

		// Flags
		bool stallFlag;
		bool stallSoundFlag = false;
		bool rattleSoundFlag = false;
		bool stressSoundFlag = false;
		bool crashFlag = false;
		bool crashBlinkFlag = false;
		bool graphFlag = false;

		// Audio players
		ofSoundPlayer stallSound;
		ofSoundPlayer rattleSound;
		ofSoundPlayer engineSound;
		ofSoundPlayer crashSound;
		ofSoundPlayer audioSound;
		ofSoundPlayer stressSound;
		

};
