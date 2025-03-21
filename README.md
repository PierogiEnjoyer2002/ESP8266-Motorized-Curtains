# 🪟 AutoBlinds – ESP8266 Motorized Curtains Controller

## Overview
AutoBlinds is a DIY smart curtains controller built using **ESP8266 (Wemos D1 Mini)** and a stepper motor. This project allows you to control your window curtains via a web interface and physical buttons.

## Features
- **Dual Control Methods**
  - Web interface accessible from any device on your network
  - Physical buttons for manual operation
  
- **Smart Positioning System**
  - Precise position control with memory
  - Configuration mode to set open/close limits
  
- **Reliable & Customizable**
  - Position memory persists through power cycles (EEPROM storage)
  - Simple configuration for adaptation to different window sizes
  - JSON API for integration with home automation systems

## Bill of Materials
Here's a complete list of components needed for this project:

| Quantity | Component | Specifications |
|----------|-----------|----------------|
| 1 | NEMA17 Stepper Motor | 1.8° step angle, 2.4 kg/cm holding torque |
| 1 | Wemos D1 Mini | ESP8266-based microcontroller |
| 1 | A4988 Stepper Driver | Pololu or compatible |
| 1 | Mini-360 DC-DC Buck Converter | 12V to 5V for Wemos power |
| 1 | 2.1mm Barrel Jack | With terminal block for power input |
| 2 | Push Buttons | Momentary, normally open |
| 1 | Capacitor | 100uF, 25V (for power stabilization) |
| 1 | Power Supply | 12V DC (not included in list) |
| - | Various Wires | For connections between components |

## How It Works
1. **The system controls a stepper motor** that opens or closes curtains.
2. **Users operate curtains** via web interface or physical buttons.
3. **Position data is saved** to ensure consistent operation over time.

## 3D Printed Components
This project uses several custom 3D printed parts to create a complete curtain control system:

### Electronics Housing
![Electronics Housing](https://github.com/user-attachments/assets/375f0630-2c29-4b1d-a603-a216a4e5caa4)

A custom-designed enclosure for the electronics that:
- Securely houses the Wemos D1 Mini, A4988 driver, and DC-DC converter
- Features openings for power and button connections

### Curtain String Guide
![String Guide](https://github.com/user-attachments/assets/a87de6df-7ef4-40c2-8a71-6d0216517962)

A specially designed bracket that:
- Guides the curtain string through a smooth path
- Reduces friction for smoother operation
- Prevents tangling during operation
- Easily mounts to window frames of various sizes

### NEMA17 Motor Mount
For the motor mount, I used a design by stevetheprinter:
- Original design available on [Thingiverse](https://www.thingiverse.com/stevetheprinter/designs)
- Features secure mounting for NEMA17 stepper motor

*All STL files for the 3D printed components are included in the `/3d_models` directory of this repository.*

## Installation & Setup

### Hardware Assembly
<p float="left">
  <img src="https://github.com/user-attachments/assets/cb732f5c-0fd2-478b-8482-e8111db2d3c6" width="48%" alt="Complete Setup" />
  <img src="https://github.com/user-attachments/assets/43ed4d16-b7e1-4699-9089-2fc834aebec1" width="48%" alt="Controller Box" />
</p>

### Software Setup
1. Copy code from repository

2. Configure WiFi Settings
Edit the following lines in the code to match your network:
```cpp
const char* ssid = "Your_WiFi_SSID";       
const char* password = "Your_WiFi_Password"; 
```

3. Configure Static IP
Edit the following lines if you want to use a different IP:
```cpp
IPAddress staticIP(192, 168, 0, 101);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
```

4. Upload to ESP8266
Upload the code to your Wemos D1 Mini using the Arduino IDE.

## Usage Instructions

### Web Interface
Access the web interface by navigating to the device's IP address (default: 192.168.0.101) in your web browser.

The interface offers:
- Full open/close buttons
- Stop button
- Position slider for precise control
- Current position and status information

### Physical Buttons
- **Normal Mode:**
  - Short press UP button: Fully open blinds
  - Short press DOWN button: Fully close blinds
  - Press both buttons simultaneously for 3 seconds: Enter configuration mode

- **Configuration Mode:**
  - Short press UP button: Move up continuously (stop with any button)
  - Short press DOWN button: Move down continuously (stop with any button)
  - Long press UP button (3+ seconds): Set current position as fully open
  - Long press DOWN button (3+ seconds): Set current position as fully closed
  - Press both buttons simultaneously for 3 seconds: Exit configuration mode and save settings

### API Endpoint
The system provides a JSON API endpoint at `/status` that returns current position, target position, and operational state.

## Project Images
![Complete Setup](https://github.com/user-attachments/assets/29987295-71c2-46f6-b6d6-9a0ad8443a06)
![Controller Box](https://github.com/user-attachments/assets/bcea2902-0e48-4ac9-80b4-3086d0ee6d5f)
![Web Interface](https://github.com/user-attachments/assets/607cacd4-f4b3-4e02-a4c6-9a9d132fecde)

## Video Demonstration
![Video showing blinds operation](https://github.com/user-attachments/assets/65a25d11-5372-433e-80af-dea25c34709f)

## Challenges and Solutions
- **Motor Torque Requirements:** Selected NEMA17 motor with sufficient torque for smooth operation.
- **Position Memory:** Implemented EEPROM storage to maintain settings through power cycles.
- **User-Friendly Interface:** Created an intuitive web interface for easy control.
- **3D Printed Components:** Designed custom parts to ensure proper mechanical operation and clean installation.

## Future Improvements
- Add MQTT support for smart home integration (Home Assistant, etc.)

## Author
- **[PierogiEnjoyer2002](https://github.com/PierogiEnjoyer2002)**

## License
This project is open-source and available under the GPL License.

## Contact
Have questions or suggestions? Open an issue or reach out via email at [kamil.piotr.wojnarski@gmail.com](kamil.piotr.wojnarski@gmail.com).
