# 🌟 ESP LED Mesh Controller 🌟

```

   ,--,                                                                                                                              
,---.'|                                            ____                                ,--,                              ,-.----.    
|   | :       ,---,.    ,---,                    ,'  , `.    ,---,.  .--.--.         ,--.'|            ,---,.  .--.--.   \    /  \   
:   : |     ,'  .' |  .'  .' `\               ,-+-,.' _ |  ,'  .' | /  /    '.    ,--,  | :          ,'  .' | /  /    '. |   :    \  
|   ' :   ,---.'   |,---.'     \           ,-+-. ;   , ||,---.'   ||  :  /`. / ,---.'|  : '        ,---.'   ||  :  /`. / |   |  .\ : 
;   ; '   |   |   .'|   |  .`\  |         ,--.'|'   |  ;||   |   .';  |  |--`  |   | : _' |        |   |   .';  |  |--`  .   :  |: | 
'   | |__ :   :  |-,:   : |  '  |        |   |  ,', |  '::   :  |-,|  :  ;_    :   : |.'  |        :   :  |-,|  :  ;_    |   |   \ : 
|   | :.'|:   |  ;/||   ' '  ;  :        |   | /  | |  ||:   |  ;/| \  \    `. |   ' '  ; :        :   |  ;/| \  \    `. |   : .   / 
'   :    ;|   :   .''   | ;  .  |        '   | :  | :  |,|   :   .'  `----.   \'   |  .'. |        |   :   .'  `----.   \;   | |`-'  
|   |  ./ |   |  |-,|   | :  |  '        ;   . |  ; |--' |   |  |-,  __ \  \  ||   | :  | '        |   |  |-,  __ \  \  ||   | ;     
;   : ;   '   :  ;/|'   : | /  ;         |   : |  | ,    '   :  ;/| /  /`--'  /'   : |  : ;        '   :  ;/| /  /`--'  /:   ' |     
|   ,/    |   |    \|   | '` ,/          |   : '  |/     |   |    \'--'.     / |   | '  ,/         |   |    \'--'.     / :   : :     
'---'     |   :   .';   :  .'            ;   | |`-'      |   :   .'  `--'---'  ;   : ;--'          |   :   .'  `--'---'  |   | :     
          |   | ,'  |   ,.'              |   ;/          |   | ,'              |   ,/              |   | ,'              `---'.|     
          `----'    '---'                '---'           `----'                '---'               `----'                  `---`     
                                                                                                                                     

```

Turn your ESP devices into a synchronized LED light show! 🎪✨

## 🎯 Features

- 🔄 Self-organizing mesh network
- 🎨 Synchronized LED patterns
- 🔧 No hub required - all devices run identical code
- 📡 Over-the-air updates
- 🎛️ Physical controls with status feedback
- ⚡ Fast ESP-NOW communication

## 🛠️ Hardware Requirements

### Per Device
- ESP32 development board
- WS2812B LED strip
- 6x momentary push buttons
- 74HC595 shift register
- 6x status LEDs
- Various resistors and wires

## 📌 Pin Configuration

```
ESP32 Pin Map
┌────────────────┐
│ LED Strip  : 5 │
│ Shift Data :13 │    Button Layout
│ Shift Clock:14 │    ┌─┐ ┌─┐ ┌─┐
│ Shift Latch:15 │    │1│ │2│ │3│
│ Buttons:       │    └─┘ └─┘ └─┘
│  1: 16        │    ┌─┐ ┌─┐ ┌─┐
│  2: 17        │    │4│ │5│ │6│
│  3: 18        │    └─┘ └─┘ └─┘
│  4: 19        │
│  5: 21        │    Status LEDs
│  6: 22        │    ○ ○ ○ ○ ○ ○
└────────────────┘
```

## 🚀 Getting Started

1. Clone this repository
```bash
git clone https://github.com/jackson-dean-cricut/overkill-nametag-lights.git
```

2. Install required libraries
```bash
# In Arduino IDE:
- FastLED
- ESP32 Board Support
```

3. Upload the code to your ESP32s
   - They'll automatically organize themselves into a mesh
   - First device to boot becomes the coordinator

## 💡 How It Works

Each device controls its own LED strip segment and communicates changes to all other devices in the mesh. The system uses ESP-NOW for fast, reliable communication between devices.

```
Device 1 ⟷ Device 2 ⟷ Device 3
   ↕         ↕         ↕
Device 4 ⟷ Device 5 ⟷ Device 6
```

## 🎮 Controls

Each device has:
- 6 buttons to control different sections of the LED strip
- 6 status LEDs showing current section states
- Automatic sync with all other devices in range

## 🔄 OTA Updates

Update any device wirelessly:
1. Connect to the ESP's WiFi network
2. Upload new firmware through Arduino IDE
3. Watch the pretty progress indicator on the LED strip! 🌈

## 🛠️ Future Improvements

- [ ] Add more complex light patterns
- [ ] Implement mesh-based update distribution
- [ ] Add configuration storage
- [ ] Create mobile app control interface
- [ ] Add sound reactivity

## 🐛 Troubleshooting

If devices aren't syncing:
1. Check if they're within range
2. Verify they're all running the same firmware
3. Watch the status LEDs for communication indicators
4. Try turning it off and on again (yes, really! 😄)

## 📝 License

This project is licensed under the MIT License - see the LICENSE file for details

## 🙏 Acknowledgments

- FastLED library developers
- ESP32 community
- Coffee ☕

## 🤝 Contributing

Pull requests are welcome! For major changes, please open an issue first to discuss what you would like to change.

Remember the first rule of LED projects: Don't look directly at the LEDs! 👀✨

---
