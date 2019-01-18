//esp8266-wifi-car.ino
/*
This example will open a configuration portal for 60 seconds when first powered up. ConfigOnSwitch is a a better example for most situations but this has the advantage 
that no pins or buttons are required on the ESP8266 device at the cost of delaying 
the user sketch for the period that the configuration portal is open.

Also in this example a password is required to connect to the configuration portal 
network. This is inconvenient but means that only those who know the password or those 
already connected to the target WiFi network can access the configuration portal and 
the WiFi network credentials will be sent from the browser over an encrypted connection and
can not be read by observers.

Hints from:
https://github.com/squix78/esp8266-projects/tree/master/arduino-ide/wifi-car
https://github.com/ric96/esp8266car
https://github.com/nouyang/robot_esp8266-12E/tree/master/wireless_motor_gui_AP
https://github.com/Alictronix/ESP8266-12E-Robot

https://github.com/jshaw/NewPingESP8266/
https://community.thinger.io/t/echogarage-project/339

https://gist.github.com/NeoCat/a57f73b8db0605e1763d3ca1a1f75941

// ESP8266 has 32bit integers
// ESP8266 Pins
//  4(SDA) --- AMG8833 SDA
//  5(SCL) --- AMG8833 SCL
//  13     --- LED (Anode) via 100ohm

*/

// #define DEBUG 1

#include <Arduino.h>

#include <ESP8266WiFi.h>
// #include <DNSServer.h>
// #include <WiFiClient.h> 
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <ArduinoOTA.h>

// WiFi parameters
const char* ssid = "XT1039-7628-p";
const char* password = "90b0db55ff9a";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
String html_home;
// String css_style;

// Onboard I/O pins on the NodeMCU board
const int LED_PIN = D4; // GPIO2 on NodeMCU and WeMos controls the onboard LED.

// I2C 0.96" OLED display
// SSD1306  display(0x3c, D5, D6); //sda-D5, sck -D6
// on GPIO6 and GPIO7

const int MOTOR_PWM_LEFT = D1;  // GPIO5
const int MOTOR_PWM_RIGHT = D2;  // GPIO4
const int MOTOR_DIR_LEFT = D3;  // GPIO0
const int MOTOR_DIR_RIGHT = D4;  // GPIO2
#define MOTOR_BACK LOW
#define MOTOR_FWD HIGH

// int flagf = 0, flagb = 0, flagr = 0, flagl = 0;


// Sonar HC-SR04
// D0: GPIO4 not working that great
// SD2: GPIO9 internally used to control the flash memory
// SD3: GPIO10 can be used as input only

const int SONAR_LEFT = D7;  // 13; // D7
const int SONAR_CENTER = D6;  // 12; // D6
const int SONAR_RIGHT = D5;  //14; // D5

unsigned int left_distance = 0;
unsigned int center_distance=0;
unsigned int right_distance = 0;
String old_str_distance="";

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot() {
    server.send(200, "text/html", "<h1>You are connected</h1>");
}

void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET)?"GET":"POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i<server.args(); i++){
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}

void setup() {
    Serial.begin(115200);
    unsigned long startedAt = millis();
    Serial.print(F("startedAt: "));
    Serial.println(startedAt);
    Serial.println(F("Serial started at 115200"));

    setupServer();
    setupOTA();
    setupPins();

    Serial.print("setup() done in ");
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis()- startedAt);
    Serial.print(waited / 1000);
    Serial.print(" secs in setup(). Connection result = ");
    Serial.println(connRes);
    if (WiFi.status()!=WL_CONNECTED)
    {
        Serial.println("Failed to connect to WiFi but setup finished anyway");
    }
}

/* * * Movement Functions * * */
/*** Motors stop Function ***/
void stop() {
    digitalWrite(LED_PIN, HIGH);
    #ifdef DEBUG
       Serial.println("stop");
    #endif
    // analogWrite(MOTOR_PWM_LEFT, 512);
    analogWrite(MOTOR_PWM_LEFT, 0);
    // analogWrite(MOTOR_PWM_RIGHT, 512);
    analogWrite(MOTOR_PWM_RIGHT, 0);
}

