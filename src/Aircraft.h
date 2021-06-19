#include "ofMain.h"
#include "Graph.h"

#define WIDTH 1280
#define HEIGHT 768

#define FRAME_RATE 60
#define MAX_DRAG_COEFFICENT 0.5
#define MIN_DRAG_COEFFICENT 0.02
#define POST_STALL_PERFORMANCE 3 //실속 직전 양력이 줄어드는 구간 (degree) POST_STALL_PERFORMANCE^2 < maxAngleOfAttack


class Aircraft {
public:
	Graph tacoBell = Graph(10, 500, 1200, 200);
	Graph tacoBell2 = Graph(10, 700, 1200, 200);


	//항공기 이미지
	ofImage image;
	//이미지 위치 지정
	int imageX = 100, imageY = 100;
	//항공기의 조종석 기울기(조종석이 살짝 아래를 보고있는 경우가 많다)
	float cockpitSlope = 0;

	//---------------- 항공기의 자세 -----------------
	//항공기의 수평속도
	float groundSpeed;
	//항공기의 수직속도
	float verticalSpeed;
	//항공기의 속력
	float airSpeed;
	//항공기의 고도
	float altitude;
	//항공기의 기울기
	float pitch;
	//항공기의 진행방향
	float flightVector;
	//항공기의 받음각(항공기의 진행방향과 기수의 각도)
	float angleOfAttack;
	
	//---------------- 항공기의 물리 특성 -----------------
	//항공기의 무게(kg)
	float mass;
	//항공기의 추력(kN)
	float thrust;
	//항공기의 pitch를 조정하는 승강키의 각도. 승강키의 각도가 양수이면 항공기의 기수가 상승.(degree)
	float elevator;
	//항공기의 추력을 조정하는 throttle의 세기 0~100
	float throttle;
	//항공기의 최대 추력
	float thrustLimit;
	//항공기 승강키의 최대 각도.
	float elevatorLimit;
	//항공기 승강키의 기본 유효성 0~1
	float elevatorEffectiveness;
	//항공기 양력계수
	float liftCoefficent;
	//항공기 양력 유효성 (aoa - lift coefficent의 기울기)
	float liftEffectiveness;
	//항공기 항력 유효성 (drag coefficnet함수 + 1의 지수)
	float dragEffectiveness;
	//항공기 저항계수
	float dragCoefficent;
	//항공기의 최대받음각
	float maxAngleOfAttack;
	//항공기의 스톨 특성 (degree)
	float postStallResistance;
	//항공기의 익면적
	float wingArea;
	//항공기의 익폭
	float wingSpan;
	//항공기의 oswald 상수 보통 날개 형상에 의해 결정된다.
	float oswaldFactor;
	//항공기의 최소 속도
	float stallSpeed;
	//항공기의 받음각0에서의 양력
	float zeroAOALiftCoefficient;


	//---------------- 항공기의 계산되는 특성 -----------------
	// 항공기의 양력
	float lift;
	//항공기 승강키의 유효성 
	float elevatorFlowEffectiveness;

	//---------------- 인풋 인터폴레이터 -----------------
	float elevatorDifference = 0;

	//---------------- 테스트용 -----------------
	float elevatorEffectivenessTEST;


