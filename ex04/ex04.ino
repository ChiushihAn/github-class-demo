// 定义触摸引脚 
#define TOUCH_PIN 4
// 定义LED引脚 
#define LED_PIN 2

// 阈值：未触摸约1800，触摸时<10，设为100足够
int threshold = 100; 

// 状态变量
bool ledState = false;          // LED当前状态
bool stableTouchState = false;  // 当前稳定的触摸状态
bool lastRawTouch = false;      // 上一次读取的原始触摸状态
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;  // 防抖时间

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);   // 初始熄灭LED
  Serial.println("自锁触摸开关已启动");
}

void loop() {
  int touchValue = touchRead(TOUCH_PIN);
  bool rawTouch = (touchValue < threshold);

  // 如果原始状态发生变化，重置防抖计时器，并记录当前状态
  if (rawTouch != lastRawTouch) {
    lastDebounceTime = millis();
    lastRawTouch = rawTouch;  // 更新上次读取状态
  }

  // 当状态稳定超过防抖时间后，如果稳定状态与当前原始状态不同，则更新稳定状态
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (rawTouch != stableTouchState) {
      stableTouchState = rawTouch;

      // 边缘检测：只在稳定状态从 false 变为 true 翻转LED
      if (stableTouchState == true) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
        Serial.print("LED toggled -> ");
        Serial.println(ledState ? "ON" : "OFF");
      }
    }
  }

  delay(50);
}