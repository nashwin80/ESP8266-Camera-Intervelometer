# ESP8266-Camera-Intervelometer

##What another camera intervelometer?
  I had an old Panasomic Lumix DMC-LX5 camera which did not have a remote shutter. I had made one using an ATMEGA328 and a servo a long time back but lost it somewhere when I shifted to a new place. Since i had a few ESP8266's(nodeMcu v0.9) lying around I decided to use that instead.
##Parts used:
  - NodeMcu v0.9
  - Metal gear servo
  - USB cable
  - Power Bank
  - Sawed Off Ruler
  - SuperGlue to stick the servo to the sawed off ruler
  - 1/4" Mount Adapter for Tripod Screw to Flash Hot Shoe
  - DuPont wire
  - Breadboard Jumper Cable
  
##Usage:
  Upload sketch to your ESP8266
  Upload the data folder to your ESP8266 using the Tools-->ESP8266 Sketch Data Upload feature
  
  In the code GPIO16 (D0 on my nodeMcu) is used as the signal for the servo. Feel free to change it to a different port if required.
  
  Open the URL 192.168.4.1 which is the default ESP8266 ip when in access point mode. 
  
  The interface will let you choose the following:-
######On Angle
    This is the angle at which the servo will press the camera button to take a shot
######Off Angle
    This is the angle at which the servo will release the camera button
######DelaySeconds
    The is the delay between shots in seconds
######DelayMinutes
    The is the delay between shots in Minutes
######Number of Shots
    This limits how many shots are taken. After the number of shots the servo will not fire unless the start button is clicked again.
    
##TBD
  - Power Saving by turning off WiFi after hitting start
  - Slicker interface
  - Tutorial on how and why the code the written the way it is
  - YouTube video with demo of how to use the program
