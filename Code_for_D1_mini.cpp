#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

// WiFi Configuration
const char* ssid = "Your_WiFi_SSID";       
const char* password = "Your_WiFi_Password"; 
const char* hostname = "SmartCurtains";          

// Pin definitions
const int stepPin = D8;
const int dirPin = D7;
const int buttonUp = D1;        // Button for opening
const int buttonDown = D2;      // Button for closing
const int ledPin = LED_BUILTIN; // Builti-in LED on wemos D1 mini for configuration mode

// Motor parameters
const int stepsPerRevolution = 200;
const int microstepsPerStep = 16;
const int motorSpeed = 1000;

// Global variables
int currentPosition = 0;
int targetPosition = 0;
int fullOpenPosition = 5000;
int fullClosePosition = 0;
bool isMoving = false;
bool configMode = false;
bool ledState = false;
unsigned long ledBlinkTime = 0;

// Button handling variables
int buttonUpPrevState = HIGH;
int buttonDownPrevState = HIGH;
unsigned long buttonPressStartTime = 0;
bool bothButtonsPressed = false;
bool upPressed_prev = false;
bool downPressed_prev = false;

ESP8266WebServer server(80);
// EEPROM addresses
const int EEPROM_OPEN_ADDR = 0;
const int EEPROM_CLOSE_ADDR = 4;

void setup() {
  Serial.begin(115200);
  // EEPROM initialization
  EEPROM.begin(512);
  // Load saved positions from EEPROM
  loadPositionsFromEEPROM();
  
  // Pin configuration
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  
  // Static IP configuration (change acording to your)
  IPAddress staticIP(192, 168, 0, 101);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);
  WiFi.config(staticIP, gateway, subnet, dns);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);
  
  // Wait for connection
  int wifiAttempts = 0;
  while (WiFi.status() != WL_CONNECTED && wifiAttempts < 20) {
    delay(500);
    wifiAttempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    setupWebServer();
  }
  
  // Signal ready state - flash LED
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(100);
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    server.handleClient();
  }
  handleButtons();
  
  // Blink LED in config mode
  if (configMode) {
    if (millis() - ledBlinkTime > 500) {
      ledState = !ledState;
      digitalWrite(ledPin, ledState ? LOW : HIGH);
      ledBlinkTime = millis();
    }
  }
  if (isMoving) {
    moveMotor();
  }
}

// Load saved positions from EEPROM
void loadPositionsFromEEPROM() {
  int savedOpenPosition = 0;
  int savedClosePosition = 0;
  
  EEPROM.get(EEPROM_OPEN_ADDR, savedOpenPosition);
  EEPROM.get(EEPROM_CLOSE_ADDR, savedClosePosition);
  
  if (savedOpenPosition > 0 && savedOpenPosition < 100000) {
    fullOpenPosition = savedOpenPosition;
  }
  
  if (savedClosePosition >= 0 && savedClosePosition < 100000) {
    fullClosePosition = savedClosePosition;
  }
  
  // Make sure open position is greater than closed position
  if (fullOpenPosition <= fullClosePosition) {
    // Default values in case if something is wrong
    fullOpenPosition = 5000;
    fullClosePosition = 0;
  }
  
  // Set default position to closed
  currentPosition = fullClosePosition;
  targetPosition = fullClosePosition;
}

// Save positions to EEPROM
void savePositionsToEEPROM() {
  EEPROM.put(EEPROM_OPEN_ADDR, fullOpenPosition);
  EEPROM.put(EEPROM_CLOSE_ADDR, fullClosePosition);
  EEPROM.commit();
}

