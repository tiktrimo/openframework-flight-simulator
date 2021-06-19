#include "ofMain.h"
#include "Graph.h"

#define WIDTH 1280
#define HEIGHT 768

#define FRAME_RATE 60
#define MAX_DRAG_COEFFICENT 0.5
#define MIN_DRAG_COEFFICENT 0.02
#define POST_STALL_PERFORMANCE 3 //�Ǽ� ���� ����� �پ��� ���� (degree) POST_STALL_PERFORMANCE^2 < maxAngleOfAttack


class Aircraft {
public:
	Graph tacoBell = Graph(10, 500, 1200, 200);
	Graph tacoBell2 = Graph(10, 700, 1200, 200);


	//�װ��� �̹���
	ofImage image;
	//�̹��� ��ġ ����
	int imageX = 100, imageY = 100;
	//�װ����� ������ ����(�������� ��¦ �Ʒ��� �����ִ� ��찡 ����)
	float cockpitSlope = 0;

	//---------------- �װ����� �ڼ� -----------------
	//�װ����� ����ӵ�
	float groundSpeed;
	//�װ����� �����ӵ�
	float verticalSpeed;
	//�װ����� �ӷ�
	float airSpeed;
	//�װ����� ��
	float altitude;
	//�װ����� ����
	float pitch;
	//�װ����� �������
	float flightVector;
	//�װ����� ������(�װ����� �������� ����� ����)
	float angleOfAttack;
	
	//---------------- �װ����� ���� Ư�� -----------------
	//�װ����� ����(kg)
	float mass;
	//�װ����� �߷�(kN)
	float thrust;
	//�װ����� pitch�� �����ϴ� �°�Ű�� ����. �°�Ű�� ������ ����̸� �װ����� ����� ���.(degree)
	float elevator;
	//�װ����� �߷��� �����ϴ� throttle�� ���� 0~100
	float throttle;
	//�װ����� �ִ� �߷�
	float thrustLimit;
	//�װ��� �°�Ű�� �ִ� ����.
	float elevatorLimit;
	//�װ��� �°�Ű�� �⺻ ��ȿ�� 0~1
	float elevatorEffectiveness;
	//�װ��� ��°��
	float liftCoefficent;
	//�װ��� ��� ��ȿ�� (aoa - lift coefficent�� ����)
	float liftEffectiveness;
	//�װ��� �׷� ��ȿ�� (drag coefficnet�Լ� + 1�� ����)
	float dragEffectiveness;
	//�װ��� ���װ��
	float dragCoefficent;
	//�װ����� �ִ������
	float maxAngleOfAttack;
	//�װ����� ���� Ư�� (degree)
	float postStallResistance;
	//�װ����� �͸���
	float wingArea;
	//�װ����� ����
	float wingSpan;
	//�װ����� oswald ��� ���� ���� ���� ���� �����ȴ�.
	float oswaldFactor;
	//�װ����� �ּ� �ӵ�
	float stallSpeed;
	//�װ����� ������0������ ���
	float zeroAOALiftCoefficient;


	//---------------- �װ����� ���Ǵ� Ư�� -----------------
	// �װ����� ���
	float lift;
	//�װ��� �°�Ű�� ��ȿ�� 
	float elevatorFlowEffectiveness;

	//---------------- ��ǲ ������������ -----------------
	float elevatorDifference = 0;

	//---------------- �׽�Ʈ�� -----------------
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
	*	��Ȳ�� �����Ͽ� �������� ����Ѵ�.
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

