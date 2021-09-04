const int PinOutTrig=32;
const int PinOutValveIn=12;
const int PinOutValveOut=13;
const int PinInFlowIn=26;
const int PinInpH=25;
const int PinInTDS=35;
const int PinInTurb=34;
const int PinInEcho=33;

const int measDelay=5000;

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "GravityTDS.h"

const char* ssid="MORITA24G";
const char* pw="agoras69";

const char* mqtt_server="amejia.aulal.org";
const int mqtt_port=1883;

const char* mainTopic="2monitoreoagua/";
const char* sensorsTopic="sensores/";
char turbTopic[50],pHTopic[50],TDSTopic[50],LevelTopic[50],FlowInTopic[50],ValveInTopic[50],ValveOutTopic[50];

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg=0;
char msg[50];
int value=0;

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid,pw);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback_FlowControl(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();


  if (strcmp(topic,ValveInTopic)==0) {
    Serial.print("Input Valve set to ");
    if(messageTemp == "1"){
      Serial.println("Open");
      digitalWrite(PinOutValveIn, HIGH);
    }
    else if(messageTemp == "0"){
      Serial.println("Closed");
      digitalWrite(PinOutValveIn, LOW);
    }
  }else if(strcmp(topic,ValveOutTopic)==0){
    Serial.print("Output Valve set to ");
    if(messageTemp == "1"){
      Serial.println("Open");
      digitalWrite(PinOutValveOut, HIGH);
    }
    else if(messageTemp == "0"){
      Serial.println("Closed");
      digitalWrite(PinOutValveOut, LOW);
    }
  }
  
}

void createMQTTTopics(){
  strcpy(turbTopic,mainTopic);
  strcat(turbTopic,sensorsTopic);
  strcat(turbTopic,"Turbidez");

  strcpy(pHTopic,mainTopic);
  strcat(pHTopic,sensorsTopic);
  strcat(pHTopic,"pH");

  strcpy(TDSTopic,mainTopic);
  strcat(TDSTopic,sensorsTopic);
  strcat(TDSTopic,"TDS");

  strcpy(LevelTopic,mainTopic);
  strcat(LevelTopic,sensorsTopic);
  strcat(LevelTopic,"nivel");

  strcpy(FlowInTopic,mainTopic);
  strcat(FlowInTopic,sensorsTopic);
  strcat(FlowInTopic,"FlujoIn");

  strcpy(ValveInTopic,mainTopic);
  strcat(ValveInTopic,sensorsTopic);
  strcat(ValveInTopic,"ValvulaIn");

  strcpy(ValveOutTopic,mainTopic);
  strcat(ValveOutTopic,sensorsTopic);
  strcat(ValveOutTopic,"ValvulaOut");

}

void setup() {
  Serial.begin(115200);
  client.setServer(mqtt_server,mqtt_port);
  client.setCallback(callback_FlowControl);
  createMQTTTopics();

  pinMode(PinOutValveIn,OUTPUT);
  pinMode(PinOutValveOut,OUTPUT);
  pinMode(PinInFlowIn,INPUT);
  pinMode(PinInpH,INPUT);
  pinMode(PinInTDS,INPUT);
  pinMode(PinInTurb,INPUT);
  pinMode(PinInEcho,INPUT);
  pinMode(PinOutTrig,OUTPUT);
  setupFlowIn();
  xTaskCreate(TaskCheckConnection,"WiFiCheck",128,NULL,2,NULL);
  xTaskCreate(TaskMeasureFlowIn,"MeasFLowIn",256,NULL,3,NULL);
  xTaskCreate(TaskMeasureTDS,"MeasTDS",128,NULL,1,NULL);
  xTaskCreate(TaskTurbidity,"MeasTurb",128,NULL,0,NULL);
  xTaskCreate(TaskMeasureLevel,"MeasLevel",128,NULL,0,NULL);
  xTaskCreate(TaskpH,"MeaspH",128,NULL,0,NULL);
  xTaskCreate(TaskValveIn,"ValveIn",128,NULL,3,NULL);
  xTaskCreate(TaskValveOut,"ValveOut",128,NULL,3,NULL);
}


void loop() {
  //Nothing, using RTOS
}

long USecho=0;
float turb; //in NTU
float TDS=0;//
float pH=0;
float level=0;//in cm



void TaskCheckConnection(void *pvParameters){
  (void) pvParameters;
  for(;;){
    if (!client.connected()) {
      Serial.println("Reconnecting...");
      reconnect();
    }
    client.loop();
    vTaskDelay(200/portTICK_PERIOD_MS);
  }
}


