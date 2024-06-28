// BEGIN Librerias
#include <WiFi.h>          // Libreria para el manejo del modulo ESP8266/ESP32
#include <PubSubClient.h>  // Libreria para suscribirse al servidor MQTT
#include <Preferences.h>   // Libreria para almacenar datos en la memoria no volatil
// END Librerias

// BEGIN Variables
String ssid = "Madre rusia";         // Nombre de tu SSID
String password = "El chepa";        // Contraseña de tu WIFI
String mqtt_server;  // Dirección IP del broker MQTT
String mqtt_user;  // Usuario MQTT
String mqtt_password;  // Contraseña MQTT
const int relayPin = 4;  // Pin de control del relé
const int interruPin = 6;  // Pin de control del interruptor
const int ledPin = 3;  // Pin del LED
int lastInterruState;  // Estado anterior del pin del interruptor
String IdModule;       // Identificador del módulo
String TopincAnswer;   // Tópico al que responde
String numModule;      // Identificador del módulo, para el IdRegion
String Firma;          // Firma para responder
String ServerSup;      // Tópico al que está suscrito
Preferences preferences;  // Variable para gestionar y guardar valores en la memoria no volátil
// END Variables

WiFiClient espClient;  // Configuración TCP/IP del módulo
PubSubClient client(espClient);  // Configuración para unir el broker con el módulo

// BEGIN void setup
void setup() {
  Serial.begin(115200);  // Inicializar comunicación serial
  pinMode(relayPin, OUTPUT);  // Establecer el pin del relé como salida
  pinMode(interruPin, INPUT);  // Establecer el pin del interruptor como entrada
  digitalWrite(relayPin, LOW);  // Asegurar que el relé esté apagado inicialmente

  // Inicializar Preferences
  preferences.begin("my-app", false);
  // Leer valores almacenados o usar valores por defecto si no están almacenados
  IdModule = preferences.getString("IdModule", "0.00.01.00");
  TopincAnswer = preferences.getString("TopincAnswer", "Resection");
  ServerSup = preferences.getString("ServerSup", "moduleClient/" + IdModule);
  numModule = preferences.getString("numModule", "00");
  mqtt_server = preferences.getString("mqtt_server","192.168.194");
  mqtt_user = preferences.getString("mqtt_user","Nadyword");
  mqtt_password = preferences.getString("mqtt_password","Sa753951.");
  numModule = preferences.getString("numModule", "00");
  preferences.end();
  // Formar la firma combinando IdModule y numModule
  Firma = "[" + IdModule + "/" + numModule + "]";

  setup_wifi();  // Conectarse a la red WiFi
  client.setServer(mqtt_server.c_str(), 1883);  // Configurar el broker MQTT
  client.setCallback(callback);  // Establecer el callback para manejar mensajes MQTT entrantes
  lastInterruState = digitalRead(interruPin);  // Leer el estado inicial del interruptor
}
// END void setup

// BEGIN void loop
void loop() {
  // Reconectar al servidor MQTT si la conexión se pierde
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Mantener la conexión MQTT activa
  ChangeInterructor();  // Verificar cambios en el estado del interruptor
}
// END void loop

// -----------------------------------------------------METODOS-----------------------------------------------------

// Enviar mensajes a través de MQTT
void SendRequest(String Message) {
  client.publish(TopincAnswer.c_str(), (Message + Firma).c_str());
}

// Consultar el estado del relé
String StatudRele() {
  return digitalRead(relayPin) ? "ON" : "OFF";
}

// Cambiar el estado del relé
void ChangeStatusRele() {
  digitalWrite(relayPin, !digitalRead(relayPin));
}

// Conectarse a la red WiFi
void setup_wifi() {
  delay(15);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(550);
  }
}

// Reconectar al servidor MQTT si la conexión se pierde
void reconnect() {
  while (!client.connected()) {
    if (client.connect(IdModule.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
      client.subscribe(ServerSup.c_str());  // Suscribirse al tópico
    } else {
      delay(3000);  // Esperar 3 segundos antes de reintentar
    }
  }
}

// Refrescar la conexión y cambiar suscripciones
void ChangeSubscription(String NewTopic) {
  client.unsubscribe(ServerSup.c_str());  // Desuscribirse del tópico antiguo
  ServerSup = NewTopic;  // Actualizar el tópico suscrito
  client.subscribe(NewTopic.c_str());  // Suscribirse al nuevo tópico
  // Guardar el nuevo tópico en Preferences
  preferences.begin("my-app", false);
  preferences.putString("ServerSup", NewTopic);
  preferences.end();
}

// Procesar los mensajes MQTT recibidos
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];  // Convertir el payload a un String
  }
  int separatorIndex = message.indexOf('#');
  String Request = message.substring(0, separatorIndex);  // Extraer la solicitud
  String Value = message.substring(separatorIndex + 1);  // Extraer el valor

  // Procesar la solicitud
  if (Request == "ChangeRele") {
    ChangeStatusRele();
  } else if (Request == "ConsultRele") {
    SendRequest(StatudRele());
  } else if (Request == "ChangeIdModule") {
    ChangeIdModule(Value);
  } else if (Request == "ChangeTopicAnswer") {
    ChangeTopincAnswer(Value);
  } else if (Request == "ChangeSubscription") {
    ChangeSubscription(Value);
  } else if (Request == "InfoVariable") {
    InfoVariable();
  } else if (Request == "ChangeNumModule") {
    resetModule();
  } else if (Request == "Reset") {
    ChangeNumModule(Value);
  } else if (Request == "ChangeBroker") {
    int startIndex = Value.indexOf("Server:") + 7;
    int endIndex = Value.indexOf(";", startIndex);
    String mqtt_server = Value.substring(startIndex, endIndex);

    startIndex = Value.indexOf("User:") + 5;
    endIndex = Value.indexOf(";", startIndex);
    String mqtt_user = Value.substring(startIndex, endIndex);

    startIndex = Value.indexOf("Pass:") + 5;
    endIndex = Value.indexOf(";", startIndex);
    String mqtt_password = Value.substring(startIndex, endIndex);
    
    ChangeBroker(mqtt_server,mqtt_user,mqtt_password);
  }
  
  Serial.println(Request + " - " + Value);  // Imprimir la solicitud y el valor
}