		//�׷����� ������ �߰��Ѵ�.
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
	*	���õ� ����⸦ ���� pitch�� �°� �׸���.
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
	*	���� ������� ��������� �׸���.
	*	input	: offsetX, offsetY : ������ ��ġ�� �����ϰ� �����Ѵ�.
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
	*	���� ������� ���� �׸���.
	*	input	: offsetX, offsetY : ������ ��ġ�� �����ϰ� �����Ѵ�.
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
	*	�°�Ű�� �������� ���� ���� �����Ѵ�.
	*	input	:
	*		_elevator : ������ �°�Ű ��
	*	return	: none
	***********************************************************/
	void setElevator(float _elevator) {
		if (_elevator > elevatorLimit || _elevator < -elevatorLimit) return;

		elevator = _elevator;
	}

	/***********************************************************
	*	�°�Ű �������� �����Ѵ�. ������ �̷� �°�Ű�� �����Ӹ��� difference�� ���Ѵ�.
	*	input	:
	*		_elevator : ������ �°�Ű ��
	*	return	: none
	***********************************************************/
	void setElevatorDifference(float _elevatorDifference) {
		elevatorDifference = _elevatorDifference;
	}

	/***********************************************************
	*	����Ʋ�� �������� ���� ���� �����Ѵ�.
	*	input	:
	*		_throttle : ������ ����Ʋ ��
	*	return	: none
	***********************************************************/
	void setThrottle(float _throttle) {
		if (_throttle > 100 || _throttle < 0) return;

		throttle = _throttle;
	}
	/***********************************************************
	*	�׷� ����� �������� ���� ����Ѵ�. �ִ밪 MAX_DRAG_COEFFICENT �ּҰ� MIN_DRAG_COEFFICENT
	*	�׷� ����� �����Լ��� ����� �ϰ� �ִ�.
	*	input	:
	*		_maxAngleOfAttack : �װ����� �ִ� ������
	*		_angleOfAttack : �װ����� ������
	*	return	: 
	*		dragCoefficient : �׷� ���
	***********************************************************/
	float getDragCoefficient(float _maxAngleOfAttack, float _angleOfAttack) {
		float expoenentFactor = std::abs(_angleOfAttack / _maxAngleOfAttack);
		if (std::abs(_angleOfAttack) > 80)expoenentFactor = std::abs(80 / _maxAngleOfAttack);
		return std::pow(MAX_DRAG_COEFFICENT + 1, expoenentFactor) + MIN_DRAG_COEFFICENT - 1;
	
	}

	/***********************************************************
	*	induced drag�� ����Ѵ�. �� �׷��� �ӵ��� �������� �۾�����.
	*	input	:
	*		_maxAngleOfAttack : �װ����� �ִ� ������
	*		_angleOfAttack : �װ����� ������
	*	return	:
	*		dragCoefficient : �׷� ���
	***********************************************************/
	float getInduceDrag(float _mass, float _wingSpan, float _airSpeed, float _altitude, float _oswaldFactor, float _stallSpeed) {
		float ias = _airSpeed;
		if (std::abs(_airSpeed) < _stallSpeed ) ias = _stallSpeed ;
		return 2 * std::pow(_mass*9.8, 2) / (getDensity(_altitude) *std::pow(ias, 2) * PI * std::pow(_wingSpan, 2) * _oswaldFactor);
	}

	

