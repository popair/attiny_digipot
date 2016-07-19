/*
  byte CS = 1;
  byte dataPin  = 2;
  byte clockPin  = 0;
  byte analogPin  = 3;
  byte sw = 7;
*/

byte CS = 10;
byte dataPin  = 11;
byte clockPin  = 13;         

#define analogPin A0        ///>  Set Analog Read Potentiometer pin
#define sdPin A1            ///>  Set Sub Div analog read pin
byte sw = 7;                ///>  Set swtich input pin

void setup() {
  pinMode(CS, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(analogPin, INPUT);
  pinMode(sw, INPUT);

  digitalWrite(sw, HIGH);

  delay(200);
  Serial.begin(9600);
  delay(200);
}

int lastVal = 0;
bool hasChanged = false;

/*
   0: Timer Pot
   1: Tap Switch
*/
byte lastAccess = 0;

unsigned int db = 100;
unsigned long dbTime = millis();

unsigned long pressMillis = millis();
byte cnt = 0;
byte numStep = 0;
unsigned long mArr[2] = {0, 0};
unsigned long lastMillis = millis();
unsigned long maxReadTime = 4000;

//unsigned int minMillis = 60000 / maxBPM;
//unsigned int maxMillis = 60000 / minBPM;

unsigned int minMillis = 57;
unsigned int maxMillis = 1000;

bool cntHasReset = true;
float avgMillis = 0;

float divisions[] = {1, 0.5, 0.25, 0.6666667, 0.333333};   
int subDivisions[6] = {0, 204, 408, 612, 816, 1024};
byte currentDiv = 2;

bool swReset = false;

int val = 0;        ///>  Value to be send to digital potentiometer


int subDiv = 0;

void loop() {

  val = analogRead(analogPin);
  val = map(val, 0, 1024, 0, 128);

  potHasChanged(val);

  if (switchPressed()) {
    Serial.println("Sw pressed");
    if (numStep == 0) {
      Serial.println("num0");
      lastMillis = millis();
      numStep++;
    } else if (numStep == 1) {
      Serial.println("num1");
      mArr[cnt] = millis() - lastMillis;
      cycleCount();
      lastMillis = millis();
      numStep++;
    } else if (numStep >= 2) {
      float tempMillis = 0;
      Serial.println("num>=2");
      hasChanged = true;
      mArr[cnt] = millis() - lastMillis;
      for (byte i = 0; i < 2; i++) {
        tempMillis += mArr[i];
      }
      avgMillis = (tempMillis / 2);
      Serial.print("Avg Delay:");
      Serial.println(avgMillis);
      
      avgMillis *= divisions[currentDiv];
      
      float prob = (((maxMillis - avgMillis) * 1.0) / (maxMillis - minMillis));
      prob = constrain(prob, 0, 1);
      val = 128.0 * prob;
      
      cycleCount();
      lastMillis = millis();
      numStep++;
      Serial.print("Proportion:" );
      Serial.println(prob);
      Serial.print("Val:");
      Serial.println(val);
      Serial.print("Avg Delay subDiv:");
      Serial.println(avgMillis);
      Serial.print("numStep: ");
      Serial.println(numStep);

    }
    cntHasReset = false;
  }

  // Reset counter after 4 seconds of no switch press
  if (!cntHasReset) {
    if (millis() - lastMillis > maxReadTime) {
      cnt = 0;
      numStep = 0;
      avgMillis = 0;
      cntHasReset = true;
      Serial.println("Reset");
    }
  }

  if (hasChanged) {
    digitalWrite (CS, LOW);
    shiftOut (dataPin, clockPin, MSBFIRST, highByte (val) );
    shiftOut (dataPin, clockPin, MSBFIRST, lowByte (val) );
    digitalWrite (CS, HIGH);
    hasChanged =  false;

    Serial.print("Setting val to ");
    Serial.println(val);
  }


}

bool switchPressed() {
  if (swReset && digitalRead(sw) == HIGH && millis() - dbTime > db) {
    swReset = false;
    dbTime = millis();
  }
  if (digitalRead(sw) == LOW && millis() - dbTime > db && !swReset) {
    swReset = true;
    dbTime = millis();
    return true;
  }
  return false;
}

void readSubDivisonPin () {
  subDiv = analogRead(sdPin);

}

bool potHasChanged(byte inVal) {
  if (val != lastVal) {
    hasChanged = true;
    lastVal = val;
    lastAccess = 0;
    return true;
  }
  return false;
}

void cycleCount() {
  cnt++;
  if (cnt > 1) cnt = 0;
}

