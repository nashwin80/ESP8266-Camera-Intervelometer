#include "Arduino.h"

#include <Servo.h>

#include<ESP8266WiFi.h>
#include "ESP8266WebServer.h"

#include <WebSocketsServer.h>
#include <Hash.h>

#include <FS.h>

#define USE_SERIAL Serial

#define MYSSID "Lumix"
#define MYPASSWORD "LumixDMCLX5"

ESP8266WebServer server(80);

WebSocketsServer webSocket = WebSocketsServer(81);

Servo servo1;

bool running = false;
bool fire = false;

unsigned int offAngle=0, onAngle=0, interval=0, limit=0, shotCount = 0;

extern "C" {
#include "user_interface.h"
}

os_timer_t myTimer;

void timerCallback(void *pArg) {
      fire = true;
}

void user_init(void) {
  os_timer_disarm(&myTimer);
  os_timer_setfn(&myTimer, timerCallback, NULL);
  os_timer_arm(&myTimer, interval*1000, true);
}

void initServos(){
  servo1.write(20);
  delay(500);
}

void fireServos(){
  if ((fire == true) && (running == true)){
    servo1.write(onAngle);
    delay(500);
    servo1.write(offAngle);
    delay(500);    
    shotCount++;
    if (limit > 0 && shotCount > limit){
      running = false;
    }
    fire = false;
  }
  
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
                // send message to client
                webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            {
              USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
  
              
              String text = String((char *) &payload[0]);
              
              if (text.startsWith("stop:")) {
                Serial.println("Servo Stopped and set to offAngle");
                interval = 0;
                running = false;
                shotCount = 0;
              }
      
              if (text.startsWith("start:")) {
                String myString = text;
                int colonIndex = myString.indexOf(':');
                int commaIndex = myString.indexOf(',',colonIndex+1);
                int secondCommaIndex = myString.indexOf(',', commaIndex+1);
                int thirdCommaIndex = myString.indexOf(',', secondCommaIndex+1);
                
                String firstValue = myString.substring(colonIndex+1, commaIndex);
                String secondValue = myString.substring(commaIndex+1, secondCommaIndex);
                String thirdValue = myString.substring(secondCommaIndex+1, thirdCommaIndex);
                String fourthValue = myString.substring(thirdCommaIndex+1);

                onAngle = firstValue.toInt();
                offAngle = secondValue.toInt();
                interval = thirdValue.toInt();
                limit = fourthValue.toInt();

                onAngle = (onAngle < 20) ? 20 : onAngle;
                onAngle = (onAngle > 180) ? 180 :onAngle;

                offAngle = (offAngle < 20) ? 20 : offAngle;
                offAngle = (offAngle > 180) ? 180 :offAngle;

                interval = (interval < 0) ? 0: interval;

                limit = (limit < 0) ? 0: limit;

                running = true;
                fire = true;
                fireServos();
                user_init();
              }

              if (text.startsWith("onAngle:")) {
                String myString = text;
                int colonIndex = myString.indexOf(':');
                int commaIndex = myString.indexOf(',',colonIndex+1);
                
                String firstValue = myString.substring(colonIndex+1, commaIndex);

                onAngle = firstValue.toInt();
                servo1.write(onAngle);
                running = false;
                shotCount = 0;
              }

              if (text.startsWith("offAngle:")) {
                String myString = text;
                int colonIndex = myString.indexOf(':');
                int commaIndex = myString.indexOf(',',colonIndex+1);
                
                String firstValue = myString.substring(colonIndex+1, commaIndex);

                offAngle = firstValue.toInt();
                servo1.write(offAngle);
                running = false;
                shotCount = 0;
              }

              if (text.startsWith("delaySeconds:")) {
                String myString = text;
                int colonIndex = myString.indexOf(':');
                int commaIndex = myString.indexOf(',',colonIndex+1);
                
                String firstValue = myString.substring(colonIndex+1, commaIndex);

                interval = firstValue.toInt();
                user_init();
              }

              if (text.startsWith("delayMinutes:")) {
                String myString = text;
                int colonIndex = myString.indexOf(':');
                int commaIndex = myString.indexOf(',',colonIndex+1);
                
                String firstValue = myString.substring(colonIndex+1, commaIndex);

                interval = firstValue.toInt();
                interval = interval * 60;
                user_init();
              }

              if (text.startsWith("limit:")) {
                String myString = text;
                int colonIndex = myString.indexOf(':');
                int commaIndex = myString.indexOf(',',colonIndex+1);
                
                String firstValue = myString.substring(colonIndex+1, commaIndex);

                limit = firstValue.toInt();
              }

              if (text.startsWith("shotCount")){
                unsigned char temp[30];
                
                sprintf((char*)temp,"Shot Count: %d",shotCount);
                USE_SERIAL.printf("%s",temp);
                webSocket.sendTXT(num, temp);
              } else {
                // send message to client
//                webSocket.sendTXT(num, payload);
              }
              

            // send data to all connected clients
//            webSocket.broadcastTXT("message here");
            }
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[%u] get binary lenght: %u\n", num, lenght);
            hexdump(payload, lenght);

            // send message to client
            // webSocket.sendBIN(num, payload, lenght);
            break;
    }

}

void handle_root(){
  server.send(200, "text/plain", "Camera Intervelometer");
  delay(100);
}


//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}




bool handleFileRead(String path){
  USE_SERIAL.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void setup(){
  Serial.begin(115200);

  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      USE_SERIAL.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    USE_SERIAL.printf("\n");
  }

  WiFi.softAP(MYSSID, MYPASSWORD);
  IPAddress ip = WiFi.softAPIP();

  Serial.println(ip);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "FileNotFound");
  });

  server.begin();
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  servo1.attach(16);
  initServos();
}

void loop(){
//  Serial.println("handle client - begin");
  server.handleClient();
  delay(10);
  yield();
  webSocket.loop();
  yield();
  delay(10);
  fireServos();
  yield();
}

