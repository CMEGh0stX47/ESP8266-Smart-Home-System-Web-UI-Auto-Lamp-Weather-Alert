#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

/* ===== PIN ===== */
#define DHT_PIN 2     
#define DHT_TYPE DHT11
#define LDR_PIN A0    
#define LAMP_PIN 5    
#define BUZZER_PIN 4  

/* ===== WIFI CONFIG (GANTI SESUAI HOTSPOT HP ANDA UNTUK API CUACA) ===== */
const char* apSSID = "SMART_CLOCK";  
const char* apPassword = "12345678";
const char* wifiSSID = "";   //
const char* wifiPassword = "";

/* ===== MQTT ===== */
const char* mqttServer = "";
const int mqttPort = 1883;
const char* mqttUsername = "";
const char* mqttPassword = "";

/* ===== OBJECTS ===== */
ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
DHT dht(DHT_PIN, DHT_TYPE);

/* ===== STATE ===== */
float t = 0, h = 0, l = 0; 
float weatherTemp = 0; 
bool autoMode = false;
bool lampState = false;
bool buzzerState = false; 
bool rainCondition = false;

/* ===== HTML (PREMIUM UI) ===== */
const char PAGE_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="id">
<head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>Smart Home Pro</title>
<link href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css" rel="stylesheet">
<link href="https://fonts.googleapis.com/css2?family=Plus+Jakarta+Sans:wght@400;500;700;800&display=swap" rel="stylesheet">
)rawliteral";