/*** Motors forward Function ***/
void forward() {
    // digitalWrite(LED_PIN, LOW);
    #ifdef DEBUG
       Serial.println("forward");
    #endif
    digitalWrite(MOTOR_DIR_LEFT, MOTOR_FWD);
    digitalWrite(MOTOR_DIR_RIGHT, MOTOR_FWD);

    analogWrite(MOTOR_PWM_LEFT, 1023);
    analogWrite(MOTOR_PWM_RIGHT, 1023);
}

/*** Motors backward Function ***/
void backward() {
    #ifdef DEBUG
        Serial.println("backward");
    #endif
    digitalWrite(MOTOR_DIR_LEFT, MOTOR_BACK);
    digitalWrite(MOTOR_DIR_RIGHT, MOTOR_BACK);

    // analogWrite(MOTOR_PWM_LEFT, 0);
    analogWrite(MOTOR_PWM_LEFT, 1023);
    // analogWrite(MOTOR_PWM_RIGHT, 0);
    analogWrite(MOTOR_PWM_RIGHT, 1023);
}

/*** Motor turn Left Function ***/
void left() {
    #ifdef DEBUG
        Serial.println("left");
    #endif
    digitalWrite(MOTOR_DIR_LEFT, MOTOR_BACK);
    digitalWrite(MOTOR_DIR_RIGHT, MOTOR_FWD);

    analogWrite(MOTOR_PWM_LEFT, 0);
    analogWrite(MOTOR_PWM_RIGHT, 1023);
}

/*** Motor turn Right Function ***/
void right() {
    #ifdef DEBUG
        Serial.println("right");
    #endif
    digitalWrite(MOTOR_DIR_LEFT, MOTOR_FWD);
    digitalWrite(MOTOR_DIR_RIGHT, MOTOR_BACK);

    analogWrite(MOTOR_PWM_LEFT, 1023);
    analogWrite(MOTOR_PWM_RIGHT, 0);
}


/* * * Setup Methods * * */
/*** Prepare web page files from fs ***/
void prepareFile() {
    Serial.println("Prepare file system");
    SPIFFS.begin();

    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
        Serial.println("HTML file open failed");  
    } 
    else {
        Serial.println("HTML file open success");

        html_home = "";
        while (file.available()) {
            // Serial.write(file.read());
            String line = file.readStringUntil('\n');
            html_home += line + "\n";
        }
        file.close();

        // Serial.print(html_home);
    }

    // File css_file = SPIFFS.open("/style.css", "r");
    // if (!css_file) {
    //     Serial.println("CSS file open failed");  
    // } 
    // else {
    //     Serial.println("CSS file open success");

    //     css_style = "";
    //     while (css_file.available()) {
    //         // Serial.write(file.read());
    //         String line = css_file.readStringUntil('\n');
    //         css_style += line + "\n";
    //     }
    //     css_file.close();
    //     // Serial.print(html_home);
    // }
}


/*** Setup pins for LEDs, motors control and sonar sensors ***/
void setupPins() {
    // setup LEDs and Motors
    Serial.println("Setup LED and motor pins");
    pinMode(LED_PIN, OUTPUT);    //Pin D0 is LED
    digitalWrite(LED_PIN, HIGH); //Initial state is HIGH (OFF)

    pinMode(MOTOR_PWM_LEFT, OUTPUT);
    pinMode(MOTOR_DIR_LEFT, OUTPUT);
    pinMode(MOTOR_PWM_RIGHT, OUTPUT);
    pinMode(MOTOR_DIR_RIGHT, OUTPUT);

    digitalWrite(MOTOR_PWM_LEFT, HIGH); //Initial state is HIGH (OFF)
    digitalWrite(MOTOR_PWM_RIGHT, HIGH); //Initial state is HIGH (OFF)
    // Set initial speed to 0
    analogWrite(MOTOR_PWM_LEFT, 0);
    analogWrite(MOTOR_PWM_RIGHT, 0);

    // Sonar sensors
    pinMode(SONAR_RIGHT, INPUT); // Sets the echoPin as an Input
    pinMode(SONAR_CENTER, INPUT); // Sets the echoPin as an Input
    pinMode(SONAR_LEFT, INPUT); // Sets the echoPin as an Input
}


