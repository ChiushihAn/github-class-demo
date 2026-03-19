const int ledPin = 2;            // LED 引脚

// 状态机状态定义
enum State {
  STATE_WORD_PAUSE,   // 单词间长停顿
  STATE_SHORT_ON,     // 短闪点亮
  STATE_SHORT_OFF,    // 短闪熄灭间隔
  STATE_LONG_ON,      // 长闪点亮
  STATE_LONG_OFF,     // 长闪熄灭间隔
  STATE_LETTER_PAUSE  // 字母间停顿
};

State currentState = STATE_WORD_PAUSE;  // 当前状态
int repeatCount = 0;      // 当前字母已闪烁的次数 (0,1,2 对应三次)
int letterIndex = 0;       // 当前字母索引: 0=S, 1=O, 2=S
unsigned long previousMillis = 0;  // 上次状态改变的时间戳

// 时间常量（单位：毫秒）
const int SHORT_MS = 200;           // 短闪持续时间
const int LONG_MS = 600;            // 长闪持续时间
const int INTER_FLASH_PAUSE = 200;  // 同一字母内闪之间的停顿
const int LETTER_PAUSE = 500;       // 字母之间的停顿
const int WORD_PAUSE = 2000;        // 单词结束后的长停顿

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);        // 初始熄灭
}

void loop() {
  unsigned long currentMillis = millis();

  switch (currentState) {
    // ---------- 单词间长停顿 ----------
    case STATE_WORD_PAUSE:
      // 等待长停顿结束，然后开始新的 SOS 序列
      if (currentMillis - previousMillis >= WORD_PAUSE) {
        letterIndex = 0;            // 从第一个字母 S 开始
        repeatCount = 0;
        currentState = STATE_SHORT_ON;  // 进入短闪点亮状态
        previousMillis = currentMillis;
        Serial.println("Start SOS");    // 调试信息
      }
      break;

    // ---------- 短闪点亮 ----------
    case STATE_SHORT_ON:
      digitalWrite(ledPin, HIGH);   // 点亮 LED
      if (currentMillis - previousMillis >= SHORT_MS) {
        digitalWrite(ledPin, LOW);  // 时间到，熄灭
        repeatCount++;              // 完成一次短闪
        previousMillis = currentMillis;

        if (repeatCount < 3) {
          // 尚未完成三次短闪，进入闪间熄灭停顿
          currentState = STATE_SHORT_OFF;
        } else {
          // 三次短闪完成，重置计数，准备进入字母间停顿
          repeatCount = 0;
          currentState = STATE_LETTER_PAUSE;
        }
      }
      break;

    // ---------- 短闪熄灭间隔 ----------
    case STATE_SHORT_OFF:
      // 等待间隔时间结束，然后再次点亮进行下一次短闪
      if (currentMillis - previousMillis >= INTER_FLASH_PAUSE) {
        currentState = STATE_SHORT_ON;
        previousMillis = currentMillis;
      }
      break;

    // ---------- 长闪点亮 ----------
    case STATE_LONG_ON:
      digitalWrite(ledPin, HIGH);
      if (currentMillis - previousMillis >= LONG_MS) {
        digitalWrite(ledPin, LOW);
        repeatCount++;
        previousMillis = currentMillis;

        if (repeatCount < 3) {
          currentState = STATE_LONG_OFF;   // 进入长闪熄灭间隔
        } else {
          repeatCount = 0;
          currentState = STATE_LETTER_PAUSE; // 三次长闪完成，字母间停顿
        }
      }
      break;

    // ---------- 长闪熄灭间隔 ----------
    case STATE_LONG_OFF:
      if (currentMillis - previousMillis >= INTER_FLASH_PAUSE) {
        currentState = STATE_LONG_ON;
        previousMillis = currentMillis;
      }
      break;

    // ---------- 字母间停顿 ----------
    case STATE_LETTER_PAUSE:
      // 字母间停顿结束后，切换到下一个字母
      if (currentMillis - previousMillis >= LETTER_PAUSE) {
        letterIndex++;   // 移到下一个字母

        if (letterIndex == 1) {
          // 当前完成的是 S，下一个是 O
          currentState = STATE_LONG_ON;
        } else if (letterIndex == 2) {
          // 当前完成的是 O，下一个是 S
          currentState = STATE_SHORT_ON;
        } else if (letterIndex == 3) {
          // 三个字母全部完成，进入单词间长停顿
          currentState = STATE_WORD_PAUSE;
        }
        previousMillis = currentMillis;
      }
      break;

    default:
      // 出错保护，重置到单词停顿
      currentState = STATE_WORD_PAUSE;
      previousMillis = currentMillis;
      break;
  }
}