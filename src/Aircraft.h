#include "ofMain.h"
#include "Graph.h"

#define WIDTH 1280
#define HEIGHT 768

#define FRAME_RATE 60
#define MAX_DRAG_COEFFICENT 0.5
#define MIN_DRAG_COEFFICENT 0.05


class Aircraft {
public:
	//---------------- Graphical components -----------------
	// Image of Aircraft
	int type;
	ofImage image;
	// graphs of Aircraft
	Graph graph = Graph(10, 650, WIDTH, 200);
	Graph graph2 = Graph(10, 650, WIDTH, 200);
	// Offset position of image
	int imageX = 100, imageY = 100;
	// inclination of cockpit. some cockpits have down slope to ground
	float cockpitSlope = 0;
	// Available graphical graph attribute
	char* attributes[12] = {
		"G", "Air Speed(m/s)", "Vertical Speed(m/s)", "Ground Speed(m/s)", "Altitude(m)", 
		"Thrust(kN)", "Lift(kN)", "Wing Lift(kN)" ,"Drag Lift(kN)","Drag(kN)", 
		"Parasite Drag(kN)", "Induced Drag(kN)"};
	// garphs's attribute id
	int attributeID1 = 0;
	int attributeID2 = 4;
	// Attribute's value multipler (unit)
	float attributeMul[12] = {
		1, 1, 1, 1, 1, 
		0.001, 0.001, 0.001, 0.001, 0.001, 
		0.001, 0.001 };
	// graph's vertical scale value
	float attributeMulHidden[12] = {
		35, 1, 1, 1, 0.5,
		1, 0.3, 0.3, 0.3, 1,
		1, 1
	};

	//---------------- Status of Aircraft -----------------
	// horizontal velocity of aircraft
	float groundSpeed;
	// vertical speed of aircraft
	float verticalSpeed;
	// airspeed = sqrt(groundSpeed^2 + verticalSpeed^2)
	float airSpeed;
	// altitude of aircraft
	float altitude;
	// pitch(angle) of aircraft
	float pitch;
	// flightVector(angle) of aircraft
	float flightVector;
	// aoa = pitch - flightVector
	float angleOfAttack;
	// throttle status. have value of 1 ~ 100 (percent). 
	float throttle;
	
	//---------------- Physical Factors -----------------
	// mass of aircraft. (In this simulator, assume that there is no fuel spent during flight)
	float mass;
	// thrust of engines. sum of engine's individual thrust
	float thrust;
	// |NOT_REALISTIC| elevator's actuated angle.
	float elevator;
	// max thrust of engines. sum if engine's individual max thrust
	float thrustLimit;
	// |NOT_REALISTIC| elevator's max actuated angle
	float elevatorLimit;
	// |NOT_REALISTIC| elevator's effectiveness. determined by ratio between mass and elevator's area
	float elevatorEffectiveness;

	// lift coefficient calculated by formula. for more information visit : https://www.grc.nasa.gov/www/k-12/airplane/liftco.html
	float liftCoefficent;
	// slope of liftCoefficient's linear part. for more information visit : https://en.wikipedia.org/wiki/Lift_coefficient#/media/File:Lift_curve.svg
	float liftEffectiveness;
	// lift coefficient at zero aoa. 
	float zeroAOALiftCoefficient;

	// drag coefficient calculated by formula. for more information visit : https://www.grc.nasa.gov/www/k-12/airplane/dragco.html
	float dragCoefficent;
	// base of dragCoefficient's parasite drag part. for more information visit : https://en.wikipedia.org/wiki/Drag_coefficient#/media/File:Drag_curves_for_aircraft_in_flight.svg
	float dragEffectiveness;

	// max g force that aircraft can sustain
	float maxG;
	// max aoa of aircraft. determined by lift coefficient graph. equls graph's end of linear part
	float maxAngleOfAttack;
	// |NOT_REALISTIC| after max lift aoa, there is part where lift coefficient decrease. end of decrease is postStallResistance. determined by how aircraft handle excessive aoa.
	float postStallResistance;
	// stall speed. 
	float stallSpeed;

	// wing area of aircraft.
	float wingArea;
	// wing span(width) of aircraft
	float wingSpan;
	// oswal factor of aircraft. determined by wing's section slice
	float oswaldFactor;