/*** webSocket responder ***/
void webSocketEvent(
    uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: 
        {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            // send message to client
            webSocket.sendTXT(num, "Connected");
            break;
        }
        case WStype_TEXT:
            #ifdef DEBUG
                Serial.printf("[%u] get text: %s\n", num, payload);
            #endif
            if (payload[0] == '#') {
                if(payload[1] == 'F') {
                  forward();
                }
                else if(payload[1] == 'B') {
                  backward();
                }
                else if(payload[1] == 'L') {
                  left();
                }
                else if(payload[1] == 'R') {
                  right();
                }
                else {
                  stop();
                }
            }
            break;
    }
}

/*** Setup Over The Air update***/
void setupOTA(){
    Serial.print("Configuring OTA update...");
    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
            Serial.print("Unmounting SPIFFS...");
            SPIFFS.end();
            Serial.println("done");
        }
        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
          Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
          Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
          Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
          Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
    Serial.println("done");
}

/*** Setup WiFi and Webserver ***/
void setupServer() {
    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    // read the html code to html_homes
    prepareFile();

    /* WiFiManager */
    WiFiManager wifiManager;

    // wifiManager.setSTAStaticIPConfig(
    //   IPAddress(192, 168, 4, 1),
    //   IPAddress(192, 168, 4, 1),
    //   IPAddress(255, 255, 255, 0)
    // );

    // Sets timeout in seconds until configuration portal gets turned off.
    // if not specified device will remain in configuration mode until switched
    // off via webserver.
    // if (WiFi.SSID() != "") wifiManager.setConfigPortalTimeout(60); //If no access point name has been previously entered disable timeout.
    // wifiManager.autoConnect(ssid, password);
    // or use this for auto generated name ESP + ChipID
    wifiManager.autoConnect();

    // It starts an access point 
    // and goes into a blocking loop awaiting configuration
    /* Set these to your desired credentials. */
    // const char *ssid = "esp8266-wifi-car";
    // const char *password = "esp8266-wifi-car-toto";  // must be 8 chars for WPA-PSK
    // if (!wifiManager.startConfigPortal(ssid, password))  //Delete these two parameters if you do not want a WiFi password on your configuration access point
    // {
    //     Serial.print("Configuring access point...");
    //     // WiFi.mode(WIFI_AP);
    //     /* You can remove the password parameter if you want the AP to be open. */
    //     // WiFi.softAP(ssid, password);
    //     // IPAddress ip(192, 168, 4, 1);
    //     // IPAddress gateway(192, 168, 4, 1);
    //     // IPAddress subnet(255, 255, 255, 0);
    //     // WiFi.softAPConfig(ip, gateway, subnet);
    //     // WiFi.begin();
    //     delay(500);
    //     IPAddress myIP = WiFi.softAPIP();
    //     Serial.print("AP IP address: ");
    //     Serial.println(myIP);
    //     Serial.printf("MAC address = %s\n", WiFi.softAPmacAddress().c_str());

    // If we get here we have connected to the WiFi
    Serial.println("Connected to WiFi network.");
    Serial.print("IP: ");              
    Serial.println(WiFi.localIP()); 
    WiFi.printDiag(Serial); //Remove this line if you do not want to see WiFi password printed

    // Start webSocket server
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    if(MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
    } else {
        Serial.println("Error setting up MDNS responder!");
    }

    // Handle index
    server.on("/", []() {
        // send home.html
        server.send(200, "text/html", html_home);
    });

    // server.on("/style.css", []() {
    //     // Send home.html
    //     server.send(200, "text/css", css_style);
    // });

    server.on("/current", [](){
        String str;
        server.send(200, "text/plain", get_current_values_str(str));
    });

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");

    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);

    Serial.printf("MDNS started\n");
}