// Button handling function
void handleButtons() {
  // Read button states
  bool upPressed = digitalRead(buttonUp) == LOW;
  bool downPressed = digitalRead(buttonDown) == LOW;
  
  // Handle simultaneous button press (mode switching)
  if (upPressed && downPressed) {
    if (isMoving) {
      isMoving = false;
      targetPosition = currentPosition;
    }
    
    if (!bothButtonsPressed) {
      // Start countdown
      buttonPressStartTime = millis();
      bothButtonsPressed = true;
    } else {
      // Check if 3 seconds have passed
      unsigned long elapsedTime = millis() - buttonPressStartTime;
      
      if (elapsedTime >= 3000) {
        // Change mode
        configMode = !configMode;
        
        // Set LED accordingly
        digitalWrite(ledPin, configMode ? LOW : HIGH);
        
        // When exiting config mode, save settings to EEPROM
        if (!configMode) {
          savePositionsToEEPROM();
        }
        bothButtonsPressed = false;
        delay(10);
        while (digitalRead(buttonUp) == LOW || digitalRead(buttonDown) == LOW) {
          delay(10);
        }
        
        // Additional delay to avoid bounce
        delay(100);
        return;
      }
    }
    
    upPressed_prev = upPressed;
    downPressed_prev = downPressed;
    return;
  } else {
    if (bothButtonsPressed) {
      bothButtonsPressed = false;
    }
    
    // CONFIG MODE button handling
    if (configMode) {
      // Static variables for config mode
      static unsigned long upPressStartTime = 0;
      static unsigned long downPressStartTime = 0;
      static bool upWaitingForAction = false;
      static bool downWaitingForAction = false;
      
      if (upPressed && !upPressed_prev) {
        upPressStartTime = millis();
        upWaitingForAction = true;
        
        if (isMoving) {
          isMoving = false;
          targetPosition = currentPosition;
          upWaitingForAction = false;
        }
      }
      
      // UP button - detect long press
      if (upPressed && upWaitingForAction) {
        unsigned long pressDuration = millis() - upPressStartTime;
        
        if (pressDuration >= 3000) {
          fullOpenPosition = currentPosition;
          
          // Flash LED for confirmation
          for (int i = 0; i < 3; i++) {
            digitalWrite(ledPin, HIGH);
            delay(100);
            digitalWrite(ledPin, LOW);
            delay(100);
          }
          upWaitingForAction = false;
        }
      }
      
      // UP button - detect release (short press)
      if (!upPressed && upPressed_prev && upWaitingForAction) {
        unsigned long pressDuration = millis() - upPressStartTime;
        
        if (pressDuration < 1000) {
          targetPosition = currentPosition + 10000;
          isMoving = true;
        }
        upWaitingForAction = false;
      }
      
      // DOWN button - detect press
      if (downPressed && !downPressed_prev) {
        downPressStartTime = millis();
        downWaitingForAction = true;
        
        if (isMoving) {
          isMoving = false;
          targetPosition = currentPosition;
          downWaitingForAction = false;
        }
      }
      
      // DOWN button - detect long press
      if (downPressed && downWaitingForAction) {
        unsigned long pressDuration = millis() - downPressStartTime;
        
        if (pressDuration >= 3000) {
          fullClosePosition = currentPosition;
          
          for (int i = 0; i < 3; i++) {
            digitalWrite(ledPin, HIGH);
            delay(100);
            digitalWrite(ledPin, LOW);
            delay(100);
          }
          
          downWaitingForAction = false;
        }
      }
      
      // DOWN button - detect release (short press)
      if (!downPressed && downPressed_prev && downWaitingForAction) {
        unsigned long pressDuration = millis() - downPressStartTime;
        
        if (pressDuration < 1000) {
          targetPosition = currentPosition - 10000; // Large negative value, will stop with button
          isMoving = true;
        }
        downWaitingForAction = false;
      }
    } 
    // NORMAL MODE button handling
    else {
      // Static variables for normal mode
      static unsigned long upNormalPressStartTime = 0;
      static unsigned long downNormalPressStartTime = 0;
      static bool upNormalWaitingForAction = false;
      static bool downNormalWaitingForAction = false;
      
      // UP button - detect press
      if (upPressed && !upPressed_prev) {
        upNormalPressStartTime = millis();
        upNormalWaitingForAction = true;
        
        if (isMoving) {
          isMoving = false;
          targetPosition = currentPosition;
          upNormalWaitingForAction = false;
        }
      }
      
      // UP button - detect release (short press)
      if (!upPressed && upPressed_prev && upNormalWaitingForAction) {
        unsigned long pressDuration = millis() - upNormalPressStartTime;
        
        if (pressDuration < 1000) {
          targetPosition = fullOpenPosition;
          isMoving = true;
        }
        
        upNormalWaitingForAction = false;
      }
      
      if (downPressed && !downPressed_prev) {
        downNormalPressStartTime = millis();
        downNormalWaitingForAction = true;
        
        if (isMoving) {
          isMoving = false;
          targetPosition = currentPosition;
          downNormalWaitingForAction = false;
        }
      }
      
      // DOWN button - detect release (short press)
      if (!downPressed && downPressed_prev && downNormalWaitingForAction) {
        unsigned long pressDuration = millis() - downNormalPressStartTime;
        
        if (pressDuration < 1000) {
          targetPosition = fullClosePosition;
          isMoving = true;
        }
        downNormalWaitingForAction = false;
      }
    }
  }
  
  upPressed_prev = upPressed;
  downPressed_prev = downPressed;
}