	//---------------- Calculated Factors -----------------
	// lift calculated by lift coefficient. for more information visit : https://www.grc.nasa.gov/www/k-12/airplane/lifteq.html
	float lift;
	// |NOT_REALISTIC| at slow speed elevator doesn't produce smae torque as high speed. 
	float elevatorFlowEffectiveness;
	// lift produced by wing. calculated by lift coefficient formula
	float wingLift;
	// lift vector component of drag
	float dragLift;
	// induced drag, parasite drag for more information visit : https://en.wikipedia.org/wiki/Drag_coefficient#/media/File:Drag_curves_for_aircraft_in_flight.svg
	float inducedDrag;
	float parasiteDrag;
	// drag.(chord vector)
	float drag;
	// g force
	float G;

	//---------------- contorl input interplation -----------------
	// for smooth input
	float elevatorDifference = 0;


	
	Aircraft(int _type) {
		// Initialize graphs
		strcpy(graph.graphName, attributes[attributeID1]);
		strcpy(graph2.graphName, attributes[attributeID2]);
		graph.hiddenMul = attributeMulHidden[attributeID1];
		graph2.hiddenMul = attributeMulHidden[attributeID2];

		// Set type of aircraft and initilize physical values of aircraft
		type = _type;
		switchPlane(_type);
		
		// Initilize status values of aircraft
		pitch = 5;
		flightVector = 0;
		altitude = 500;
		groundSpeed = 100;
		airSpeed = 150;
		elevator = 0;
		throttle = 100;
		
	}
	/***********************************************************
	*	simulate one frame. 
	*	input	: none
	*	return	: none
	***********************************************************/
	void simulate() {
		// Change elevator using input interpolator
		if (std::abs(elevator + elevatorDifference) < elevatorLimit)
			elevator += elevatorDifference;

		// Calculate pitch and aoa. pitch difference = pitch change by Elevator - pitch stabilization by Aircraft
		float elevatorFlowInduced = getElevatorFlowEffectiveness(elevatorEffectiveness, elevator, maxAngleOfAttack, angleOfAttack, airSpeed, altitude) * elevator;
		float aircraftFlowInduced = getAircraftFlowEffectiveness(wingArea, mass, airSpeed, altitude, angleOfAttack) * angleOfAttack;
		pitch += (elevatorFlowInduced - aircraftFlowInduced) / FRAME_RATE;
		angleOfAttack = fmod(pitch - flightVector, 360);
		
		// Calculate drags
		inducedDrag = getInduceDrag(mass, wingSpan, airSpeed, altitude, oswaldFactor, stallSpeed);
		parasiteDrag = getDragCoefficient(maxAngleOfAttack, angleOfAttack) * getDensity(altitude) * std::pow(airSpeed, 2) * wingArea / 2;
		drag = parasiteDrag * std::cos(angleOfAttack * PI / 180) + inducedDrag;

		// Calculate Lift
		dragLift = parasiteDrag * std::sin(angleOfAttack * PI / 180);
		wingLift = getLiftCoefficient(maxAngleOfAttack, postStallResistance, angleOfAttack, liftEffectiveness, zeroAOALiftCoefficient) * getDensity(altitude) * std::pow(airSpeed, 2) * wingArea / 2;
		lift = dragLift + wingLift;

		// Calculate TurnRadius and G force
		float turnRadius = getTurnRadius(flightVector, mass, airSpeed, lift);
		float vectorChange = (airSpeed / turnRadius / FRAME_RATE) * 180 / PI;
		G = getGforce(turnRadius, flightVector);

		// Change flight vector based on turn radius
		flightVector = flightVector + vectorChange;
		if (std::abs(pitch) > 360 && std::abs(flightVector) > 360) {
			pitch = fmod(pitch, 360);
			flightVector = fmod(flightVector, 360);
		}

		// Calculate speed(airSpeed, v-speed, h-speed)
		thrust = getThrust(thrustLimit * throttle / 100, altitude, airSpeed, angleOfAttack);
		float altChange = getAltiudeChange(airSpeed, flightVector);
		altitude = altitude + altChange;
		airSpeed = getAirSpeed(flightVector, airSpeed, altChange, thrust, drag, mass);
		if (airSpeed <= 1)
		{
			airSpeed = 1;
		}
		groundSpeed = airSpeed * std::cos(flightVector* PI / 180);
		verticalSpeed = airSpeed * std::sin(flightVector* PI / 180);

		// insert data into graphs
		graph.insert(getAttributeValue(attributeID1));
		graph2.insert(getAttributeValue(attributeID2));
	}

