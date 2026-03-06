#include "esp_camera.h"
#include <WiFi.h>
#include <ESPmDNS.h>           
#include <Preferences.h>
#include "esp_http_server.h"
#include "soc/soc.h"           
#include "soc/rtc_cntl_reg.h"  

// =========================================================
// == USER CONFIGURATION ===================================
// =========================================================
const char* ap_ssid = "ESP32_Collimator"; 
const char* ap_password = NULL; // Open network (No password)
const char* HOSTNAME = "collimator";

// =========================================================
// == CORRECT AI-THINKER PIN DEFINITIONS (VERIFIED) ========
// =========================================================
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21 // Fixed from 13
#define Y4_GPIO_NUM       19 // Fixed from 33
#define Y3_GPIO_NUM       18 // Fixed from 15
#define Y2_GPIO_NUM        5 // Fixed from 14

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

Preferences prefs;

// =========================================================
// == WEBSERVER HANDLERS ===================================
// =========================================================

// 1. STREAM HANDLER (Standard MJPEG Chunking)
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t stream_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    char * part_buf = (char *)malloc(128);

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK) { free(part_buf); return res; }

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) { delay(10); continue; }

        if(res == ESP_OK) res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        if(res == ESP_OK){
            size_t hlen = snprintf(part_buf, 128, _STREAM_PART, fb->len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        
        esp_camera_fb_return(fb);
        if(res != ESP_OK) break;
    }
    free(part_buf);
    return res;
}

// 2. SAVE SETTINGS
static esp_err_t save_handler(httpd_req_t *req){
    char b[128], ox[16], oy[16], r1[16], r2[16];
    if (httpd_req_get_url_query_str(req, b, 128) == ESP_OK) {
        prefs.begin("coll", false);
        if (httpd_query_key_value(b, "ox", ox, 16) == ESP_OK) prefs.putInt("ox", atoi(ox));
        if (httpd_query_key_value(b, "oy", oy, 16) == ESP_OK) prefs.putInt("oy", atoi(oy));
        if (httpd_query_key_value(b, "r1", r1, 16) == ESP_OK) prefs.putInt("r1", atoi(r1));
        if (httpd_query_key_value(b, "r2", r2, 16) == ESP_OK) prefs.putInt("r2", atoi(r2));
        prefs.end();
    }
    return httpd_resp_send(req, "OK", 2);
}

// 3. LOAD SETTINGS
static esp_err_t status_handler(httpd_req_t *req){
    prefs.begin("coll", true);
    int ox = prefs.getInt("ox", 0); int oy = prefs.getInt("oy", 0);
    int r1 = prefs.getInt("r1", 60); int r2 = prefs.getInt("r2", 100);
    prefs.end();
    char s[128];
    snprintf(s, 128, "{\"ox\":%d,\"oy\":%d,\"r1\":%d,\"r2\":%d}", ox, oy, r1, r2);
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, s, strlen(s));
}

