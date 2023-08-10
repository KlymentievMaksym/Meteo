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
#include <Wire.h>
//#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

#define BOTtoken "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "XXXXXXXXXX"

#define DHTPIN 14     // Digital pin connected to the DHT sensor
#define SEALEVELPRESSURE_HPA (1013.25)
//#define GREENPIN 4
//#define REDPIN 0
//#define YELLOWPIN 2

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT11     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

DHT dht(DHTPIN, DHTTYPE);


Adafruit_BME280 bme;

unsigned long delayTime;

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;
float p = 0.0;
float a = 0.0;
float t_dht = 0.0;
float h_dht = 0.0;

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
    <title>Dadmons_weather</title>
    <style>
        body {
            font-family: Arial;
            text-align: center;
            display: flex;
            flex-direction: column;
            box-sizing: border-box;
            margin: 0;
            padding: 0;
             /*padding: 0 52px;*/
            height: 100vh;
            width: 100%;
            background-image: url("https://raw.githubusercontent.com/KlymentievMaksym/Meteo/main/142982-kosmos-liniya-zvezdnyj_sled-astronomicheskij_obekt-nebo-3840x2160.jpg");
            background-size: cover;
            background-repeat: no-repeat;
            color: lightgray;
        }
        ::-webkit-scrollbar {
            width: 15px;
            height: 100%;
        }

        ::-webkit-scrollbar-track {
            background-color: #10B981;
        }

        ::-webkit-scrollbar-thumb {
            background-color: #1C1C1C;
            border-radius: 14px;
            border: 3px solid #10B981 ;
        }

        ::selection {
            background-color: #1C1C1C;
        }

        header{
            display: flex;
            justify-content: flex-end;
            
        }

        .mode{
            width: 40px;
            height: 40px;
            margin: 16px;
            background-color: white;
            font-size:1.5rem;
            border-radius: 8px;
            text-align: center;
        }

        .mode > p {
            margin: 0px;
            padding: 6px;
        }

        .mode:hover{
            box-shadow: 0 4px 50px rgba(109, 70, 133, 0.7);
        }


        .dark-mode {
            background-color: rgb(4, 3, 56);
        }

        .all-section-dark-mode{
            background-color: rgb(69, 69, 69);
        }

        .all-section{
            display: flex;
            justify-content: center;
            flex-direction: column;
            height:fit-content;
            background-color: white;
            padding: 24px 32px;
            border-radius: 25px;
            margin: auto;
        }
        .all-section:hover {
            box-shadow: 0 4px 50px rgba(109, 70, 133, 0.7);
        }
        .welcome-div{
            background-color: black;
            border-radius: 25px;
            display: flex;
            align-items: center;
            padding: 0px 50px;
            margin: 3rem 0;
        }
        .welcome-paragraph{
            font-size: 3rem;
        }
        .welcome-text {
            font-size: 3rem;
        }

        .main-section{
            margin: 2rem 0px 0px 0px;
        }
        .main-section-dht22-div{
            background-color: black;
            border-radius: 14px;
            padding: 0px 28px;
        }
        .main-section-dht22-paragraph{
            font-size: 1.3rem;
            padding: 14px 0px;
        }
        @media screen and (max-width: 500px){

        .all-section {
            padding: 12px 16px;
        }

        .welcome-div {
            padding: 0px 32px;
            margin: 1rem 0;
        }


        }
    
      </style>