	/***********************************************************
	*	rotate aircraft image by given pitch
	*	input	: none
	*	return	: none
	***********************************************************/
	void draw() {
		ofPushMatrix();
		ofTranslate(imageX + image.getWidth() / 2, imageY + image.getHeight() / 2, 0);//move pivot to centre
		ofRotate(-pitch - cockpitSlope, 0, 0, 1);//rotate from centre
		ofPushMatrix();
		ofTranslate(-image.getWidth() / 2, -image.getHeight() / 2, 0);//move back by the centre offset
		image.draw(0, 0);
		ofPopMatrix();
		ofPopMatrix();
	}

	/***********************************************************
	*	draw flight vector as line scaled by speed
	*	input	: 
	*		offsetX, offsetY : offset of flight vector line's origin
	*	return	: none
	***********************************************************/
	void showFlightVector(float offsetX, float offsetY) {
		float originX = imageX + offsetX + image.getWidth() / 2;
		float originY = imageY + offsetY +image.getHeight() / 2;
		ofSetLineWidth(1);
		ofDrawLine(originX, originY, originX + 2* airSpeed * std::cos(flightVector * PI / 180), originY - 2 *airSpeed * std::sin(flightVector * PI / 180));

		// draw speed text at end of the line
		std::ostringstream speed;
		speed << (int)(airSpeed * 3.6) << " km/h";
		std::string speedo(speed.str());
		ofDrawBitmapString(speedo, originX + 2 * airSpeed * std::cos(flightVector * PI / 180), originY - 2 * airSpeed * std::sin(flightVector * PI / 180));
	}

	/***********************************************************
	*	draw altitude as line. also show altitude as number(meter)
	*	input	:
	*		offsetX, offsetY : offset of flight vector line's origin
	*	return	: none
	***********************************************************/
	void showAltitude(float offsetX, float offsetY) {
		float originX = imageX + offsetX + image.getWidth() / 2;
		float originY = imageY + offsetY + image.getHeight() / 2;
		float targetY = altitude * 3.6 + imageY + 130;
		ofSetLineWidth(1);
		ofDrawLine(originX, originY, originX, targetY);

		// draw altitude text at center of the line
		std::ostringstream alt;
		alt << (int)altitude << " m";
		std::string alto(alt.str());
		if(targetY < HEIGHT)
			ofDrawBitmapString(alto, originX + 5, 0.5 * targetY + 0.5* originY);
		else 
			ofDrawBitmapString(alto, originX + 5, 0.5 * HEIGHT + 0.5* originY);
		
		
	}

	/***********************************************************
	*	set elevator's actuated angle. 
	*	input	:
	*		_elevator : target actuated angle.
	*	return	: none
	***********************************************************/
	void setElevator(float _elevator) {
		if (_elevator > elevatorLimit || _elevator < -elevatorLimit) return;

		elevator = _elevator;
	}

	/***********************************************************
	*	set elevatorDifference. difference will apply change at every frame
	*	input	:
	*		_elevator : new elevator value
	*	return	: none
	***********************************************************/
	void setElevatorDifference(float _elevatorDifference) {
		elevatorDifference = _elevatorDifference;
	}

	/***********************************************************
	*	set throttle value between 0 ~ 100
	*	input	:
	*		_throttle : new throttle value
	*	return	: none
	***********************************************************/
	void setThrottle(float _throttle) {
		if (_throttle > 100 || _throttle < 0) return;

		throttle = _throttle;
	}
	/***********************************************************
	*	calculate parasite drag coefficient. formula can be found on this page : https://www.grc.nasa.gov/www/k-12/airplane/dragco.html
	*	also this function will generate left side of graph : https://en.wikipedia.org/wiki/Drag_coefficient#/media/File:Drag_curves_for_aircraft_in_flight.svg
	*	input	:
	*		_maxAngleOfAttack : aoa where lift coefficient stats to degrade
	*		_angleOfAttack : aoa
	*	return	: 
	*		dragCoefficient : parasite drag coefficient
	***********************************************************/
	float getDragCoefficient(float _maxAngleOfAttack, float _angleOfAttack) {
		float expoenentFactor = std::abs(_angleOfAttack / _maxAngleOfAttack);
		if (std::abs(_angleOfAttack) > 80)expoenentFactor = std::abs(80 / _maxAngleOfAttack);
		return std::pow(MAX_DRAG_COEFFICENT + 1, expoenentFactor) + MIN_DRAG_COEFFICENT - 1;
	
	}

