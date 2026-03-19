// 使用 millis() 实现 1Hz 闪烁
const int ledPin = 2;
unsigned long previousMillis = 0;
const long interval = 1000;
int ledState = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(ledPin, ledState);
    Serial.println(ledState == HIGH ? "LED ON" : "LED OFF");
  }
}