</head>
<body>
    <header>
        <div class="mode" id="mode">
            <p>
                <i class="fas fa-solid fa-moon fa-spin" style="color:#faef21;"></i> 
            </p>
        </div>

    </header>
    <section class="all-section" id="all-section">
        <section class="title-section">
            <div class="welcome-div" id="welcome-div">
                <p class="welcome-paragraph">
                    <i class="fas fa-cloud" style="color:#FA2121;"></i> 
                    <h2 class="welcome-text"> Welcome</h2>
                </p>
            </div>
        </section>
        <section class="main-section">
            <div class="main-section-dht22-div" id="main-sec-one">
                <p class="main-section-dht22-paragraph">
                    <i class="fas fa-thermometer-half" style="color:#FA2121;"></i> 
                    <span class="main-section-dht22-div-text">Temperature</span> 
                    <span id="temperature">%TEMPERATURE%</span>
                    <sup class="units">&deg;C</sup>
                </p>
            </div>
            <div class="main-section-dht22-div" id="main-sec-two">
                <p class="main-section-dht22-paragraph">
                    <i class="fas fa-tint" style="color:#FA2121;"></i> 
                    <span class="main-section-dht22-div-text">Humidity</span>
                    <span id="humidity">%HUMIDITY%</span>
                    <sup class="units">%</sup>
                </p>
            </div>
            <div class="main-section-dht22-div" id="main-sec-three">
                <p class="main-section-dht22-paragraph">
                    <i class="fas fa-compress" style="color:#FA2121;"></i>
                    <span class="main-section-dht22-div-text">Pressure</span> 
                    <span id="pressure">%PRESSURE%</span>
                    <sup class="units">hPa</sup>
                </p>
            </div>
            <div class="main-section-dht22-div" id="main-sec-four">
                <p class="main-section-dht22-paragraph">
                    <i class="fas fa-text-height" style="color:#FA2121;"></i> 
                    <span class="main-section-dht22-div-text">Altitude</span> 
                    <span id="altitude">%ALTITUDE%</span>
                    <sup class="units">m</sup>
                </p>
            </div>
        </section>
    </section>