	/***********************************************************
	*	calculate induced drag coefficient. formula can be found on this page : https://wright.nasa.gov/airplane/drageq.html#:~:text=The%20induced%20drag%20coefficient%20is,divided%20by%20the%20wing%20area.
	*	also this function will generate right side of graph : https://en.wikipedia.org/wiki/Drag_coefficient#/media/File:Drag_curves_for_aircraft_in_flight.svg
	*	input	:
	*		_maxAngleOfAttack :  aoa where lift coefficient stats to degrade
	*		_angleOfAttack : aoa
	*	return	:
	*		dragCoefficient : induced drag coefficient
	***********************************************************/
	float getInduceDrag(float _mass, float _wingSpan, float _airSpeed, float _altitude, float _oswaldFactor, float _stallSpeed) {
		float ias = _airSpeed;
		if (std::abs(_airSpeed) < _stallSpeed ) ias = _stallSpeed ;
		return 2 * std::pow(_mass*9.8, 2) / (getDensity(_altitude) *std::pow(ias, 2) * PI * std::pow(_wingSpan, 2) * _oswaldFactor);
	}

	

	/***********************************************************
	*	calculate lift coefficient by given aoa. formula can be fount on this page : https://www.grc.nasa.gov/www/k-12/airplane/liftco.html
	*	lift coefficient will grow until aoa hit max aoa. after max aoa lift decreatse quickly : https://en.wikipedia.org/wiki/Lift_coefficient#/media/File:Lift_curve.svg
	*	input	:
	*		_maxAngleOfAttack :  aoa where lift coefficient stats to degrade
	*		_angleOfAttack : aoa
	*		_liftEffectiveness: slope of liftCoefficient's linear part.
	*	return	:
	*		liftCoefficient : lift coefficient at given aoa
	***********************************************************/
	float getLiftCoefficient(float _maxAngleOfAttack, float _postStallResistance, float _angleOfAttack, float _liftEffectiveness, float _zeroAOALiftCoefficient) {
		float absAOA = std::abs(_angleOfAttack);
		if (absAOA <= _maxAngleOfAttack) // linear part
			return _liftEffectiveness * _angleOfAttack + _zeroAOALiftCoefficient;
		else if (_angleOfAttack > _maxAngleOfAttack) // stall part
		{
			float ceiling = _liftEffectiveness * _maxAngleOfAttack + _zeroAOALiftCoefficient;
			if (_angleOfAttack < _maxAngleOfAttack + _postStallResistance) // post stall 
				return ceiling;
			else // decreasing part
			{
				float effectiveAOA = (_maxAngleOfAttack + 2 * _postStallResistance) - angleOfAttack;
				if (effectiveAOA < 0) effectiveAOA = 0;
				return ceiling / std::pow(_postStallResistance, 0.5)*std::pow(effectiveAOA, 0.5);
			}
		}
		else if (_angleOfAttack < -_maxAngleOfAttack ) // stall part
		{
			float ceiling = _liftEffectiveness * -_maxAngleOfAttack + _zeroAOALiftCoefficient;
			if (_angleOfAttack > -(_maxAngleOfAttack + _postStallResistance)) // post stall 
				return ceiling;
			else // decreasing part
			{
				float effectiveAOA = (_maxAngleOfAttack + 2 * _postStallResistance) + _angleOfAttack;
				if (effectiveAOA < 0) effectiveAOA = 0;
				return ceiling / std::pow(_postStallResistance, 0.5)*std::pow(effectiveAOA, 0.5);
			}
		}
		else return 0;

	}

