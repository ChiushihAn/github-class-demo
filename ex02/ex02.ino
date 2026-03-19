const int ledPin = 2;                      // LED 引脚
unsigned long previousMillis = 0;  // 上一次状态改变的时间戳（毫秒）
const long interval = 1000;        // 闪烁间隔 1000ms (1秒)
int ledState = LOW;                   // LED 当前状态

void setup() {
  Serial.begin(115200);          // 初始化串口通信
  pinMode(ledPin, OUTPUT);       // 设置 LED 引脚为输出模式
  digitalWrite(ledPin, ledState); // 初始状态为熄灭
}

void loop() {
  // 获取当前系统运行毫秒数
  unsigned long currentMillis = millis();

  // 判断是否到达切换时间
  if (currentMillis - previousMillis >= interval) {
    // 保存本次切换的时间戳
    previousMillis = currentMillis;

    // 翻转 LED 状态
    ledState = (ledState == LOW) ? HIGH : LOW;
    digitalWrite(ledPin, ledState);

    // 串口输出当前状态（便于调试）
    Serial.println(ledState == HIGH ? "LED ON" : "LED OFF");
  }

  // 此处可以执行其他任务，不会阻塞
}