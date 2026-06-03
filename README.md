# 🎹 ESP32 Piano Web Server

An interactive digital piano built with **ESP32**, exposing a real-time web interface and generating sound through a buzzer.

The system can be accessed from any browser (optimized for mobile landscape mode) and provides a **2-octave piano keyboard** with continuous touch-drag support.

---

## 📌 Main Features

✅ Embedded web server running on ESP32  
✅ Fullscreen responsive interface  
✅ Optimized for mobile landscape usage  
✅ 2 full octaves (24 keys)  
✅ Sharp notes support (#)  
✅ Continuous sound while pressed  
✅ Slide between keys (touch move support)  
✅ Immediate stop on release  
✅ Text selection disabled  
✅ Minimal and clean visual design  

---

## 🧠 System Architecture

### 1️⃣ Web Layer (Embedded Frontend)

The HTML is embedded directly in `PROGMEM` inside:

```
PianoWebServer.ino
```

It includes:

- Inline CSS
- Inline JavaScript
- Touch and mouse event handling
- Communication via `fetch()` to the ESP32

No external files or dependencies are required.

---

### 2️⃣ Server Layer (ESPAsyncWebServer)

The ESP32 exposes the following endpoints:

### Root Endpoint
```
GET /
```
Returns the piano HTML interface.

### Play Note Endpoint
```
GET /note?note=Do1
```

Example:
```
/note?note=FaS0
```

It:
- Parses note name
- Parses octave
- Calculates frequency
- Executes `tone()`

### Stop Note Endpoint
```
GET /stop
```

Executes:
```
noTone(BUZZER_PIN);
```

---

### 3️⃣ Audio Engine

Frequencies are calculated using scientific pitch notation based on:

```
A4 = 440 Hz
```

Formula used:

```
frequency = 440 * 2^((semitonesFromA4)/12)
```

This allows dynamic multi-octave support.

---

## 🎼 Piano Range

Octave 0:
```
Do0 → Si0
```

Octave 1:
```
Do1 → Si1
```

Sharps included:
```
DoS, ReS, FaS, SolS, LaS
```

Total: **24 notes**

---

## 📱 Touch Behavior

### ✔ Touchstart
- Activates note
- Highlights key
- Starts continuous sound

### ✔ Touchmove
- Detects new key under finger
- Stops previous note
- Activates new note
- Simulates real instrument sliding

### ✔ Touchend
- Stops sound
- Removes active state

---

## 🎨 Visual Design

Pressed white keys:
```
#dddddd
```

Pressed black keys:
```
#555555
```
 
Text selection is disabled using:

```
user-select: none;
touch-action: none;
pointer-events: none;
```

---

## 🔊 Hardware Requirements

- ESP32
- Passive buzzer (recommended)

Configured pin:
```
#define BUZZER_PIN 7
```

---

## 🌐 How to Use

1. Upload the sketch to the ESP32.
2. Open Serial Monitor.
3. Connect to the displayed IP address:
   ```
   Server running at: http://192.168.X.X
   ```
4. Rotate your phone to landscape mode.
5. Start playing.

---

## 📏 Limitations

- The buzzer can only play **one note at a time**.
- Mobile browsers do not allow permanent fullscreen without PWA mode.
- No multitouch support (hardware limitation).

---

## 🚀 Possible Future Improvements

- Convert into a full PWA (standalone app mode)
- Add dynamic octave selector
- Add haptic feedback
- Improve black key positioning
- MIDI support via WebSocket
- Use DAC for improved audio quality

---

## 🧩 Project Structure

```
PianoWebServer.ino
README.md
```

All frontend logic is embedded inside the `.ino` file.

---

## 🏷 Author

Developed as an experiment combining:

- ESP32
- Embedded web interfaces
- Touch interaction
- Mathematical frequency generation

---

## 📜 License

Free to use for educational and experimental purposes.

---

# 🎹 Result

A fully functional real-time web piano running directly from an ESP32 microcontroller.
