#include <TFT_eSPI.h>
#include <SPI.h>
#include <math.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "Wifi DP";
const char* password = "lorenrufi";

AsyncWebServer server(80);
AsyncEventSource events("/events");

// ==================================================
// HTML PIANO 2 OCTAVAS (NO MODIFICADO EN ESTRUCTURA)
// ==================================================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<title>ESP32 Piano</title>

<style>
* { box-sizing: border-box; }

body {
  margin: 0;
  background: #222;
  display: flex;
  justify-content: center;
  align-items: center;
  height: 100vh;
  overflow: hidden;
  user-select: none;
  -webkit-user-select: none;
  -ms-user-select: none;
  touch-action: none;
}

.piano {
  position: relative;
  display: flex;
  width: 100vw;
  height: 100vh;
}

.white {
  flex: 1;
  background: white;
  border: 1px solid #000;
  position: relative;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 12px;
  font-weight: bold;
}

.white.active { background: #dddddd; }

.black {
  position: absolute;
  width: 5%;
  height: 60%;
  background: black;
  top: 0;
  z-index: 2;
  border-radius: 0 0 5px 5px;
  display: flex;
  align-items: flex-end;
  justify-content: center;
  color: white;
  font-size: 10px;
}

.black.active { background: #555555; }

.label {
  position: absolute;
  bottom: 60px;
  pointer-events: none;
  user-select: none;
  -webkit-user-select: none;
  font-size: 14px;
}
/* ===== BOTONERA ESTILO SINTETIZADOR 90s ===== */

#botonera{
  position:absolute;
  top:0;
  left:0;
  width:100%;
  height:24px;
  display:flex;
  z-index:999;
  background:linear-gradient(to bottom,#444,#222);
  border-bottom:2px solid #000;
}

.synthBtn{
  flex:2;
  display:flex;
  align-items:center;
  justify-content:center;
  position:relative;
  font-family:monospace;
  font-size:11px;
  font-weight:bold;
  color:#00ffcc;
  background:linear-gradient(to bottom,#555,#333);
  border-top:2px solid #777;
  border-left:2px solid #777;
  border-bottom:2px solid #111;
  border-right:2px solid #111;
  text-shadow:0 0 4px #00ffcc;
  cursor:pointer;
}

/* LED rojo */
.synthBtn::after{
  content:"";
  position:absolute;
  right:4px;
  width:6px;
  height:6px;
  border-radius:50%;
  background:#300;
  box-shadow:0 0 2px #000;
}

.synthBtn.activeLed::after{
  background:#ff0000;
  box-shadow:0 0 6px #ff0000;
}

.synthBtn:active{
  border-top:2px solid #111;
  border-left:2px solid #111;
  border-bottom:2px solid #777;
  border-right:2px solid #777;
  background:linear-gradient(to bottom,#222,#111);
}

</style>
</head>
<body>

<div id="botonera">

  <div class="synthBtn" data-song="1">Song 1</div>
  <div class="synthBtn" data-song="2">Song 2</div>
  <div class="synthBtn" data-song="3">Song 3</div>
  <div class="synthBtn" data-song="4">Song 4</div>
  <div class="synthBtn" data-song="5">Song 5</div>
  <div class="synthBtn" data-song="6">Song 6</div>
  <div class="synthBtn" data-song="7">Song 7</div>

</div>

<div class="piano">

  <!-- OCTAVA 0 -->
  <div class="white key" data-note="Do0"><div class="label">Do</div></div>
  <div class="white key" data-note="Re0"><div class="label">Re</div></div>
  <div class="white key" data-note="Mi0"><div class="label">Mi</div></div>
  <div class="white key" data-note="Fa0"><div class="label">Fa</div></div>
  <div class="white key" data-note="Sol0"><div class="label">Sol</div></div>
  <div class="white key" data-note="La0"><div class="label">La</div></div>
  <div class="white key" data-note="Si0"><div class="label">Si</div></div>

  <!-- OCTAVA 1 -->
  <div class="white key" data-note="Do1"><div class="label">Do</div></div>
  <div class="white key" data-note="Re1"><div class="label">Re</div></div>
  <div class="white key" data-note="Mi1"><div class="label">Mi</div></div>
  <div class="white key" data-note="Fa1"><div class="label">Fa</div></div>
  <div class="white key" data-note="Sol1"><div class="label">Sol</div></div>
  <div class="white key" data-note="La1"><div class="label">La</div></div>
  <div class="white key" data-note="Si1"><div class="label">Si</div></div>

  <!-- NEGRAS OCTAVA 0 -->
  <div class="black key" style="left:6%;" data-note="DoS0"></div>
  <div class="black key" style="left:13%;" data-note="ReS0"></div>
  <div class="black key" style="left:27%;" data-note="FaS0"></div>
  <div class="black key" style="left:34%;" data-note="SolS0"></div>
  <div class="black key" style="left:41%;" data-note="LaS0"></div>

  <!-- NEGRAS OCTAVA 1 -->
  <div class="black key" style="left:56%;" data-note="DoS1"></div>
  <div class="black key" style="left:63%;" data-note="ReS1"></div>
  <div class="black key" style="left:77%;" data-note="FaS1"></div>
  <div class="black key" style="left:84%;" data-note="SolS1"></div>
  <div class="black key" style="left:91%;" data-note="LaS1"></div>

</div>

<script>

let currentKey = null;
let fullscreenActivated = false;
let demoKey = null;
let currentSong = null;

document.querySelectorAll(".synthBtn").forEach(btn=>{
  btn.addEventListener("click",()=>{
    const id = btn.getAttribute("data-song");

    if(currentSong === id){
      fetch("/song?stop=1");
      return;
    }

    fetch("/song?id="+id);
  });
});


function enableFullscreen() {
  if (fullscreenActivated) return;
  const elem = document.documentElement;
  if (elem.requestFullscreen) elem.requestFullscreen();
  else if (elem.webkitRequestFullscreen) elem.webkitRequestFullscreen();
  else if (elem.msRequestFullscreen) elem.msRequestFullscreen();
  fullscreenActivated = true;
}

function startNote(key) {

  if (demoKey) {
    key.classList.add("active");
    setTimeout(()=>key.classList.remove("active"),150);
    return;
  }

  if (currentKey === key) return;

  if (currentKey) currentKey.classList.remove('active');

  currentKey = key;
  key.classList.add('active');

  const note = key.getAttribute('data-note');
  fetch('/note?note=' + note);
}

function stopCurrentNote() {
  if (currentKey) {
    currentKey.classList.remove('active');
    currentKey = null;
    fetch('/stop');
  }
}

// TOUCH
document.addEventListener('touchstart', (e) => {
  enableFullscreen();
  const target = document.elementFromPoint(
    e.touches[0].clientX,
    e.touches[0].clientY
  );
  if (target && target.classList.contains('key')) startNote(target);
});

document.addEventListener('touchmove', (e) => {
  const target = document.elementFromPoint(
    e.touches[0].clientX,
    e.touches[0].clientY
  );
  if (target && target.classList.contains('key')) startNote(target);
});

document.addEventListener('touchend', () => stopCurrentNote());

// MOUSE
document.querySelectorAll('.key').forEach(key => {
  key.addEventListener('mousedown', () => startNote(key));
  key.addEventListener('mouseup', () => stopCurrentNote());
  key.addEventListener('mouseleave', () => stopCurrentNote());
});

// SSE para mostrar himno
const evtSource = new EventSource("/events");

evtSource.onmessage = function(event){

  if(event.data.startsWith("SONG_START:")){
    const id = event.data.split(":")[1];
    currentSong = id;
    document.querySelectorAll(".synthBtn").forEach(b=>b.classList.remove("activeLed"));
    const btn = document.querySelector(`.synthBtn[data-song='${id}']`);
    if(btn) btn.classList.add("activeLed");
    return;
  }

  if(event.data === "SONG_END"){
    currentSong = null;
    document.querySelectorAll(".synthBtn").forEach(b=>b.classList.remove("activeLed"));
    return;
  }

  if(event.data === "NOTE_OFF"){
    if(demoKey){
      demoKey.classList.remove("active");
      demoKey = null;
    }
    return;
  }


  const key = document.querySelector(`[data-note='${event.data}']`);

  if(key){
    if(demoKey === key){

      // PARPADEO MÁS VISIBLE (invertir + escala)
      key.style.transition = "transform 0.08s, background 0.08s";
      key.style.transform = "scale(0.9)";

      if(key.classList.contains("white")){
        key.style.background = "#000";
      }else{
        key.style.background = "#fff";
      }

      setTimeout(()=>{
        key.style.transform = "scale(1)";
        key.style.background = "";
      },120);

    }else{
      if(demoKey) demoKey.classList.remove("active");
      key.classList.add("active");
      demoKey = key;
    }
  }
};

</script>
</body>
</html>
)rawliteral";

// ==================================================
// AUDIO
// ==================================================

#define BUZZER_PIN 4
#define TRIGGER_PIN 13

struct Nota {
  String nombre;
  float frecuencia;
};

int BPM = 120;
const float LA_BASE = 440.0;

float aplicarPuntillo(float duracionBase, const char* puntillo) {
  if (puntillo != nullptr && puntillo[0] == '.') {
    return duracionBase * 1.5;  // figura con puntillo
  }
  return duracionBase;
}

float NEGRA(const char* p = nullptr) {
  return aplicarPuntillo(60000.0 / BPM, p);
}
float BLANCA(const char* p = nullptr) {
  return aplicarPuntillo((60000.0 / BPM) * 2, p);
}
float CORCHEA(const char* p = nullptr) {
  return aplicarPuntillo((60000.0 / BPM) / 2, p);
}
float SEMICORCHEA(const char* p = nullptr) {
  return aplicarPuntillo((60000.0 / BPM) / 4, p);
}
float REDONDA(const char* p = nullptr) {
  return aplicarPuntillo((60000.0 / BPM) * 4, p);
}

void silencio(float dur) {
  delay(dur);
}

float calcularFrecuencia(int indiceDesdeDo, int octava) {
  int semitonosDesdeA4 = (indiceDesdeDo - 9) + (octava * 12);
  return LA_BASE * pow(2.0, semitonosDesdeA4 / 12.0);
}

bool detenerMelodia = false;

void tocar(Nota n, float duracion) {
  if(events.count() > 0){
    events.send(n.nombre.c_str());
  }
  tone(BUZZER_PIN, n.frecuencia);

  unsigned long inicio = millis();

  while (millis() - inicio < duracion) {

    if (detenerMelodia) {
      noTone(BUZZER_PIN);
  if(events.count() > 0){
    events.send("NOTE_OFF");
  }
      return;
    }

    delay(5);
  }

  noTone(BUZZER_PIN);
  events.send("NOTE_OFF");
}

// NOTAS
Nota Do(int o) {
  return { "Do" + String(o), calcularFrecuencia(0, o) };
}
Nota DoS(int o) {
  return { "DoS" + String(o), calcularFrecuencia(1, o) };
}
Nota Re(int o) {
  return { "Re" + String(o), calcularFrecuencia(2, o) };
}
Nota ReS(int o) {
  return { "ReS" + String(o), calcularFrecuencia(3, o) };
}
Nota Mi(int o) {
  return { "Mi" + String(o), calcularFrecuencia(4, o) };
}
Nota Fa(int o) {
  return { "Fa" + String(o), calcularFrecuencia(5, o) };
}
Nota FaS(int o) {
  return { "FaS" + String(o), calcularFrecuencia(6, o) };
}
Nota Sol(int o) {
  return { "Sol" + String(o), calcularFrecuencia(7, o) };
}
Nota SolS(int o) {
  return { "SolS" + String(o), calcularFrecuencia(8, o) };
}
Nota La(int o) {
  return { "La" + String(o), calcularFrecuencia(9, o) };
}
Nota LaS(int o) {
  return { "LaS" + String(o), calcularFrecuencia(10, o) };
}
Nota Si(int o) {
  return { "Si" + String(o), calcularFrecuencia(11, o) };
}

void song1() {
  BPM = 110;

  tocar(La(0), BLANCA());
  tocar(DoS(1), NEGRA("."));
  tocar(La(0), CORCHEA());
  tocar(Mi(1), BLANCA("."));

  tocar(DoS(1), CORCHEA());
  tocar(Si(0), CORCHEA());
  tocar(SolS(0), NEGRA());
  tocar(SolS(0), NEGRA());
  tocar(SolS(0), NEGRA());
  tocar(SolS(0), NEGRA());
  
  tocar(SolS(0), NEGRA("."));
  tocar(La(0), CORCHEA());
  tocar(Si(0), CORCHEA());
  tocar(La(0), NEGRA(".")); 
  silencio(CORCHEA());
  //C5
  tocar(La(0), NEGRA());
  tocar(DoS(1), NEGRA());
  tocar(FaS(1), NEGRA());
  tocar(Mi(1), NEGRA());
  tocar(ReS(1), NEGRA());
  tocar(Re(1), NEGRA("."));

  tocar(Si(0), NEGRA());
  tocar(Mi(1), NEGRA());
  tocar(DoS(1), NEGRA());
  tocar(La(0), BLANCA());

  
  tocar(Mi(1), NEGRA());
  tocar(DoS(1), NEGRA());
  tocar(La(0), BLANCA());

  //BAjada
  tocar(Mi(0), SEMICORCHEA());
  tocar(FaS(0), SEMICORCHEA());
  tocar(SolS(0), SEMICORCHEA());
  tocar(La(0), SEMICORCHEA());
  tocar(Si(0), SEMICORCHEA());
  tocar(DoS(1), SEMICORCHEA());
  tocar(Re(1), SEMICORCHEA());


  tocar(Mi(1), CORCHEA());
  tocar(Mi(1), NEGRA());
  tocar(Re(1), NEGRA());
  tocar(Si(0), BLANCA());

  
  tocar(Si(0), NEGRA());
  tocar(DoS(1), CORCHEA());
  tocar(Re(1), CORCHEA());
  tocar(Mi(1), NEGRA());
  tocar(Re(1), NEGRA());
  tocar(Si(0), BLANCA());

  tocar(Si(0), NEGRA());
  tocar(DoS(1), CORCHEA());
  tocar(Re(1), CORCHEA());
  tocar(Mi(1), NEGRA());
  tocar(ReS(1), NEGRA());
  tocar(Re(1), NEGRA());
}



void song2() {
  BPM = 120;
  // Versión en Sol mayor (afinación correcta)

  tocar(Sol(0), CORCHEA());
  tocar(Sol(0), CORCHEA());
  tocar(La(0), NEGRA());
  tocar(Sol(0), NEGRA());
  tocar(Do(1), NEGRA());
  tocar(Si(0), BLANCA());

  tocar(Sol(0), CORCHEA());
  tocar(Sol(0), CORCHEA());
  tocar(La(0), NEGRA());
  tocar(Sol(0), NEGRA());
  tocar(Re(1), NEGRA());
  tocar(Do(1), BLANCA());

  tocar(Sol(0), CORCHEA());
  tocar(Sol(0), CORCHEA());
  tocar(Sol(1), NEGRA());
  tocar(Mi(1), NEGRA());
  tocar(Do(1), NEGRA());
  tocar(Si(0), NEGRA());
  tocar(La(0), BLANCA());

  tocar(Fa(1), CORCHEA());
  tocar(Fa(1), CORCHEA());
  tocar(Mi(1), NEGRA());
  tocar(Do(1), NEGRA());
  tocar(Re(1), NEGRA());
  tocar(Do(1), BLANCA());
}

void song3() {
  BPM = 120;
  // Enter Sandman - motivo reconocible simplificado

  tocar(Mi(0), CORCHEA());
  tocar(Sol(0), CORCHEA());
  tocar(La(0), NEGRA());
  tocar(Sol(0), CORCHEA());
  tocar(Mi(0), CORCHEA());

  tocar(Re(0), CORCHEA());
  tocar(Mi(0), CORCHEA());
  tocar(Sol(0), NEGRA());
  tocar(Mi(0), NEGRA());
}

// --------------------------------------------------

void song4() {
  BPM = 120;
  // Pantera - Walk (motivo rítmico icónico simplificado)

  tocar(Mi(0), NEGRA());
  tocar(Mi(0), NEGRA());
  tocar(Sol(0), NEGRA());
  tocar(Mi(0), NEGRA());

  tocar(La(0), NEGRA());
  tocar(Sol(0), NEGRA());
  tocar(Mi(0), NEGRA());
  silencio(NEGRA());
}

// --------------------------------------------------

void song5() {

  // Nothing Else Matters - intro arpegiado más completo (simplificado)
  // Tempo recomendado: BPM = 60–70
  BPM = 70;
  tocar(Mi(0), CORCHEA());
  tocar(Si(0), CORCHEA());
  tocar(Mi(1), CORCHEA());
  tocar(Sol(1), CORCHEA());

  tocar(FaS(1), CORCHEA());
  tocar(Mi(1), CORCHEA());
  tocar(Si(0), CORCHEA());
  tocar(Mi(0), CORCHEA());

  tocar(Mi(0), CORCHEA());
  tocar(Si(0), CORCHEA());
  tocar(Mi(1), CORCHEA());
  tocar(Sol(1), CORCHEA());

  tocar(FaS(1), CORCHEA());
  tocar(Mi(1), CORCHEA());
  tocar(Si(0), CORCHEA());
  tocar(Mi(0), CORCHEA());
}

// --------------------------------------------------

void song6() {

  // Master of Puppets - intro melódico simplificado
  // Tempo recomendado: BPM = 105–110
  BPM = 110;
  tocar(Mi(0), CORCHEA());
  tocar(Fa(0), CORCHEA());
  tocar(Sol(0), CORCHEA());
  tocar(Mi(0), CORCHEA());

  tocar(Fa(0), CORCHEA());
  tocar(Sol(0), CORCHEA());
  tocar(La(0), CORCHEA());
  tocar(Sol(0), CORCHEA());

  tocar(Fa(0), CORCHEA());
  tocar(Mi(0), CORCHEA());
  tocar(Re(0), CORCHEA());
  tocar(Mi(0), CORCHEA());

  tocar(Sol(0), NEGRA());
  tocar(Mi(0), NEGRA());
}

void song7() {
  BPM = 110;

  tocar(Sol(0), NEGRA());
  tocar(Sol(0), NEGRA());
  tocar(La(0), NEGRA());
  tocar(Si(0), NEGRA());

  tocar(Si(0), NEGRA());
  tocar(La(0), NEGRA());
  tocar(Sol(0), BLANCA());

  tocar(Re(0), NEGRA());
  tocar(Re(0), NEGRA());
  tocar(Mi(0), NEGRA());
  tocar(FaS(0), NEGRA());

  tocar(Sol(0), BLANCA());


  tocar(Sol(0), NEGRA());
  tocar(La(0), NEGRA());
  tocar(Si(0), NEGRA());
  tocar(Do(1), NEGRA());

  tocar(Si(0), NEGRA());
  tocar(La(0), NEGRA());
  tocar(Sol(0), BLANCA());

  tocar(Re(0), NEGRA());
  tocar(Mi(0), NEGRA());
  tocar(FaS(0), NEGRA());
  tocar(Sol(0), REDONDA());
}


bool reproduciendo = false;
int songActual = 0;
bool songSolicitada = false;
bool detenerSong = false;

void setupWebServer() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
    req->send_P(200, "text/html", index_html);
  });

  server.on("/song", HTTP_GET, [](AsyncWebServerRequest* request) {

    if(request->hasParam("stop")){
      detenerMelodia = true;
      detenerSong = false;
      request->send(200,"text/plain","STOP");
      return;
    }

    if(request->hasParam("id")){
      songActual = request->getParam("id")->value().toInt();

      // Si ya hay una canción sonando, la cortamos inmediatamente
      if(reproduciendo){
        detenerMelodia = true;
      }

      songSolicitada = true;
    }

    request->send(200,"text/plain","OK");
  });

  Serial.println("WiFi conectado.");
  Serial.print("Servidor en: http://");
  Serial.println(WiFi.localIP());
  // ====== RESTAURAR SONIDO MANUAL ======
  server.on("/note", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (reproduciendo) {
      request->send(200, "text/plain", "IGNORED");
      return;
    }

    if (request->hasParam("note")) {
      String note = request->getParam("note")->value();
      String nombre = note.substring(0, note.length() - 1);
      int octava = note.substring(note.length() - 1).toInt();

      int indice = -1;

      if (nombre == "Do") indice = 0;
      else if (nombre == "DoS") indice = 1;
      else if (nombre == "Re") indice = 2;
      else if (nombre == "ReS") indice = 3;
      else if (nombre == "Mi") indice = 4;
      else if (nombre == "Fa") indice = 5;
      else if (nombre == "FaS") indice = 6;
      else if (nombre == "Sol") indice = 7;
      else if (nombre == "SolS") indice = 8;
      else if (nombre == "La") indice = 9;
      else if (nombre == "LaS") indice = 10;
      else if (nombre == "Si") indice = 11;

      if (indice >= 0) {
        float freq = calcularFrecuencia(indice, octava);
        tone(BUZZER_PIN, freq);
      }
    }

    request->send(200, "text/plain", "OK");
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (!reproduciendo) {
      noTone(BUZZER_PIN);
    }

    request->send(200, "text/plain", "STOP");
  });

  server.addHandler(&events);
  server.begin();
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  setupWebServer();
}

void loop() {

  static bool lastState = HIGH;
  bool currentState = digitalRead(TRIGGER_PIN);

  if (lastState == HIGH && currentState == LOW) {

    if (!reproduciendo) {
      detenerMelodia = false;
      reproduciendo = true;
      song1();
      reproduciendo = false;
    } else {
      detenerMelodia = true;
    }
  }

  if(songSolicitada && !reproduciendo){
    songSolicitada = false;
    detenerMelodia = false;
    reproduciendo = true;

    if(events.count() > 0){
      events.send(("SONG_START:"+String(songActual)).c_str());
    }

    switch(songActual){
      case 1: song1(); break;
      case 2: song2(); break;
      case 3: song3(); break;
      case 4: song4(); break;
      case 5: song5(); break;
      case 6: song6(); break;
      case 7: song7(); break;
    }

    reproduciendo = false;
    if(events.count() > 0){
      events.send("SONG_END");
    }
  }

  lastState = currentState;
}
