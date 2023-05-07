#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>

const char* ssid = "WIFI_NAVN";
const char* password = "WIFI_KODE";

String old_value;
float value;
bool pinStatus = false;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

char html_template[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Vertikal vindmølle</title>
    <style>
      body {
        font-family: Arial, Helvetica, sans-serif;
        background-color: #f0f0f0;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;
        min-height: 100vh;
        margin: 0;
      }
      h1 {
        font-size: 36px;
        color: #333;
        margin-top: 20px;
      }
      h2 {
        font-size: 24px;
        color: #333;
        margin-top: 30px;
      }
      p {
        font-size: 60px;
        color: #333;
        margin: 0;
      }
      button {
        background-color: #007bff;
        border: none;
        color: white;
        padding: 12px 24px;
        text-align: center;
        text-decoration: none;
        display: inline-block;
        font-size: 16px;
        margin: 20px 0;
        cursor: pointer;
        border-radius: 4px;
        transition: background-color 0.3s;
      }
      button:hover {
        background-color: #0056b3;
      }

      footer {
        background-color: #333;
        color: white;
        padding: 15px 0;
        position: absolute;
        bottom: 0;
        width: 100%;
        text-align: center;
        font-size: 14px;
      }
      
    .container {
            display: flex;
            justify-content: center;
            flex-wrap: wrap;
            gap: 20px;
          }
          .column {
            flex: 1;
            max-width: 45%;
            min-width: 300px;
          }
          .card {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 15px;
            margin-bottom: 20px;
          }

        .chart {
            background-color: white;
            border: 1px solid #ddd;
            border-radius: 8px;
            padding: 15px;
            margin-bottom: 20px;
            min-width: 40%;
          }
          

      
    </style>

    
    <script>
      // OpenWeatherMap API key 
      const openWeatherApiKey = "API_NØGLE";

      // Function to fetch wind data from OpenWeatherMap
      async function fetchWindData() {
        const response = await fetch(
          `http://api.openweathermap.org/data/2.5/weather?q=Valby,DK&appid=${openWeatherApiKey}&units=metric`
        );
        if (response.ok) {
          const data = await response.json();
          if (data.wind && data.wind.speed) {
            updateWindSpeed(data.wind.speed);
          }
        } else {
          console.error("Failed to fetch wind data");
        }
      }

      // Function to update the displayed wind speed
      function updateWindSpeed(speed) {
        document.getElementById("wind_speed").innerHTML = speed + " m/s";
      }

      // Fetch the wind data when the webpage is loaded
      document.addEventListener("DOMContentLoaded", fetchWindData);

      socket = new WebSocket("ws:/" + "/" + location.host + ":81");
      socket.onopen = function(e) { console.log("[socket] socket.onopen "); };
      socket.onerror = function(e) { console.log("[socket] socket.onerror "); };
      socket.onmessage = function(e) {
        const data = JSON.parse(e.data);
        console.log("[socket]", data);
        if (data.type === "a0") {
          document.getElementById("mrdiy_value").innerHTML = data.value;
        } else if (data.type === "d0") {
          document.getElementById("pin_status").innerHTML = data.value ? "HIGH" : "LOW";
        }
      };

      function togglePin() {
        socket.send('toggle');
      }


      // Graf kode
      let wattageChart;
 
      // Function to create the wattage chart
      function createWattageChart() {
        const ctx = document.getElementById('wattageChart').getContext('2d');
        wattageChart = new Chart(ctx, {
          type: 'bar',
          data: {
            labels: Array.from({ length: 24 }, (_, i) => `${i}:00`),
            datasets: [{
              label: 'Gennemsnitlig effekt (W) pr. time',
              data: Array(24).fill(0),
              backgroundColor: 'rgba(75, 192, 192, 0.2)',
              borderColor: 'rgba(75, 192, 192, 1)',
              borderWidth: 1
            }]
          },
          options: {
            scales: {
              y: {
                beginAtZero: true
              }
            }
          }
        });
      }
      
      // Function to generate colors for the chart based on wattage data
      function generateColors(wattageData) {
        return wattageData.map(watt => {
          if (watt <= 3) {
            return { backgroundColor: "rgba(255, 99, 132, 0.2)", borderColor: "rgba(255, 99, 132, 1)" };
          } else if (watt <= 6) {
            return { backgroundColor: "rgba(255, 206, 86, 0.2)", borderColor: "rgba(255, 206, 86, 1)" };
          } else {
            return { backgroundColor: "rgba(75, 192, 192, 0.2)", borderColor: "rgba(75, 192, 192, 1)" };
          }
        });
      }

      // Function to update the wattage chart data
      function updateWattageChartData(wattageData) {
        const colors = generateColors(wattageData);
        wattageChart.data.datasets[0].data = wattageData;
        wattageChart.data.datasets[0].backgroundColor = colors.map(color => color.backgroundColor);
        wattageChart.data.datasets[0].borderColor = colors.map(color => color.borderColor);
        wattageChart.update();
      }

      // Create the wattage chart when the webpage is loaded
      document.addEventListener("DOMContentLoaded", createWattageChart);

      setTimeout(() => updateWattageChartData([2, 6, 1, 8, 5, 9, 7, 4, 10, 3, 5, 8, 1, 6, 7, 9, 3, 2, 10, 4, 5, 6, 7, 3]), 1000);
      
    </script>
  </head>
   <body>
    <h1>Vertikal vindmølle software 💨</h1>

   
    <div class="container">
      <div class="column">
        <div class="card">
          <h2>Værdien af A0</h2>
          <p id="mrdiy_value"></p>
        </div>
      </div>

      <div class="column">
        <div class="card">
          <h2>Vindhastighed i Valby</h2>
          <p id="wind_speed"></p>
        </div>
      </div>
      
    </div>

      <div class="chart">
          <canvas id="wattageChart"></canvas>
      </div>

    <footer>
      Udviklet af Valdemar 3.E & Victor 3.M
    </footer>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>

  </body>

</html>
)=====";


void handleMain() {
  server.send_P(200, "text/html", html_template);
}
void handleNotFound() {
  server.send(404, "text/html", "<html><body><p>404 siden kunne ikke findes</p></body></html>");
}

void loop() {
  webSocket.loop();
  server.handleClient();

  value = analogRead(A0);
  float voltage = ((float) value)/1024.0*3.3*2;
  webSocket.broadcastTXT(String("{\"type\":\"a0\",\"value\":\"" + String(voltage) + "\"}").c_str());
  delay(50);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // Send initial values to client
        value = analogRead(A0);
        float voltage = ((float) value)/1024.0*3.3*2;
        webSocket.sendTXT(num, String("{\"type\":\"a0\",\"value\":\"" + String(voltage) + "\"}").c_str());
        
        String pinStatusStr = pinStatus ? "true" : "false";
        webSocket.sendTXT(num, String("{\"type\":\"d0\",\"value\":" + pinStatusStr + "}").c_str());
      }
      break;

    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);

      if (strcmp((char *)payload, "toggle") == 0) {
        pinStatus = !pinStatus;
        digitalWrite(D0, pinStatus ? HIGH : LOW);
        String pinStatusStr = pinStatus ? "true" : "false";
        webSocket.broadcastTXT(String("{\"type\":\"d0\",\"value\":" + pinStatusStr + "}").c_str());
      }

      break;

    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\n", num, length);
      hexdump(payload, length);
      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(A0, INPUT);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Forbundet til: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.on("/", handleMain);
  server.onNotFound(handleNotFound);
  server.begin();
}
