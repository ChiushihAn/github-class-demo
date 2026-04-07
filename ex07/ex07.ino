#include <WiFi.h>
#include <WebServer.h>

// ========== 用户配置区域 ==========
const char* ssid = "你的WiFi名称";      // 修改为你的WiFi名称
const char* password = "你的WiFi密码";  // 修改为你的WiFi密码
const int LED_PIN = 2;                  
// ==================================

WebServer server(80);
int currentDuty = 0;  // 当前亮度值 (0-255)

// 处理根路径请求：返回带有滑动条的HTML页面
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes">
  <title>ESP32 PWM LED控制</title>
  <style>
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      text-align: center;
      margin-top: 60px;
      background-color: #f5f5f5;
      color: #333;
    }
    .container {
      background: white;
      max-width: 500px;
      margin: 0 auto;
      padding: 30px 20px;
      border-radius: 20px;
      box-shadow: 0 4px 15px rgba(0,0,0,0.1);
    }
    h1 {
      font-size: 1.8rem;
      color: #0077ff;
    }
    .slider-label {
      font-size: 1.2rem;
      margin: 20px 0 10px;
    }
    input[type=range] {
      width: 80%;
      height: 8px;
      -webkit-appearance: none;
      background: #ddd;
      border-radius: 5px;
      outline: none;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 24px;
      height: 24px;
      background: #0077ff;
      border-radius: 50%;
      cursor: pointer;
      box-shadow: 0 1px 3px rgba(0,0,0,0.2);
    }
    .brightness-value {
      font-size: 1.5rem;
      font-weight: bold;
      margin: 15px 0;
      color: #0077ff;
    }
    .note {
      font-size: 0.85rem;
      color: #777;
      margin-top: 25px;
      border-top: 1px solid #eee;
      padding-top: 15px;
    }
  </style>
</head>
<body>
<div class="container">
  <h1>🎚️ PWM 呼吸灯控制</h1>
  <p>拖动滑条实时改变 LED 亮度</p>
  <div class="slider-label">亮度调节 (0 ~ 255)</div>
  <input type="range" id="brightnessSlider" min="0" max="255" value=")rawliteral";
  html += String(currentDuty);          // 插入当前亮度值
  html += R"rawliteral(" step="1">
  <div class="brightness-value">
    当前亮度: <span id="brightnessSpan">)rawliteral";
  html += String(currentDuty);
  html += R"rawliteral(</span>
  </div>
  <div class="note">
    💡 提示：拖动滑条时，ESP32 板载 LED 亮度会同步变化<br>
    基于 PWM 脉冲宽度调制技术
  </div>
</div>

<script>
  const slider = document.getElementById('brightnessSlider');
  const span = document.getElementById('brightnessSpan');

  // 更新页面显示并发送请求到ESP32
  function updateBrightness(value) {
    span.innerText = value;
    // 使用 fetch 发送 GET 请求，实时将数值传给服务器
    fetch('/set?value=' + value)
      .then(response => {
        if (!response.ok) {
          console.warn('请求失败:', response.status);
        }
      })
      .catch(error => console.error('网络错误:', error));
  }

  // 监听滑动条的 input 事件（实时拖动）
  slider.addEventListener('input', function() {
    const val = this.value;
    updateBrightness(val);
  });
</script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html; charset=utf-8", html);
}

// 处理 /set 请求：解析参数并设置PWM占空比
void handleSet() {
  if (server.hasArg("value")) {
    int newValue = server.arg("value").toInt();
    // 限制范围 0-255
    newValue = constrain(newValue, 0, 255);
    if (newValue != currentDuty) {
      currentDuty = newValue;
      ledcWrite(LED_PIN, currentDuty);   // 更新PWM输出
      Serial.print("PWM亮度已设置为: ");
      Serial.println(currentDuty);
    }
    // 返回简单文本表示成功
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing parameter: value");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // 配置LED引脚为PWM输出
  // 频率 5000Hz, 分辨率 8位 (0-255)
  ledcAttach(LED_PIN, 5000, 8);
  ledcWrite(LED_PIN, 0);   // 初始亮度为0（熄灭）

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
  server.on("/set", handleSet);
  server.begin();
  Serial.println("HTTP服务器已启动");
}

void loop() {
  server.handleClient();   // 持续监听客户端请求
}