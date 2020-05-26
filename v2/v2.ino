// Load Wi-Fi library
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>


// Replace with your network credentials
const char *ssid = "Dom";
const char *password = "przepustka1";

// Set web server port number to 80
AsyncWebServer server(80);

int sliderValue = 0;
int ledred=32;
int ledblue=33;
int ledgreen=27;

int redValue = 0;
int blueValue = 0;
int greenValue = 0;

void setup()
{
  Serial.begin(9600);

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // pinMode(ledred, OUTPUT);
  // pinMode(ledblue, OUTPUT);
  // pinMode(ledgreen, OUTPUT);
  ledcAttachPin(ledred, 0);
  ledcAttachPin(ledblue, 1);
  ledcAttachPin(ledgreen, 2);
  ledcSetup(0, 4000, 8);
  ledcSetup(1, 4000, 8);
  ledcSetup(2, 4000, 8);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("/color")
  }
  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request){
    // request->send(200, "text/html", index_html);
    request->send(SPIFFS, "/index.html", "text/html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/colorData", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("MADE IT TO COLOR DATA ROUTE");
    if(request->hasArg("red") && request->hasArg("blue") && request->hasArg("green")){
      String red = request->arg("red");
      Serial.print("red recieved is ");
      Serial.println(red);
      redValue = request->arg("red").toInt();
      blueValue = request->arg("blue").toInt();
      greenValue = request->arg("green").toInt();
    }
    else {
      Serial.println("No 'colorData' field found");
    }

    request->send(200);
  });

  server.begin();
}

void loop()
{
  ledcWrite(0, redValue); // set the brightness of the LED
  ledcWrite(1, blueValue); // set the brightness of the LED
  ledcWrite(2, greenValue); // set the brightness of the LED

  // delay(30);
}