	Aircraft(int type) {
		switch (type) {
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
			break;
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
			break;
		}
		
		pitch = 5;
		flightVector = 0;
		altitude = 500;
		groundSpeed = 100;
		airSpeed = 150;
		elevator = 0;
		throttle = 100;
		
	}
	/***********************************************************
	*	상황을 종합하여 물리량을 계산한다.
	*	input	: none
	*	return	: none
	***********************************************************/
	void simulate() {

		if (std::abs(elevator + elevatorDifference) < elevatorLimit)
			elevator += elevatorDifference;

		elevatorEffectivenessTEST = getElevatorFlowEffectiveness(elevatorEffectiveness, elevator, maxAngleOfAttack, angleOfAttack, airSpeed, altitude);
		float elevatorFlowInduced = getElevatorFlowEffectiveness(elevatorEffectiveness, elevator, maxAngleOfAttack, angleOfAttack, airSpeed, altitude) * elevator;
		float aircraftFlowInduced = getAircraftFlowEffectiveness(wingArea, mass, airSpeed, altitude, angleOfAttack) * angleOfAttack;
		pitch = pitch + (elevatorFlowInduced - aircraftFlowInduced) / FRAME_RATE;
		angleOfAttack = fmod(pitch - flightVector, 360);
		
		float inducedChordVectorDrag = getInduceDrag(mass, wingSpan, airSpeed, altitude, oswaldFactor, stallSpeed);
		float flightVectorDrag = getDragCoefficient(maxAngleOfAttack, angleOfAttack) * getDensity(altitude) * std::pow(airSpeed, 2) * wingArea / 2;
		float chordVectorDrag = flightVectorDrag * std::cos(angleOfAttack * PI / 180) + inducedChordVectorDrag;
		float liftVectorDrag = flightVectorDrag * std::sin(angleOfAttack * PI / 180);
		float liftInducedByWings = getLiftCoefficient(maxAngleOfAttack, postStallResistance, angleOfAttack, liftEffectiveness, zeroAOALiftCoefficient) * getDensity(altitude) * std::pow(airSpeed, 2) * wingArea / 2;
		float lift = liftVectorDrag + liftInducedByWings;
		float gravity = mass * 9.8;
		float turnRadius = getTurnRadius(flightVector, mass, airSpeed, lift);
		float vectorChange = (airSpeed / turnRadius / FRAME_RATE) * 180 / PI;

		flightVector = flightVector + vectorChange;
		if (std::abs(pitch) > 360 && std::abs(flightVector) > 360) {
			pitch = fmod(pitch, 360);
			flightVector = fmod(flightVector, 360);
		}

		//그래프에 정보를 추가한다.
		tacoBell.insert(inducedChordVectorDrag / 1000);
		tacoBell2.insert(getThrust(thrustLimit * throttle / 100, altitude, airSpeed, angleOfAttack) / 1000);
		

		float altChange = getAltiudeChange(airSpeed, flightVector);
		altitude = altitude + altChange;
		airSpeed = getAirSpeed(flightVector, airSpeed, altChange, getThrust(thrustLimit * throttle / 100, altitude, airSpeed, angleOfAttack), chordVectorDrag, mass);
		if (airSpeed <= 1)
		{
			airSpeed = 1;
		}
		groundSpeed = airSpeed * std::cos(flightVector* PI / 180);
		verticalSpeed = airSpeed * std::sin(flightVector* PI / 180);
		

		printf(" iDRG:%f \n cDRG:%f \n lDRG:%f \n lWNG:%f \n lift:%f \n grav:%f \n eleS:%f \n eleD:%f \n eleE:%f \n _AOA:%f \n elvI:%f \n actI:%f \n turn:%f \n fVec:%f \n vCng:%f \n aSpd:%f \n bAlt:%f \n thtl:%f \n thst:%f \n------------------------\n",
			inducedChordVectorDrag,
			chordVectorDrag,
			liftVectorDrag,
			liftInducedByWings,
			lift,
			gravity,
			elevator,
			elevator - angleOfAttack,
			getElevatorFlowEffectiveness(elevatorEffectiveness,elevator , maxAngleOfAttack, angleOfAttack, airSpeed, altitude),
			angleOfAttack,
			elevatorFlowInduced,
			aircraftFlowInduced,
			turnRadius,
			flightVector,
			vectorChange,
			airSpeed * 3.6,
			altitude,
			throttle,
			getThrust(thrustLimit * throttle / 100, altitude, airSpeed, angleOfAttack)
		);

	}

	/***********************************************************
	*	선택된 비행기를 현재 pitch에 맞게 그린다.
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
	*	현재 비행기의 진행방향을 그린다.
	*	input	: offsetX, offsetY : 원점의 위치를 세밀하게 조정한다.
	*	return	: none
	***********************************************************/
	void showFlightVector(float offsetX, float offsetY) {
		float originX = imageX + offsetX + image.getWidth() / 2;
		float originY = imageY + offsetY +image.getHeight() / 2;
		ofSetLineWidth(1);
		ofDrawLine(originX, originY, originX + 2* airSpeed * std::cos(flightVector * PI / 180), originY - 2 *airSpeed * std::sin(flightVector * PI / 180));
		std::ostringstream speed;
		speed << (int)(airSpeed * 3.6) << " km/h";
		std::string speedo(speed.str());
		ofDrawBitmapString(speedo, originX + 2 * airSpeed * std::cos(flightVector * PI / 180), originY - 2 * airSpeed * std::sin(flightVector * PI / 180));
	}