const char PAGE_STYLE[] PROGMEM = R"rawliteral(
<style>
:root {
 --bg: #eef2f6;
 --card-bg: rgba(255, 255, 255, 0.85); /* Glass effect */
 --primary-grad: linear-gradient(135deg, #6366f1 0%, #4f46e5 100%);
 --accent: #4f46e5;
 --text: #1e293b;
 --text-light: #64748b;
 --danger: #ef4444;
 --success: #10b981;
 --shadow-lg: 0 10px 30px -5px rgba(0, 0, 0, 0.1);
 --shadow-sm: 0 4px 6px -1px rgba(0, 0, 0, 0.05);
}

* { margin:0; padding:0; box-sizing:border-box; -webkit-tap-highlight-color: transparent; }
body { 
 font-family: 'Plus Jakarta Sans', sans-serif; 
 background: var(--bg); 
 color: var(--text); 
 min-height: 100vh;
 display: flex; justify-content: center;
 padding: 2rem 1rem;
 background-image: radial-gradient(#e2e8f0 1px, transparent 1px);
 background-size: 20px 20px;
}

.app { 
 width: 100%; max-width: 500px; 
 display: flex; flex-direction: column; gap: 1.5rem; 
 animation: slideUp 0.8s cubic-bezier(0.2, 0.8, 0.2, 1);
}

@keyframes slideUp { from{opacity:0; transform:translateY(40px);} to{opacity:1; transform:translateY(0);} }

/* HERO WIDGET */
.hero {
 background: var(--card-bg);
 backdrop-filter: blur(12px); -webkit-backdrop-filter: blur(12px);
 border: 1px solid rgba(255,255,255,0.5);
 padding: 2rem; border-radius: 2.5rem;
 box-shadow: var(--shadow-lg);
 text-align: center;
 position: relative;
 overflow: hidden;
}
.hero-circle { position:absolute; top:-30%; left:-20%; width:150px; height:150px; background:var(--primary-grad); border-radius:50%; filter:blur(60px); opacity:0.3; }
.hero-circle-2 { position:absolute; bottom:-30%; right:-20%; width:150px; height:150px; background:#f43f5e; border-radius:50%; filter:blur(60px); opacity:0.2; }

.time { font-size: 4rem; font-weight: 800; color: var(--text); line-height: 1; letter-spacing: -2px; margin: 0.5rem 0; z-index:2; position:relative; }
.date { font-size: 0.9rem; color: var(--accent); font-weight: 700; text-transform: uppercase; letter-spacing: 1.5px; z-index:2; position:relative; }

/* WEATHER BADGE */
.weather-badge {
 display: inline-flex; align-items: center; gap: 0.5rem;
 background: rgba(255,255,255,0.6); padding: 0.5rem 1rem; border-radius: 2rem;
 margin-top: 1rem; font-weight: 600; color: var(--text-light); font-size: 0.9rem;
 border: 1px solid rgba(0,0,0,0.05); z-index:2; position:relative;
}

/* CARDS GRID */
.grid { display: grid; grid-template-columns: 1fr 1fr; gap: 1rem; }
.card {
 background: var(--card-bg);
 border: 1px solid rgba(255,255,255,0.8);
 padding: 1.5rem; border-radius: 1.8rem;
 box-shadow: var(--shadow-sm); transition: 0.3s cubic-bezier(0.4, 0, 0.2, 1);
 display: flex; flex-direction: column; align-items: center; justify-content: center;
 position: relative; overflow: hidden;
}
.card:hover { transform: translateY(-5px); box-shadow: var(--shadow-lg); }
.card-icon { font-size: 1.6rem; color: var(--accent); margin-bottom: 0.5rem; background: rgba(99, 102, 241, 0.1); padding: 0.8rem; border-radius: 1rem; }
.val { font-size: 1.8rem; font-weight: 800; color: var(--text); }
.lbl { font-size: 0.75rem; color: var(--text-light); font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; margin-top: 0.2rem; }

/* CONTROLS */
.controls-card {
 background: white; padding: 1.5rem; border-radius: 2rem;
 box-shadow: var(--shadow-lg);
 display: flex; flex-direction: column; gap: 1rem;
}
.btn-group { display: flex; gap: 0.8rem; }
button {
 flex: 1; border: none; padding: 1.2rem; border-radius: 1.2rem;
 font-family: inherit; font-size: 0.9rem; font-weight: 700;
 cursor: pointer; transition: 0.2s;
 display: flex; flex-direction: column; align-items: center; gap: 0.5rem;
 background: #f8fafc; color: var(--text-light); border: 2px solid transparent;
}
button:active { transform: scale(0.96); }

/* STATES */
.active-lamp { background: var(--primary-grad); color: white; box-shadow: 0 8px 20px rgba(79, 70, 229, 0.3); }
.active-auto { background: #d1fae5; color: #059669; border-color: #10b981; }
.active-buzz { background: #fee2e2; color: #dc2626; border-color: #ef4444; animation: pulseRed 1s infinite; }

@keyframes pulseRed { 0% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0.4); } 70% { box-shadow: 0 0 0 10px rgba(239, 68, 68, 0); } 100% { box-shadow: 0 0 0 0 rgba(239, 68, 68, 0); } }

/* ALERT BOX */
.rain-alert {
 background: #fff1f2; border: 2px solid #fda4af; color: #e11d48;
 padding: 1rem; border-radius: 1.5rem; text-align: center;
 font-weight: 700; display: none; margin-top: -0.5rem;
 animation: shake 0.5s infinite;
}
@keyframes shake { 0%,100%{transform:translateX(0)} 20%{transform:translateX(-3px)} 80%{transform:translateX(3px)} }

</style>
</head>
)rawliteral";

const char PAGE_BODY[] PROGMEM = R"rawliteral(
<body>
 <div class="app">
  
  <!-- HERO SECTION WITH ENHANCED DESIGN -->
  <div class="hero">
   <div class="hero-circle"></div><div class="hero-circle-2"></div>
   
   <!-- Date Badge with Icon -->
   <div style="display:flex; align-items:center; justify-content:center; gap:8px; margin-bottom:8px; position:relative; z-index:2;">
    <i class="fas fa-calendar-days" style="color:var(--accent); font-size:0.85rem;"></i>
    <div class="date" id="date">...</div>
   </div>
   
   <!-- Time Display -->
   <div class="time" id="clock">00:00</div>
   
   <!-- Weather & Location Info -->
   <div style="display:flex; flex-direction:column; gap:10px; margin-top:1.2rem; position:relative; z-index:2;">
    <div class="weather-badge">
      <i class="fas fa-location-dot" style="color:#ef4444"></i> 
      <span style="font-weight:700;">Bandung</span>
      <span style="margin:0 6px; opacity:0.3;">|</span>
      <i class="fas fa-cloud-sun" style="color:#f59e0b; margin-right:4px;"></i>
      <span id="wt">--</span>&deg;C
    </div>
   </div>
  </div>

  <!-- RAIN ALERT -->
  <div id="rainAlert" class="rain-alert">
    <i class="fas fa-triangle-exclamation" style="margin-right:8px;"></i> 
    <span style="font-weight:800;">PERINGATAN HUJAN!</span>
  </div>

  <!-- SENSOR DATA CARDS -->
  <div class="grid">
   <!-- Temperature Card -->
   <div class="card">
    <div class="card-icon">
     <i class="fas fa-temperature-full"></i>
    </div>
    <div class="val"><span id="t">--</span>&deg;</div>
    <div class="lbl">
     <i class="fas fa-home" style="font-size:0.7rem; margin-right:3px; opacity:0.6;"></i>
     Suhu Ruang
    </div>
   </div>
   
   <!-- Humidity Card -->
   <div class="card">
    <div class="card-icon" style="color:#0ea5e9; background:rgba(14,165,233,0.1)">
     <i class="fas fa-droplet"></i>
    </div>
    <div class="val"><span id="h">--</span>%</div>
    <div class="lbl">
     <i class="fas fa-water" style="font-size:0.7rem; margin-right:3px; opacity:0.6;"></i>
     Kelembaban
    </div>
   </div>
   
   <!-- Light Sensor Card -->
   <div class="card">
    <div class="card-icon" style="color:#f59e0b; background:rgba(245,158,11,0.1)">
     <i class="fas fa-sun"></i>
    </div>
    <div class="val"><span id="l">--</span>%</div>
    <div class="lbl">
     <i class="fas fa-brightness" style="font-size:0.7rem; margin-right:3px; opacity:0.6;"></i>
     Intensitas Cahaya
    </div>
   </div>
   
   <!-- Lamp Status Card -->
   <div class="card">
    <div class="card-icon" style="color:#8b5cf6; background:rgba(139,92,246,0.1)">
     <i class="far fa-lightbulb" id="iconL"></i>
    </div>
    <div class="val" id="textL" style="font-size:1.4rem">OFF</div>
    <div class="lbl">
     <i class="fas fa-toggle-off" style="font-size:0.7rem; margin-right:3px; opacity:0.6;"></i>
     Status Lampu
    </div>
   </div>
  </div>

  <!-- CONTROL CENTER -->
  <div class="controls-card">
   <!-- Header with Gradient Accent -->
   <div style="display:flex; align-items:center; justify-content:space-between; margin-bottom:1rem;">
    <div style="display:flex; align-items:center; gap:8px;">
     <div style="width:8px; height:8px; background:var(--accent); border-radius:50%; animation:pulse 2s infinite;"></div>
     <span style="font-size:0.85rem; font-weight:800; color:var(--text); text-transform:uppercase; letter-spacing:1.2px;">
      <i class="fas fa-sliders" style="margin-right:6px;"></i>Control Center
     </span>
    </div>
    <div style="font-size:0.7rem; color:var(--text-light); font-weight:600;">
     <i class="fas fa-circle" style="color:#10b981; font-size:0.5rem; margin-right:4px;"></i>Online
    </div>
   </div>
   
   <!-- Control Buttons -->
   <div class="btn-group">
    <button id="bL" onclick="cmd('lamp')">
      <i class="fas fa-lightbulb" style="font-size:1.3rem;"></i> 
      <span style="font-size:0.75rem; margin-top:4px;">LAMPU</span>
    </button>
    <button id="bA" onclick="cmd('auto')">
      <i class="fas fa-wand-magic-sparkles" style="font-size:1.3rem;"></i> 
      <span style="font-size:0.75rem; margin-top:4px;">AUTO</span>
    </button>
    <button id="bB" onclick="cmd('buzz')">
      <i class="fas fa-bell" style="font-size:1.3rem;"></i> 
      <span style="font-size:0.75rem; margin-top:4px;">ALARM</span>
    </button>
   </div>
  </div>

  <!-- Footer Info -->
  <div style="text-align:center; padding:1rem 0; opacity:0.5;">
   <div style="font-size:0.7rem; color:var(--text-light); font-weight:600;">
    <i class="fas fa-microchip" style="margin-right:4px;"></i>
    Smart Home IoT System v2.0
   </div>
  </div>

 </div>
)rawliteral";

const char PAGE_SCRIPT[] PROGMEM = R"rawliteral(
<script>
 function upC() {
  const d=new Date();
  document.getElementById('clock').innerText=String(d.getHours()).padStart(2,'0')+":"+String(d.getMinutes()).padStart(2,'0');
  document.getElementById('date').innerText=d.toLocaleDateString('id-ID',{weekday:'long',day:'numeric',month:'long'});
 }
 setInterval(upC,1000); upC();

 let st={l:0,a:0,b:0,r:0,wt:0};

 function sync() {
  fetch('/status').then(r=>r.json()).then(d=>{
   if(d.t) document.getElementById('t').innerText=d.t.toFixed(0);
   if(d.h) document.getElementById('h').innerText=d.h.toFixed(0);
   if(d.l) document.getElementById('l').innerText=d.l.toFixed(0);
   if(d.wt && d.wt!=0) document.getElementById('wt').innerText=d.wt.toFixed(0);
   
   st={l:d.lamp,a:d.auto,b:d.buzz,r:d.rain};
   
   const bL=document.getElementById('bL'), iL=document.getElementById('iconL'), tL=document.getElementById('textL');
   if(st.l){ bL.className='active-lamp'; iL.className='fas fa-lightbulb'; tL.innerText='ON'; tL.style.color='#4f46e5'; }
   else { bL.className=''; iL.className='far fa-lightbulb'; tL.innerText='OFF'; tL.style.color='var(--text)'; }

   document.getElementById('bA').className = st.a ? 'active-auto' : '';
   
   const bB=document.getElementById('bB');
   if(st.b) { bB.className='active-buzz'; bB.innerHTML='<i class="fas fa-stop"></i> STOP'; }
   else { bB.className=''; bB.innerHTML='<i class="fas fa-bell"></i> ALARM'; }

   document.getElementById('rainAlert').style.display = st.r ? 'block' : 'none';
  });
 }

 function cmd(t) {
  if(t=='lamp' && st.a) return alert("Matikan Mode Auto dulu!");
  let v='0';
  if(t=='lamp') v=st.l?'OFF':'ON';
  if(t=='auto') v=st.a?'0':'1';
  if(t=='buzz') v=st.b?'OFF':'ON';
  
  if(t=='buzz') { const bB=document.getElementById('bB'); bB.className=v=='ON'?'active-buzz':''; }
  fetch('/cmd?type='+t+'&val='+v).then(sync);
 }
 setInterval(sync, 1000); sync(); // 1s sync
</script>
</body>
</html>
)rawliteral";

