#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const char* ssid = "Shikhar";
const char* password = "12345678";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 
#define RAIN_ANALOG_PIN 34 
#define RAIN_DIGITAL_PIN 35 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_BME280 bme; 
WebServer server(80); 

float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;
int rainPercentage = 0;
String rainStatus = "Dry";

unsigned long previousMillis = 0;
const long interval = 2000;

void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<meta http-equiv=\"refresh\" content=\"5\">"; 
  html += "<style>body { font-family: Arial; text-align: center; margin-top: 50px; }";
  html += "h1 { color: #333333; } .data { font-size: 24px; color: #0066cc; margin: 10px; }</style></head>";
  html += "<body><h1>ESP32 Weather Station</h1>";
  html += "<div class=\"data\">Temperature: " + String(temperature) + " &deg;C</div>";
  html += "<div class=\"data\">Humidity: " + String(humidity) + " %</div>";
  html += "<div class=\"data\">Pressure: " + String(pressure) + " hPa</div>";
  html += "<hr style=\"width:50%;\">";
  html += "<div class=\"data\">Status: " + rainStatus + "</div>";
  html += "<div class=\"data\">Rain Intensity: " + String(rainPercentage) + " %</div>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(RAIN_DIGITAL_PIN, INPUT);

  // init OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;); 
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Starting up...");
  display.display();

  // init BME280
  if (!bme.begin(0x76)) { 
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  WiFi.begin(ssid, password);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Connecting to WiFi");
  display.display();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  server.on("/", handleRoot);
  server.begin();
  
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // read sensors
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F; 

    if (digitalRead(RAIN_DIGITAL_PIN) == LOW) {
      rainStatus = "RAINING!";
    } else {
      rainStatus = "Dry";
    }

    int rawRainValue = analogRead(RAIN_ANALOG_PIN);
    Serial.println("Raw ADC: " + String(rawRainValue)); 
    
    rainPercentage = map(rawRainValue, 4095, 0, 0, 100); 
    
    if(rainPercentage < 0) rainPercentage = 0;
    if(rainPercentage > 100) rainPercentage = 100;

    // update display
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.println("---------------------");
    
    display.print("Temp: "); display.print(temperature); display.println(" C");
    display.print("Hum:  "); display.print(humidity); display.println(" %");
    display.print("Pres: "); display.print(pressure); display.println(" hPa");
    
    display.print("Rain: "); 
    if (rainStatus == "Dry") {
      display.println("Dry");
    } else {
      display.print(rainPercentage); display.println("%");
    }
    
    display.display();
  }
}