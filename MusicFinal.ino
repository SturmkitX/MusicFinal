#include "PCM.h"

#define DEBUG true
#define BUFFER_SIZE 3072

volatile unsigned char sample_list[BUFFER_SIZE];
volatile bool can_play = false; // can only be true when we have received 1024 bytes (1024 / 64 = 16 chunks)
volatile int total_received = 0;  // per 1024-bytes chunk
volatile bool has_finished = true;
int old_received = 0;

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
      delay(50);
      int connectionId = Serial1.read() - 48; // read() function returns
      // ASCII decimal value and 0 (the first decimal number) starts at 48

      // if there is anything left in the buffer, it must be some music
      Serial1.find(":");
      while (Serial1.available())
      {
        sample_list[total_received++] = Serial1.read();
      }

      if (total_received != old_received)
      {
        Serial.println(total_received);
        old_received = total_received;
      }

      if (total_received >= BUFFER_SIZE)
      {
        can_play = true;
      }

      if (can_play)
      {
        has_finished = false;
        can_play = false;
        total_received = 0;
        Serial.println("Play some music now");
        startPlayback(sample_list, BUFFER_SIZE, &has_finished);
      }
//      else
      if (has_finished)
      {
        String request = "SENDC\r\n";
        String cipSend = "AT+CIPSEND=";
        cipSend += connectionId;
        cipSend += ",";
        cipSend += request.length();
        cipSend += "\r\n";
        Serial1.print(cipSend);
        delay(25);
        Serial1.print(request);
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

