// 定义两个 LED 引脚
const int ledPinR = 2;   // 红灯 
const int ledPinY = 4;   // 黄灯 

// PWM 配置
const int pwmFreq = 5000;       // 频率 5kHz
const int pwmResolution = 8;    // 分辨率 8 位

void setup() {
  // 初始化两个 PWM 通道（ledcAttach 会自动分配可用通道）
  ledcAttach(ledPinR, pwmFreq, pwmResolution);
  ledcAttach(ledPinY, pwmFreq, pwmResolution);
  
  // 可选：串口输出启动信息
  Serial.begin(115200);
  Serial.println("警车双闪灯效已启动");
}

void loop() {
  // 阶段1：红灯逐渐变亮，黄灯 逐渐变暗
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(ledPinR, duty);           //红灯亮度增加
    ledcWrite(ledPinY, 255 - duty);     // 黄灯 亮度减少
    delay(10);                          // 控制渐变速度（10ms 过渡平滑）
  }
  
  // 阶段2：红灯逐渐变暗，黄灯逐渐变亮
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(ledPinR, duty);           // 红灯亮度减少
    ledcWrite(ledPinY, 255 - duty);     //黄灯亮度增加
    delay(10);
  }
}