void sonar_distance_measurements(
    unsigned int *ptr_distance, const int PIN) {
    #ifdef DEBUG
    // Performance benchmark
        unsigned int startedAt2, waited, waited2;
        unsigned int startedAt = micros();
    #endif
    /* Sonar distance measurements */
    // We change pinMode to use only one pin as TRIGGER and ECHO pins
    // 10us starting pulse
    // 25us to 150us PWM signal based of the distance
    // If no obstacle, then a 38us pulse is generated to confirm no obstacle.
    // Distance = 1/2 * Time_2wayEcho * Sonic speed.
    // HARDWARE limit is at 320-350cm because of loses with wide emission angle
    // https://arduino.stackexchange.com/questions/29742/how-do-i-use-two-pulsein-functions-simultaneously-in-arduino

    pinMode(PIN, OUTPUT);

    // Clears the trigPin
    digitalWrite(PIN, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN, LOW);

    pinMode(PIN, INPUT);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    // noInterrupts();  // Only for ARM based
    unsigned int duration = pulseIn(PIN, HIGH, 2500);  // 2500 param is 2ms
    // interrupts();  // Only for ARM based

    // pulseIn takes roughly 1ms per 10cm distance
    // We set a software limit timeout at 30cm, thus 0.3*2*0.03432=2ms
    #ifdef DEBUG
        waited = micros()- startedAt;
        startedAt2 = micros();
    #endif
    // Calculating the distance
    microsecondsToCentimeters(duration, ptr_distance);

    #ifdef DEBUG
        waited2 = micros()- startedAt2;
        // Prints the distance on the Serial Monitor
        Serial.print("SONAR_PIN");
        Serial.print(PIN);
        Serial.print(": ");
        // Serial.print(distance, 5);
        Serial.print(*ptr_distance);
        Serial.print("cm");
        Serial.print(" | sens: ");
        Serial.print(waited);
        Serial.print("us | comp:");
        Serial.print(waited2);
        Serial.println("us");
    #endif
}

void microsecondsToCentimeters(unsigned int duration, unsigned int *ptr_distance) {   
    // We fix a software limit at 200cm, thus 2*2*0.034=136ms
    // if (duration > 13600){
    //     Serial.print(duration);
    //     return 200;
    // }
    // float soundVelocity = 0.03432;  // Sonic speed is 3432m/s
    // return (float (duration) * soundVelocity) / 2.0; // compute time=10us
    // return (float (duration) * 0.034) / 2; // compute time=10us
    // return int((duration * 0.034) / 2);  // compute time=10us
    // return int(float((duration / 2.0) / 29.155);
    // return int((duration / 2.0) / 29.155);

    // return int((duration / 2) / 29.155);
    // The speed of sound is 340 m/s or 29 microseconds per centimeter.
    // The ping travels out and back, so to find the distance of the
    // object we take half of the distance travelled.
    if(duration == 0){
        *ptr_distance = 36;  // max possible sensable is 35cm.
    } else {
        *ptr_distance = duration / 29 / 2; // compute time 1us: probably shifting register
    }
}

String& get_current_values_str(String& ret){
    sonar_distance_measurements(&left_distance, SONAR_LEFT);  // 1ms to 2.5ms
    sonar_distance_measurements(&center_distance, SONAR_CENTER);  // 1ms to 2.5ms
    sonar_distance_measurements(&right_distance, SONAR_RIGHT);  // 
    ret = "";
    ret += left_distance;
    ret += ",";
    ret +=  center_distance;
    ret += ",";
    ret +=  right_distance;
    ret += "";
    return ret;
}

void loop() {
    ArduinoOTA.handle();
    String str;
    static unsigned long last_read_ms = millis();
    unsigned long now = millis();
    if (now - last_read_ms > 100) {
        last_read_ms += 100;

        /* Sensors reading */
        unsigned int old_left_distance = left_distance;
        unsigned int old_center_distance = center_distance;
        unsigned int old_right_distance = right_distance;
        sonar_distance_measurements(&left_distance, SONAR_LEFT);  // Duration: 1ms to 2.5ms
        sonar_distance_measurements(&center_distance, SONAR_CENTER);
        sonar_distance_measurements(&right_distance, SONAR_RIGHT);
        /* Only send if values are modified */
        if ((old_left_distance != left_distance) || (( old_center_distance != center_distance)) || (( old_right_distance != right_distance))){
            str = "";
            str += left_distance;
            str += ",";
            str +=  center_distance;
            str += ",";
            str +=  right_distance;
            str += "";
            webSocket.broadcastTXT(str);
            #ifdef DEBUG
                Serial.println(str);
            #endif
        }
    }

    webSocket.loop();
    server.handleClient();
}

/* Benchmarks: */
/* Use int32 for shift register division with ESP8266
/*   ms |Arduino16MHz|ESP8266@160MHz|TEENSY31@96MHz|ARDUINO DUE@84MHz|
/* float|       3326 |          216 |          505 |             636 |
/* int32|       4446 |           28 |            9 |              28 |
/*      |ESP32@240MHz|TEENSY36@240MH|
/* float|         95 |           10 |      
/* int32|          3 |            4 |  
/* https://forum.arduino.cc/index.php?topic=337653.0
*/