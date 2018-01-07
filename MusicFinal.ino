#include "PCM.h"

#define DEBUG true
#define BUFFER_SIZE 3072

volatile unsigned char sample_list[BUFFER_SIZE];
volatile bool can_play = false; // can only be true when we have received 1024 bytes (1024 / 64 = 16 chunks)
volatile int total_received = 0;  // per 1024-bytes chunk
volatile bool has_finished = true;
bool should_disconnect = false;
bool client_ready = false;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  sendData("AT+RST\r\n", 2000, false); // reset module
  sendData("AT+CWMODE=2\r\n", 1000, false); // configure as access point
  sendData("AT+CIFSR\r\n", 1000, DEBUG); // get ipaddress
  sendData("AT+CWSAP?\r\n", 2000, DEBUG); // get SSID info (network name)
  sendData("AT+CIPMUX=1\r\n", 1000, false); // configure for multiple connections
  sendData("AT+CIPSERVER=1,5678\r\n", 1000, false); // turn on server on port 80
}

void loop() {
//  Serial.println(has_finished);
  if (Serial1.available()) {
    if (Serial1.find("+IPD,")) {
      delay(500);
      int connectionId = Serial1.read() - 48; // read() function returns
      // ASCII decimal value and 0 (the first decimal number) starts at 48
      
//      if(Serial1.indexOf("READY") != -1)
//      {
//        client_ready = true;
//        Serial1.find("READY");
//      }
//      else

      String status = "";
      for(int i=0; i<5; i++)
      {
        status += (char)Serial1.read();
      }
      
      if(status.indexOf("QUITS") != -1)
      {
        should_disconnect = true;
      }

      // if there is anything left in the buffer, it must be some music
      while(Serial1.available())
      {
        sample_list[total_received % BUFFER_SIZE] = Serial1.read();
        total_received++;
      }

      if(!should_disconnect)
      {
        if(can_play)
        {
          has_finished = false;
          can_play = false;
          total_received = 0;
          startPlayback(sample_list, BUFFER_SIZE, &has_finished);
        }
        else
        if(has_finished)
        {
          if(total_received == BUFFER_SIZE)
          {
            can_play = true;
          }
          else
          {
            String request = "SENDC\r\n";
            String cipSend = "AT+CIPSEND=";
            cipSend += connectionId;
            cipSend += ",";
            cipSend += request.length();
            cipSend += "\r\n";
            sendData(cipSend, 100, DEBUG);
            sendData(request, 150, DEBUG);
          }
        }
      }
      else
      {
        String closeCommand = "AT+CIPCLOSE=";
        closeCommand += connectionId; // append connection id
        closeCommand += "\r\n";
        sendData(closeCommand, 300, DEBUG);
        should_disconnect = false;
      }

      
    }
  }
}

String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  Serial1.print(command); // send command to the esp8266
  long int time = millis();
  while ((time + timeout) > millis()) {
    while (Serial1.available()) {
      char c = Serial1.read(); // read next char
      response += c;
    }
  }
  if (debug) {
    Serial.print(response);
  }
  return response;
}

