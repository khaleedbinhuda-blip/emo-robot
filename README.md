# 🤖 Emo Robot — Expressive OLED Eyes on ESP32

An emotion-driven robot face built with an ESP32 and a 128×64 OLED display. The robot reacts to touch and can be controlled wirelessly via Bluetooth Serial. Built with the [FluxGarage RoboEyes](https://github.com/FluxGarage/RoboEyes) library.

---

## ✨ Features

- **8 distinct emotions** — Happy, Angry, Sad, Excited, Tired, Confused, Love, Default
- **Touch-reactive** — short tap = Happy, hold 1s = Angry, hold 3s = Love, release = back to Default
- **Bluetooth Serial control** — send single-character commands from your phone or PC
- **Smooth animations** — shake (Happy), bounce (Excited), tilt (Confused) eye movements
- **Auto-blink & idle** — each emotion has tuned blink rate and idle wandering behaviour
- **State guard** — re-applying the same emotion is a no-op; no redundant display writes

---

## 🔧 Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 DevKit V1 (any 38-pin variant) |
| Display | SSD1306 OLED 128×64, I2C |
| Touch input | Capacitive touch sensor module *or* bare wire on GPIO4 |
| Power | USB 5V via micro-USB or 3.7V LiPo with boost converter |

---

## 📌 Wiring

```
SSD1306 OLED   →   ESP32
-----------        ------
VCC            →   3.3V
GND            →   GND
SDA            →   GPIO 21
SCL            →   GPIO 22

Touch Sensor   →   ESP32
------------       ------
VCC            →   3.3V
GND            →   GND
OUT / SIG      →   GPIO 4
```

> **Tip:** If your OLED I2C address is `0x3D` instead of `0x3C`, change it in `display.begin()`.  
> **Tip:** For wire runs longer than ~30 cm, add 4.7 kΩ pull-up resistors on SDA and SCL to 3.3V.

See `docs/circuit_diagram.png` for the full schematic.

---

## 📚 Libraries Required

Install all of these through Arduino IDE → *Sketch → Include Library → Manage Libraries*:

| Library | Install name |
|---|---|
| Adafruit SSD1306 | `Adafruit SSD1306` |
| Adafruit GFX | `Adafruit GFX Library` |
| FluxGarage RoboEyes | `RoboEyes` (search "RoboEyes") |
| BluetoothSerial | Built into ESP32 Arduino core |

---

## 🚀 Setup & Upload

1. Install [Arduino IDE](https://www.arduino.cc/en/software) (v1.8+ or v2.x)
2. Add ESP32 board support: *File → Preferences → Additional Board Manager URLs* → paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. Go to *Tools → Board → Boards Manager*, search **esp32**, install
4. Select board: *Tools → Board → ESP32 Dev Module*
5. Install all required libraries (see above)
6. Clone this repo and open `src/emo_robot.ino`
7. Select the correct COM port and click **Upload**

---

## 🎮 Bluetooth Control

The robot advertises as **`DoraBrain`** over Bluetooth Classic.

**How to connect:**
- Android: pair with `DoraBrain` in Bluetooth settings, then use a Serial Bluetooth Terminal app
- PC: pair, then open the COM port at 115200 baud in any serial terminal

**Commands (single key):**

| Key | Emotion |
|---|---|
| `h` | 😄 Happy |
| `a` | 😠 Angry |
| `s` | 😢 Sad |
| `e` | 🤩 Excited |
| `t` | 😴 Tired |
| `c` | 😕 Confused |
| `l` | 🥰 Love |
| `d` | 😐 Default |

You can also send commands through the Arduino IDE Serial Monitor at **115200 baud**.

---

## 🧠 How It Works

### State machine
Each emotion is a state. `applyEmotion()` is guarded — calling it with the current emotion does nothing. `resetToDefault()` forces a transition by temporarily setting an invalid state.

### Touch behaviour
```
Touch starts  → Happy (+ shake animation)
Hold 1 second → Angry
Hold 3 seconds→ Love
Touch released→ Default (reset)
```

### Animations
Three non-blocking timers (shake, bounce, tilt) run alongside `roboEyes.update()` in the main loop. They use `millis()` — no `delay()` anywhere, so the display stays responsive.

---

## 📁 Project Structure

```
emo-robot/
├── src/
│   └── emo_robot.ino       # Main Arduino sketch
├── docs/
│   └── circuit_diagram.png # Wiring schematic
├── README.md
└── LICENSE
```

---

## 🔮 Ideas for Future Upgrades

- [ ] Add a microphone + sound-reactive emotion (loud noise → Angry)
- [ ] Servo motors for physical head tilt on Confused
- [ ] Mobile app with named emotion buttons instead of letter commands
- [ ] Add more emotions: Surprised, Sleepy, Winking
- [ ] Save last emotion to NVS (non-volatile storage) for power-cycle memory

---

## 📄 License

MIT License — free to use, modify, and share. See `LICENSE` for details.

---

## 🙏 Credits

- [FluxGarage RoboEyes](https://github.com/FluxGarage/RoboEyes) — the eye-animation engine that makes this possible
- [Adafruit](https://github.com/adafruit) — SSD1306 and GFX libraries
- Built by **Khaleed** as a fun embedded systems project
