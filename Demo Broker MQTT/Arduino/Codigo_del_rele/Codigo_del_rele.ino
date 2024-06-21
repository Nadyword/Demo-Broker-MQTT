#include <WiFi.h>//---------------------------------------------> Libreria para el maneto del modulo ESP8266
#include <PubSubClient.h>//-------------------------------------> Libreria para suscribirme al servidor MQTT

const char* ssid = "Madre rusia";  //---------------------------> Nombre de tu SSID
const char* password = "El chepa";  //--------------------------> Tu contraseña del WIFI

const char* mqtt_server = "154.56.41.105";  //------------------> Ip del broker MQTT
const char* mqtt_user = "Nadyword";  //-------------------------> Usuario MQTT
const char* mqtt_password = "Sa753951.";  //--------------------> Contraseña MQTT

const int relayPin = 4;  //-------------------------------------> el pin de control del relé
const int interruPin = 6;  //-----------------------------------> el pin de control del interruptor
const int ledPin = 3;  //---------------------------------------> el pin del led
int lastInterruState;//-----------------------------------------> Estado anterior del pin del interruptor

const String IdClient = "0.00.03.00";  //-----------------------> Identificador del modulo
const String TopincResul =  "Resection";  //--------------------> Topinc al que responde
const String Firma =  " [" + IdClient + "/01]";  //-------------> Firma para responder

WiFiClient espClient;//-----------------------------------------> Configuraicon TCP/IP del modulo
PubSubClient client(espClient);//-------------------------------> Configuraicon para unir el boker con el modulo

void setup() {
  pinMode(relayPin, OUTPUT);//----------------------------------> Establer el pin del rele como salida
  pinMode(interruPin, INPUT);//---------------------------------> Establer el pin del rele como salida
  digitalWrite(relayPin, LOW);//--------------------------------> Asegura que el relé esté apagado inicialmente
  setup_wifi();//-----------------------------------------------> Conectarse a la red Wifi
  client.setServer(mqtt_server, 1883);//------------------------> Conectarse al boker
  client.setCallback(callback);
  lastInterruState = digitalRead(interruPin);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  ChangeInterructor();
}

//-----------------------------------------------------METODOS-----------------------------------------------------//

//Envio de mensajes
void SendRequest(String Message){
  client.publish(TopincResul.c_str(), (Message + Firma).c_str());  
}

//Cunsultar del estado del rele
String StatudRele(){
  String estado;
  if (digitalRead(relayPin))
  {
    estado = "ON" + Firma;
  }
  else
  {
    estado = "OFF" + Firma;
  }
  return estado;
}

//Cambiar el estado del rele
void ChangeStatusRele(){
  digitalWrite(relayPin,!digitalRead(relayPin));
}

//Conectarse a la red Wifi
void setup_wifi() {
  delay(15);
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(550);
  }
}

//Reconectar al servidor si se pierde la coneccion
void reconnect() {
  while (!client.connected()) {
    if (client.connect(IdClient.c_str(), mqtt_user, mqtt_password)) {
      client.subscribe("Client/0.00.07.00.01");
    } else {
      delay(5000);
    }
  }
}

//Procesamiento de las llamadas
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "Client/0.00.07.00.01") {
    if (message == "Change") {
      ChangeStatusRele(); 
    } else if (message == "Consult") {
      SendRequest(StatudRele());
    }
  }
}

void ChangeInterructor(){
  int currentInterruState = digitalRead(interruPin);

  if (currentInterruState != lastInterruState) {
    ChangeStatusRele();
    lastInterruState = currentInterruState;
    delay(330);  // Pequeño retardo para evitar rebotes
  }
}