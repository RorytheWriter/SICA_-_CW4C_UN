/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

 #include <WiFi.h>
#include <PubSubClient.h>
//#include <Wire.h>



const char* ssid = "Ivan tamayo sub red";
const char* password = "1E7084C16CC3D";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.0.25";
const char* mqtt_server = "192.168.0.25";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;





float turbidez = 0;
float ph = 0;
float nivel = 0;


int sensorValue = 0; 


int ledPin = 2;


int in_1=0;

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
  pinMode(23  , INPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();


  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }

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
void loop() { 

     turbidez = analogRead(36);
     ph=analogRead(39);
     //nivel=analogRead(34);
  
  

  
  in_1=digitalRead(23);
     
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

 

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    

    
  
    char turbString[8];
    dtostrf(turbidez, 1, 2, turbString);
    Serial.print("Turbidez: ");
    Serial.println(turbString);
    client.publish("monitoreoagua/sensores/turbidez", turbString);






//   ph sensor;


    
    // Convert the value to a char array
    char phString[8];
    dtostrf(ph, 1, 2, phString);
    Serial.print("ph: ");
    Serial.println(phString);
    client.publish("monitoreoagua/sensores/ph", phString);
    //Serial.println(sensorValue);



//   nivel sensor;

    
    // Convert the value to a char array
    char nivelString[8];
    dtostrf(in_1, 1, 2, nivelString);
    Serial.print("nivel: ");
    Serial.println(nivelString);
    client.publish("monitoreoagua/sensores/nivel", nivelString);
    //Serial.println(sensorValue); 

       
    
  }
}
