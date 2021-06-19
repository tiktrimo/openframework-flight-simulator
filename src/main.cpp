#include "ofMain.h"
#include "ofApp.h"
#include <ctime>

//========================================================================
int main( ){
	ofSetupOpenGL(WIDTH, HEIGHT, OF_WINDOW);			// <-------- setup the GL context
	srand(time(NULL));

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
