// BEGIN Librerias
#include <WiFi.h>          // Libreria para el manejo del modulo ESP8266/ESP32
#include <PubSubClient.h>  // Libreria para suscribirse al servidor MQTT
#include <Preferences.h>   // Libreria para almacenar datos en la memoria no volatil
// END Librerias

// BEGIN Variables
String ssid;                  // Nombre de tu SSID
String password;              // Contraseña de tu WIFI
String mqtt_server;           // Dirección IP del broker MQTT
String mqtt_user;             // Usuario MQTT
String mqtt_password;         // Contraseña MQTT
const int relayPin = 4;       // Pin de control del relé
const int interruPin = 6;     // Pin de control del interruptor
const int ledPin = 3;         // Pin del LED
int lastInterruState;         // Estado anterior del pin del interruptor
String IdModule;              // Identificador del módulo
String TopincAnswer;          // Tópico al que responde
String numModule;             // Identificador del módulo, para el IdRegion
String Firma;                 // Firma para responder
String ServerSup;             // Tópico al que está suscrito
String OldValue;              // Variable auxiliar
String TopicDiacnostic = "Tdiacnostic";        // Tópico al se enviar los resultados para ver cambios
Preferences preferences;      // Variable para gestionar y guardar valores en la memoria no volátil
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
  ssid = preferences.getString("ssid", "Madre rusia");
  password = preferences.getString("password", "El chepa");
  IdModule = preferences.getString("IdModule", "00.00.00");
  TopincAnswer = preferences.getString("TopincAnswer", "Resection");
  ServerSup = preferences.getString("ServerSup", "moduleClient/" + IdModule);
  numModule = preferences.getString("numModule", "01");
  mqtt_server = preferences.getString("mqtt_server", "192.168.194");
  mqtt_user = preferences.getString("mqtt_user", "Nadyword");
  Firma = preferences.getString("Firma", "; [" + IdModule + "/" + numModule + "]");
  mqtt_password = preferences.getString("mqtt_password", "Sa753951.");
  preferences.end();

  setup_wifi();  // Conectarse a la red WiFi
  client.setServer(mqtt_server.c_str(), 1883);  // Configurar el broker MQTT
  client.setCallback(callback);  // Establecer el callback para manejar mensajes MQTT entrantes
  lastInterruState = digitalRead(interruPin);  // Leer el estado inicial del interruptor

  // Crear una tarea para monitorear el interruptor
  xTaskCreatePinnedToCore(
    monitorInterruptorTask,  // Función de la tarea
    "MonitorInterruptor",    // Nombre de la tarea
    10000,                   // Tamaño de la pila de la tarea
    NULL,                    // Parámetro de entrada de la tarea
    1,                       // Prioridad de la tarea
    NULL,                    // Manejo de la tarea
    1                        // Núcleo donde ejecutar la tarea
  );
}
// END void setup

// BEGIN void loop
void loop() {
  // Reconectar al servidor MQTT si la conexión se pierde
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // Mantener la conexión MQTT activa
}
// END void loop

// -----------------------------------------------------METODOS-----------------------------------------------------

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

// Conectarse a la red WiFi
void setup_wifi() {
  delay(15);
  WiFi.begin(ssid.c_str(), password.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(550);
  }
}

// Enviar mensajes a través de MQTT
void SendRequest(String Message) {
  client.publish(TopincAnswer.c_str(), (Message + Firma).c_str());
}

// Enviar mensajes a través de MQTT (Diacnostico)
void SendRequestD(String Message) {
  client.publish(TopicDiacnostic.c_str(), (Message + Firma).c_str());
}

// Procesar los mensajes MQTT recibidos
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];  // Convertir el payload a un String
  }
  String Request = message.substring(0, message.indexOf('/'));  // Extraer la solicitud
  String Value = message.substring(message.indexOf('#') + 1);  // Extraer el valor
  String Modulo = message.substring(message.indexOf('/') + 1, message.indexOf('/') + 3);// Extraer le modulo al que se ejecutar la accion
  ExecuteActions(Request, Value, Modulo);  
}