/* ===== LOGIC ===== */
void handleRoot() { server.send(200,"text/html",String(FPSTR(PAGE_HEAD))+String(FPSTR(PAGE_STYLE))+String(FPSTR(PAGE_BODY))+String(FPSTR(PAGE_SCRIPT))); }
void handleStatus() {
 DynamicJsonDocument doc(512);
 doc["t"]=t; doc["h"]=h; doc["l"]=l; doc["wt"]=weatherTemp;
 doc["lamp"]=lampState; doc["auto"]=autoMode; doc["buzz"]=buzzerState; doc["rain"]=rainCondition;
 String json; serializeJson(doc,json); server.send(200,"application/json",json);
}
void handleCmd() {
 String type=server.arg("type"), val=server.arg("val");
 if(type=="lamp") { lampState=(val=="ON"); digitalWrite(LAMP_PIN, lampState?HIGH:LOW); }
 if(type=="auto") { autoMode=(val=="1"); }
 if(type=="buzz") { buzzerState=(val=="ON"); digitalWrite(BUZZER_PIN, buzzerState?HIGH:LOW); } 
 server.send(200,"text/plain","OK");
}

/* ===== MAIN ===== */
void setup() {
 Serial.begin(115200); delay(500);
 dht.begin(); 
 pinMode(LAMP_PIN,OUTPUT); digitalWrite(LAMP_PIN,LOW);
 pinMode(BUZZER_PIN,OUTPUT); digitalWrite(BUZZER_PIN,LOW);
 
 WiFi.mode(WIFI_AP_STA);
 WiFi.softAP(apSSID,apPassword);
 WiFi.begin(wifiSSID,wifiPassword);
 
 server.on("/",handleRoot); server.on("/status",handleStatus); server.on("/cmd",handleCmd);
 server.begin();
}