</body>
<script>
    let button = document.getElementById("mode");
    let mainSection = document.getElementById("all-section")
    let mainDiv = document.getElementById("welcome-div")
    let mainSecOne = document.getElementById("main-sec-one")
    let mainSecTwo = document.getElementById("main-sec-two")
    let mainSecThree = document.getElementById("main-sec-three")
    let mainSecFour = document.getElementById("main-sec-four")
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

    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("pressure").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/pressure", true);
      xhttp.send();
    }, 10000 ) ;

    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          document.getElementById("altitude").innerHTML = this.responseText;
        }
      };
      xhttp.open("GET", "/altitude", true);
      xhttp.send();
    }, 10000 ) ;

    button.addEventListener('click', function toggleFunction() {
    button.classList.toggle("dark-mode");
        
        if (mainSection.style.backgroundColor === 'black') {
            mainSection.style.backgroundColor = 'white';
            mainDiv.style.backgroundColor = 'black'
            mainSecOne.style.backgroundColor = 'black'
            mainSecTwo.style.backgroundColor = 'black'
            mainSecThree.style.backgroundColor = 'black'
            mainSecFour.style.backgroundColor = 'black'
        } else {
            mainSection.style.backgroundColor = 'black';
            mainDiv.style.backgroundColor = '	#101010'
            mainSecOne.style.backgroundColor = '#101010'
            mainSecTwo.style.backgroundColor = '#101010'
            mainSecThree.style.backgroundColor = '#101010'
            mainSecFour.style.backgroundColor = '#101010'

        }
    });

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
  else if(var == "PRESSURE"){
    return String(p);
  }
  else if(var == "ALTITUDE"){
    return String(a);
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

int findElementIndexInArray(String list[5], String id){
  int j;
  int k = -1;
  for (j = 0; j < 11  && k == -1; j++){
    if (list[j] == id){
      k = j;
      return k;
    }
  }
  return -1;
}
int findFreeSpaceInArray(String list[5]){
  int j;
  int k = -1;
  for (j = 0; j < 11  && k == -1; j++){
    if (list[j].indexOf("") == -1){
      k = j;
      return k;
    }
  }
  return -1;
}

void changeGlobalValue(int i) {
    neededHour = i; // Change the value of the global variable
  }
void changeGlobalList(String oldList[5], String local, String id, String newList[5]) {
    int index1;
    int index2;
    index1 = findElementIndexInArray(oldList, id); // Change the value of the global variable
    index2 = findFreeSpaceInArray(newList);
    if (index2 != -1){
      oldList[index1] = "";
      newList[index2] = id;
    }
    else
      bot.sendMessage(id, "List is Full! 5/5 are taken!", "");
  }


int findElementInArray(String list[5], String id){
  int j;
  int k = -1;
  for (j = 0; j < 11  && k == -1; j++){
    if (list[j] == id){
      k = j;
      return true;
    }
  }
  return false;
}

String englishList[5];

void handleNewMessages(int numNewMessages, int neededHour, float t, float h, float p, float a, float t_dht, float h_dht, String englishList[5], String ArrayOfNamesForSend[5], String ArrayOfNamesForLocalizationU[5], String ArrayOfNamesForLocalizationR[5])
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  
  String locals[3] = {"Ukrainian", "Russian", "English"} ;

  String answer;
  for (int i = 0; i < numNewMessages; i++)
  {
    telegramMessage &msg = bot.messages[i];
    Serial.println("Received " + msg.text);
    if (findElementInArray(ArrayOfNamesForLocalizationU, msg.chat_id)){
      if (msg.text == "/inside")
        answer = "Температура: "+ String(t) + "°C\n" + "Вологість: " + String(h) + "%\n" + "Тиск: "+ String(p) + "гПа\n" + "Висота над рівнем моря: " + String(a) + "м";
      else if (msg.text == "/data"){
        answer = "Дім\n\nТемпература: "+ String(t) + "°C\n" + "Вологість: " + String(h) + "%\n" + "Тиск: "+ String(p) + "гПа\n" + "Висота над рівнем моря: " + String(a) + "м" + "\n\nВулиця\n\nТемпература: " + String(t_dht) + "°C\n" + "Вологість: " + String(h_dht) + "%";
        //Serial.println(answer);
        }
      else if (msg.text.indexOf("/setneededtime") != -1){
        if (msg.text == "/setneededtime") {
          Serial.println(neededHour);
          answer = String(neededHour) + " зараз використовується";
        }
        else {
          Serial.println(msg.text);
          for (int i = 0; i <= 23; i++) {
            //Serial.println(msg.text + " == " + "/setneededtime " + String(i) + ", " + String(msg.text == ("/setneededtime " + String(i))));
            if (msg.text == ("/setneededtime " + String(i))){
              //Serial.println(i);
              answer = String(i) + " було виставлено";
              changeGlobalValue(i);
            }
          }
        //answer = "The time to send for now is: " + String(neededHour);
        }
      }
      else if (msg.text == "/outside")
        answer = "Температура: "+ String(t_dht) + "°C\n" + "Вологість: " + String(h_dht) + "%";
      else if (msg.text.indexOf("/change") != -1){
        if (msg.text == "/change") {
          answer = "Зараз використовується Українська";
        }
        else {
          Serial.println(msg.text);
          for (int i = 0; i <= 2; i++) {
            //Serial.println(msg.text + " == " + "/setneededtime " + String(i) + ", " + String(msg.text == ("/setneededtime " + String(i))));
            if (msg.text == ("/change " + String(i))){
              //Serial.println(i);
              answer = locals[i] + " було встановлено";
              //Serial.println();
              if (i == 1)
                changeGlobalList(ArrayOfNamesForLocalizationU, locals[i], msg.chat_id, ArrayOfNamesForLocalizationR);
              else if (i == 2)
                changeGlobalList(ArrayOfNamesForLocalizationU, locals[i], msg.chat_id, englishList);
              //Serial.println();
            }
          }
        //answer = "The time to send for now is: " + String(neededHour);
        }
      }
      else if (msg.text == "/about")
        answer = "Привіт, мене звати Метео. Я бот який каже тобі температуру та вологість як у домі, так і на вулиці, а також тиск та висоту над рівнем моря лише вдома.";
      else if (msg.text == "/start")
        answer = "Привіт, мене звати Метео. Я бот який каже тобі температуру та вологість як у домі, так і на вулиці, а також тиск та висоту над рівнем моря лише вдома.";
      else
        answer = "Що ти сказав? Я не зрозумів :(";
    }
    else if (findElementInArray(ArrayOfNamesForLocalizationR, msg.chat_id)){
      if (msg.text == "/inside")
        answer = "Температура: "+ String(t) + "°C\n" + "Влажность: " + String(h) + "%\n" + "Давление: "+ String(p) + "гПа\n" + "Высота над уровнем моря: " + String(a) + "м";
      else if (msg.text == "/data"){
        answer = "Дом\n\nТемпература: "+ String(t) + "°C\n" + "Влажность: " + String(h) + "%\n" + "Давление: "+ String(p) + "гПа\n" + "Высота над уровнем моря: " + String(a) + "м" + "\n\nУлица\n\nТемпература: " + String(t_dht) + "°C\n" + "Влажность: " + String(h_dht) + "%";
        //Serial.println(answer);
        }
      else if (msg.text.indexOf("/setneededtime") != -1){
        if (msg.text == "/setneededtime") {
          Serial.println(neededHour);
          answer = String(neededHour) + " сейчас используеться";
        }
        else {
          Serial.println(msg.text);
          for (int i = 0; i <= 23; i++) {
            //Serial.println(msg.text + " == " + "/setneededtime " + String(i) + ", " + String(msg.text == ("/setneededtime " + String(i))));
            if (msg.text == ("/setneededtime " + String(i))){
              //Serial.println(i);
              answer = String(i) + " было поставлено";
              changeGlobalValue(i);
            }
          }
        //answer = "The time to send for now is: " + String(neededHour);
        }
      }
      else if (msg.text == "/outside")
        answer = "Температура: "+ String(t_dht) + "°C\n" + "Влажность: " + String(h_dht) + "%";
      else if (msg.text.indexOf("/change") != -1){
        if (msg.text == "/change") {
          answer = "Русский сейчас используеться";
        }
        else {
          Serial.println(msg.text);
          for (int i = 0; i <= 2; i++) {
            //Serial.println(msg.text + " == " + "/setneededtime " + String(i) + ", " + String(msg.text == ("/setneededtime " + String(i))));
            if (msg.text == ("/change " + String(i))){
              //Serial.println(i);
              answer = locals[i] + " был поставлен";
              if (i == 0)
                changeGlobalList(ArrayOfNamesForLocalizationR, locals[i], msg.chat_id, ArrayOfNamesForLocalizationU);
              else if (i == 2)
                changeGlobalList(ArrayOfNamesForLocalizationR, locals[i], msg.chat_id, englishList);
            }
          }
        //answer = "The time to send for now is: " + String(neededHour);
        }
      }
      else if (msg.text == "/about")
        answer = "Привет, я - Метео. Я бот который говорит тебе температуру, влажность, давление и высоту над уровнем моря дома и только температуру и влажность на улице";
      else if (msg.text == "/start")
        answer = "Привет, я - Метео. Я бот который говорит тебе температуру, влажность, давление и высоту над уровнем моря дома и только температуру и влажность на улице";
      else
        answer = "Что ты сказал? Я не смог понять :(";
    }
    else{
      if (msg.text == "/inside")
        answer = "Temp: "+ String(t) + "°C\n" + "Humidity: " + String(h) + "%\n" + "Pressure: "+ String(p) + "hPa\n" + "Altitude: " + String(a) + "m";
      else if (msg.text == "/data"){
        answer = "House\n\nTemp: " + String(t) + "°C\n" + "Humidity: " + String(h) + "%\n" + "Pressure: " + String(p) + "hPa\n" + "Approx. Altitude: " + String(a) + "m" + "\n\nStreet\n\nTemp: " + String(t_dht) + "°C\n" + "Humidity: " + String(h_dht) + "%";
        //Serial.println(answer);
        }
      else if (msg.text.indexOf("/setneededtime") != -1){
        if (msg.text == "/setneededtime") {
          Serial.println(neededHour);
          answer = String(neededHour) + " is used now";
        }
        else {
          Serial.println(msg.text);
          for (int i = 0; i <= 23; i++) {
            //Serial.println(msg.text + " == " + "/setneededtime " + String(i) + ", " + String(msg.text == ("/setneededtime " + String(i))));
            if (msg.text == ("/setneededtime " + String(i))){
              //Serial.println(i);
              answer = String(i) + " was setted";
              changeGlobalValue(i);
            }
          }
        //answer = "The time to send for now is: " + String(neededHour);
        }
      }
      else if (msg.text == "/outside")
        answer = "Temp: "+ String(t_dht) + "°C\n" + "Humidity: " + String(h_dht) + "%";
      else if (msg.text.indexOf("/change") != -1){
        if (msg.text == "/change") {
          answer = "English is used right now";
        }
        else {
          Serial.println(msg.text);
          for (int i = 0; i <= 2; i++) {
            //Serial.println(msg.text + " == " + "/setneededtime " + String(i) + ", " + String(msg.text == ("/setneededtime " + String(i))));
            if (msg.text == ("/change " + String(i))){
              //Serial.println(i);
              answer = locals[i] + " was setted";
              if (i == 0)
                changeGlobalList(englishList, locals[i], msg.chat_id, ArrayOfNamesForLocalizationU);
              else if (i == 1)
                changeGlobalList(englishList, locals[i], msg.chat_id, ArrayOfNamesForLocalizationR);
            }
          }
        //answer = "The time to send for now is: " + String(neededHour);
        }
      }
      else if (msg.text == "/about")
        answer = "Konnichiwa, my name is Meteo. I am a bot that tells you temperature and humidity";
      else if (msg.text == "/start")
        answer = "Konnichiwa, my name is Meteo. I am a bot that tells you temperature and humidity";
      else
        answer = "What have you said? I don't understand :(";
    }
    Serial.println(msg.chat_id + "was who asked me about these");
    Serial.println(answer);
    bot.sendMessage(msg.chat_id, answer, "Markdown");
    
  }
}

