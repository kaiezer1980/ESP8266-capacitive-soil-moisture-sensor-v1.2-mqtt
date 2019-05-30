
 #include <ESP8266WiFi.h>
 #include <PubSubClient.h>
 using namespace std;

 // WiFi credentials.
//const char* WIFI_SSID = "VOTRE NOM WIFI";
//const char* WIFI_PASS = "VOTRE MOT DE PASSE WIFI";
//const char* device_id = "1940";

const char* WIFI_SSID = "D-Link";
const char* WIFI_PASS = "yahia19400000@@@@";
const char* device_id = "1940";

// MQTT credentials.
const char* mqtt_server = "192.168.1.60";
const int mqtt_port = 1883;
const char* mqtt_user = "VOTRE NOM D'UTILISATEUR MQTT";
const char* mqtt_password = "VOTRE MOT DE PASSE MQTT";

#define in_topic "1940/statusIn"
#define out_topic "1940/statusOut"
#define waterLevel "1940/waterLevel"
#define mqtt_percentage "1940/mqtt_percentage"

// définit l'int pour l'impression numérique
// n'est plus utilisé car analogique est utilisé
int digitalPin;
const int AirValue = 735;   // vous devez remplacer cette valeur par Value_1
const int WaterValue = 250;  // vous devez remplacer cette valeur par Value_2
int intervals = (AirValue - WaterValue)/3;
int soilMoistureValue = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// code à exécuter lorsque l'abonné mqtt reçoit quelque chose
void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrivé dans le sujet: ");
  Serial.print(topic);
  Serial.println();
  Serial.print("Message: ");
  for(int i=0; i<length; i++){
    Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("---------------------");
  }

// se connecter au wifi et mqtt
void connect() {

  // Connexion au Wifi.
  Serial.println();
  Serial.println();
  Serial.println("Connexion à ");
  Serial.println(WIFI_SSID);

   // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connecté");
  Serial.println("Adresse IP: ");
  Serial.print(WiFi.localIP());

  // Se connecter à MQTT.
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println();
  Serial.println("Connexion à CloudMQTT ...");

  while(!client.connected()) {
    delay(500);

    if(client.connect(device_id, mqtt_user, mqtt_password)){
      Serial.println("Connecté à CloudMQTT");
      Serial.println();
      Serial.println("Lecture en cours du statut de waterbowl.");
      }
      else{
        Serial.println("a échoué avec l'état ");
        Serial.println(client.state());
        }

      Serial.print(".");
  }

}



void setup() {
  Serial.begin(115200);
  pinMode(digitalPin, INPUT);
  Serial.println("capteur d'humidité du sol");
  Serial.println("--------------------------------------");
  delay(2000);

  connect();

}

void loop() {

  //do I need to reconnect?
  bool toReconnect = false;

  //If I lose connection reconnect
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Déconnecté du WiFi");
    toReconnect = true;
  }

  if(!client.connected()) {
    Serial.println("Déconnecté de CloutMQTT");
    toReconnect = true;
  }

  if(toReconnect) {
    connect();
  }

  String stat = "";
  int level = analogRead(A0);

   soilMoistureValue = analogRead(A0);  // met le capteur inséré dans le sol
if(soilMoistureValue > WaterValue && soilMoistureValue < (WaterValue + intervals))
{
  stat =("Très humide");
}
else if(soilMoistureValue > (WaterValue + intervals) && soilMoistureValue < (AirValue - intervals))
{
  stat =("humide");
}
else if(soilMoistureValue < AirValue && soilMoistureValue > (AirValue - intervals))
{
  stat =("Sèche");

    }
  
  int percentage = map(level, AirValue, WaterValue, 0, 100);

   Serial.print("capteur Status: ");
   Serial.print(stat);
   Serial.print(" | Percentage: ");
   Serial.print(percentage);
   Serial.println("%");
   Serial.println();
   Serial.print("capteur Level: ");
   Serial.print(level);
   
  delay(1000);

  client.loop();

  string lev = convertInt(level);
  string perc = convertInt(percentage);

  //publish the readings
  client.publish(out_topic, stat.c_str());
  client.publish(waterLevel, lev.c_str());
  client.publish(mqtt_percentage, perc.c_str());
  delay(1000);
  client.subscribe(out_topic);
  client.subscribe(waterLevel);
  client.subscribe(mqtt_percentage);
  delay(1000);
}

// convertit un entier en quelque chose de plus lisible, interprétable
string convertInt(int number)
{
    if (number == 0)
        return "0";
    string temp="";
    string returnvalue="";
    while (number>0)
    {
        temp+=number%10+48;
        number/=10;
    }
    for (int i=0;i<temp.length();i++)
        returnvalue+=temp[temp.length()-i-1];
    return returnvalue;
}