	/***********************************************************
	*	��� ����� �������� ���� ����Ѵ�. 
	*	��� ����� _maxAngleOfAttack���� ���������� ��� �� �ް��ϰ� �����Ѵ�
	*	input	:
	*		_maxAngleOfAttack : �װ����� �ִ� ������
	*		_angleOfAttack : �װ����� ������
	*		_liftEffectiveness: ��°���� ����
	*	return	:
	*		liftCoefficient : ��� ���
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
	*	�ӷ¿� ���� �°�Ű�� ȿ���� ����Ѵ�. ��°����� ��¦ �����Ѵ�.
	*	�������� Ŀ���� �°�Ű�� �ǵ��� ������ �ݴ�� �ۿ��ϱ⵵ �Ѵ�.
	*	input	:
	*		_elevatorEffectiveness : �°�Ű�� �⺻ ȿ�� (�°�Ű�� ũ�� Ŀ����)
	*		_postStallResistance : ���� ������ Ư��(degree) (�ִ� ������ + _postStallResistance) ������ ����� �ָ��ϰԴ� �����Ѵ�
	*		_maxAngleOfAttack : �װ����� �ִ� ������
	*		_angleOfAttack: ���� ������
	*		_airSpeed : �װ����� �ӷ�
	*		_altitude : �װ����� ��
	*	return	:
	*		elevatorFlowEffectiveness : ���� �°�Ű�� ȿ�� (����~���)
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
	*	�ӷ¿� ���� �װ����� stabilze ȿ���� ����Ѵ�. ��°����� ��¦ �����Ѵ�.
	*	input	:
	*		_wingArea : �װ����� �͸���
	*		_mass : �װ����� ����
	*		_airSpeed : �װ����� �ӷ�
	*		_altitude : �װ����� ��
	*		_angleOfAttack : �װ����� ������
	*	return	:
	*		aircragtFlowEffectiveness : �װ����� stailize ȿ��
	***********************************************************/
	float getAircraftFlowEffectiveness(float _wingArea, float _mass, float _airSpeed, float _altitude, float _angleOfAttack) {
		return getDensity(_altitude) * std::pow(_airSpeed, 2) * std::pow(std::abs(_angleOfAttack), 0.2) * _wingArea / _mass * 0.01;
	}

	/***********************************************************
	*	�߷��� ����� �Ʒ��� ������ ���� ���Ѵ�.
	*	input	:
	*		_wingArea : �װ����� �͸���
	*		_mass : �װ����� ����
	*		_airSpeed : �װ����� �ӷ�
	*		_altitude : �װ����� ��
	*		_angleOfAttack : �װ����� ������
	*	return	:
	*		elevatorFlowEffectiveness : ���� �°�Ű�� ȿ��
	***********************************************************/
	float getAircraftGravityEffectiveness(float _pitch) {

	}

	/***********************************************************
	*	���� �е��� ���� ���� ����Ѵ�. 
	*	input	:
	*		alititude : ��
	*	return	:
	*		density : ���� �е�
	***********************************************************/
	float getDensity(float _altitude) {
		return  1.22480130 * std::pow((1 - 0.0065 * _altitude / 288.15), 4.25142574);
	}

	/***********************************************************
	*	����� ���ɷ°� �����Ƿ� �̸� �̿��� ȸ�� �ݰ��� ���Ѵ�. ���� �߷°�굵 �ؾ��Ѵ�.
	*	input	:
	*		_flightVector : �װ����� �������
	*		_mass : �װ����� ����
	*		_airSpeed : �װ����� �ӷ�
	*		_lift : �װ����� ���
	*	return	:
	*		flightVectorChange : �װ��� ��������� ��ȭ����
	***********************************************************/
	float getTurnRadius(float _flightVector, float _mass, float _airSpeed, float _lift) {
		float pitchModifiedGravity = _mass * 9.8 * std::cos(_flightVector * PI / 180);
		return _mass  * _airSpeed * _airSpeed / (_lift - pitchModifiedGravity);
	}

	/***********************************************************
	*	����ȭ�� �� �ӵ���ȭ
	*	input	:
	*		_flightVector : �װ����� �������
	*		_mass : �װ����� ����
	*		_airSpeed : �װ����� �ӷ�
	*		_lift : �װ����� ���
	*	return	:
	*		flightVectorChange : �װ��� ��������� ��ȭ����
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
	*	���� �����ϸ� ������ ����� �پ���, �ӵ��� �����ϸ� ����� �þ��.
	*	input	:
	*		_thrust : �װ����� �߷�
	*		_altitude : �װ����� ��
	*		_airSpeed : �װ����� �ӵ�
	*		_angleOfAttack : �װ��� ������������ ���Ǵ� ������� ���� �� ���
	*	return	:
	*		thrust : �װ��� ������ �߷�
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