/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com/esp8266-dht11dht22-temperature-and-humidity-web-server-with-arduino-ide/
*********/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

#define BOTtoken "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "XXXXXXXXXX"

#define DHTPIN 5     // Digital pin connected to the DHT sensor
//#define GREENPIN 4
//#define REDPIN 0
//#define YELLOWPIN 2

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated
int count = 0;

// Updates DHT readings every 10 seconds
const long interval = 10000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <title>Document</title>
    <style>
        html {
            height: 100%;
            padding: 0;
            overflow: hidden;
            font-family: Arial;
            text-align: center;
        }
        body {
            background-image: url("https://raw.githubusercontent.com/KlymentievMaksym/Meteo/main/142982-kosmos-liniya-zvezdnyj_sled-astronomicheskij_obekt-nebo-3840x2160.jpg");
            background-size: cover;
            background-repeat: no-repeat;
            color: lightgray;
            overflow: auto;
            display: flex;
            justify-content: center;
        }
        @media screen and (max-width: 1000px){
            .all-section{
                background-color: white;
                padding: 24px 32px;
                border-radius: 25px;
                margin-top: 52px;
            }
            .welcome-div{
                background-color: black;
                border-radius: 25px;
                display: flex;
                align-items: center;
                padding: 0px 28px;
            }
            .welcome-paragraph{
                font-size: 3rem;
            }
            .welcome-text {
                font-size: 3rem;
            }
            .main-section-dht22-div{
                background-color: black;
                border-radius: 25px;
                padding: 0px 28px;
            }
            .main-section-dht22-paragraph{
                font-size: 1.5rem;
                padding: 4px 0px;
            }
        }
        @media screen and (min-width: 1000px){
            .all-section{
                background-color: white;
                padding: 24px 32px;
                border-radius: 25px;
                margin-top: 52px;
            }
            .welcome-div{
                background-color: black;
                border-radius: 25px;
                display: flex;
                align-items: center;
                padding: 0px 28px;
            }
            .welcome-paragraph{
                font-size: 3rem;
            }
            .welcome-text {
                font-size: 3rem;
            }
            .main-section-dht22-div{
                background-color: black;
                border-radius: 25px;
                padding: 0px 28px;
            }
            .main-section-dht22-paragraph{
                font-size: 1.5rem;
                padding: 4px 0px;
            }
        }
    
      </style>
</head>
<body>
    <section class="all-section">
        <section class="title-section">
            <div class="welcome-div">
                <p class="welcome-paragraph">
                    <i class="fas fa-cloud" style="color:#FA2121;"></i> 
                    <h2 class="welcome-text">Welcome</h2>
                </p>
            </div>
        </section>
        <section class="main-section">
            <div class="main-section-dht22-div">
                <p class="main-section-dht22-paragraph">
                    <i class="fas fa-thermometer-half" style="color:#FA2121;"></i> 
                    <span class="main-section-dht22-div-text">Temperature</span> 
                    <span id="temperature">%TEMPERATURE%</span>
                    <sup class="units">&deg;C</sup>
                </p>
            </div>
            <div class="main-section-dht22-div">
                <p class="main-section-dht22-paragraph">
                    <i class="fas fa-tint" style="color:#FA2121;"></i> 
                    <span class="main-section-dht22-div-text">Humidity</span>
                    <span id="humidity">%HUMIDITY%</span>
                    <sup class="units">%</sup>
                </p>
            </div>
        </section>
    </section>
</body>
<script>
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("temperature").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/temperature", true);
      xhttp.send();
    }, 10000 ) ;
    
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("humidity").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/humidity", true);
      xhttp.send();
    }, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}

int neededHour = 6;
int resetHour = 0;

const unsigned long BOT_MTBS = 1000; // mean time between scan messages

unsigned long bot_lasttime; // last time messages' scan has been done

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void handleNewMessages(int numNewMessages, int neededHour, float t, float h)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  
  String answer;
  for (int i = 0; i < numNewMessages; i++)
  {
    telegramMessage &msg = bot.messages[i];
    Serial.println("Received " + msg.text);
    if (msg.text == "/getdata")
      answer = "Temp: "+ String(t) + "\n" + "Humidity: " + String(h);
    else if (msg.text == "/getneededtime")
      answer = "The time to send for now is: " + String(neededHour);
    else if (msg.text == "/about")
      answer = "All is good here, thanks for asking!";
    else
      answer = "Say what?";

    bot.sendMessage(msg.chat_id, answer, "Markdown");
  }
}


void bot_setup()
{
  const String commands = F("["
                            "{\"command\":\"getdata\",  \"description\":\"Get Temp and Humidity\"},"
                            "{\"command\":\"getneededtime\",  \"description\":\"Get time for getting SMS\"},"
                            "{\"command\":\"about\", \"description\":\"About\"}" // no comma on last command
                            "]");
  bot.setMyCommands(commands);
  //bot.sendMessage("25235518", "Hola amigo!", "Markdown");
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);

  
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org

  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Bot started up", "");

  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(10800);

  bot_setup();

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      
    }
  }
  timeClient.update();
  int currentHour = timeClient.getHours();  
  if (currentHour == neededHour and count == 0 and (t != 0 or h != 0)) {
    bot.sendMessage(CHAT_ID, "Temp: " + String(t) + "\n" + "Humidity: " + String(h), "");
    count = 1;
    Serial.print("Hour: ");
    Serial.println(currentHour);
    Serial.print("Temp: ");
    Serial.println(t);
    Serial.print("Humidity: ");
    Serial.println(h);
  }
  else if (currentHour == resetHour and count == 1) {
    count = 0;
  }

  if (millis() - bot_lasttime > BOT_MTBS)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages, neededHour, t, h);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

}
