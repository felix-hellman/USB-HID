
int togPin = 6;
int butPin = 7;
int ledPin = 13;
int gndPin = 5;

void setup() {

  pinMode(ledPin, OUTPUT);
  pinMode(butPin, INPUT);
  pinMode(togPin, INPUT);
  pinMode(gndPin, OUTPUT);
  Serial.begin(9600);
  digitalWrite(gndPin,0);
}

int buttonStatus;
int toggleStatus;
int serialValue;
int sendData;
int n;
int prevData;

elapsedMillis msUntilNextSend;

byte transferData[64];

void loop(){
 
 n = RawHID.recv(transferData,0);
 toggleStatus = digitalRead(togPin);
 if(digitalRead(butPin) == 1) buttonStatus = 1;
 serialValue =  analogRead(2);
 sendData = serialValue + pow((toggleStatus*2),10) + pow((buttonStatus*2),11);  
 convertData(sendData,transferData);
  if(msUntilNextSend > 100){
  msUntilNextSend = msUntilNextSend -100;
  n = RawHID.send(transferData,100);
  buttonStatus = 0;
  }
}

void convertData(unsigned int value, byte *convertedValue)
{
 int i;
 int k;
 int data[6];
 int count;
 count = 2;
 k = value;
 for(i = 0; i < 6;i++) data[i] = 0;
 while( k > 0)
 {
   i = k % 16;
   k = k / 16;
   data[count] = i; 
   count = count - 1;
 }
 
 for(i = 0; i < 3; i++)
 {
  convertedValue[i] = data[i*2]*16 + data[i*2+1];
 }
}