	/***********************************************************
	*	|NOT_REALISTIC| calculate elevator's flow effectiveness at given speed & aoa
	*	slight modified version of lift coefficient's formula
	*	input	:
	*		_elevatorEffectiveness : |NOT_REALISTIC| elevator's effectiveness. 
	*		_postStallResistance : |NOT_REALISTIC| determined by how aircraft handle excessive aoa.
	*		_maxAngleOfAttack : aoa where lift coefficient stats to degrade
	*		_angleOfAttack: aoa
	*		_airSpeed : air speed
	*		_altitude : altitude
	*	return	:
	*		elevatorFlowEffectiveness : elevator flow effectiveness (can be negative)
	***********************************************************/
	float getElevatorFlowEffectiveness(float _elevatorEffectiveness, float _elevator, float _maxAngleOfAttack, float _angleOfAttack, float _airSpeed, float _altitude) {
		float elevatorFlowEffectiveness = std::pow(_airSpeed, 2) * _elevatorEffectiveness;
		float hydraulicBoosterEffectiveness = std::pow(std::abs(airSpeed), 0.5) ;
		
		float aoaFactor = 0.05;
		/*if (std::abs(_angleOfAttack) > _maxAngleOfAttack)
			aoaFactor = 0.1 / (_elevator - _angleOfAttack);*/
		return  0.05 * getDensity(_altitude) * (elevatorFlowEffectiveness + hydraulicBoosterEffectiveness)   * 0.0005;
	}

	/***********************************************************
	*	|NOT_REALISTIC| calculate stabilization torque of aircraft. determined by mass and wing area.
	*	input	:
	*		_wingArea : area of wing(sum of both wing area)
	*		_mass : mass of aircraft
	*		_airSpeed : air speed
	*		_altitude : altitude
	*		_angleOfAttack : aoa
	*	return	:
	*		aircraftFlowEffectiveness : (always positive)
	***********************************************************/
	float getAircraftFlowEffectiveness(float _wingArea, float _mass, float _airSpeed, float _altitude, float _angleOfAttack) {
		return getDensity(_altitude) * std::pow(_airSpeed, 2) * std::pow(std::abs(_angleOfAttack), 0.3) * _wingArea / _mass * 0.01;
	}


	/***********************************************************
	*	calculate air density (kg/m^3) at given altitude. formula can be found on this page : https://en.wikipedia.org/wiki/Density_of_air
	*	input	:
	*		alititude : altitude
	*	return	:
	*		density : (kg/m^3)
	***********************************************************/
	float getDensity(float _altitude) {
		return  1.22480130 * std::pow((1 - 0.0065 * _altitude / 288.15), 4.25142574);
	}

	/***********************************************************
	*	plane rotate its pitch using centripetal force = lift + something. 
	*	get turn radius at givec situation. 
	*	input	:
	*		_flightVector : flight vector of aircraft
	*		_mass : mass of aircraft
	*		_airSpeed : air speed
	*		_lift : calculated lift
	*	return	:
	*		turnRadius : aircraft's turn radius
	***********************************************************/
	float getTurnRadius(float _flightVector, float _mass, float _airSpeed, float _lift) {
		float pitchModifiedGravity = _mass * 9.8 * std::cos(_flightVector * PI / 180);
		return _mass  * _airSpeed * _airSpeed / (_lift - pitchModifiedGravity);
	}

	/***********************************************************
	*	calculate altitude change at give frame seconds.
	*	input	:
	*		_flightVector : flight vector of aircraft
	*		_airSpeed :  air speed
	*	return	:
	*		altiudeChange : change of altitude
	***********************************************************/
	float getAltiudeChange(float _airSpeed, float _flightVector) {
		return _airSpeed / FRAME_RATE * std::sin(_flightVector * PI / 180);
	}

	/***********************************************************
	*	calculated airspeed using conservation of energy. (potential Energy <=> kinetic Energy)
	*	also calculate accelerated speed using thrust
	*	input	:
	*		_flightVector : flight vector of aircraft
	*		_mass :  mass of aircraft
	*		_airSpeed :  air speed
	*		_lift :  calculated lift
	*		altChange : used for calcuation of potential energy
	*		_thrust : used for calculation of acceletaion
	*		drag : used for calculation of acceletaion
	*	return	:
	*		airspeed : get updated air speed
	***********************************************************/
	float getAirSpeed(float _flightVector, float _airSpeed, float altChange, float _thrust, float drag, float _mass) {
		float energyConverted = std::pow(-altChange *  9.8 + _airSpeed * _airSpeed, 0.5);
		float gravity = _mass * 9.8 * std::sin(_flightVector * PI / 180);
		float thrustToDrag = (_thrust - drag - gravity) / _mass / FRAME_RATE;
		
		return energyConverted + thrustToDrag;
	}

