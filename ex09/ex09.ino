#include <WiFi.h>
#include <WebServer.h>

// ========== 用户配置区域 ==========
const char* ssid = "你的WiFi名称";      // 修改为你的WiFi名称
const char* password = "你的WiFi密码";  // 修改为你的WiFi密码
// ==================================

// 触摸引脚定义 (T0 = GPIO4)
const int TOUCH_PIN = 4;

WebServer server(80);

// 读取触摸传感器的原始数值 (范围大约 0~100+，未触摸时通常 > 50，触摸后降到 20 以下)
int getTouchValue() {
  return touchRead(TOUCH_PIN);
}

// 处理根路径请求：返回带实时数据更新功能的仪表盘HTML页面
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=yes">
  <title>ESP32 实时触摸传感器仪表盘</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    body {
      background: linear-gradient(145deg, #1e2a3a 0%, #0f1724 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      font-family: 'Segoe UI', 'Roboto', 'Poppins', sans-serif;
      padding: 20px;
    }
    .dashboard {
      background: rgba(22, 34, 48, 0.75);
      backdrop-filter: blur(12px);
      border-radius: 60px;
      padding: 40px 30px;
      box-shadow: 0 25px 45px rgba(0,0,0,0.4), inset 0 1px 2px rgba(255,255,255,0.1);
      text-align: center;
      max-width: 550px;
      width: 100%;
      transition: all 0.2s;
      border: 1px solid rgba(255,255,255,0.2);
    }
    h1 {
      font-size: 1.8rem;
      font-weight: 500;
      letter-spacing: 1px;
      color: #c0e0ff;
      margin-bottom: 15px;
      text-shadow: 0 2px 5px rgba(0,0,0,0.3);
    }
    .sensor-label {
      font-size: 1.1rem;
      color: #a0c4e8;
      margin-bottom: 20px;
      border-bottom: 1px dashed #3a5a78;
      display: inline-block;
      padding-bottom: 6px;
    }
    .value-container {
      background: #0a121c;
      border-radius: 70px;
      margin: 20px 0 25px;
      padding: 30px 20px;
      box-shadow: inset 0 4px 8px rgba(0,0,0,0.5), 0 2px 4px rgba(255,255,255,0.05);
    }
    .sensor-value {
      font-size: 5.2rem;
      font-weight: 800;
      font-family: 'Courier New', 'Fira Mono', monospace;
      color: #6eff9e;
      text-shadow: 0 0 12px #3eff7a80;
      letter-spacing: 4px;
      transition: 0.1s;
    }
    .unit {
      font-size: 2rem;
      font-weight: 400;
      color: #8aaec0;
    }
    .description {
      font-size: 1rem;
      color: #bdd4e8;
      background: #00000044;
      padding: 12px 20px;
      border-radius: 50px;
      margin-top: 15px;
    }
    .footer {
      margin-top: 25px;
      font-size: 0.75rem;
      color: #6e8daa;
    }
    .progress-bar-bg {
      width: 90%;
      height: 8px;
      background: #1e2f3c;
      border-radius: 10px;
      margin: 15px auto 0;
      overflow: hidden;
    }
    .progress-fill {
      height: 100%;
      width: 0%;
      background: linear-gradient(90deg, #3eff9e, #00cc66);
      border-radius: 10px;
      transition: width 0.1s ease;
    }
    @keyframes pulse {
      0% { opacity: 0.6; }
      100% { opacity: 1; }
    }
    .live-badge {
      display: inline-block;
      background: #ff3a4d;
      border-radius: 30px;
      font-size: 0.7rem;
      padding: 4px 12px;
      margin-left: 12px;
      vertical-align: middle;
      animation: pulse 0.8s infinite alternate;
    }
  </style>
</head>
<body>
<div class="dashboard">
  <h1>📡 实时电容触摸仪表 <span class="live-badge">LIVE</span></h1>
  <div class="sensor-label">🔌 TOUCH PIN (GPIO4 / T0)</div>
  <div class="value-container">
    <span class="sensor-value" id="touchValue">--</span>
    <span class="unit">raw</span>
    <div class="progress-bar-bg">
      <div class="progress-fill" id="progressFill"></div>
    </div>
  </div>
  <div class="description">
    ✨ 手指靠近或触摸引脚 → 数值变小 &nbsp;&nbsp;|&nbsp;&nbsp; 
    🚀 离开后数值恢复 &nbsp;&nbsp;|&nbsp;&nbsp;
    ⚡ AJAX 实时刷新
  </div>
  <div class="footer">
    ESP32 Web Server | 数据采集演示 (上报模式)
  </div>
</div>

<script>
  // 显示触摸数值的元素
  const valueSpan = document.getElementById('touchValue');
  const progressFill = document.getElementById('progressFill');

  // 更新仪表盘的函数 (通过fetch拉取最新数值)
  async function updateSensor() {
    try {
      const response = await fetch('/data');
      if (!response.ok) throw new Error('HTTP error');
      const rawValue = await response.text();
      let num = parseInt(rawValue, 10);
      if (isNaN(num)) num = 0;
      
      // 更新大数字显示
      valueSpan.innerText = num;
      
      // 更新进度条: 假设最大值100 (实际触摸值一般最大在80~100左右, 也可以动态适配)
      // 为了让进度条直观：数值越小代表触摸越强 → 进度条越长？或者进度条反映数值比例。
      // 这里为了清晰：进度条长度 = (最大值 - 当前值) / 最大值 * 100% 这样触摸时进度条变长。
      // 设置参考最大值 = 100 (未触摸时一般70~80, 触摸后降到10~30)
      const maxRef = 100;
      let percent = ((maxRef - num) / maxRef) * 100;
      if (percent < 0) percent = 0;
      if (percent > 100) percent = 100;
      progressFill.style.width = percent + '%';
      
      // 可依据数值改变大数字颜色 (触摸时偏红，正常时偏绿)
      if (num < 35) {
        valueSpan.style.color = '#ffaa66';
        valueSpan.style.textShadow = '0 0 10px #ff884d';
      } else {
        valueSpan.style.color = '#6eff9e';
        valueSpan.style.textShadow = '0 0 12px #3eff7a80';
      }
    } catch (err) {
      console.error('获取数据失败:', err);
      valueSpan.innerText = 'ERR';
    }
  }

  // 页面加载后立即更新一次，然后每 100 毫秒拉取一次 (实现流畅的实时感)
  updateSensor();
  setInterval(updateSensor, 100);
</script>
</body>
</html>
)rawliteral";
  server.send(200, "text/html; charset=utf-8", html);
}