// 4. MAIN INTERFACE (Locked Layout)
static const char* index_html = R"rawliteral(
<html><head><meta name="viewport" content="width=device-width, initial-scale=1">
<style>
    body{font-family:sans-serif;text-align:center;background:#111;color:white;margin:0;padding:10px;}
    #viewer { position:relative; width:320px; height:240px; margin:10px auto; overflow:hidden; border:2px solid #444; background:#000; }
    #stream { width:100%; height:100%; object-fit: contain; transform-origin: center; }
    #overlay { position:absolute; top:0; left:0; width:320px; height:240px; pointer-events:none; }
    .controls { display:flex; flex-wrap:wrap; justify-content:center; max-width:450px; margin:auto; }
    .box { background:#222; padding:8px; margin:5px; border-radius:8px; flex:1 1 140px; border: 1px solid #333;}
    button { padding:10px; margin:2px; background:#444; color:white; border:none; border-radius:4px; min-width:45px; cursor:pointer; font-weight:bold;}
    button:active { background:#007bff; }
    span { color: #00ff00; font-family: monospace; }
</style></head>
<body>
    <h3>Collimation Monitor</h3>
    <div id="viewer">
        <img id="stream" src="/stream" onerror="this.src='/stream'">
        <svg id="overlay" viewBox="0 0 320 240">
            <line id="ch-v" x1="160" y1="0" x2="160" y2="240" stroke="white" stroke-width="1" />
            <line id="ch-h" x1="0" y1="120" x2="320" y2="120" stroke="white" stroke-width="1" />
            <circle id="c1" cx="160" cy="120" r="60" fill="none" stroke="#00ff00" stroke-width="1.5" />
            <circle id="c2" cx="160" cy="120" r="100" fill="none" stroke="#00ff00" stroke-width="1" stroke-dasharray="4" />
        </svg>
    </div>
    <div class="controls">
        <div class="box">Zoom: <span id="zVal">1.0</span>x<br><button onclick="zoom(0.1)">+</button><button onclick="zoom(-0.1)">-</button></div>
        <div class="box">Position<br><button onclick="move(0,-2)">UP</button><br><button onclick="move(-2,0)">L</button><button onclick="move(0,0,true)">Rst</button><button onclick="move(2,0)">R</button><br><button onclick="move(0,2)">DN</button></div>
        <div class="box">Inner Circle: <span id="r1Val">60</span><br><button onclick="rad(1,5)">+</button><button onclick="rad(1,-5)">-</button></div>
        <div class="box">Outer Circle: <span id="r2Val">100</span><br><button onclick="rad(2,5)">+</button><button onclick="rad(2,-5)">-</button></div>
    </div>
<script>
    let z = 1.0, ox = 0, oy = 0, r1 = 60, r2 = 100;
    window.onload = () => {
        fetch('/status').then(r => r.json()).then(d => { ox = d.ox; oy = d.oy; r1 = d.r1; r2 = d.r2; update(false); });
    }
    function zoom(v) { z = Math.min(Math.max(z + v, 1), 10); update(false); }
    function move(x, y, reset=false) { if(reset){ox=0;oy=0;} else {ox += x; oy += y;} update(true); }
    function rad(n, v) { if(n==1) r1=Math.max(5,r1+v); else r2=Math.max(5,r2+v); update(true); }
    function update(save) {
        document.getElementById('stream').style.transform = `scale(${z})`;
        document.getElementById('zVal').innerText = z.toFixed(1);
        document.getElementById('r1Val').innerText = r1;
        document.getElementById('r2Val').innerText = r2;
        let cx = 160 + ox; let cy = 120 + oy;
        document.getElementById('ch-v').setAttribute('x1', cx); document.getElementById('ch-v').setAttribute('x2', cx);
        document.getElementById('ch-h').setAttribute('y1', cy); document.getElementById('ch-h').setAttribute('y2', cy);
        document.getElementById('c1').setAttribute('cx', cx); document.getElementById('c1').setAttribute('cy', cy);
        document.getElementById('c1').setAttribute('r', r1);
        document.getElementById('c2').setAttribute('cx', cx); document.getElementById('c2').setAttribute('cy', cy);
        document.getElementById('c2').setAttribute('r', r2);
        if(save) fetch(`/save?ox=${ox}&oy=${oy}&r1=${r1}&r2=${r2}`);
    }
</script></body></html>)rawliteral";

// =========================================================
// == SETUP & LOOP =========================================
// =========================================================

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
    Serial.begin(115200);

    // Camera Configuration
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    
    // PSRAM Optimization
    if(psramFound()){
        config.frame_size = FRAMESIZE_UXGA; // Init with high res buffer
        config.jpeg_quality = 12;
        config.fb_count = 2; // Double buffering for smooth video
        config.grab_mode = CAMERA_GRAB_LATEST; 
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    }
    
    // Init Camera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }

    // Drop down to QVGA for faster streaming & B/W mode
    sensor_t * s = esp_camera_sensor_get();
    if(s){
        s->set_framesize(s, FRAMESIZE_QVGA);
        s->set_special_effect(s, 2); // 2 = Grayscale
    }

    // Start WiFi AP
    if (ap_password != NULL && strlen(ap_password) > 0) {
        WiFi.softAP(ap_ssid, ap_password);
    } else {
        WiFi.softAP(ap_ssid);
    }
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Start mDNS service
    if (MDNS.begin(HOSTNAME)) {
        Serial.println("mDNS responder started");
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("Error setting up MDNS responder!");
    }

    // Start Web Server
    httpd_handle_t server = NULL;
    httpd_config_t http_config = HTTPD_DEFAULT_CONFIG();
    http_config.stack_size = 8192; // Increase stack size for stability

    if (httpd_start(&server, &http_config) == ESP_OK) {
        httpd_uri_t index_uri = {"/", HTTP_GET, [](httpd_req_t* r){return httpd_resp_send(r,index_html,strlen(index_html));}, NULL};
        httpd_uri_t stream_uri = {"/stream", HTTP_GET, stream_handler, NULL};
        httpd_uri_t save_uri = {"/save", HTTP_GET, save_handler, NULL};
        httpd_uri_t status_uri = {"/status", HTTP_GET, status_handler, NULL};
        
        httpd_register_uri_handler(server, &index_uri);
        httpd_register_uri_handler(server, &stream_uri);
        httpd_register_uri_handler(server, &save_uri);
        httpd_register_uri_handler(server, &status_uri);
    }
}

void loop() { delay(1); }