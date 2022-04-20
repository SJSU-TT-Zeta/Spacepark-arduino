//car calculator declarations
#include <time.h>

//constant measurements-length between middle road and sensor needs to be calculated 
#define MIDDLEROAD 1

//thresholds for velcity and width 
#define velBig 1000.00
#define velSmol 0.1
#define widBig 1000.00
#define widSmol 0.01

//outlines pins connecting to ultrasonic sensor
#define trigPin1 12
#define echoPin1 11
#define trigPin2 10
#define echoPin2 9

//stuff that changes
float duration1, distance1, duration2, distance2, wid;
double velocity;



//firebase declarations
#include "Firebase_Arduino_WiFiNINA.h"

#define DATABASE_URL "spacepark-8c822-default-rtdb.firebaseio.com" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "n3n2f3SbEebl22U8n1buPwZFqoEjjaW7vq5s6DcO"
#define WIFI_SSID "Pen"
#define WIFI_PASSWORD "conrad123"

//Define Firebase data object
FirebaseData fbdo;
int carNum=0;



/*
sets up ultrasonic sensor inputs and outputs 
sets up wifi and firebase
*/
void setup()
{

  Serial.begin(115200);
  delay(100);
  Serial.println();

  Serial.print("Connecting to Wi-Fi");
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED)
  {
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Provide the autntication data
  Firebase.begin(DATABASE_URL, DATABASE_SECRET, WIFI_SSID, WIFI_PASSWORD);

  //counter car setup
    pinMode(trigPin1, OUTPUT);
    pinMode(echoPin1, INPUT);
    pinMode(trigPin2, OUTPUT);
    pinMode(echoPin2, INPUT);

}

//driver function
void loop()
{
  firebaseUpdate();

  //car counter
    sensorIntegrator();
    float placeholder = 999999.0;
    int ultraSen1 = detection(distance1, placeholder);
    int ultraSen2 = detection(distance2, placeholder);
    float sec = 0;

    carAdd(ultraSen1, ultraSen2, sec);
    carSub(ultraSen1, ultraSen2, sec);
    delay(100);

    //testing - lines below can be deleted
    if(velocity>0){
      delay(5000);
    }
    Serial.println();
    Serial.println("distances");
    Serial.println(distance1);
    Serial.println(distance2);
    Serial.println("data");
    Serial.println(velocity);
    Serial.println(wid);
    Serial.print("number of cars= ");
    Serial.println(carNum);
}




//functions

//updates realtime firebase with carNum/occupancy of cars and prints out value in Serial Monitor
void firebaseUpdate(){
  //updates firebase with number
    Firebase.reconnectWiFi(true);

  String path = "/garage";
  String jsonStr;
  int val=carNum;

  Serial.print("Set int... ");

  if (Firebase.setInt(fbdo, path + "/int", val)) //support large number
  {
    Serial.println("ok");
    Serial.println("path: " + fbdo.dataPath());
    Serial.println("type: " + fbdo.dataType());
    Serial.print("value: ");
    if (fbdo.dataType() == "int")
      Serial.println(fbdo.intData());
    if (fbdo.dataType() == "int64")
      Serial.println(fbdo.int64Data());
    if (fbdo.dataType() == "uint64")
      Serial.println(fbdo.uint64Data());
    else if (fbdo.dataType() == "double")
      Serial.println(fbdo.doubleData());
    else if (fbdo.dataType() == "float")
      Serial.println(fbdo.floatData());
    else if (fbdo.dataType() == "boolean")
      Serial.println(fbdo.boolData() == 1 ? "true" : "false");
    else if (fbdo.dataType() == "string")
      Serial.println(fbdo.stringData());
    else if (fbdo.dataType() == "json")
      Serial.println(fbdo.jsonData());
    else if (fbdo.dataType() == "array")
      Serial.println(fbdo.arrayData());
  }
  else
  {
    Serial.println("error, " + fbdo.errorReason());
  }
  
  Serial.println();
  // clear internal memory used
  fbdo.clear();
}


//takes the time taken from the sensor to send sound return back and translates it to determine distance. 
void sensorIntegrator() {

    // Write a pulse to the HC-SR04 Trigger Pin

    digitalWrite(trigPin1, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);
    duration1 = pulseIn(echoPin1, HIGH);

    digitalWrite(trigPin2, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin2, LOW);
    duration2 = pulseIn(echoPin2, HIGH);

    // Determine distance from duration
    // Use 343 metres per second as speed of sound

    distance1 = ((duration1 / 2) / 29.1) * 0.01;
    distance2 = ((duration2 / 2) / 29.1) * 0.01;

}