// 处理 /data 请求：返回触摸传感器的原始数值 (纯文本)
void handleData() {
  int touchVal = getTouchValue();
  server.send(200, "text/plain", String(touchVal));
}

// 可选：返回JSON格式数据 (与上面二选一即可，为了扩展性也可以单独做，但纯文本简单)
void handleDataJSON() {
  int touchVal = getTouchValue();
  String json = "{\"touch_value\":" + String(touchVal) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // 触摸引脚无需设置pinMode，直接使用touchRead即可

  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("正在连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功");
  Serial.print("ESP32 IP 地址: ");
  Serial.println(WiFi.localIP());
  
  // 配置Web服务器路由
  server.on("/", handleRoot);        // 主页面
  server.on("/data", handleData);    // 返回纯文本数值 (供AJAX轮询)
  server.on("/data.json", handleDataJSON); // 备用JSON接口，但不强制使用
  server.begin();
  Serial.println("HTTP服务器已启动，请通过浏览器访问上述IP地址");
  
  // 串口输出参考触摸值，方便调试
  Serial.println("========= 触摸传感器参考值 =========");
  for (int i = 0; i < 5; i++) {
    int val = touchRead(TOUCH_PIN);
    Serial.print("当前触摸值: ");
    Serial.println(val);
    delay(1000);
  }
  Serial.println("提示：未触摸时数值较大(60~100)，手指触摸或靠近后数值减小(低于30)");
}

void loop() {
  server.handleClient();  // 持续处理HTTP请求
}