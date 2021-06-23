#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
	ofSetFrameRate(FRAME_RATE);
	ofSetBackgroundColor(23, 120, 194);
	ofDisableTextureEdgeHack();

	stallSound.load("sounds/stallWarning.mp3");
	rattleSound.load("sounds/rattle.mp3");
	engineSound.load("sounds/engine.mp3");
	crashSound.load("sounds/crash.mp3");
	audioSound.load("sounds/audio.mp3");
	stressSound.load("sounds/stress.mp3");
	engineSound.play();
	engineSound.setLoop(true);
}

//--------------------------------------------------------------
void ofApp::update() {
	if (aircraft.altitude <= 0 || (std::abs(aircraft.G) > aircraft.maxG && rand() % 100 < 5)) {
		if(!crashFlag) // plane crashed. stop simulation()
		{
			crashFlag = true;

			// stop playing flying sounds
			stallSound.stop();
			rattleSound.stop();
			engineSound.stop();
			stressSound.stop();

			// play crash sound
			crashSound.play();
			audioSound.play();
			audioSound.setVolume(0.08);

			// blink the screen
			flipper = 600;
			crashBlinkFlag = true;
			ofSetBackgroundColor(54, 4, 2);

			// show graph to anlyze suituation
			graphFlag = true;
		}
	}

	if(!crashFlag)
	{
		// update vertical line. this line visualize speed of aircraft
		gridStartX = gridStartX - aircraft.groundSpeed / 10;
		while (gridStartX < 0) {
			gridStartX = gridStartX + 150;
		}
		while (gridStartX > 150) {
			gridStartX = gridStartX - 150;
		}

		// update vertical line. this line visualize ground
		groundStartX = groundStartX - aircraft.groundSpeed / 10;
		while (groundStartX < 0) {
			groundStartX = groundStartX + 300;
		}
		while (groundStartX > 300) {
			groundStartX = groundStartX - 300;
		}

		// update horizontal line. this line visualize altitude of aircraft
		gridStartY = gridStartY + aircraft.verticalSpeed / 5;
		while (gridStartY > 150) {
			gridStartY = gridStartY - 150;
		}
		while (gridStartY < 0) {
			gridStartY = gridStartY + 150;
		}

		// simulate aircraft
		aircraft.simulate();

		// update flags
		if (aircraft.angleOfAttack > aircraft.maxAngleOfAttack)
			stallFlag = true;
		else
			stallFlag = false;

		// update audio
		if (flipper % 5 == 1) {
			// play plane rattle sound
			if (std::abs(aircraft.angleOfAttack) > 3) {
				if (!rattleSoundFlag) {
					rattleSound.setVolume(0.01);
					rattleSound.play();
					rattleSound.setLoop(true);
					rattleSoundFlag = true;
				}
				else {
					// rattle sound get bigger as aoa increase
					float volume = std::min(std::pow(std::min((std::abs(aircraft.angleOfAttack) - 3), 20.0f), 2) * 0.1 * std::min(aircraft.airSpeed / 600, 1.0f), 7.0);
					rattleSound.setVolume(volume);
				}
			}
			else { // stop playing rattle sound
				if (rattleSoundFlag) {
					rattleSound.setVolume(rattleSound.getVolume() - 0.03);

					if (rattleSound.getVolume() <= 0.01) {
						rattleSoundFlag = false;
						rattleSound.stop();
					}
				}
			}

			// play stall warning sound(stick shaker)
			if (stallFlag && !stallSoundFlag) {
				stallSound.setVolume(0.54);
				stallSound.play();
				stallSoundFlag = true;
			}
			else if (!stallFlag && stallSoundFlag) { // stop playing warning sound
				stallSound.setVolume(stallSound.getVolume() - 0.1);

				if (stallSound.getVolume() <= 0.01) {
					stallSoundFlag = false;
					stallSound.stop();
				}
			}

			// play airframe stress sound
			if (std::abs(aircraft.G) > aircraft.maxG - 2  && !stressSoundFlag) {
				stressSound.setVolume(0.8);
				stressSound.play();
				stressSoundFlag = true;
			}
			else if (std::abs(aircraft.G) <= aircraft.maxG - 2 && stressSoundFlag) { // stop playing airframe stress sound
				stressSound.setVolume(stressSound.getVolume() - 0.1);

				if (stressSound.getVolume() <= 0.01) { 
					stressSoundFlag = false;
					stressSound.stop();
				}
			}

			// update engine volume
			engineSound.setSpeed(0.5 + aircraft.throttle / 150);
			engineSound.setVolume(0.6 + aircraft.throttle / 800);
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw() {
	// draw vertical line to visualize speed
	ofSetColor(255, 255, 255);
	ofSetLineWidth(1);
	int barX = gridStartX;
	while (barX < WIDTH) {
		ofDrawLine(barX, HEIGHT, barX, HEIGHT - 30);
		barX = barX + 150;
	}
	// draw horizontal line to visualize altitude
	int barY = gridStartY;
	while (barY < HEIGHT) {
		ofDrawLine(0, barY, 30, barY);
		barY = barY + 150;
	}

	// draw ground
	ofSetLineWidth(2);
	float groundY = (aircraft.altitude) * 3.6 + aircraft.imageY + 130;
	ofDrawLine(0, groundY, WIDTH, groundY);
	barX = groundStartX;
	while (barX < WIDTH) {
		ofDrawLine(barX, groundY, barX, groundY + 15);
		barX = barX + 300;
	}

	// draw radar altitude
	ofSetLineWidth(2);
	float rdrX = std::min(aircraft.altitude - 150, WIDTH / 2.0f);
	float rdrY = std::min(groundY, HEIGHT - 1.0f);
	ofDrawLine(rdrX, rdrY, WIDTH - rdrX, rdrY);

	// draw aircraft
	aircraft.draw();
	float originX = 25 * std::sin(aircraft.pitch * PI / 180);
	float originY = 25 * std::cos(aircraft.pitch * PI / 180);
	aircraft.showFlightVector(originX, originY);
	aircraft.showAltitude(originX, originY);

	// draw graph
	if(graphFlag)
	{
		aircraft.graph.draw();
		aircraft.graph2.draw();
	}
	

	//draw throttle status
	ofSetLineWidth(10);
	ofDrawLine(WIDTH - 40, 200, WIDTH - 40, 200 - aircraft.throttle * 1.8);
	std::ostringstream throttleText;
	throttleText << "THTL\n"<<aircraft.throttle << "%";
	std::string throttleTxt(throttleText.str());
	ofDrawBitmapString(throttleTxt, WIDTH - 50, 220);

	//draw elevator status
	ofSetLineWidth(10);
	ofDrawLine(WIDTH - 80, 100, WIDTH - 80, 100 - aircraft.elevator * 3);
	std::ostringstream elevText;
	elevText << std::fixed << setprecision(1) <<  "ELEV\n" << aircraft.elevator;
	std::string elevTxt(elevText.str());
	ofDrawBitmapString(elevTxt, WIDTH - 90, 220);

	//draw aoa status
	if (std::abs(aircraft.angleOfAttack) > aircraft.maxAngleOfAttack)
		ofSetColor(255, 0, 0);
	ofSetLineWidth(10);
	ofDrawLine(40, 100, 40, 100 - aircraft.angleOfAttack);
	std::ostringstream aoaText;
	aoaText << std::fixed << setprecision(1) << "AOA\n" << aircraft.angleOfAttack;
	std::string aoaTxt(aoaText.str());
	ofDrawBitmapString(aoaTxt, 30, 220);

	//draw stall indicator
	if (stallFlag && flipper % 10 < 2) {
		ofDrawBitmapStringHighlight("STALL!!", 15, 20, colorSet.red, colorSet.blue);
	}
	
	// draw crash dialog
	if (crashFlag) {
		if (flipper > 550) crashBlinkFlag = true;
		if (flipper < 300) crashBlinkFlag = false;
		ofSetColor(0, 0, 0, flipper / 2);
		ofDrawRectangle(0, 0, WIDTH, HEIGHT);

		ofSetColor(255, 255, 255);
		std::ostringstream reText;
		if(aircraft.altitude <= 0)
			reText << "Plane crashed into ground at speed of " << std::fixed << setprecision(0) << aircraft.airSpeed * 3.6 << "km/h \nPress r to restart" << std::endl;
		else if (aircraft.G > aircraft.maxG)
			reText << "Plane disintegrated by excessive g force of " << std::fixed << setprecision(1) << aircraft.G << "G \nPress r to restart" << std::endl;
		std::string reTxt(reText.str());
		ofDrawBitmapString(reTxt, WIDTH / 2 - 300, HEIGHT / 2);
	}

	// update flipper
	if (crashBlinkFlag) flipper--;
	else flipper++;

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case OF_KEY_DOWN: // pitch up
		aircraft.setElevatorDifference(0.1);
		break;
	case OF_KEY_RIGHT: // throttle up
		aircraft.setThrottle(aircraft.throttle + 1);
		break;
	case OF_KEY_UP: // pitch down
		aircraft.setElevatorDifference(-0.1);
		break;
	case OF_KEY_LEFT: // throttle down
		aircraft.setThrottle(aircraft.throttle - 1);
		break;
	case '>': // change graph's shown attribute
		aircraft.attributeID1 = ++aircraft.attributeID1 > 11 ? 0 : aircraft.attributeID1;
		strcpy(aircraft.graph.graphName, aircraft.attributes[aircraft.attributeID1]);
		aircraft.graph.hiddenMul = aircraft.attributeMulHidden[aircraft.attributeID1];
		break;
	case '<': // change graph's shown attribute
		aircraft.attributeID1 = --aircraft.attributeID1 < 0 ? 11 : aircraft.attributeID1;
		strcpy(aircraft.graph.graphName, aircraft.attributes[aircraft.attributeID1]);
		aircraft.graph.hiddenMul = aircraft.attributeMulHidden[aircraft.attributeID1];
		break; 
	case '.': // change graph2's shown attribute
		aircraft.attributeID2 = ++aircraft.attributeID2 > 11 ? 0 : aircraft.attributeID2;
		strcpy(aircraft.graph2.graphName, aircraft.attributes[aircraft.attributeID2]);
		aircraft.graph2.hiddenMul = aircraft.attributeMulHidden[aircraft.attributeID2];
		break;
	case ',': // change graph2's shown attribute
		aircraft.attributeID2 = --aircraft.attributeID2 < 0 ? 11 : aircraft.attributeID2;
		strcpy(aircraft.graph2.graphName, aircraft.attributes[aircraft.attributeID2]);
		aircraft.graph2.hiddenMul = aircraft.attributeMulHidden[aircraft.attributeID2];
		break;
	case 'r': // restart simulation
		ofSetBackgroundColor(23, 120, 194);
		aircraft.pitch = 5;
		aircraft.flightVector = 0;
		aircraft.altitude = 500;
		aircraft.groundSpeed = 100;
		aircraft.airSpeed = 150;
		aircraft.elevator = 0;
		aircraft.throttle = 100;
		aircraft.G = 0;
		crashFlag = false;
		crashBlinkFlag = false;
		engineSound.play();
		engineSound.setLoop(true);
		audioSound.stop();
		break;
	case 'd': // change aircraft
		if (aircraft.type == 27) aircraft.switchPlane(737400);
		else aircraft.switchPlane(27);
		break;
	case 'g': // show/hide graphs
		graphFlag = !graphFlag;
		break;

	}

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
	switch (key) {
	case  OF_KEY_DOWN:
	case  OF_KEY_UP:
		aircraft.setElevatorDifference(0);
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