void loop() {
 server.handleClient();
 static unsigned long tmr=0; 
 if(millis()-tmr>2000) {
   tmr=millis();
   float nt=dht.readTemperature(); if(!isnan(nt)) t=nt;
   float nh=dht.readHumidity(); if(!isnan(nh)) h=nh;
   l=(1.0-analogRead(LDR_PIN)/1023.0)*100.0;
   
   if(autoMode) {
     if(l<20 && !lampState) { lampState=true; digitalWrite(LAMP_PIN,HIGH); }
     else if(l>=20 && lampState) { lampState=false; digitalWrite(LAMP_PIN,LOW); }
   }
   
   bool isRaining = (t > 0 && t < 25.0 && h > 75.0);
   if(isRaining && !rainCondition) { rainCondition=true; buzzerState=true; digitalWrite(BUZZER_PIN, HIGH); }
   else if(!isRaining && rainCondition) { rainCondition=false; buzzerState=false; digitalWrite(BUZZER_PIN, LOW); }
 }
 
 static unsigned long bzTmr=0; static bool bzS=1;
 if(buzzerState) {
   int interval = rainCondition?150:400; 
   if(millis()-bzTmr>interval) { bzTmr=millis(); bzS=!bzS; digitalWrite(BUZZER_PIN,bzS?HIGH:LOW); }
 } 
 
 static unsigned long wTmr=0;
 if(wTmr==0 || millis()-wTmr>600000) {
   if(WiFi.status()==WL_CONNECTED) {
     WiFiClient c; HTTPClient h;
     // USER AGENT
     if(h.begin(c,"http://api.openweathermap.org/data/2.5/weather?q=INDONESIA&units=metric&appid=your_api")){
        int code = h.GET(); // Cek Code
        if(code==200){ DynamicJsonDocument d(1024); deserializeJson(d,h.getString()); weatherTemp=d["main"]["temp"]; }
        h.end();
     }
   }
   wTmr=millis(); if(wTmr==0) wTmr=1;
 }
}