#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);  

const char* ssid = "Pomodoro Timer";

const char* PARAM_SESSIONS = "sessions";
const char* PARAM_FOCUS = "focus";
const char* PARAM_BREAK = "break";

unsigned short int sessions, focusTime, breakTime, passedTime = 0, passedSessions = 0;
bool started = false;
bool focusing = true;

const unsigned short int red = 25, green = 26, blue = 27, pauseButton = 35, buzzer = 32;

AsyncWebServer server(80);


void lightRBG(int r, int g, int b) {
  analogWrite(red,   r);
  analogWrite(green, g);
  analogWrite(blue,  b);
}

String secondsToTimestamp(int totalSeconds) {
    // Calculate hours, minutes, and seconds
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    // Format the timestamp as a string
    char timestamp[9]; // "hh:mm:ss" requires 9 characters including null terminator
    snprintf(timestamp, sizeof(timestamp), "%02d:%02d:%02d", hours, minutes, seconds);

    return String(timestamp);
}

int stringToInt(const String& str) {
    int result = 0;
    bool isNegative = false;
    int i = 0;

    // Check if the number is negative
    if (str[0] == '-') {
        isNegative = true;
        i++;
    }

    // Convert each character to its numeric value
    for (; i < str.length(); i++) {
        char c = str[i];
        if (c < '0' || c > '9') {
            // Handle invalid characters (optional)
            return 0; // or any other error handling
        }
        result = result * 10 + (c - '0');
    }

    return isNegative ? -result : result;
}

void printOnLCD(auto message, int row, int col) {
  lcd.setCursor(col, row);
  lcd.print(message);
}

String getPage(String content) {
  String html = "<!DOCTYPE html><html><head>";
  // ... styling and navigation ...
  html += content;
  html += "</div></body></html>";
  return html;
}

void playIntro() {
  tone(buzzer, 440);
  delay(500);
  tone(buzzer, 494);
  delay(500);
  tone(buzzer, 523);
  delay(500);
  tone(buzzer, 587);
  delay(500);
  tone(buzzer, 659);
  delay(500);
  noTone(buzzer);
}

void playUpdateSound() {
  tone(buzzer, 659, 500);
  delay(500);
  tone(buzzer, 659, 500);
}