	/***********************************************************
	*	현재 비행기의 고도를 그린다.
	*	input	: offsetX, offsetY : 원점의 위치를 세밀하게 조정한다.
	*	return	: none
	***********************************************************/
	void showAltitude(float offsetX, float offsetY) {
		float originX = imageX + offsetX + image.getWidth() / 2;
		float originY = imageY + offsetY + image.getHeight() / 2;
		float targetY = altitude * 3.6 + imageY + 100;
		ofSetLineWidth(1);
		ofDrawLine(originX, originY, originX, targetY);
		std::ostringstream alt;
		alt << (int)altitude << " m";
		std::string alto(alt.str());
		if(targetY < HEIGHT)
			ofDrawBitmapString(alto, originX + 5, 0.5 * targetY + 0.5* originY);
		else 
			ofDrawBitmapString(alto, originX + 5, 0.5 * HEIGHT + 0.5* originY);
		
		
	}

	/***********************************************************
	*	승강키를 조종가능 범위 내로 설정한다.
	*	input	:
	*		_elevator : 설정할 승강키 값
	*	return	: none
	***********************************************************/
	void setElevator(float _elevator) {
		if (_elevator > elevatorLimit || _elevator < -elevatorLimit) return;

		elevator = _elevator;
	}

	/***********************************************************
	*	승강키 변위값을 설정한다. 설정한 이루 승강키는 프레임마다 difference를 더한다.
	*	input	:
	*		_elevator : 설정할 승강키 값
	*	return	: none
	***********************************************************/
	void setElevatorDifference(float _elevatorDifference) {
		elevatorDifference = _elevatorDifference;
	}

	/***********************************************************
	*	쓰로틀을 조종가능 범위 내로 설정한다.
	*	input	:
	*		_throttle : 설정할 쓰로틀 값
	*	return	: none
	***********************************************************/
	void setThrottle(float _throttle) {
		if (_throttle > 100 || _throttle < 0) return;

		throttle = _throttle;
	}
	/***********************************************************
	*	항력 계수를 받음각에 따라 계산한다. 최대값 MAX_DRAG_COEFFICENT 최소값 MIN_DRAG_COEFFICENT
	*	항력 계수는 지수함수의 모습을 하고 있다.
	*	input	:
	*		_maxAngleOfAttack : 항공기의 최대 받음각
	*		_angleOfAttack : 항공기의 받음각
	*	return	: 
	*		dragCoefficient : 항력 계수
	***********************************************************/
	float getDragCoefficient(float _maxAngleOfAttack, float _angleOfAttack) {
		float expoenentFactor = std::abs(_angleOfAttack / _maxAngleOfAttack);
		if (std::abs(_angleOfAttack) > 80)expoenentFactor = std::abs(80 / _maxAngleOfAttack);
		return std::pow(MAX_DRAG_COEFFICENT + 1, expoenentFactor) + MIN_DRAG_COEFFICENT - 1;
	
	}

	/***********************************************************
	*	induced drag를 계산한다. 이 항력은 속도가 높아지면 작아진다.
	*	input	:
	*		_maxAngleOfAttack : 항공기의 최대 받음각
	*		_angleOfAttack : 항공기의 받음각
	*	return	:
	*		dragCoefficient : 항력 계수
	***********************************************************/
	float getInduceDrag(float _mass, float _wingSpan, float _airSpeed, float _altitude, float _oswaldFactor, float _stallSpeed) {
		float ias = _airSpeed;
		if (std::abs(_airSpeed) < _stallSpeed ) ias = _stallSpeed ;
		return 2 * std::pow(_mass*9.8, 2) / (getDensity(_altitude) *std::pow(ias, 2) * PI * std::pow(_wingSpan, 2) * _oswaldFactor);
	}

	