void ExecuteActions(String Request, String Value, String Modulo){
  if(Modulo == "00" || Modulo == numModule)
  {
    // Procesar la solicitud
    if (Request == "ChangeRele") {
      ChangeStatusRele();
    } else if (Request == "ConsultRele") {
      SendRequest(ConsultRele());
    } else if (Request == "ChangeIdModule") {    
      SendRequestD(ChangeIdModule(Value));
    } else if (Request == "ChangeTopicAnswer") {
      SendRequestD(ChangeTopicAnswer(Value));
    } else if (Request == "ChangeSubscription") {
      SendRequestD(ChangeSubscription(Value));
    } else if (Request == "InfoVariable") {
      SendRequestD(InfoVariable());
    } else if (Request == "ChangeNumModule") {
      SendRequestD(ChangeNumModule(Value));
    } else if (Request == "ResetValues") {
      SendRequestD(ResetValues()); 
    } else if (Request == "ChangeBroker") {    
      SendRequestD(ChangeBroker(Value));  
    } else if (Request == "ChangeWifi") {    
      SendRequestD(ChangeWifi(Value));  
    } else if (Request == "Reset") {
      resetModule();
    }
  }
  
  Serial.print(String(Request + " - " + Value + " - " + Modulo));
}

// Cambiar el estado del relé
void ChangeStatusRele() {
  digitalWrite(relayPin, !digitalRead(relayPin));
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

// Tarea para monitorear el estado del interruptor
void monitorInterruptorTask(void * parameter) {
  for (;;) {  // Bucle infinito
    ChangeInterructor();  // Verificar cambios en el estado del interruptor
    vTaskDelay(100 / portTICK_PERIOD_MS);  // Esperar 100 ms
  }
}

// Consultar el estado del relé
String ConsultRele() {
  return digitalRead(relayPin) ? "ON" : "OFF";
}

// Cambiar el identificador del módulo
String ChangeIdModule(String Value) {
  OldValue = IdModule;
  IdModule = Value;
  
  preferences.begin("my-app", false);
  preferences.putString("IdModule", IdModule);  // Guardar el nuevo IdModule en Preferences
  preferences.putString("Firma","; [" + IdModule + "/" + numModule + "]");
  preferences.end();
  Firma = "; [" + IdModule + "/" + numModule + "]";
  return "Se cambió el valor 'IdModule' de " + OldValue + " a " + IdModule;
}

// Cambiar el tópico de respuesta
String ChangeTopicAnswer(String Value) {
  OldValue = TopincAnswer; 
  TopincAnswer = Value;
  preferences.begin("my-app", false);
  preferences.putString("TopincAnswer", TopincAnswer);  // Guardar el nuevo TopincAnswer en Preferences
  preferences.end();
  return "Se cambió el valor 'TopincAnswer' de " + OldValue + " a " + TopincAnswer;
}

// Refrescar la conexión y cambiar suscripciones
String ChangeSubscription(String NewTopic) {
  OldValue = ServerSup; 
  client.unsubscribe(ServerSup.c_str());  // Desuscribirse del tópico antiguo
  ServerSup = NewTopic;  // Actualizar el tópico suscrito
  client.subscribe(NewTopic.c_str());  // Suscribirse al nuevo tópico
  // Guardar el nuevo tópico en Preferences
  preferences.begin("my-app", false);
  preferences.putString("ServerSup", NewTopic);
  preferences.end();
  return "Se cambió el valor 'ServerSup' de " + OldValue + " a " + ServerSup;
}

// Enviar información de todas las variables
String InfoVariable() {
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
  return mensaje;  // Enviar el mensaje con la información de las variables
}

// Cambiar el número del módulo
String ChangeNumModule(String Value) {
  OldValue = numModule;
  numModule = Value;
  preferences.begin("my-app", false);
  preferences.putString("numModule", numModule);  // Guardar el nuevo numModule en Preferences
  preferences.putString("Firma","; [" + IdModule + "/" + numModule + "]");
  preferences.end();
  Firma = "; [" + IdModule + "/" + numModule + "]";
  return "Se cambió el valor 'numModule' de " + OldValue + " a " + numModule;
}

// Reiniciar valores de fábrica
String ResetValues() {
  preferences.begin("my-app", false);
  preferences.clear(); // Borra todas las claves y valores almacenados
  preferences.end();  
  Firma = preferences.getString("Firma");
  return "Se limpió 'preferences'";
}

// Cambiar el broker MQTT
String ChangeBroker(String Value) {
  int startIndex = Value.indexOf("Server:") + 7;
  int endIndex = Value.indexOf(";", startIndex);
  String newServer = Value.substring(startIndex, endIndex);

  startIndex = Value.indexOf("User:") + 5;
  endIndex = Value.indexOf(";", startIndex);
  String newUser = Value.substring(startIndex, endIndex);

  startIndex = Value.indexOf("Pass:") + 5;
  endIndex = Value.indexOf(";", startIndex);
  String newPassword = Value.substring(startIndex, endIndex);

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
    return "Conexión exitosa con los nuevos valores. Configuración guardada.";
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
        return "Conexión fallida con los nuevos valores. Revertido a la configuración anterior.";
      }
      delay(3000);  // Esperar 3 segundos antes de reintentar
    }
    return "Conexión fallida con la configuración anterior. Revisa tu configuración.";
  }
  resetModule();
}