int i;
String ArrayOfNamesForSend[5];
String ArrayOfNamesForLocalizationR[5];
String ArrayOfNamesForLocalizationU[5];


void bot_setup()
{
  const String commands = F("["
                            "{\"command\":\"data\",  \"description\":\"Get All Data\"},"
                            "{\"command\":\"inside\",  \"description\":\"Get Temp, Pressure, Altitude and Humidity from inside\"},"
                            "{\"command\":\"outside\",  \"description\":\"Get Temp and Humidity from outside\"},"
                            "{\"command\":\"setneededtime\",  \"description\":\"Get or set time for getting SMS\"},"
                            "{\"command\":\"change\",  \"description\":\"Change localization to: 0 - Ukrainian, 1 - Russian, 2 - English, leave empty - discover on what language you are now\"},"
                            "{\"command\":\"about\", \"description\":\"About\"}" // no comma on last command
                            "]");
  bot.setMyCommands(commands);
  for (i = 0; i < 5; i++){
    //bot.sendMessage(String(ArrayOfNamesForSend[i]), "Bot started up", "");
    if (findElementInArray(ArrayOfNamesForLocalizationU, ArrayOfNamesForSend[i])){//ArrayOfNamesForLocalizationU.indexOf(String(ArrayOfNamesForSend[i])) != -1){
      bot.sendMessage(ArrayOfNamesForSend[i], "Бот запустився", "");
    }
    else if (findElementInArray(ArrayOfNamesForLocalizationR, ArrayOfNamesForSend[i])){
      bot.sendMessage(ArrayOfNamesForSend[i], "Бот запущен", "");
    }
    else{
      bot.sendMessage(ArrayOfNamesForSend[i], "Bot started up", "");
    }
  }
  
  //bot.sendMessage("25235518", "Hola amigo!", "Markdown");
}

