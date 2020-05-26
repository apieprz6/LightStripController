int analogPin=A0;
int strobePin=3;
int resetPin=4;
int ledred=6;
int ledblue=9;
int ledgreen=10;
int spectrumValue[7];
int filter=80;
const int MAXBRIGHTNESS = 255;

void setup(){
  
  Serial.begin(9600);
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  pinMode(ledred, OUTPUT);
  pinMode(ledblue, OUTPUT);
  pinMode(ledgreen, OUTPUT);
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
}

void loop(){
  
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);
  for (int i=0;i<7;i++){
    digitalWrite(strobePin, LOW);
    delay(8);
    spectrumValue[i]=analogRead(analogPin);
    spectrumValue[i]=constrain(spectrumValue[i], filter, 1023);
    spectrumValue[i]=map(spectrumValue[i], filter,1023,0,255);
    Serial.print(spectrumValue[i]);
    Serial.print(" ");
    digitalWrite(strobePin, HIGH);
  }
  Serial.println();
//  analogWrite(ledred,spectrumValue[0]);
//  analogWrite(ledred,spectrumValue[1]);
//  analogWrite(ledblue,spectrumValue[3]);
//  analogWrite(ledblue,spectrumValue[4]);
//  analogWrite(ledgreen,spectrumValue[5]);
//  analogWrite(ledgreen,spectrumValue[6]);
    double blueBright = 50;
    double greenBright = 50;
    double redBright = 255;
    double averageRed = ((double)spectrumValue[0] * .75 +  (double)spectrumValue[1] * .25);
    double averageBlue = (spectrumValue[3] + spectrumValue[4]) / 2;
    double averageGreen = (spectrumValue[5] + spectrumValue[6]) / 2;
    
    analogWrite(ledred,(int)(averageRed * ((double)redBright / MAXBRIGHTNESS)));
    //analogWrite(ledred,(int)(spectrumValue[1] * ((double)redBright / MAXBRIGHTNESS)));
    //averageBlue = averageBlue * (averageRed / MAXBRIGHTNESS);
    //averageGreen = averageGreen * (averageRed / MAXBRIGHTNESS); //ties green and red to base and causes brightness to vary with bass
    analogWrite(ledblue,(int)(averageBlue * ((double)blueBright / MAXBRIGHTNESS)));
    //analogWrite(ledblue,(int)(spectrumValue[4] * ((double)blueBright / MAXBRIGHTNESS)));
    analogWrite(ledgreen,(int)(averageGreen * ((double)greenBright / MAXBRIGHTNESS)));
    //analogWrite(ledgreen,(int)(spectrumValue[6] * ((double)greenBright / MAXBRIGHTNESS)));
}

//void writeColor(int red, int green, int blue, int brightness){
//    analogWrite(bluePin, (int)(blue * ((double)brightness / MAXBRIGHTNESS)));
//    analogWrite(redPin, (int)(red * ((double)brightness / MAXBRIGHTNESS)));
//    analogWrite(greenPin, (int)(green * ((double)brightness / MAXBRIGHTNESS)));
//}
