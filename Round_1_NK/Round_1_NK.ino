#define sensorNum 8
#define maxSpeed 250
#define rotationSpeed 180

int blackLimit[sensorNum];
int digitalReading[sensorNum];

const int motorPin1=6,motorPin2=5;        //right motor
const int motorPin3=11,motorPin4=10;       //left motor

float error, prevError=0;

float mappedValue, targetValue =7;     //changed ferom 4.5 to 9
float safety=0.35;

float kp=40;                             //45 IF DOESN'T WORK
float kd=50;

int leftIR = 0, rightIR = 0;

int linegapCount=0, allWhiteCount=0;
int motorResponse;
float correction;

int leftSpeed,rightSpeed;


int time=5;
int lastBlack;
int i;



void setup()
{

    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);
    pinMode(motorPin3, OUTPUT);
    pinMode(motorPin4, OUTPUT);

    Serial.begin(9600);
    calibration();
    
}







void loop()
{

//     for(int i=0; i<8; i++ )
//     {
//         Serial.print(analogRead(i));
//         Serial.print(" ");
//
//
//     }

    Serial.print(leftIR);
    Serial.print(" ");
    Serial.print(rightIR);
    Serial.println();

    sensorMapping();
    if(digitalReading[0]==0 && digitalReading[7]==0){
          linegapCount++;
          if(linegapCount>40){
              
              rightIR = 0;
              leftIR = 0;
            }
          }
     else
        linegapCount=0;

    if(mappedValue == 100){
         allWhiteCount++;
          if(allWhiteCount>500){
              brake();
            }
          }
     else
        allWhiteCount=0;
        
    if(mappedValue!=100)  //if bot is not fully on white
    {
        pid();
        motor(leftSpeed,rightSpeed);
    }
    
    
    
    //Acute turn
    else
    {

        if(leftIR == 1 && rightIR ==0)
        {

            plannedACRotate();
              
            while(digitalReading[3] == 0 && digitalReading[4] == 0 )
            {
                sensorMapping();

            }

            leftIR = 0;
            rightIR = 0;
        }
        else if(leftIR == 0 && rightIR == 1 )
        {
            plannedCRotate();
                
            while(digitalReading[3] == 0&&digitalReading[4] == 0)
            {
                sensorMapping();

            }

            rightIR = 0;
            leftIR = 0;

        }
        else if(leftIR == 0 && rightIR ==0)
        {
            goForward();
            while(digitalReading[3] == 0&&digitalReading[4] == 0)
            {
                sensorMapping();

            }
            rightIR = 0;
            leftIR = 0;
        }
    }



    Serial.println();

}

void sensorMapping()
{
    int sum=0,count=0;

    for (int i = 0; i <sensorNum; i++)
    {

        if (analogRead(i) < blackLimit[i])
        {
            digitalReading[i] = 1;

            if(i > 0 && i < 7)
            {
            sum += i*2;
            count++;
            }
        }
        else
            digitalReading[i] = 0;

    }
    if(count!=0)
    {
        mappedValue = sum / count;
    }
    else
        mappedValue=100;

    if(digitalReading[0] || digitalReading[7])
    {
        leftIR = digitalReading[0];
        rightIR = digitalReading[7];
    }

}


void pid()
{

    error=targetValue-mappedValue;
    correction=(kp*error)+(kd*(error-prevError));
    prevError=error;
    motorResponse=(int)correction;

    if(motorResponse>maxSpeed)
        motorResponse=maxSpeed;

    if(motorResponse<-maxSpeed)
        motorResponse=-maxSpeed;

    if(motorResponse>0)
    {
        rightSpeed=maxSpeed;
        leftSpeed=maxSpeed-motorResponse;
    }
    else
    {
        rightSpeed=maxSpeed+ motorResponse;
        leftSpeed=maxSpeed;
    }

}

void motor(int left, int right)
{

    if(right>0)
    {
        analogWrite(motorPin1,right);
        analogWrite(motorPin2,0);
    }
    else
    {
        analogWrite(motorPin1,0);
        analogWrite(motorPin2,-right);
    }

    if(left>0)
    {
        analogWrite(motorPin3,left);
        analogWrite(motorPin4,0);
    }
    else
    {
        analogWrite(motorPin3,0);
        analogWrite(motorPin4,-left);
    }

}

void plannedCRotate()
{
    analogWrite(motorPin1,0);
    analogWrite(motorPin2, rotationSpeed);
    analogWrite(motorPin3, rotationSpeed);
    analogWrite(motorPin4,0);

}

void plannedACRotate()
{
    analogWrite(motorPin1,rotationSpeed);
    analogWrite(motorPin2, 0);
    analogWrite(motorPin3, 0);
    analogWrite(motorPin4,rotationSpeed);

}

void brake(void)
{
    analogWrite(motorPin1, 0);
    analogWrite(motorPin2, 0);
    analogWrite(motorPin3, 0);
    analogWrite(motorPin4, 0);
}

void goForward(void)
{
    analogWrite(motorPin1,rotationSpeed );
    analogWrite(motorPin2, 0);
    analogWrite(motorPin3, rotationSpeed);
    analogWrite(motorPin4, 0);


}
void goBack(void)
{
    analogWrite(motorPin1,0 );
    analogWrite(motorPin2, rotationSpeed);
    analogWrite(motorPin3, 0);
    analogWrite(motorPin4, rotationSpeed);


}
void calibration()
{
    plannedCRotate();
    int sensorArray[sensorNum][2];

    for(int i = 0; i < sensorNum; i++)
    {
        sensorArray[i][0] = analogRead(A0+i);
        sensorArray[i][1] = analogRead(A0+i);
    }


    int loopCounter = (int)(time * 1000 / 2.5);
    while(loopCounter)
    {
        for(int i = 0; i < sensorNum; i++)
        {
            if(analogRead(A0+i)<sensorArray[i][0])
                sensorArray[i][0]=analogRead(A0+i);
            if(analogRead(A0+i)>sensorArray[i][1])
                sensorArray[i][1]=analogRead(A0+i);
        }
        loopCounter--;
    }

    for(int i=0; i < sensorNum; i++)
        blackLimit[i] = (int)(sensorArray[i][0] + safety * (sensorArray[i][1] - sensorArray[i][0]));

    brake();
    delay(2000);
}



