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

struct ServerParams {
   char key[BUFFER_SIZE];
   int value;
};

int server_input_index;
char server_input;
char server_buffer[BUFFER_SIZE];

int server_response_pending;
char server_response_buffer[BUFFER_SIZE];
char server_response[BUFFER_SIZE];
char server_response_template[] =
"HTTP/1.1 200 OK\r\n"
"Connection: close\r\n"
"Content-Type: application/json\r\n"
"Content-Length: %d\r\n"
"\r\n"
"%s";

int blink_last_state;
int blink_last_rate;
long blink_last_changed;

void setup()
{
  server_input_index = 0;
  blink_last_state = LOW;
  blink_last_rate = 0;
  blink_last_changed = millis();
  server_response_pending = 0;
  Serial.begin(57600);
  pinMode(13, OUTPUT);
}

int is_header(char line[]){
    return (strstr(line, "GET") != NULL || strstr(line, "POST") != NULL);
}

// ie., GET /led/500/ HTTP/1.1
struct ServerParams get_params(char* line) {
  struct ServerParams params;
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
void blink_do_loop(){

    if (blink_last_rate == 0) {
      blink_last_state = LOW;
      digitalWrite(13, blink_last_state);
    } else {
      long now = millis();
      if ((now - blink_last_changed) > blink_last_rate) {
        if (blink_last_state == LOW) {
          blink_last_state = HIGH;
        } else {
          blink_last_state = LOW;
        }
        digitalWrite(13, blink_last_state);
        blink_last_changed = now;
      }
    }
}

void server_do_response() {
  if (!server_response_pending) {
    return;
  }
  server_response_pending = 0;

  int count = strlen(server_response_buffer);
  sprintf(server_response, server_response_template, count, server_response_buffer);
  Serial.print(server_response);
}

void loop()
{
  if (Serial.available() > 0) {

    // Do not overflow buffer
    if (server_input_index >= BUFFER_SIZE) {
      server_input_index = 0;
    }

    server_input = Serial.read();

    if (server_input == '\n') {
      server_buffer[server_input_index] = '\0';
      server_input_index = 0;
    } else {
      server_buffer[server_input_index++] = server_input;
    }

    if (server_input == '\n' && is_header(server_buffer)) {
      struct ServerParams params = get_params(server_buffer);

      // first example is to make led to blink
      if (strcmp(params.key, "led") == 0) {
        blink_last_rate = params.value;
        strcpy(server_response_buffer, "");
      }

      //second example is to read from light sensors
      if (strcmp(params.key, "sensors") == 0) {
        int sensorRightState = analogRead(A6);
        int sensorLeftState = analogRead(A3);
        sprintf(server_response_buffer, "[%d, %d]", sensorRightState, sensorLeftState);
      }

      server_response_pending = 1;
    }
  }

  blink_do_loop();
  server_do_response();
}