	/***********************************************************
	*	|NOT_REALISTIC| most of turbo fan - jet engine gain thrust at hig speed but loose their power at high altitude.
	*	formula is different from engine to engine. for simplicity, used Pratt & Whitney JT8D-17 jet engine : https://www.grc.nasa.gov/www/k-12/Missions/Jim/Project1ans.htm
	*	input	:
	*		_thrust : current uncalculated thrust
	*		_altitude : altitude
	*		_airSpeed : velocity
	*		_angleOfAttack : aoa, 
	*	return	:
	*		thrust : calculated thrust
	***********************************************************/
	float getThrust(float _thrust, float _altitude, float _airSpeed, float _angleOfAttack) {
		float aoa = std::min(_angleOfAttack, 70.0f);
		
		float ias = std::abs(_airSpeed * std::cos(aoa * PI / 180));
		if (_airSpeed < 0) ias = 0;
		float speedFactor = std::pow(std::min(ias / 300, 1.0f), 0.5) * 0.5 + 0.5;

		float angel = _altitude / 1000;
		float altitudeFactor = std::pow(0.8577, angel) * 0.9 + 0.1;

		return _thrust * speedFactor * altitudeFactor;
	}

	/***********************************************************
	*	claculate g force applied to aircraft
	*	input	:
	*		_turnRadius : turnRadius of aircraft
	*		_flightVector : degree
	*	return	:
	*		thrust : calculated G
	***********************************************************/
	float getGforce(float _turnRadius, float _flightVector) {
		float centripetal = std::pow(airSpeed, 2) / _turnRadius;

		return (centripetal + 9.8 * std::cos(_flightVector * PI / 180)) * 0.102;
	}

	/***********************************************************
	*	return attribute value of aircraft by attributeID
	*	input	:
	*		attributeID : target attribute's id
	*	return	:
	*		value : aircarft attribute's value
	***********************************************************/
	float getAttributeValue(int attributeID) {
		float value;
		switch (attributeID) {
		case 0:
			value = G;
			break;
		case 1:
			value = airSpeed;
			break;
		case 2:
			value = verticalSpeed;
			break;
		case 3:
			value = groundSpeed;
			break;
		case 4:
			value = altitude;
			break;
		case 5:
			value = thrust;
			break;
		case 6:
			value = lift;
			break;
		case 7:
			value = wingLift;
			break;
		case 8:
			value = dragLift;
			break;
		case 9:
			value = drag;
			break;
		case 10:
			value = parasiteDrag;
			break;
		case 11:
			value = inducedDrag;
			break;
		}

		return value * attributeMul[attributeID];
	}

	/***********************************************************
	*	initialize aircraft with given values. 
	*	input	:
	*		_type : for now, there is only two type(737400 or 27)
	*	return	:
	***********************************************************/
	void switchPlane(int _type) {
		type = _type;
		elevator = 0;
		switch (_type) {
		case 737400:
			image.load("images/737_400.png");
			mass = 44225;
			thrustLimit = 210000;
			elevatorLimit = 15;
			elevatorEffectiveness = 5;
			maxAngleOfAttack = 15;
			postStallResistance = 3;
			liftEffectiveness = 0.147;
			dragEffectiveness = 1.9;
			wingArea = 91;
			wingSpan = 29;
			oswaldFactor = 0.8;
			stallSpeed = 60;
			zeroAOALiftCoefficient = 0.4;
			maxG = 5.95;
			break;
		default:
		case 27:
			image.load("images/27.png");
			mass = 23100;
			thrustLimit = 244000;
			elevatorLimit = 25;
			elevatorEffectiveness = 12;
			maxAngleOfAttack = 30;
			postStallResistance = 10;
			liftEffectiveness = 0.053;
			dragEffectiveness = 1.21;
			wingArea = 62;
			wingSpan = 15;
			oswaldFactor = 0.81;
			stallSpeed = 75;
			cockpitSlope = -1;
			zeroAOALiftCoefficient = 0;
			maxG = 11.95;
			break;
		}
	}
};