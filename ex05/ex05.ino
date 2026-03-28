// 引脚定义
#define LED_PIN     2       // LED 引脚
#define TOUCH_PIN   4       // 触摸引脚

// PWM 配置
const int pwmFreq = 5000;       // PWM 频率 5kHz
const int pwmResolution = 8;    // 分辨率 8 位

// 档位设置
#define LEVEL_MIN    1          // 最小档位
#define LEVEL_MAX    3          // 最大档位
int currentLevel = 1;           // 当前档位（1:慢速, 2:中速, 3:快速）

// 各档位对应的步进延时（ms），延时越长呼吸越慢
const int levelDelay[LEVEL_MAX + 1] = {0, 20, 10, 5};   // 索引 1~3

// 触摸阈值
const int touchThreshold = 20;

// 触摸状态控制（防抖、单次触发）
bool lastTouchState = false;     // 上一次触摸状态（true=触摸中）
bool canSwitch = true;            // 是否允许切换档位（防止连续触发）

// 呼吸灯状态机变量
int duty = 0;                     // 当前占空比（0~255）
int step = 1;                     // 方向：1 增加，-1 减少

void setup() {
  Serial.begin(115200);
  delay(1000);                    // 等待串口稳定

  // 配置 PWM 输出
  ledcAttach(LED_PIN, pwmFreq, pwmResolution);
  // 初始占空比设为 0
  ledcWrite(LED_PIN, 0);

  Serial.println("多档位触摸调速呼吸灯已启动");
  Serial.print("当前档位：");
  Serial.println(currentLevel);
}

void loop() {
  // --- 1. 触摸检测与档位切换 ---
  int touchValue = touchRead(TOUCH_PIN);
  bool currentTouchState = (touchValue < touchThreshold);   // 触摸时值变小

  // 检测下降沿（从松开变为触摸）
  if (currentTouchState && !lastTouchState && canSwitch) {
    // 切换档位
    currentLevel++;
    if (currentLevel > LEVEL_MAX) currentLevel = LEVEL_MIN;
    
    // 输出档位信息
    Serial.print("档位切换为：");
    Serial.print(currentLevel);
    Serial.print("  (延时 ");
    Serial.print(levelDelay[currentLevel]);
    Serial.println(" ms/步)");
    
    canSwitch = false;   // 防止本次触摸过程中再次切换
  }
  
  // 触摸释放时，允许下一次切换
  if (!currentTouchState) {
    canSwitch = true;
  }
  
  // 更新触摸状态记录
  lastTouchState = currentTouchState;

  // --- 2. 呼吸灯状态机（非阻塞步进）---
  // 更新占空比
  duty += step;
  
  // 边界检测，反转方向
  if (duty >= 255) {
    duty = 255;
    step = -1;          // 开始变暗
  } else if (duty <= 0) {
    duty = 0;
    step = 1;           // 开始变亮
  }
  
  // 输出当前占空比
  ledcWrite(LED_PIN, duty);
  
  // 根据当前档位延时，控制呼吸节奏
  delay(levelDelay[currentLevel]);
}