	/***********************************************************
	*	양력 계수를 받음각에 따라 계산한다. 
	*	양력 계수는 _maxAngleOfAttack까지 선형적으로 상승 후 급격하게 감소한다
	*	input	:
	*		_maxAngleOfAttack : 항공기의 최대 받음각
	*		_angleOfAttack : 항공기의 받음각
	*		_liftEffectiveness: 양력계수의 기울기
	*	return	:
	*		liftCoefficient : 양력 계수
	***********************************************************/
	float getLiftCoefficient(float _maxAngleOfAttack, float _postStallResistance, float _angleOfAttack, float _liftEffectiveness, float _zeroAOALiftCoefficient) {
		float absAOA = std::abs(_angleOfAttack);
		if (absAOA <= _maxAngleOfAttack)
			return _liftEffectiveness * _angleOfAttack + _zeroAOALiftCoefficient;
		else if (_angleOfAttack > _maxAngleOfAttack)
		{
			float ceiling = _liftEffectiveness * _maxAngleOfAttack + _zeroAOALiftCoefficient;
			if (_angleOfAttack < _maxAngleOfAttack + _postStallResistance)
				return ceiling;
			else 
			{
				float effectiveAOA = (_maxAngleOfAttack + 2 * _postStallResistance) - angleOfAttack;
				if (effectiveAOA < 0) effectiveAOA = 0;
				return ceiling / std::pow(_postStallResistance, 0.5)*std::pow(effectiveAOA, 0.5);
			}
		}
		else if (_angleOfAttack < -_maxAngleOfAttack )
		{
			float ceiling = _liftEffectiveness * -_maxAngleOfAttack + _zeroAOALiftCoefficient;
			if (_angleOfAttack > -(_maxAngleOfAttack + _postStallResistance))
				return ceiling;
			else {
				float effectiveAOA = (_maxAngleOfAttack + 2 * _postStallResistance) + _angleOfAttack;
				if (effectiveAOA < 0) effectiveAOA = 0;
				return ceiling / std::pow(_postStallResistance, 0.5)*std::pow(effectiveAOA, 0.5);
			}
		}
		else return 0;

	}

	/***********************************************************
	*	속력에 따른 승강키의 효율을 계산한다. 양력계산식을 살짝 변형한다.
	*	받음각이 커지면 승강키가 의도한 방향의 반대로 작용하기도 한다.
	*	input	:
	*		_elevatorEffectiveness : 승강키의 기본 효율 (승강키가 크면 커진다)
	*		_postStallResistance : 스톨 직전의 특성(degree) (최대 받음각 + _postStallResistance) 까지는 양력이 애매하게는 존재한다
	*		_maxAngleOfAttack : 항공기의 최대 받음각
	*		_angleOfAttack: 현재 받음각
	*		_airSpeed : 항공기의 속력
	*		_altitude : 항공기의 고도
	*	return	:
	*		elevatorFlowEffectiveness : 현재 승강키의 효율 (음수~양수)
	***********************************************************/
	float getElevatorFlowEffectiveness(float _elevatorEffectiveness, float _elevator, float _maxAngleOfAttack, float _angleOfAttack, float _airSpeed, float _altitude) {
		float elevatorFlowEffectiveness = std::pow(_airSpeed, 2) * _elevatorEffectiveness;
		float hydraulicBoosterEffectiveness = std::pow(std::abs(airSpeed), 0.5) ;
		
		float aoaFactor = 0.05;
		/*if (std::abs(_angleOfAttack) > _maxAngleOfAttack)
			aoaFactor = 0.1 / (_elevator - _angleOfAttack);*/
		return  aoaFactor * getDensity(_altitude) * (elevatorFlowEffectiveness + hydraulicBoosterEffectiveness)   * 0.0005;
	}

	/***********************************************************
	*	속력에 따른 항공기의 stabilze 효율을 계산한다. 양력계산식을 살짝 변형한다.
	*	input	:
	*		_wingArea : 항공기의 익면적
	*		_mass : 항공기의 하중
	*		_airSpeed : 항공기의 속력
	*		_altitude : 항공기의 고도
	*		_angleOfAttack : 항공기의 받음각
	*	return	:
	*		aircragtFlowEffectiveness : 항공기의 stailize 효율
	***********************************************************/
	float getAircraftFlowEffectiveness(float _wingArea, float _mass, float _airSpeed, float _altitude, float _angleOfAttack) {
		return getDensity(_altitude) * std::pow(_airSpeed, 2) * std::pow(std::abs(_angleOfAttack), 0.2) * _wingArea / _mass * 0.01;
	}

