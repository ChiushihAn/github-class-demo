// ex03: 使用 millis() 实现 SOS 信号
const int ledPin = 2;

// 状态定义
enum State {
  STATE_WORD_PAUSE,   // 单词间长停顿
  STATE_SHORT_ON,     // 短闪点亮
  STATE_SHORT_OFF,    // 短闪熄灭停顿
  STATE_LONG_ON,      // 长闪点亮
  STATE_LONG_OFF,     // 长闪熄灭停顿
  STATE_LETTER_PAUSE  // 字母间停顿
};

State currentState = STATE_WORD_PAUSE;
int repeatCount = 0;      // 当前字母已闪次数（0~2）
int letterIndex = 0;       // 0:S, 1:O, 2:S
unsigned long previousMillis = 0;

// 时间常量（毫秒）
const int SHORT_MS = 200;
const int LONG_MS = 600;
const int INTER_FLASH_PAUSE = 200;   // 同一字母内闪之间的停顿
const int LETTER_PAUSE = 500;        // 字母之间的停顿
const int WORD_PAUSE = 2000;         // 单词结束后的长停顿

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    case STATE_WORD_PAUSE:
      // 长停顿结束后，开始新的 SOS
      if (currentMillis - previousMillis >= WORD_PAUSE) {
        letterIndex = 0;
        repeatCount = 0;
        currentState = STATE_SHORT_ON; // 开始第一个短闪
        previousMillis = currentMillis;
        Serial.println("Start SOS");
      }
      break;

    case STATE_SHORT_ON:
      digitalWrite(ledPin, HIGH);
      if (currentMillis - previousMillis >= SHORT_MS) {
        digitalWrite(ledPin, LOW);
        repeatCount++;
        previousMillis = currentMillis;
        if (repeatCount < 3) {
          currentState = STATE_SHORT_OFF;
        } else {
          repeatCount = 0;
          currentState = STATE_LETTER_PAUSE;
        }
      }
      break;

    case STATE_SHORT_OFF:
      if (currentMillis - previousMillis >= INTER_FLASH_PAUSE) {
        currentState = STATE_SHORT_ON;
        previousMillis = currentMillis;
      }
      break;

    case STATE_LONG_ON:
      digitalWrite(ledPin, HIGH);
      if (currentMillis - previousMillis >= LONG_MS) {
        digitalWrite(ledPin, LOW);
        repeatCount++;
        previousMillis = currentMillis;
        if (repeatCount < 3) {
          currentState = STATE_LONG_OFF;
        } else {
          repeatCount = 0;
          currentState = STATE_LETTER_PAUSE;
        }
      }
      break;

    case STATE_LONG_OFF:
      if (currentMillis - previousMillis >= INTER_FLASH_PAUSE) {
        currentState = STATE_LONG_ON;
        previousMillis = currentMillis;
      }
      break;

    case STATE_LETTER_PAUSE:
      if (currentMillis - previousMillis >= LETTER_PAUSE) {
        // 决定下一个字母
        letterIndex++;
        if (letterIndex == 1) {
          currentState = STATE_LONG_ON;  // 第二个字母 O
        } else if (letterIndex == 2) {
          currentState = STATE_SHORT_ON; // 第三个字母 S
        } else if (letterIndex == 3) {
          currentState = STATE_WORD_PAUSE; // 完成一个单词
        }
        previousMillis = currentMillis;
      }
      break;
  }
}