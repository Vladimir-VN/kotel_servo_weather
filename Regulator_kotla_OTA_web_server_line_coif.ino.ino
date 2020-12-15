// настройки для прошивки по воздуху
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// настройки сервера и датчиков
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <Servo.h> //сервопривод

Servo myservo;  // create servo object to control a servo

// twelve servo objects can be created on most boards
// Data wire is plugged into port D2 on the ESP8266

#define ONE_WIRE_BUS D4

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

float tempSensor1, tempSensor2;//, tempSensor3
int season;

uint8_t sensor1[8] = { 0x28, 0xFF, 0x18, 0x25, 0x2, 0x17, 0x3, 0xD4 };
uint8_t sensor2[8] = { 0x28, 0x90, 0xE3, 0xDF, 0x5B, 0x14, 0x1, 0x93 };
//uint8_t sensor3[8] = { 0x28, 0x61, 0x64, 0x12, 0x3F, 0xFD, 0x80, 0xC6  };

const int relay_season = 5; // контакт, к которому подключено реле D1=5

/*Put your SSID & Password*/
const char* ssid = "ASUS";  // Enter SSID here
const char* password = "Nv334566n";  //Enter Password here


ESP8266WebServer server(80);         

MDNSResponder mdns;

//variabls for blinking an LED with Millis
const int led = D0; // ESP8266 Pin to which onboard LED is connected
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 1000;  // interval at which to blink (milliseconds)
int ledState = LOW;  // ledState used to set the LED

int coif = 90;

void setup() {
pinMode(led, OUTPUT);

    
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin(); // настройки для прошивки по воздуху
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(100);



  pinMode(relay_season, OUTPUT);
  
  sensors.begin();              

  myservo.attach(4);  // attaches the servo on GPIO4 to the servo object

  Serial.println("Connecting to ");
  Serial.println(ssid);

  //connect to your local wi-fi network
  WiFi.begin(ssid, password);

  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
               //  "Запущен MDNSresponder"
  }
  
  server.on("/", handle_OnConnect);
  server.on("/ChangeCoif", [](){
     coif += 10;
     if(coif >150) coif = 80;
    /* sensors.requestTemperatures();
  tempSensor1 = sensors.getTempC(sensor1); // Gets the values of the temperature
  tempSensor2 = sensors.getTempC(sensor2); // Gets the values of the temperature
// tempSensor3 = sensors.getTempC(sensor3); // Gets the values of the temperature
  season = digitalRead(relay_season);
  server.send(200, "text/html", SendHTML(tempSensor1, tempSensor2, season,coif)); //,tempSensor3
               
    //delay(1000);*/
   
  server.sendHeader("Location", "/",true);
  server.send(302, "text/plane",""); 
  });
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  
}

   
 

void loop() {
  
  sensors.requestTemperatures();
  Serial.print("Requesting temperatures...");
  Serial.print("Sensor Temp C: ");
  Serial.print(String(sensors.getTempC(sensor1)) + " C, ");
  
// int coif = 90;//Коэффициент для расчета угла поворота при коэф 90 температура -18 дает угол 0 и +18 угол 180, при коэф 110 температура - 22 угол 0 +14 угол 180
int posserv;
posserv = (int(5 * double((sensors.getTempC(sensor1))) + coif)); //считываем значение с датчика температуры, и преобразуем в линейное значение для угла поворота по формуле У=5Х+коэф
Serial.print("position read:");
Serial.println(posserv);
delay (500);


if (posserv < 2)//если угол сервы меньше 1 делаем 1
{
myservo.write(1);
Serial.print("SET DEGREE = 1:");
Serial.println(myservo.read());
//digitalWrite(relay_season, HIGH);                       //переключение режима зима/лето !!!!! ЗАКОММЕНТИТЬ В БОЕВОМ скетче!!!!!
//Serial.print("Season:");
//Serial.println(digitalRead(relay_season));
}
delay (1000);

if (posserv >180) //если угол сервы больше 177 делаем 177
{
myservo.write(180);
digitalWrite(relay_season, LOW);//переключение режима зима/лето
Serial.print("SET DEGREE = 180:");
Serial.println(myservo.read());
Serial.print("Season:");
Serial.println(digitalRead(relay_season));
}
delay (1000);

if (posserv >= 2 and posserv <=179) //рабочий диапазон сервы 1-177 градуса
{
myservo.write(posserv);
Serial.print("SERVO SET 1-180:");
Serial.println(posserv);
Serial.print("SERVO READ 1-180:");
Serial.println(myservo.read());
digitalWrite(relay_season, HIGH);//переключение режима зима/лето
Serial.print("Season:");
Serial.println(digitalRead(relay_season));
}


Serial.print("SERVO READ END:");
Serial.println(myservo.read());
//Serial.print("Position: (s5)");
delay (1000);

  server.handleClient();

ArduinoOTA.handle(); 
}

void handle_OnConnect() {
  sensors.requestTemperatures();
  tempSensor1 = sensors.getTempC(sensor1); // Gets the values of the temperature
  tempSensor2 = sensors.getTempC(sensor2); // Gets the values of the temperature
// tempSensor3 = sensors.getTempC(sensor3); // Gets the values of the temperature
   season  = digitalRead(relay_season);
   /*switch (season){
    case 1: "SUMMER";
    break;
    case 2: "WINTER";
    break;
   }
   // if (season == 0){
   // return season "summer";}
  */
  server.send(200, "text/html", SendHTML(tempSensor1, tempSensor2, season,coif)); //,tempSensor3
  
} 


void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}
//,float tempSensor3
String SendHTML(float tempSensor1,float tempSensor2, int season,int coif){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP8266 Temperature Monitor</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr +="p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  
ptr +="<script>\n"; //ajax запрос на обновление данных с датчика без перезагрузки web страницы
ptr +="setInterval(loadDoc,1000);\n"; 
ptr +="function loadDoc() {\n";
ptr +="var xhttp = new XMLHttpRequest();\n";
ptr +="xhttp.onreadystatechange = function() {\n";
ptr +="if (this.readyState == 4 && this.status == 200) {\n";
ptr +="document.body.innerHTML =this.responseText}\n";
ptr +="};\n";
ptr +="xhttp.open(\"GET\", \"/\", true);\n";
ptr +="xhttp.send();\n";
ptr +="}\n";
ptr +="</script>\n";
  // конец запроса данных
  
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>ESP8266 Monitor</h1>\n";
  ptr +="<p>OUT DOOR: ";
  ptr +=tempSensor1;
  ptr +="&deg;C</p>";
  ptr +="<p>INLET WATER: ";
  ptr +=tempSensor2;
  ptr +="&deg;C</p>";

  if (season == 0)
  {ptr +="<p>Season: " "SUMMER";}
  else 
  {ptr +="<p>Season: " "WINTER";}
  ptr +="</p>";
  ptr +="</div>\n";
/*

ptr += "<form action="get">";
 ptr += "<input1: <input type="text" name="input1">";
 ptr += "<input type="submit" value="Submit">";
ptr += "</form><br>\n";
*/

/*
ptr += "<div class="seldiv">";
ptr += "<label>Select a value: </label><select id="selectvalue">";
  ptr += "  <option>5</option>\";
  ptr += "   <option>10</option>\";
  ptr += "  <option>15</option>\";
  ptr += "  <option>20</option>\";
  ptr += "  <option>25</option>\";
  ptr += "</select> \";
ptr += "</div>\n";
 */ 
 
  ptr +="<p>Coif: ";
  ptr +=coif;
  ptr +="</p>";
  ptr +="<p>Coif <a href=\"ChangeCoif\"><button>Change</button></a><p>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
  }
