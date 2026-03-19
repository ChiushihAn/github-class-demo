// 定义LED引脚，ESP32板载LED通常连接在GPIO 2
const int ledPin = 2;

// 定义摩尔斯码时序（单位：毫秒）
const int dotDuration = 200;      // 点（短闪）持续时间
const int dashDuration = 600;     // 划（长闪）持续时间
const int elementGap = 200;       // 同一字母内点划之间的间隔
const int letterGap = 500;        // 字母之间的间隔
const int wordGap = 2000;         // 单词（SOS整体）之间的间隔

void setup() {
  Serial.begin(115200);           // 初始化串口，波特率115200
  pinMode(ledPin, OUTPUT);        // 设置LED引脚为输出模式
  Serial.println("SOS Signal Started");
}

void loop() {
  // --- 发送字母 S (三个点) ---
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);   // 点亮LED
    Serial.println("S - DOT ON");
    delay(dotDuration);           // 保持点亮（点时长）
    digitalWrite(ledPin, LOW);    // 熄灭LED
    if (i < 2) {                  // 前两个点后需要点间间隔
      delay(elementGap);
    }
  }
  delay(letterGap);               // 字母S与O之间的间隔

  // --- 发送字母 O (三个划) ---
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);   // 点亮LED
    Serial.println("O - DASH ON");
    delay(dashDuration);          // 保持点亮（划时长）
    digitalWrite(ledPin, LOW);    // 熄灭LED
    if (i < 2) {                  // 前两个划后需要点间间隔（实际是划间间隔，但摩尔斯码中间隔统一）
      delay(elementGap);
    }
  }
  delay(letterGap);               // 字母O与S之间的间隔

  // --- 发送字母 S (三个点) ---
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);   // 点亮LED
    Serial.println("S - DOT ON");
    delay(dotDuration);           // 保持点亮（点时长）
    digitalWrite(ledPin, LOW);    // 熄灭LED
    if (i < 2) {
      delay(elementGap);
    }
  }
  delay(wordGap);                 // 整个SOS单词结束后的长间隔，然后重复循环
}