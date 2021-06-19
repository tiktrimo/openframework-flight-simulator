#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetFrameRate(FRAME_RATE);
	ofSetBackgroundColor(23, 120, 194);
	ofDisableTextureEdgeHack();
}

//--------------------------------------------------------------
void ofApp::update(){
	// 시각적으로 속도감을 느낄 수 있게 수직선의 위치를 갱신한다.
	gridStartX = gridStartX - aircraft.groundSpeed / 10 ;
	while (gridStartX < 0) {
		gridStartX = gridStartX + 150;
	}
	while (gridStartX > 150) {
		gridStartX = gridStartX - 150;
	}

	// 시각적으로 속도감을 느낄 수 있게 수직선의 위치를 갱신한다.
	groundStartX = groundStartX - aircraft.groundSpeed / 10;
	while (groundStartX < 0) {
		groundStartX = groundStartX + 300;
	}
	while (groundStartX > 300) {
		groundStartX = groundStartX - 300;
	}

	// 시각적으로 고도를 느낄 수 있게 수평선의 위치를 갱신한다.
	gridStartY = gridStartY + aircraft.verticalSpeed / 5;
	while (gridStartY > 150) {
		gridStartY = gridStartY - 150;
	}
	while (gridStartY < 0) {
		gridStartY = gridStartY + 150;
	}

	// 비행기의 상태를 계산한다.
	aircraft.simulate();

	
}

//--------------------------------------------------------------
void ofApp::draw(){
	// 시각적으로 속도감을 느낄 수 있게 수직선을 그려준다.
	ofSetColor(255, 255, 255);
	ofSetLineWidth(1);
	int barX = gridStartX;
	while (barX < WIDTH) {
		ofDrawLine(barX, 0, barX, 15);
		barX = barX + 150;
	}
	int barY = gridStartY;
	while (barY < HEIGHT) {
		ofDrawLine(0, barY, 15, barY);
		barY = barY + 150;
	}

	// 지상을 그린다.
	ofSetLineWidth(2);
	float groundY = (aircraft.altitude ) * 3.6 + aircraft.imageY + 100;
	ofDrawLine(0, groundY , WIDTH, groundY );
	barX = groundStartX;
	while (barX < WIDTH) {
		ofDrawLine(barX, groundY, barX, groundY + 15);
		barX = barX + 300;
	}

	// 비행기를 그린다.
	aircraft.draw();
	float originX = 25 * std::sin(aircraft.pitch * PI / 180);
	float originY = 25 * std::cos(aircraft.pitch * PI / 180);
	aircraft.showFlightVector(originX, originY);
	aircraft.showAltitude(originX, originY);

	//그래프를 그린다.
	aircraft.tacoBell.draw();
	aircraft.tacoBell2.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	switch (key) {
	case OF_KEY_DOWN:
		aircraft.setElevator(aircraft.elevator + 2);
		break;
	case OF_KEY_RIGHT:
		aircraft.setThrottle(aircraft.throttle + 5);
		break;
	case OF_KEY_UP:
		aircraft.setElevator(aircraft.elevator - 2);
		break;
	case OF_KEY_LEFT:
		aircraft.setThrottle(aircraft.throttle - 5);
		break;
	case '2':
		aircraft.setElevatorDifference(0.1);
		break;
	case '8':
		aircraft.setElevatorDifference(-0.1);
		break;
	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	switch (key) {
	case '2':
	case '8':
		aircraft.setElevatorDifference(0);
		break;
	}
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