// Reiniciar el módulo
void resetModule() {
  SendRequestD("En dos segundos se reiniciará el módulo");
  delay(2000);
  ESP.restart(); // Reiniciar el módulo
}

String ChangeWifi(String Value) {
  // Extraer nuevos SSID y Password del valor proporcionado
  int startIndex = Value.indexOf("SSID:") + 5;
  int endIndex = Value.indexOf(";", startIndex);
  String newSSID = Value.substring(startIndex, endIndex);

  startIndex = Value.indexOf("Password:") + 9;
  endIndex = Value.indexOf(";", startIndex);
  String newPassword = Value.substring(startIndex, endIndex);

  // Guardar los valores actuales para revertir si es necesario
  String oldSSID = ssid;
  String oldPassword = password;

  // Desconectar el WiFi actual
  WiFi.disconnect(true);
  delay(1000);

  // Asignar los nuevos valores temporalmente
  ssid = newSSID;
  password = newPassword;

  // Iniciar nueva conexión WiFi
  WiFi.begin(ssid.c_str(), password.c_str());

  // Esperar hasta que se conecte o se agoten los intentos
  int retries = 10;
  while (WiFi.status() != WL_CONNECTED && retries > 0) {
    delay(1000);
    retries--;
    Serial.println("Intentando conectar al nuevo WiFi...");
  }

  // Verificar si la conexión fue exitosa
  if (WiFi.status() == WL_CONNECTED) {
    // Guardar los nuevos valores en Preferences
    preferences.begin("my-app", false);
    preferences.putString("ssid", newSSID);
    preferences.putString("password", newPassword);
    preferences.end();
    Serial.println("Conexión WiFi exitosa con los nuevos valores.");
    return "Conexión WiFi exitosa con los nuevos valores.";
  } else {
    // Si la conexión falla, revertir a los valores anteriores
    ssid = oldSSID;
    password = oldPassword;
    WiFi.disconnect(true);
    WiFi.begin(ssid.c_str(), password.c_str());

    // Esperar hasta que se conecte con los valores anteriores
    retries = 10;
    while (WiFi.status() != WL_CONNECTED && retries > 0) {
      delay(1000);
      retries--;
      Serial.println("Intentando revertir a la configuración anterior...");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Conexión WiFi fallida. Revertido a la configuración anterior.");
      return "Conexión WiFi fallida. Revertido a la configuración anterior.";
    } else {
      Serial.println("Conexión WiFi fallida. Revisa tu configuración.");
      return "Conexión WiFi fallida. Revisa tu configuración.";
    }
  }
}