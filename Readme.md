# Pomodoro timer

This project is implementation of the Pomodoro technique using an ESP32 microcontroller. The Pomodoro technique is a time management method developed by Francesco Cirillo in the late 1980s. The technique uses a timer to break down work into intervals, traditionally 25 minutes in length, separated by short breaks. These intervals are named pomodoros.

## Features
- Web server to control the timer and settings
    - Choose the duration of the work and break intervals as well as the number of pomodoros
    - Start the timer
    - Reset the timer
- LCD display to show the current state of the timer
- RGB LED to show whether the timer is in work or break mode
- Buzzer to signal the end of the work or break interval and the end of the pomodoros
- Button to start and stop the timer

## Usage
1. Power on the ESP32
2. Connect to the WiFi network `Pomodoro Timer`
3. Open a web browser and navigate to `192.168.4.1`. This is the default IP address of the ESP32 web server
4. Set the duration of the work and break intervals as well as the number of pomodoros
5. Start the timer by clicking the `Start` button

## Components
- ESP32 microcontroller
- LCD 16x2 with I2C module
- Button module
- RGB LED module
- Buzzer

## Libraries
- WebServer.h
- LiquidCrystal_I2C.h by Frank de Brabander
- EspAsyncWebServer.h by Me-No-Dev
- AsyncTCP.h by Me-No-Dev

## Connections

### LCD 16x2 with I2C module

| LCD | ESP32 |
|-----|-------|
| GND | GND   |
| VCC | 5V    |
| SDA | D21   |
| SCL | D22   |


### Button module

| Button | ESP32 |
|--------|-------|
| GND    | GND   |
| VCC    | 3.3V  |
| OUT    | D35   |


### RGB LED module

| RGB LED | ESP32 |
|---------|-------|
| GND     | GND   |
| R       | D25   |
| G       | D26   |
| B       | D27   |

### Buzzer

| Buzzer | ESP32 |
|--------|-------|
| -      |  GND  |
| +      |  D32  |


## License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.