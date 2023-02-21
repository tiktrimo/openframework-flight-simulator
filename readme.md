[![Youtube link](https://github.com/tiktrimo/EVE-Fitting.js/blob/master/DOCS/Header.png?raw=true)](https://youtu.be/OMyQAAAB2Ko)

Play it by opening flightSim_Debug.exe in release zip file<br>
C++ distribution for openframeworks v0.11.2_vs2017_release might be need.<br>
input:<br>
- left/right arrow : throttle
- up/down arrow : pitch
- 'd' : change plane
- 'r' : restart
- 'g' : toggle show/hide graph
- '.' ',' : change content of graph1
- '<' '>' : change content of graph2
<br>
<br>
<br>


**기말프로젝트**

**20171703 한상민**

**1. 프로젝트의 목표**

이번 프로젝트의 목표는 물리 법칙을 컴퓨터로 계산하여 시각적으로 확인할 수 있도록 하는 프로그램을 만드는 것이다. 특히 여러가지 물리 법칙 중에서도 개인적으로 관심이 있는 항공기와 관련된 물리 법칙을 계산하고 시각화 하여 시뮬레이션을 만들어 보았다.

**2. 변수 설명**

**class Aircraft** 

**항공기 자세에 관한 변수들:**<br>
<img src="https://github.com/tiktrimo/openframework-flight-simulator/blob/origin/images/3-1.png" width="400">
<br>

그림3-1

float groundSpeed : 속력의 수평 성분

float verticalSpeed : 속력의 수직 성분

float airSpeed : 속력

float altitude : 고도

float pitch : 그림3-1 참조

float flightVector: 그림3-1 참조

float angleOfAttack : 받음각. 그림3-1 참조

float throttle : 엔진 출력(1~100%)

**항공기의 물리량과 관련된 변수들:**

float mass : 항공기의 질량

float thrust : 항공기의 엔진의 출력

float elevator : 항공기 승강키의 입력값 (단위 없음) |NOT\_REALISTIC|

float thrustLimit : 항공기 엔진의 최대 출력

float elevatorLimit : 항공기 승강키의 최대 입력값 |NOT\_REALISTIC|

float elevatorEffectiveness : 항공기 승강키의 유효값 |NOT\_REALISTIC|

<br>
<img src="https://github.com/tiktrimo/openframework-flight-simulator/blob/origin/images/3-2.png" width="400">
<br>
그림 3-2 (양력 계수)

float liftCoefficent : 양력 계수. 공식으로 값을 구하지 않고 실험적으로 얻어진 그래프를 근사하여 값을 구한다. 참고 : <https://wright.nasa.gov/airplane/lifteq.html>

float liftEffectiveness : 양력 계수 선형 파트의 기울기

float zeroAOALiftCoefficient : 양력 계수 선형 파트의 y절편

float dragCoefficent : 항력 계수. 공식으로 값을 구하지 않고 실험으로 얻어진 그래프를 근사하여 갑을 구한다. 참고 : <https://www.grc.nasa.gov/www/k-12/airplane/dragco.html>

float dragEffectiveness : 항력 계수(friction drag) 지수 함수의 밑 값.

float maxG : 항공기가 버팅 수 있는 최대 중력 가속도. 

float maxAngleOfAttack : 항공기가 실속에 이르는 최대 받음각. 그림 3-2에서 CLmax의 a1값

float postStallResistance : CLmax의 a1값에서 아직 양력이 급격하게 줄어들지 않는 구간까지의 a2 값

float stallSpeed : 양력을 유지하기 위한 최소 속도

float wingArea : 익면적(날개의 면적)

float wingSpan : 날개의 폭

float oswaldFactor : 항력 계수를 계산할 때 사용한다. 참고 (이 페이지에서는 efficiency factor로 표기한다) : <https://www.grc.nasa.gov/www/k-12/airplane/induced.html>

계산 결과에 관한 변수들:

float lift : 양력

float elevatorFlowEffectiveness : 승강키의 유효값. |NOT\_REALISTIC|

float wingLift : 날개에서 발생된 양력

float dragLift : 항력의 양력 방향의 성분

<img src="https://github.com/tiktrimo/openframework-flight-simulator/blob/origin/images/3-3.png" width="400">
그림 3-3 (항력)

float inducedDrag : 그림 3-3 참조

float parasiteDrag : 그림 3-3 참조(friction drag로 표기됨)

float drag : 위 두 항력의 합. 그림 3-3 참조(total drag로 표기됨)

float G : 중력 가속도

//---------------- contorl input interpolation -----------------

float elevatorDifference = 0 : 더 연속적인 입력을 만들기 위해서 사용.

**class Graph**

float circularArr[GRAPH\_LENGTH] = { 0 } : 원형 배열

int start = 0 : 원형 배열의 시작점. 이 원형 배열은 끝점이 시작점 - 1 이다.

char graphName[100] : 그래프의 이름

float x, y, width, height, hiddenMul = 1 : x, y: 그래프의 좌상단의 좌표 , hiddenMul : 그래프의 y스케일










**3. 알고리즘 및 자료구조 설명**

**class Aircraft** 

` `현실에서는 동시에 발생하는 일(e.g.작용 반작용)들을 컴퓨터는 여러 요소로 나누어 순서대로 계산해야 한다. 이 때 어떤 순서대로 계산해야 하는지가 제일 어려운 과제이다. 
<br>
<img src="https://github.com/tiktrimo/openframework-flight-simulator/blob/origin/images/chart.png" width="400">
<br>
lift coefficient, drag coefficient 등 가장 중요하게 계산되어야 할 값들은 aoa (angleOfAttack)에 따라 계산된다. aoa는 단순히 elevator에 의해서만 변화하지 않고 aoa 자체적으로 항공기가 안정화되는 방향으로 aoa가 변화하게 된다. 또한 elevator는 aoa에 의해 그 효율이 변화한다. 위의 이야기를 종합해보면 aoa가 안정되는 계산을 하고나서 elevator에 의한 aoa변화를 계산할지 아니면 elevator에 의한 aoa 변화를 먼저 계산하고 변화된 aoa를 기준으로 안정화를 계산할지 애매해진다. 말이 어렵다. 주제가 관련이 없지만 “닭이 먼저냐 알이 먼저냐” 와 같다. 본 알고리즘을 작성할 때는 elevator에 의한 변화를 먼저 계산하였다. aoa가 계산되고 나면 나머지는 단순 계산에 불과하다.

**class Graph**

|**...**|**...**|**+569**|**+570**|**current**|**+1**|**+2**|**...**|
| :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |


|**...**|**...**|**...**|**+569**|**+570**|**current**|**+1**|**+2**|
| :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |

그래프를 그릴 때 정보를 저장할 자료구조가 필요하다. 그래프에는 매 프레임마다 aircraft 클래스에서 계산된 정보가 삽입되게 된다. 이 정보를 프로그램의 시작부터 끝까지 유지할 필요 없으며 화면에 표시할 정도만 유지하면 된다.

`  `이에 변형된 circular array를 이용한다. circular array는 start와 end를 이용하여 overflow가 일어나지 않게 하지만 여기서는 지속적으로 정보를 갱신하고 오래된 정보는 필요 없으므로 overflow를 활용한다. 570프레임의 계산 결과를 저장한다면 571프레임 전의 정보를 지금 계산된 프레임의 정보로 갱신하고 start를 갱신하면 된다. start부터 start-1까지 한바퀴 출력하면 그래프가 시간에 따라 갱신되는 것처럼 보이게 된다.

**4. 함수 설명**

**class Aircraft** 

**setters :**

void setElevator(float \_elevator)

void setElevatorDifference(float \_elevatorDifference)

void setThrottle(float \_throttle)

**draw functions :**

**void simulate() :**

상황을 종합하여 물리량을 계산한다.

input	: none

return	: none

**void draw() :** 

`	`선택된 비행기를 현재 pitch에 맞게 그린다.

`	`input	: none

`	`return	: none

**void showFlightVector(float offsetX, float offsetY) :**

`	`현재 비행기의flight vector를 그린다.

`	`input	: offsetX, offsetY : 원점의 위치를 세밀하게 조정한다.

`	`return	: none




**void showAltitude(float offsetX, float offsetY) :**

현재 비행기의 고도를 그린다.

`	`input	: offsetX, offsetY : 원점의 위치를 세밀하게 조정한다.

`	`return	: none

**calculation functions :**

**float getDragCoefficient(float \_maxAngleOfAttack, float \_angleOfAttack) :**

항력 계수를 받음각에 따라 계산한다. 최대값 MAX\_DRAG\_COEFFICENT 최소값 MIN\_DRAG\_COEFFICENT

`	`항력 계수는 지수함수의 모습을 하고 있다.

`	`input	:

`		`\_maxAngleOfAttack : 항공기의 최대 받음각

`		`\_angleOfAttack : 항공기의 받음각

`	`return	: 

`		`dragCoefficient : 항력 계수

**float getInduceDrag(float \_mass, float \_wingSpan, float \_airSpeed, float \_altitude,** 

**float \_oswaldFactor, float \_stallSpeed) :**

`	`induced drag를 계산한다. 이 항력은 속도가 높아지면 작아진다.

`	`input	:

`		`\_maxAngleOfAttack : 항공기의 최대 받음각

`		`\_angleOfAttack : 항공기의 받음각

`	`return	:

`		`dragCoefficient : 항력 계수

**float getLiftCoefficient(float \_maxAngleOfAttack, float \_postStallResistance, float \_angleOfAttack, float \_liftEffectiveness, float \_zeroAOALiftCoefficient) :**

`	`양력 계수를 받음각에 따라 계산한다. 

`	`양력 계수는 \_maxAngleOfAttack까지 선형적으로 상승 후 급격하게 감소한다

`	`input	:

`		`\_maxAngleOfAttack : 항공기의 최대 받음각

`		`\_angleOfAttack : 항공기의 받음각

`		`\_liftEffectiveness: 양력계수의 기울기

`	`return	:

`		`liftCoefficient : 양력 계수

**float getElevatorFlowEffectiveness(float \_elevatorEffectiveness, float \_elevator,**

` `**float \_maxAngleOfAttack, float \_angleOfAttack,**

**float \_airSpeed, float \_altitude) :**

`	`속력에 따른 승강키의 효율을 계산한다. 양력계산식을 살짝 변형한다.

`	`받음각이 커지면 승강키가 의도한 방향의 반대로 작용하기도 한다.

`	`input	:

`		`\_elevatorEffectiveness : 승강키의 기본 효율 (승강키가 크면 커진다)

`		`\_postStallResistance : 스톨 직전의 특성(degree) 

`		`\_maxAngleOfAttack : 항공기의 최대 받음각

`		`\_angleOfAttack: 현재 받음각

`		`\_airSpeed : 항공기의 속력

`		`\_altitude : 항공기의 고도

`	`return	:

`		`elevatorFlowEffectiveness : 현재 승강키의 효율 (음수~양수)

**float getAircraftFlowEffectiveness(float \_wingArea, float \_mass, float \_airSpeed, float \_altitude,**

**float \_angleOfAttack) :**

속력에 따른 항공기의 stabilze 효율을 계산한다. 양력계산식을 살짝 변형한다.

`	`input	:

`		`\_wingArea : 항공기의 익면적

`		`\_mass : 항공기의 하중

`		`\_airSpeed : 항공기의 속력

`		`\_altitude : 항공기의 고도

`		`\_angleOfAttack : 항공기의 받음각

`	`return	:

`		`aircragtFlowEffectiveness : 항공기의 stailize 효율

**float getDensity(float \_altitude) :**

공기 밀도를 고도에 따라 계산한다. 

`	`input	:

`		`alititude : 고도

`	`return	:

`		`density : 공기 밀도



**float getTurnRadius(float \_flightVector, float \_mass, float \_airSpeed, float \_lift) :**

`	`양력은 구심력과 같으므로 이를 이용해 회전 반경을 구한다. 물론 중력계산도 해야한다.

`	`input	:

`		`\_flightVector : 항공기의 진행방향

`		`\_mass : 항공기의 하중

`		`\_airSpeed : 항공기의 속력

`		`\_lift : 항공기의 양력

`	`return	:

`		`flightVectorChange : 항공기 진행방향의 변화정도

**float getAirSpeed(float \_flightVector, float \_airSpeed, float altChange, float \_thrust, float drag,** 

**float \_mass) :**	

고도변화는 곧 속도변화

`	`input	:

`		`\_flightVector : 항공기의 진행방향

`		`\_mass : 항공기의 하중

`		`\_airSpeed : 항공기의 속력

`		`\_lift : 항공기의 양력

`	`return	:

`		`flightVectorChange : 항공기 진행방향의 변화정도

**float getThrust(float \_thrust, float \_altitude, float \_airSpeed, float \_angleOfAttack)**

`	`고도가 증가하면 엔진의 출력이 줄어든다, 속도가 증가하면 출력이 늘어난다.

`	`input	:

`		`\_thrust : 항공기의 추력

`		`\_altitude : 항공기의 고도

`		`\_airSpeed : 항공기의 속도

`		`\_angleOfAttack : 항공기 엔진정면으로 흡기되는 공기양을 구할 때 사용

`	`return	:

`		`thrust : 항공기 수정된 추력

**float getAttributeValue(int attributeID)**

`	`attributeID에 해당하는 값을 반환한다.

**void switchPlane(int \_type)**

`	`주어지는 type에 따라 변수들을 초기 설정한다.

**class Graph**

**void insert(float data)**

`	`새로운 데이터를 삽입한다.

**template<typename F>**

**void run(F &lambda)**

`	`lambda 함수를 인자로 받고 그 함수를 circularArr의 모든 element에 대하여 실행한다.

**void draw()**

`	`그래프를 그린다.


**6. 창의적인 구현 방식**

현실의 물리 법칙에 따라 계산할 때 모든 요소를 계산할 수 없다. 특히 실시간으로 시뮬레이션 하는 방법은 이 특징이 더 두드러진다. 알고리즘을 얼핏 보았을 때 현실의 항공기를 어느정도 정확하게 계산해내는 것처럼 보일 수 있으나 실상은 전혀 그렇지 않다. 이는 물리 문제에서 “이 때 마찰력은 무시하고 계산한다”와 같은 맥락이라고 할 수 있다. 
<br>
  <img src="https://github.com/tiktrimo/openframework-flight-simulator/blob/origin/images/3-4.png" width="400">
  <br>
그림 3-4

그림3-4에서 언급되었듯이 lift coefficient는 변수 간의 관계가 복잡하여 계산하지 않고 실험적으로 얻어진다고 되어 있다. 같은 이유로 프로젝트를 진행하면서 모든 값들을 수식을 이용하여 구하지 않고 실험값에 근사하는 식을 이용하였다. 

또한 구현하기에 너무 많은 시간이 소요될 것 같은 문제는 단순화시켰다. 특히 elevator와 관련된 함수와 변수들 에는 NOT\_REALISTIC 이 붙어있다. 이는 각 항공기에 대하여 elevator의 상세한 정보를 알 수 없었기 때문만 아니라 이를 계산해내도 거기서 발생하는 토크를 구하고 그 토크가 비행기에 미치는 영향을 계산하는 기능을 구현하기에는 시간이 없었다.

그래서 elevator와 관련된 것들은 “사실과 비슷하게” 꾸며졌다고 할 수 있다. 물론 아무 근거 없이 구현하지 않았다. lift를 구하는 공식을 변형하였고 공기의 밀도, 항공기의 질량 등 최대한 현실을 반영하려고 노력하였다. 

또한 시각적으로 표현하는 것만으로는 인지적으로 이게 현실과 같다고 느낄 수 없었다. 이를 보완하기 위해 소리를 활용하였고, 결과 충분히 현실과 비슷하다고 느낄 수 있었다.

**7. 실행 결과**











**8. 느낀 점 및 개선 사항**

이 프로젝트를 진행하면서 가장 크게 느낀 것은 모든 것을 계산할 수는 없다는 것이다. 물론 학부생이 제한된 지식으로 작성했기 때문에 계산의 복잡함이 매우 크다고 할 수 없다. 하지만 계산할 요소가 더 많아지고 이를 아무리 최적화한다고 해도 모든 요소를 계산하게 되면 집에 있는 컴퓨터로는 한 프레임 계산하는 데 한달은 걸릴 것이다. 

이는 실시간으로 보여주는 시뮬레이션(MSFS2020등)은 간략화 된 수식을 이용하고 현실과 “비슷하게” 보여주는 것이 한계라는 점이다. 현실에서 비행하는 것과 시뮬레이션을 하는 것은 “비슷하다” 라는 느낌을 받을 수 있지만 일어나고 있는 일은 전혀 다르다는 것이다.

이 프로젝트를 개선한다면 토크를 정확하게 계산해내고 싶다. 승상키에 관한 정확한 정보를 찾으면 얻을 수 있겠으나 그 문서를 이해하고 알고리즘을 작성하는 것은 또 별개의 문제이다. 시간이 남는다면 천천히 읽으면서 이해하고 적용해보고 그 결과가 어떻게 나오는지 궁금하다.

그림 3-1 : <https://www.researchgate.net/figure/The-pitch-angle-with-the-direction-of-an-aircraft_fig1_321450231>

그림 3-2 : <https://www.researchgate.net/figure/Lift-coefficient-plotted-as-a-function-of-angle-of-attack-schematic_fig1_2611242>

그림 3-3 : <http://www.aerodynamics4students.com/aircraft-performance/drag-and-drag-coefficient.php>

그림 3-4 : <https://www.grc.nasa.gov/www/k-12/airplane/liftco.html>















