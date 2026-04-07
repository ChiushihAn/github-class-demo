#include <WiFi.h>
#include <WebServer.h>

// ========== 用户配置区域 ==========
const char* ssid = "你的WiFi名称";      //修改名称
const char* password = "你的WiFi密码";  
// ==================================

// 硬件引脚定义
const int LED_PIN = 2;          // 板载LED (GPIO2)
const int TOUCH_PIN = 4;        // 触摸引脚 T0 (GPIO4)

// 触摸阈值 (通过串口监视器观察正常未触摸值约50-80，触摸后降到20以下)
const int TOUCH_THRESHOLD = 35;  // 小于此值认为被触摸

// 报警闪烁间隔 (毫秒)
const int ALARM_BLINK_INTERVAL = 150;  // 每秒约3-4次闪烁

// 系统状态变量
bool armed = false;      // 布防标志
bool alarming = false;   // 报警标志 (锁存)
bool ledState = false;   // LED当前物理状态 (用于闪烁)
unsigned long lastBlinkTime = 0;  // 上次LED翻转时间

WebServer server(80);

// 生成HTML页面 (包含状态显示和布防/撤防按钮)
String getHtmlPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 安防报警器</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      text-align: center;
      margin-top: 60px;
      background-color: #f0f2f5;
      color: #333;
    }
    .container {
      background: white;
      max-width: 500px;
      margin: 0 auto;
      padding: 30px 20px;
      border-radius: 20px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.1);
    }
    h1 {
      color: #d32f2f;
      margin-bottom: 10px;
    }
    .status-card {
      background: #f9f9f9;
      border-radius: 15px;
      padding: 15px;
      margin: 20px 0;
      font-size: 1.2rem;
    }
    .status-label {
      font-weight: bold;
    }
    #armedStatus, #alarmStatus {
      font-weight: bold;
      padding: 4px 12px;
      border-radius: 20px;
      display: inline-block;
    }
    .btn {
      border: none;
      padding: 12px 30px;
      margin: 10px 15px;
      font-size: 1.1rem;
      border-radius: 40px;
      cursor: pointer;
      transition: 0.2s;
      font-weight: bold;
    }
    .btn-arm {
      background-color: #4caf50;
      color: white;
      box-shadow: 0 2px 5px rgba(76,175,80,0.3);
    }
    .btn-arm:hover {
      background-color: #45a049;
      transform: scale(1.02);
    }
    .btn-disarm {
      background-color: #f44336;
      color: white;
      box-shadow: 0 2px 5px rgba(244,67,54,0.3);
    }
    .btn-disarm:hover {
      background-color: #d32f2f;
      transform: scale(1.02);
    }
    .note {
      font-size: 0.85rem;
      color: #777;
      margin-top: 25px;
      border-top: 1px solid #ddd;
      padding-top: 15px;
    }
  </style>
</head>
<body>
<div class="container">
  <h1>🔐 智能安防报警器</h1>
  <div class="status-card">
    <div>🔒 系统状态</div>
    <div class="status-label">布防状态：</div>
    <span id="armedStatus">--</span><br>
    <div class="status-label">报警状态：</div>
    <span id="alarmStatus">--</span>
  </div>
  <div>
    <button class="btn btn-arm" onclick="sendCommand('arm')">🔓 布防 (Arm)</button>
    <button class="btn btn-disarm" onclick="sendCommand('disarm')">🔒 撤防 (Disarm)</button>
  </div>
  <div class="note">
    ⚠️ 实验说明：<br>
    1. 点击【布防】后，系统进入警戒状态，LED常亮。<br>
    2. 触摸GPIO4引脚（T0），LED开始快速闪烁并锁存报警。<br>
    3. 即使手松开，LED闪烁也不会停止，直到点击【撤防】。<br>
    4. 未布防时触摸引脚无任何反应。
  </div>
</div>