	/***********************************************************
	*	중력이 기수를 아래로 내리는 힘을 구한다.
	*	input	:
	*		_wingArea : 항공기의 익면적
	*		_mass : 항공기의 하중
	*		_airSpeed : 항공기의 속력
	*		_altitude : 항공기의 고도
	*		_angleOfAttack : 항공기의 받음각
	*	return	:
	*		elevatorFlowEffectiveness : 현재 승강키의 효율
	***********************************************************/
	float getAircraftGravityEffectiveness(float _pitch) {

	}

	/***********************************************************
	*	공기 밀도를 고도에 따라 계산한다. 
	*	input	:
	*		alititude : 고도
	*	return	:
	*		density : 공기 밀도
	***********************************************************/
	float getDensity(float _altitude) {
		return  1.22480130 * std::pow((1 - 0.0065 * _altitude / 288.15), 4.25142574);
	}

	/***********************************************************
	*	양력은 구심력과 같으므로 이를 이용해 회전 반경을 구한다. 물론 중력계산도 해야한다.
	*	input	:
	*		_flightVector : 항공기의 진행방향
	*		_mass : 항공기의 하중
	*		_airSpeed : 항공기의 속력
	*		_lift : 항공기의 양력
	*	return	:
	*		flightVectorChange : 항공기 진행방향의 변화정도
	***********************************************************/
	float getTurnRadius(float _flightVector, float _mass, float _airSpeed, float _lift) {
		float pitchModifiedGravity = _mass * 9.8 * std::cos(_flightVector * PI / 180);
		return _mass  * _airSpeed * _airSpeed / (_lift - pitchModifiedGravity);
	}

	/***********************************************************
	*	고도변화는 곧 속도변화
	*	input	:
	*		_flightVector : 항공기의 진행방향
	*		_mass : 항공기의 하중
	*		_airSpeed : 항공기의 속력
	*		_lift : 항공기의 양력
	*	return	:
	*		flightVectorChange : 항공기 진행방향의 변화정도
	***********************************************************/
	float getAltiudeChange(float _airSpeed, float _flightVector) {
		return _airSpeed / FRAME_RATE * std::sin(_flightVector * PI / 180);
	}
	float getAirSpeed(float _flightVector, float _airSpeed, float altChange, float _thrust, float drag, float _mass) {
		float energyConverted = std::pow(-altChange *  9.8 + _airSpeed * _airSpeed, 0.5);
		float gravity = _mass * 9.8 * std::sin(_flightVector * PI / 180);
		float thrustToDrag = (_thrust - drag - gravity) / _mass / FRAME_RATE;
		
		return energyConverted + thrustToDrag;
	}

	/***********************************************************
	*	고도가 증가하면 엔진의 출력이 줄어든다, 속도가 증가하면 출력이 늘어난다.
	*	input	:
	*		_thrust : 항공기의 추력
	*		_altitude : 항공기의 고도
	*		_airSpeed : 항공기의 속도
	*		_angleOfAttack : 항공기 엔진정면으로 흡기되는 공기양을 구할 때 사용
	*	return	:
	*		thrust : 항공기 수정된 추력
	***********************************************************/
	float getThrust(float _thrust, float _altitude, float _airSpeed, float _angleOfAttack) {
		float aoa = _angleOfAttack;
		if (std::abs(_angleOfAttack) > 70) aoa = 70;
		float ias = std::abs(_airSpeed * std::cos(aoa * PI / 180));
		if (_airSpeed < 0) ias = 0;
		float speedFactor = std::pow(ias / 300, 0.5) * 0.9 + 0.1;

		float angel = _altitude / 1000;
		float altitudeFactor = std::pow(0.8577, angel) * 0.9 + 0.1;

		return _thrust * speedFactor * altitudeFactor;
	}

};