void setup(){
  Serial.begin(115200);

  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);

  pinMode(pauseButton, INPUT);

  pinMode(buzzer, INPUT_PULLUP);

  playIntro();

  lightRBG(255, 0, 0);

  lcd.init();
  lcd.backlight();
  
  printOnLCD("Booting up!", 0, 0);

  delay(3000);
  lcd.clear();

  WiFi.softAP(ssid);
  server.on("/", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String content = R"rawliteral(
      <div style="font-family: 'Times New Roman', serif; font-weight: bold; background: #D5D8DC; color: #2E4053; max-width: 400px; margin: 40px auto; padding: 30px; border-radius: 15px; box-shadow: 0 10px 30px rgba(46, 64, 83, 0.2);">
      <h1 style="color: #2E4053; text-align: center; font-size: 2.5em; margin-bottom: 20px; letter-spacing: 1px;">Pomodoro Timer</h1>
      <form action="/set" style="display: flex; flex-direction: column; gap: 15px;">
        <div style="display: flex; justify-content: space-between; align-items: center;">
          <label for="sessions" style="color: #2E4053; font-size: 1.8em;">Sessions:</label>
          <input type="number" name="sessions" id="sessions" min="1" style="width: 60px; padding: 8px; border: none; border-bottom: 2px solid #F1C40F; background: transparent; color: #2E4053; font-family: 'Times New Roman', serif; font-weight: bold; font-size: 1em; text-align: center;">
        </div>
        <div style="display: flex; justify-content: space-between; align-items: center;">
          <label for="focus" style="color: #2E4053; font-size: 1.8em;">Focus (min):</label>
          <input type="number" name="focus" id="focus" min="1" style="width: 60px; padding: 8px; border: none; border-bottom: 2px solid #F1C40F; background: transparent; color: #2E4053; font-family: 'Times New Roman', serif; font-weight: bold; font-size: 1em; text-align: center;">
        </div>
        <div style="display: flex; justify-content: space-between; align-items: center;">
          <label for="break" style="color: #2E4053; font-size: 1.8em;">Break (min):</label>
          <input type="number" name="break" id="break" min="1" style="width: 60px; padding: 8px; border: none; border-bottom: 2px solid #F1C40F; background: transparent; color: #2E4053; font-family: 'Times New Roman', serif; font-weight: bold; font-size: 1em; text-align: center;">
        </div>
        <input type="submit" value="Start" style="width: 100%; padding: 12px; background-color: #F1C40F; color: #2E4053; border: none; border-radius: 5px; cursor: pointer; font-family: 'Times New Roman', serif; font-weight: bold; font-size: 2.1em; margin-top: 10px; transition: all 0.3s ease; box-shadow: 0 4px 6px rgba(46, 64, 83, 0.1);">
      </form>
    </div>
    )rawliteral";

    lcd.clear();
    printOnLCD("Waiting for", 0, 0);
    printOnLCD("instructions", 1, 0);
  
    request->send(200, "text/html", getPage(content));
  });
  server.on("/set", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String sessionsStr, focusTimeStr, breakTimeStr;
    if (request->hasParam(PARAM_SESSIONS)) {
      sessionsStr = request->getParam(PARAM_SESSIONS)->value();
    } else {
      sessionsStr = "1";
    }
    if (request->hasParam(PARAM_FOCUS)) {
      focusTimeStr = request->getParam(PARAM_FOCUS)->value();
    } else {
      focusTimeStr = "45";
    }
    if (request->hasParam(PARAM_BREAK)) {
      breakTimeStr = request->getParam(PARAM_BREAK)->value();
    } else {
      breakTimeStr = "15";
    }

    passedTime = 0;
    passedSessions = 0;
    focusing = true;
    sessions = stringToInt(sessionsStr);
    focusTime = stringToInt(focusTimeStr)*60;
    breakTime = stringToInt(breakTimeStr)*60;
    started = true;

    lightRBG(0, 0, 255);
    playUpdateSound();


    // Respond with a confirmation message
    request->send(200, "text/html", "Settings received:<br>" 
                                     "Sessions: " + sessionsStr + "<br>" 
                                     "Focus Time: " + focusTimeStr + "<br>" 
                                     "Break Time: " + breakTimeStr +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.begin();

  printOnLCD("Connect to WIFI", 0, 0);
  printOnLCD("Pomodoro Timer", 1, 0);

  delay(3000);
  lcd.clear();


  IPAddress IP = WiFi.softAPIP();  
  printOnLCD("IP:", 0, 0);
  printOnLCD(IP, 1, 0);
}

void loop(){

  if (digitalRead(pauseButton) == LOW) {
    started = !started;
    delay(2000);
  }

  if (started) {
    passedTime++;
  
    if (passedTime == focusTime && focusing) {
      passedTime = 0;
      focusing = false;
      lightRBG(0, 255, 0);
      playUpdateSound();
    }
    if (passedTime == breakTime && !focusing) {
      passedSessions++;
      passedTime = 0;
      focusing = true;
      lightRBG(0, 0, 255);
      playUpdateSound();
    }

    lcd.clear();


    if (passedSessions == sessions) {
      started = false;
      lcd.clear();
      printOnLCD("Great job!", 0, 0);
      playUpdateSound();
      delay(500);
      playUpdateSound();

    } else if (focusing) {
      String msg = "Focus... ";
      msg += passedSessions;
      msg += "/";
      msg += sessions;
      printOnLCD(msg, 0, 0);
      printOnLCD(secondsToTimestamp(focusTime - passedTime), 1, 0);

    } else if (!focusing) {
      String msg = "Relax... ";
      msg += passedSessions;
      msg += "/";
      msg += sessions;
      printOnLCD(msg, 0, 0);
      printOnLCD(secondsToTimestamp(breakTime - passedTime), 1, 0);
    }
    delay(1000);
  }
}