//check if calculated width is whithin the threshold. if not return a 0. if whithin threshold, return a 1
int widthApprove(float width) {
    //returns 1 if width is size of car
    if ((width <= widBig) && (width >= widSmol)) {
        return 1;
    }
    else
        return 0;
}

//check if calculated velocity is whithin the threshold. if not return a 0. if whithin threshold, return a 1
int velApprove(double velocity) {
    //returns 1 if velocity is within threshold of car velocity
    if((velocity<1.7)||(velocity>1.8)){
      if ((velocity <= velBig) && (velocity >= velSmol)) {
        return 1;
      }
      else{
        return 0;
      }
    }
    return 0;
}


/*if the distance between middle of the road to the sensor is present, no object
is detected and function returns a 0. if distance is less, return a 1 to say object is present*/
int detection(double distance, float timeDiff) {
    if (timeDiff == 999999.0) {
        //detects the presence of a car
        if (distance < MIDDLEROAD) {
            return 1;
        }
        else {
            return 0;
        }
    }
    else{
      return 0;
    }
}

//delay function set in miliseconds
void delay(int milliseconds)
{
    //some function I copied online lol, its just cause im horny for visual studios code and it doesn't come with delay in preinstalled library
    long pause;
    clock_t now, then;

    pause = milliseconds * (CLOCKS_PER_SEC / 1000);
    now = then = clock();
    while ((now - then) < pause)
        now = clock();
}

float widthCalc(double velocity, float time) {
    //calculates width in meters
    float ans = time * velocity;
    return ans;
}

/*function to identify entering object whether it is a vehicle or not based on velocity and width and 
add it from total occupancy or cars in carNum*/
void carAdd(int ultraSen1, int ultraSen2, float sec) {
    double timeDiff = 999999.0;
    while (ultraSen1 == 1) {
      Serial.println("World Hello");
        sensorIntegrator();
        ultraSen2 = detection(distance2, timeDiff);
        delay(100);
        sec += 0.1;
        if (ultraSen2 == 1) {
          Serial.println("Hello World");
            timeDiff = sec;
            ultraSen2 = 0;
        }
        ultraSen1 = detection(distance1, 999999.0);
        if(sec>3){
          break;
        }
    }

    /*7inches=0.1778meters
    velocity is in meters per second--TIMEDIFF CHECK IS IN SECONDS OR THIS CODE GO BONKERS*/

    velocity = 0.1778 / timeDiff;

    //returned width in meters
    wid = widthCalc(velocity, sec);

    //checking if width and velocity are within range
    int ver1=0;
    int ver2=0;
    ver1 = widthApprove(wid);
    ver2 = velApprove(velocity);

    if ((ver1 == ver2) && (ver1 != 0)) {
        carNum += 1;
        ver1 = 0;
        ver2 = 0;
    }
};

/*function to identify exiting object whether it is a vehicle or not based on velocity and width and 
subtract it from total occupancy or cars in carNum*/
void carSub(int ultraSen1, int ultraSen2, float sec) {
    double timeDiff = 999999.0;
    while (ultraSen2 == 1) {
        sensorIntegrator();
        ultraSen1=detection(distance1,timeDiff);
        delay(100);
        sec += 0.1;
        if (ultraSen1 == 1) {
            timeDiff = sec;
            ultraSen1 = 0;
        }
        sensorIntegrator();
        ultraSen2 = detection(distance2, 999999.0);
        if(sec>3){
          break;
        }
    }

    /*7inches=0.1778meters
    velocity is in meters per second--TIMEDIFF CHECK IS IN SECONDS OR THIS CODE GO BONKERS*/

    velocity = 0.1778 / timeDiff;

    //returned width in meters
    wid = widthCalc(velocity, sec);

    //checking if width and velocity are within range
    int ver1=0;
    int ver2=0;
    ver1 = widthApprove(wid);
    ver2 = velApprove(velocity);

    if ((ver1 == ver2) && (ver1 != 0)) {
        carNum -= 1;
        ver1 = 0;
        ver2 = 0;
    }
};