<script>
  // 更新页面上的状态显示
  async function updateStatus() {
    try {
      const response = await fetch('/status');
      const data = await response.json();
      const armedSpan = document.getElementById('armedStatus');
      const alarmSpan = document.getElementById('alarmStatus');
      
      if (data.armed) {
        armedSpan.innerHTML = '<span style="color:#4caf50;">🔔 已布防</span>';
      } else {
        armedSpan.innerHTML = '<span style="color:#999;">⚪ 撤防</span>';
      }
      
      if (data.alarming) {
        alarmSpan.innerHTML = '<span style="color:#f44336;">🚨 报警中 (闪烁)</span>';
      } else {
        alarmSpan.innerHTML = '<span style="color:#4caf50;">✅ 正常</span>';
      }
    } catch(e) {
      console.error('获取状态失败', e);
    }
  }

  // 发送布防/撤防命令
  async function sendCommand(cmd) {
    try {
      const response = await fetch('/' + cmd);
      if (response.ok) {
        updateStatus();  // 更新显示
      } else {
        alert('命令执行失败');
      }
    } catch(e) {
      alert('网络错误');
    }
  }

  // 页面加载时获取状态，并每秒自动刷新一次
  updateStatus();
  setInterval(updateStatus, 1000);
</script>
</body>
</html>
)rawliteral";
  return html;
}

// 处理根路径请求
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", getHtmlPage());
}

// 布防命令
void handleArm() {
  armed = true;
  alarming = false;      // 布防时清除报警标志
  ledState = false;      // 重置LED状态变量
  digitalWrite(LED_PIN, HIGH);   // 布防后LED常亮表示待命
  Serial.println("系统已布防，LED常亮");
  server.send(200, "text/plain", "Armed");
}

// 撤防命令
void handleDisarm() {
  armed = false;
  alarming = false;      // 撤防时清除报警标志
  digitalWrite(LED_PIN, LOW);    // 熄灭LED
  Serial.println("系统已撤防，LED熄灭");
  server.send(200, "text/plain", "Disarmed");
}

// 获取当前状态 (JSON格式，供前端轮询)
void handleStatus() {
  String json = "{\"armed\":" + String(armed ? "true" : "false") + 
                ",\"alarming\":" + String(alarming ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

// 触摸检测与报警逻辑 (非阻塞)
void handleAlarm() {
  // 仅在布防且未报警时检测触摸
  if (armed && !alarming) {
    int touchValue = touchRead(TOUCH_PIN);
    if (touchValue < TOUCH_THRESHOLD) {
      alarming = true;   // 触发报警并锁存
      Serial.println("⚠️ 触发报警！LED开始闪烁");
      // 注意：报警瞬间LED立即开始闪烁，需重置闪烁计时
      lastBlinkTime = millis();
      ledState = false;   // 确保从当前状态开始翻转
      digitalWrite(LED_PIN, LOW);  // 先熄灭一次让闪烁节奏一致
    }
  }
}

// LED状态机：处理报警闪烁或正常指示
void handleLED() {
  if (!armed) {
    // 未布防：LED必须熄灭（确保报警时撤防也能熄灭）
    digitalWrite(LED_PIN, LOW);
    return;
  }
  
  if (alarming) {
    // 报警模式：高频闪烁
    unsigned long now = millis();
    if (now - lastBlinkTime >= ALARM_BLINK_INTERVAL) {
      lastBlinkTime = now;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
  } else {
    // 布防且未报警：LED常亮
    digitalWrite(LED_PIN, HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // 初始化引脚
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);   // 初始熄灭
  
  // 触摸引脚不需要初始化，直接使用touchRead()
  
  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("正在连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功");
  Serial.print("ESP32 IP地址: ");
  Serial.println(WiFi.localIP());
  
  // 配置Web服务器路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  server.begin();
  Serial.println("HTTP服务器已启动");
  
  // 输出触摸参考值 (帮助用户调整阈值)
  Serial.println("提示：请观察以下触摸值，未触摸时通常 > 50，触摸后 < 30");
  for (int i = 0; i < 5; i++) {
    Serial.print("触摸值: ");
    Serial.println(touchRead(TOUCH_PIN));
    delay(1000);
  }
  Serial.println("当前阈值设置为: " + String(TOUCH_THRESHOLD));
}

void loop() {
  server.handleClient();   // 处理Web请求
  handleAlarm();           // 检测触摸并触发报警（锁存）
  handleLED();             // 根据状态控制LED（常亮/熄灭/闪烁）
}