// Cambiar el estado del relé usando el interruptor de la pared
void ChangeInterructor() {
  int currentInterruState = digitalRead(interruPin);  // Leer el estado actual del interruptor
  if (currentInterruState != lastInterruState) {  // Si el estado ha cambiado
    ChangeStatusRele();  // Cambiar el estado del relé
    lastInterruState = currentInterruState;  // Actualizar el estado anterior
    delay(330);  // Pequeño retardo para evitar rebotes
  }
}

// Cambiar el servidor MQTT
void ChangeServer(String server) {
  mqtt_server = server;
}

// Cambiar el identificador del módulo
void ChangeIdModule(String Value) {
  IdModule = Value;
  preferences.begin("my-app", false);
  preferences.putString("IdModule", IdModule);  // Guardar el nuevo IdModule en Preferences
  preferences.end();
}

// Cambiar el número del módulo
void ChangeNumModule(String Value) {
  numModule = Value;
  preferences.begin("my-app", false);
  preferences.putString("numModule", numModule);  // Guardar el nuevo numModule en Preferences
  preferences.end();
}

// Cambiar el tópico de respuesta
void ChangeTopincAnswer(String Value) {
  TopincAnswer = Value;
  preferences.begin("my-app", false);
  preferences.putString("TopincAnswer", TopincAnswer);  // Guardar el nuevo TopincAnswer en Preferences
  preferences.end();
}

// Enviar información de todas las variables
void InfoVariable() {
  String mensaje = 
    "ssid:" + ssid + 
    ";password:" + password + 
    ";mqtt_server:" + mqtt_server + 
    ";mqtt_user:" + mqtt_user + 
    ";mqtt_password:" + mqtt_password +
    ";lastInterruState:" + String(lastInterruState) + 
    ";IdModule:" + IdModule + 
    ";TopincAnswer:" + TopincAnswer + 
    ";numModule:" + numModule + 
    ";ServerSup:" + ServerSup;
  SendRequest(mensaje);  // Enviar el mensaje con la información de las variables
}

void ChangeBroker(String newServer, String newUser, String newPassword) {
  // Guardar los valores actuales
  String oldServer = mqtt_server;
  String oldUser = mqtt_user;
  String oldPassword = mqtt_password;

  // Asignar los nuevos valores
  mqtt_server = newServer;
  mqtt_user = newUser;
  mqtt_password = newPassword;

  // Configurar el servidor MQTT con el nuevo valor
  client.setServer(mqtt_server.c_str(), 1883);

  // Intentar conectar con los nuevos valores hasta 3 veces
  bool connected = false;
  for (int i = 0; i < 3; i++) {
    if (client.connect(IdModule.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
      connected = true;
      break;  // Salir del bucle si la conexión es exitosa
    }
    delay(3000);  // Esperar 3 segundos antes de reintentar
  }

  if (connected) {
    // Si la conexión es exitosa, guardar los nuevos valores en Preferences
    preferences.begin("my-app", false);
    preferences.putString("mqtt_server", newServer);
    preferences.putString("mqtt_user", newUser);
    preferences.putString("mqtt_password", newPassword);
    preferences.end();
    Serial.println("Conexión exitosa con los nuevos valores. Configuración guardada.");
  } else {
    // Si la conexión falla, revertir a los valores anteriores y reconectar
    mqtt_server = oldServer;
    mqtt_user = oldUser;
    mqtt_password = oldPassword;

    // Configurar el servidor MQTT con los valores antiguos
    client.setServer(mqtt_server.c_str(), 1883);

    // Intentar reconectar con los valores anteriores hasta 3 veces
    for (int i = 0; i < 3; i++) {
      if (client.connect(IdModule.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
        Serial.println("Conexión fallida con los nuevos valores. Revertido a la configuración anterior.");
        return;
      }
      delay(3000);  // Esperar 3 segundos antes de reintentar
    }
    Serial.println("Conexión fallida con la configuración anterior. Revisa tu configuración.");
  }
}

//Reinicar el modulo
void resetModule() {
  ESP.restart(); // Reiniciar el módulo
}