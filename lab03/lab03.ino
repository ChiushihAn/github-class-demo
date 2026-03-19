
const int ledPin = 2;  


const int freq = 5000;          // 频率 5000Hz
const int resolution = 8;       // 分辨率 8位 (0-255)

void setup() {
  Serial.begin(115200);


  ledcAttach(ledPin, freq, resolution);
}

void loop() {

  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   

    ledcWrite(ledPin, dutyCycle);   
    delay(10);
  }


  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    ledcWrite(ledPin, dutyCycle);   
    delay(10);
  }
  
  Serial.println("Breathing cycle completed");
}