/*void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  // Convert temperature to Fahrenheit
  /*Serial.print("Temperature = ");
  Serial.print(1.8 * bme.readTemperature() + 32);
  Serial.println(" *F");
  
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}*/



void setup(){
  //ArrayOfNamesForSend[0] = "XXXXXXXXXX";
  //ArrayOfNamesForLocalizationU[0]= "XXXXXXXXXX";
  //ArrayOfNamesForLocalizationU[1]= "XXXXXXXXXX";
  
  // Serial port for debugging purposes
  Serial.begin(9600);

  Serial.println(F("BME280 test"));

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");
  delayTime = 15000;

  Serial.println();
  
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
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(p).c_str());
  });
  server.on("/altitude", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(a).c_str());
  });
  server.on("/temperaturedht", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t_dht).c_str());
  });
  server.on("/humiditydht", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h_dht).c_str());
  });

  // Start server
  server.begin();
  
}
 
void loop(){  
  unsigned long currentMillis = millis();
  //printValues();
  //delay(delayTime);
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = bme.readTemperature();
    float newTdht = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from sensor!");
    }
    else {
      t = newT;
      if (isnan(newTdht)) {
        Serial.println("Failed to read from sensor!");
      }
      else {
        t_dht = newTdht;
      }
    }
    // Read Humidity
    float newH = bme.readHumidity();
    float newHdht = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH; 
      if (isnan(newHdht)) {
        Serial.println("Failed to read from sensor!");
      }
      else {
        h_dht = newHdht;
      }
    }
    float newP = bme.readPressure() / 100.0F;
    // if humidity read failed, don't change h value 
    if (isnan(newP)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      p = newP; 
    }
    float newA = bme.readAltitude(SEALEVELPRESSURE_HPA);
    // if humidity read failed, don't change h value 
    if (isnan(newA)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      a = newA; 
    }
  }
  timeClient.update();
  int currentHour = timeClient.getHours();  
  if (currentHour == neededHour and count == 0 and (t != 0 or h != 0)) {
    for (i = 0; i < 5; i++){
      if (findElementInArray(ArrayOfNamesForLocalizationU, String(ArrayOfNamesForSend[i]))){
        bot.sendMessage(ArrayOfNamesForSend[i], "Дім\n______________________\nТемпература: "+ String(t) + "°C\n" + "Вологість: " + String(h) + "%\n" + "Тиск: "+ String(p) + "гПа\n" + "Висота над рівнем моря: " + String(a) + "м" + "\n\nВулиця\n_______________\nТемпература: " + String(t_dht) + "°C\n" + "Вологість: " + String(h_dht) + "%", "");
      }
      else if (findElementInArray(ArrayOfNamesForLocalizationR, String(ArrayOfNamesForSend[i]))){
        bot.sendMessage(ArrayOfNamesForSend[i], "Дом\n______________________\nТемпература: "+ String(t) + "°C\n" + "Влажность: " + String(h) + "%\n" + "Давление: "+ String(p) + "гПа\n" + "Высота над уровнем моря: " + String(a) + "м" + "\n\nУлица\n_______________\nТемпература: " + String(t_dht) + "°C\n" + "Влажность: " + String(h_dht) + "%", "");
      }
      else{
        bot.sendMessage(ArrayOfNamesForSend[i], "House\n______________________\nTemp: " + String(t) + "°C\n" + "Humidity: " + String(h) + "%\n" + "Pressure: " + String(p) + "hPa\n" + "Approx. Altitude: " + String(a) + "m" + "\n\nStreet\n_______________\nTemp: " + String(t_dht) + "°C\n" + "Humidity: " + String(h_dht) + "%", "");
      }
    }
    count = 1;
    //Serial.print("Hour: ");
    //Serial.println(currentHour);
    //Serial.print("Temp: ");
    //Serial.println(t);
    //Serial.print("Humidity: ");
    //Serial.println(h);
    //Serial.print("Pressure: ");
    //Serial.println(p);
    //Serial.print("Approx. Altitude: ");
    //Serial.println(a);
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
      handleNewMessages(numNewMessages, neededHour, t, h, p, a, t_dht, h_dht, englishList, ArrayOfNamesForSend, ArrayOfNamesForLocalizationU, ArrayOfNamesForLocalizationR);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

}
