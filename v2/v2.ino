// Load Wi-Fi library
#include <WiFi.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include "arduinoFFT.h"

#define SAMPLES 256
#define SAMPLING_FREQUENCY 20000


// Replace with your network credentials
const char *ssid = "Dom";
const char *password = "przepustka1";

//webEndpoint constants
const String colorPage = "color";
const String musicPage = "music";
const String temperaturePage = "temperature";

const int MAXBRIGHTNESS = 255;

const int redChannel = 0;
const int blueChannel = 1;
const int greenChannel = 2;
int bassChannel = redChannel;
int midChannel = blueChannel;
int highChannel = greenChannel;

// Set web server port number to 80
AsyncWebServer server(80);

//PIN DEFINITIONS
const int ledred=32;
const int ledblue=33;
const int ledgreen=27;
const int resetPin=22;
const int strobePin=21;
const int analogPin=36;

int sliderValue = 0;
int spectrumValue[7];
int filter=80;
int redValue = 0;
int blueValue = 0;
int greenValue = 0;
bool bassNone = false;
bool midNone = false;
bool highNone = false;
String colors[3];

double midWeight = 50;
double highWeight = 50;
double bassWeight = 255;

String mode = musicPage;

arduinoFFT FFT = arduinoFFT();
unsigned int sampling_period_us;
unsigned long microseconds;
double vReal[SAMPLES];
double vImag[SAMPLES];

String processor(const String& var) {
  if (var == "bassSliderValue") {
    int temp = (int)(bassWeight / 255 * 100);
    Serial.println(bassWeight);
    Serial.println("about to send this as template");
    Serial.println(temp);
    return String(temp);
  }
  return String();
}

void setup()
{
  Serial.begin(9600);

  sampling_period_us = round(1000000*(1.0/SAMPLING_FREQUENCY));
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  ledcAttachPin(ledred, redChannel);
  ledcAttachPin(ledblue, blueChannel);
  ledcAttachPin(ledgreen, greenChannel);
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
    request->redirect("/" + mode);
  });
  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/color.html", "text/html");
    mode = colorPage;
  });
  server.on("/music", HTTP_GET, [](AsyncWebServerRequest *request){
    // request->send(200, "text/html", index_html);
    request->send(SPIFFS, "/music.html", String(), false, processor);
    mode = musicPage;
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    // request->send(200, "text/html", index_html);
    request->send(SPIFFS, "/temperature.html", "text/html");
    mode = temperaturePage;
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/colorData", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("MADE IT TO COLOR DATA ROUTE");
    if(mode == colorPage && request->hasArg("red") && request->hasArg("blue") && request->hasArg("green")){
      String red = request->arg("red");
      Serial.print("red recieved is ");
      Serial.println(red);
      redValue = request->arg("red").toInt();
      blueValue = request->arg("blue").toInt();
      greenValue = request->arg("green").toInt();
      request->send(200);
    }
    else if (mode != colorPage){
      request->send(409);
    }
    else {
      Serial.println("Proper data fields not found for colorData POST request");
      request->send(400);
    }
  });
  server.on("/musicData", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("MADE IT TO MUSIC DATA ROUTE");
    if (!request->hasArg("bassColor")) { request->send(400); return;}
    if (!request->hasArg("midColor")) { request->send(400); return;}
    if (!request->hasArg("highColor")) { request->send(400); return;}
    if (!request->hasArg("bassWeight")) { request->send(400); return;}
    if (!request->hasArg("midWeight")) { request->send(400); return;}
    if (!request->hasArg("highWeight")) { request->send(400); return;}
    if(mode == musicPage){
      bassWeight = request->arg("bassWeight").toInt();
      midWeight = request->arg("midWeight").toInt();
      highWeight = request->arg("highWeight").toInt();
      colors[0] = request->arg("bassColor");
      colors[1] = request->arg("midColor");
      colors[2] = request->arg("highColor");
      int *channels[3] = { &bassChannel, &midChannel, &highChannel};
      bool *noneVars[3] = { &bassNone, &midNone, &highNone};
      for (int i = 0; i < 3; i++) {
        if(colors[i] != "none") {
          *noneVars[i] = false;
          if (colors[i] == "red") {
            *channels[i] = redChannel;
          }
          else if (colors[i] == "blue") {
            *channels[i] = blueChannel;
          }
          else {
            *channels[i] = greenChannel;
          }
        }
        else {
          *noneVars[i] = true;
        }
      }
      Serial.println(bassChannel);
      Serial.println(bassWeight);
      Serial.println(midChannel);
      Serial.println(midWeight);
      Serial.println(highChannel);
      Serial.println(highWeight);
      request->send(200);
    }
    else {
      request->send(409);
    }
  });

  server.begin();
}

