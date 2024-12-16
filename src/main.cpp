
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <AutoOTA.h>


const char* ssid = "srvrn";
const char* password =  "2155791975";

//=================================================ТОПИКИ
const char* Tgvs = "MyDev/10a8c3a2/ID/set/gvs";                       //Топик - счетчик гор воды
const char* Tmg =  "MyDev/10a8c3a2/#" ;   //870690bb/set/mg";         //топик - свет в туалете //ID клиента ноутбук
const char* Tsupdata = "MyDev/10a8c3a2/17d35acf/set/supd";            // поиск обновлений ID клиент- мой телефон
const char* Tvers = "MyDev/10a8c3a2/ID/set/vers";                     //сюда шлем версию прошивы


//   MQTT  ============================================

const char* mqtt_server = "m4.wqtt.ru";
const int mqtt_port = 9478;
const char* mqtt_user = "u_5A3C2X";
const char* mqtt_password = "HilZPRjD";

WiFiClient espClient;
PubSubClient client(espClient);

#define led 2
String ver, notes;

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(12)
char msg[MSG_BUFFER_SIZE];

int value = 0;
int count;


AutoOTA ota("0.6", "Srvrn1/LipaSensorTualet");


void ota_chek(){
  if (ota.checkUpdate(&ver, &notes)) {
    Serial.print("пришло обновление: ");
    Serial.println(ver);
    Serial.println(notes);

    client.publish(Tgvs, "Reset");
    ota.updateNow();
  }
  else Serial.println("нет обновы...");
}

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//==============================================================================================
void callback(char* topic, byte* payload, int length) {  //обрабатываем входящие топики

  if(String(topic) == String(Tsupdata)){                             //топик обновы с моего ID то идем на GitHub искать обнову
    Serial.println("смотрим обнову");
    ota_chek();                     
  };
 
  
//============================
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

//=================================
  uint8_t bkv = strlen(topic);
  if(topic[bkv-2] == 'm' && topic[bkv-1] == 'g'){           //если топик /mg не важно с какого ID
    Serial.println("Работает!!!");

    if ((char)payload[0] == '1') {
     digitalWrite(led, LOW); 
    } 
    else {
      digitalWrite(led, HIGH); 
    }
  }
  
}

void reconnect() {
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password )) {
 
      Serial.println("connected"); 

      client.publish(Tvers, ota.version().c_str());    //функция ".c_str()"" преобразует из STRING d const char

      client.subscribe(Tmg);                                   //подписка на топики
      //client.subscribe(Tsupdata);                              
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
}

void setup() {
  pinMode(led, OUTPUT);    
  digitalWrite(led, HIGH);

  Serial.begin(74880);
  Serial.println();
  Serial.println("gogo");
  Serial.print("Version ");
  Serial.println(ota.version());

  setup_wifi();
 
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //client.publish(Tvers, ota.version().c_str());    //функция ".c_str()"" преобразует из STRING d const char
  //client.publish(Tvers, "0.2");

  //String str = "hello";                    пример
  //const char *str2 = str.c_str();          пример

  
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis()- lastMsg > 4000) {
    lastMsg = millis();
    ++value;
    count++;
    snprintf (msg, MSG_BUFFER_SIZE,  "%d",count );
    Serial.print("Pub mess: ");
    Serial.println(msg);
    client.publish(Tgvs, msg);
    
  }
}