volatile int flowPulses; // Measures flow sensor pulses
float flowIn; // Calculated litres/hour
unsigned long currentTime;
unsigned long cloopTime;

void setupFlowIn(){
  digitalWrite(PinInFlowIn,HIGH);
  attachInterrupt(0, ISRflow, RISING); // Setup Interrupt
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;
}

void ISRflow(){
  flowPulses++;
}

void TaskMeasureFlowIn(void *pvParameters){
  (void) pvParameters;
  for(;;){
    vTaskDelay(500/ portTICK_PERIOD_MS );
    currentTime = millis();
  
    if(currentTime >= (cloopTime + 2000)){
      cloopTime = currentTime; // Updates cloopTime
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      flowIn = (flowPulses * 60 / 7.5); // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour
      flowPulses++ = 0; // Reset Counter
      Serial.print("Flujo de entrada: ");
      Serial.print(flowIn); // Print litres/hour
      Serial.println("L/hora");
     }
  }
}


int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

#define TdsSensorPin A1
#define VREF 5.0
#define SCOUNT  30   
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,temperature = 25;

void TaskMeasureTDS(void *pvParameters){ 
  (void) pvParameters;
  for(;;){
    vTaskDelay(measDelay/ portTICK_PERIOD_MS );  
    static unsigned long analogSampleTimepoint = millis();
    if(millis()-analogSampleTimepoint > 40U){     //every 40 milliseconds,read the analog value from the ADC
      analogSampleTimepoint = millis();
      analogBuffer[analogBufferIndex] = analogRead(PinInTDS);    //read the analog value and store into the buffer
      analogBufferIndex++;
      if(analogBufferIndex == SCOUNT) 
      analogBufferIndex = 0;
    }   
    static unsigned long printTimepoint = millis();
    if(millis()-printTimepoint > 800U){
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++) analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      TDS=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
   
    Serial.print("TDS: ");
    Serial.print(TDS)
    Serial.println("ppm")
    client.publish(TDSTopic, TDS);
    }
  }
}


int max_distance = 25;

void TaskMeasureLevel(void *pvParameters){
  (void) pvParameters;
  for(;;){
    vTaskDelay(measDelay/ portTICK_PERIOD_MS );
    pinMode(trigPin,OUTPUT);
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    pinMode(echoPin, INPUT);
    USecho = pulseIn(echoPin, HIGH);
    level = microsecondsToCentimeters(USecho);
    if (level <max_distance){
      Serial.print("Water level: ")
      Serial.print(level);
      Serial.println(" cm");
    }
  }
  
}

void TaskTurbidity(void *pvParameters){
    (void) pvParameters;
    for(;;){
      vTaskDelay( measDelay / portTICK_PERIOD_MS );
      turb=0;
      for (int i=0; i<800;i++){
        turb += ((float)analogRead(PinInTurb)/1023)*5*2.41;
      }
      turb=turb/800;
      turb=roundf(turb*powf(10.0f,2));
      if(turb<2.5){
        turb=3000;
      }else{
        turb=-1120.4*square(turb)+5742.3*turb-4353.8;//Calibration Function
      }
      
      Serial.print("Turbidity: ");
      Serial.println(turb)Serial.print
      client.publish(turbTopic, turb);

    }
}

int buffer_arr[10], temp;
int pH = 0;
unsigned long int avgval;
float calibration_value = 21.34;

void TaskpH(void *pvParameters){
  (void) pvParameters;
  for(;;){
    vTaskDelay(measDelay/ portTICK_PERIOD_MS );
    for (int i = 0; i < 10; i++)
    {
      buffer_arr[i] = analogRead(PinInpH);
      delay(10);
    }
    for (int i = 0; i < 9; i++)
    {
      for (int j = i + 1; j < 10; j++)
      {
        if (buffer_arr[i] > buffer_arr[j])
        {
          temp = buffer_arr[i];
          buffer_arr[i] = buffer_arr[j];
          buffer_arr[j] = temp;
        }
      }
    }
    avgval = 0;
    for (int i = 2; i < 8; i++) avgval += buffer_arr[i];
    float volt = (float)avgval * 5.0 / 1024 / 6;
    float pH = -5.70 * volt + calibration_value;
    Serial.print("pH: ");
    Serial.println(pH);
    client.publish(pHTopic, pH);
  }

}