void loop()
{
  if (mode == colorPage) {
    ledcWrite(redChannel, redValue); // set the brightness of the LED
    ledcWrite(blueChannel, blueValue); // set the brightness of the LED
    ledcWrite(greenChannel, greenValue); // set the brightness of the LED
  }
  else if (mode == musicPage) {
    // digitalWrite(resetPin, HIGH);
    // digitalWrite(resetPin, LOW);
    // for (int i=0;i<7;i++){
    //   digitalWrite(strobePin, LOW);
    //   delay(8);
    //   spectrumValue[i]=analogRead(analogPin);
    //   spectrumValue[i]=constrain(spectrumValue[i], filter, 1023);
    //   spectrumValue[i]=map(spectrumValue[i], filter,1023,0,MAXBRIGHTNESS);
    //   // Serial.print(spectrumValue[i]);
    //   // Serial.print(" ");
    //   digitalWrite(strobePin, HIGH);
    // }
    // Serial.println();

    /*SAMPLING*/
    for(int i=0; i<SAMPLES; i++)
    {
      microseconds = micros();    //Overflows after around 70 minutes!

      int sample = analogRead(analogPin);
      vReal[i] = analogRead(analogPin) - 2048;
      vImag[i] = 0;
      Serial.println(analogRead(analogPin));
    
      // while(micros() - microseconds < sampling_period_us){}
      while(micros() < (microseconds + sampling_period_us)){}
    }

    /*FFT*/
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    // double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);

    /*PRINT RESULTS*/
    //Serial.println(peak);     //Print out what frequency is the most dominant.
    double averageBass = 0;
    int nBass = 0;
    double averageMid = 0;
    int nMid = 0;
    double averageHigh = 0;
    int nHigh = 0;
    for(int i=0; i<(SAMPLES/2); i++)
    {
      double frequency = (i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES;
      // Serial.println(frequency,1);
      // Serial.println(vReal[i], 1);
      if (frequency > 20 && frequency <= 250) { //Bass
        nBass++;
        averageBass += vReal[i];
      }
      else if (frequency <= 4000) {
        nMid++;
        averageMid += vReal[i];
      }
      else if (frequency > 4000){
        nHigh++;
        averageHigh += vReal[i];
      }
        /*View all these three lines in serial terminal to see which frequencies has which amplitudes*/
         
        // Serial.print((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES, 1);
        // Serial.print(" ");
        // Serial.println(vReal[i], 1);    //View only this line in serial plotter to visualize the bins
    }
    // Serial.println(averageBass/nBass);
    averageBass = (averageBass / nBass);
    averageMid = (averageMid / nMid);
    averageHigh = (averageHigh / nHigh);
    // Serial.printf("Bass: %lf, Mid: %lf, High %lf\n", averageBass, averageMid, averageHigh);

    // // double averageBass = ((double)spectrumValue[0] * .75 +  (double)spectrumValue[1] * .25);
    // // double averageMid = (spectrumValue[3] + spectrumValue[4]) / 2;
    // // double averageHigh = (spectrumValue[5] + spectrumValue[6]) / 2;
    if (bassNone) averageBass = 0;
    if (midNone) averageMid = 0;
    if (highNone) averageHigh = 0;
    // ledcWrite(bassChannel, (int)(averageBass * ((double)bassWeight / MAXBRIGHTNESS)));
    // ledcWrite(midChannel, (int)(averageMid * ((double)midWeight / MAXBRIGHTNESS)));
    // ledcWrite(highChannel, (int)(averageHigh * ((double)highWeight / MAXBRIGHTNESS)));
  }
}