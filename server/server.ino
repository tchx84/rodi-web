/*
  Copyright (C) 2015 Martin Abente Lahaye - tch@sugarlabs.org.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
  USA
 */

#define BUFFER_SIZE 256

struct Params {
   char key[BUFFER_SIZE];
   int value;
};

int index;
char inChar;
char inLine[BUFFER_SIZE];

int outResponsePending;
char outResponse[] =
"HTTP/1.1 200 OK\r\n"
"Connection: close\r\n"
"Content-Type: application/json\r\n"
"Content-Length: 2\r\n"
"\r\n"
"{}";

int ledLastState;
int ledLastRate;
long ledLastChanged;

void setup()
{
  index = 0;
  ledLastState = LOW;
  ledLastRate = 0;
  ledLastChanged = millis();
  outResponsePending = 0;
  Serial.begin(57600);
  pinMode(13, OUTPUT);
}

int is_header(char line[]){
    return (strstr(line, "GET") != NULL || strstr(line, "POST") != NULL);
}

// ie., GET /led/500/ HTTP/1.1
struct Params get_params(char* line) {
  struct Params params;
  char tkey[BUFFER_SIZE];
  char tvalue[BUFFER_SIZE];

  int filled = sscanf(line, "%*[^/]%*c%[^/]%*c%[^/]", tkey, tvalue);

  if (filled != EOF) {
    strcpy(params.key, tkey);
    params.value = atoi(tvalue);
  } else {
    strcpy(params.key, "unknown");
    params.value = -1;
  }

  return params;
}

// It is extremely important to make non-blocking
void do_blink(){

    if (ledLastRate == 0) {
      ledLastState = LOW;
      digitalWrite(13, ledLastState);
    } else {
      long now = millis();
      if ((now - ledLastChanged) > ledLastRate) {
        if (ledLastState == LOW) {
          ledLastState = HIGH;
        } else {
          ledLastState = LOW;
        }
        digitalWrite(13, ledLastState);
        ledLastChanged = now;
      }
    }
}

void do_response() {
  if (!outResponsePending) {
    return;
  }
  outResponsePending = 0;
  Serial.print(outResponse);
}

void loop()
{
  if (Serial.available() > 0) {

    // Do not overflow buffer
    if (index >= BUFFER_SIZE) {
      index = 0;
    }

    inChar = Serial.read();

    if (inChar == '\n') {
      inLine[index] = '\0';
      index = 0;
    } else {
      inLine[index++] = inChar;
    }

    if (inChar == '\n' && is_header(inLine)) {
      struct Params params = get_params(inLine);

      // first example is to make led to blink
      if (strcmp(params.key, "led") == 0) {
        ledLastRate = params.value;
      }

      outResponsePending = 1;
    }
  }

  do_blink();
  do_response();
}