// Configure HTTP server
void setupWebServer() {
  // Home page - control interface
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Blind Controller</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; text-align: center; margin: 0; padding: 20px; }";
    html += "h1 { color: #0066CC; }";
    html += ".btn { background-color: #0066CC; color: white; border: none; padding: 15px 25px; ";
    html += "margin: 10px; font-size: 16px; border-radius: 5px; cursor: pointer; }";
    html += ".btn:hover { background-color: #0055AA; }";
    html += ".slider { width: 80%; margin: 30px auto; }";
    html += ".info { margin: 20px; padding: 10px; background-color: #f0f0f0; border-radius: 5px; }";
    html += "</style>";
    html += "</head><body>";
    html += "<h1>Blind Controller</h1>";
    
    // Buttons for full open/close
    html += "<div>";
    html += "<button class='btn' onclick='window.location.href=\"/up\"'>Fully Open</button>";
    html += "<button class='btn' onclick='window.location.href=\"/down\"'>Fully Close</button>";
    html += "<button class='btn' onclick='window.location.href=\"/stop\"'>Stop</button>";
    html += "</div>";
    
    // Slider for position setting
    int minPos = fullClosePosition;
    int maxPos = fullOpenPosition;
    html += "<div class='slider'>";
    html += "<input type='range' min='" + String(minPos) + "' max='" + String(maxPos) + "' value='" + String(currentPosition) + "' id='posSlider' ";
    html += "oninput='updatePosition(this.value)' onchange='setPosition(this.value)'>";
    html += "</div>";
    
    // Current position info
    html += "<div class='info'>";
    html += "Current position: <span id='posValue'>" + String(currentPosition) + "</span> / " + String(maxPos);
    html += "<br>Range: " + String(minPos) + " - " + String(maxPos);
    html += "<br>Status: " + String(isMoving ? "Moving" : "Stopped");
    html += "<br>Mode: " + String(configMode ? "Configuration" : "Normal");
    html += "</div>";
    
    // JavaScript scripts
    html += "<script>";
    html += "function updatePosition(pos) { document.getElementById('posValue').innerHTML = pos; }";
    html += "function setPosition(pos) { window.location.href = '/set?pos=' + pos; }";
    html += "</script>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
  });
  
  // Endpoint for full open
  server.on("/up", HTTP_GET, []() {
    targetPosition = fullOpenPosition;
    isMoving = true;
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  
  // Endpoint for full close
  server.on("/down", HTTP_GET, []() {
    targetPosition = fullClosePosition;
    isMoving = true;
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  
  // Endpoint for stop
  server.on("/stop", HTTP_GET, []() {
    isMoving = false;
    targetPosition = currentPosition;
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  
  // Endpoint for setting specific position
  server.on("/set", HTTP_GET, []() {
    if (server.hasArg("pos")) {
      targetPosition = server.arg("pos").toInt();
      // Limit to range fullClosePosition-fullOpenPosition
      targetPosition = constrain(targetPosition, fullClosePosition, fullOpenPosition);
      isMoving = true;
    }
    server.sendHeader("Location", "/");
    server.send(302, "text/plain", "");
  });
  
  // API endpoint for getting status as JSON
  server.on("/status", HTTP_GET, []() {
    String json = "{";
    json += "\"position\":" + String(currentPosition) + ",";
    json += "\"target\":" + String(targetPosition) + ",";
    json += "\"open_position\":" + String(fullOpenPosition) + ",";
    json += "\"close_position\":" + String(fullClosePosition) + ",";
    json += "\"is_moving\":" + String(isMoving ? "true" : "false") + ",";
    json += "\"config_mode\":" + String(configMode ? "true" : "false");
    json += "}";
    server.send(200, "application/json", json);
  });
  
  // Handle unknown paths
  server.onNotFound([]() {
    server.send(404, "text/plain", "Page not found");
  });
  server.begin();
}

// Motor movement handling
void moveMotor() {
  // Determine direction of movement
  if (currentPosition < targetPosition) {
    if (!configMode && currentPosition >= fullOpenPosition) {
      isMoving = false;
      currentPosition = fullOpenPosition;
      targetPosition = fullOpenPosition;
      return;
    }
    
    // Upward movement
    digitalWrite(dirPin, HIGH);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
    
    currentPosition++;
  } 
  else if (currentPosition > targetPosition) {
    if (!configMode && currentPosition <= fullClosePosition) {
      isMoving = false;
      currentPosition = fullClosePosition;
      targetPosition = fullClosePosition;
      return;
    }
    
    // Downward movement
    digitalWrite(dirPin, LOW);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(motorSpeed);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(motorSpeed);
    
    currentPosition--;
  } 
  else {
    isMoving = false;
  }
}
