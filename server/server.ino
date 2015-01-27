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

#include <Servo.h>

#define SERVER_BUFFER_BIG 256
#define SERVER_BUFFER_SMALL 32

#define SENSOR_RIGHT_PIN A6
#define SENSOR_LEFT_PIN A3

#define SERVO_RIGHT_PIN 6
#define SERVO_LEFT_PIN 5
#define SERVO_STOP 99

#define SERVER_BAUD 57600

#define ACTION_BLINK "blink"
#define ACTION_SENSE "sense"
#define ACTION_MOVE "move"
#define ACTION_UNKNOWN "unknown"

#define SERVER_RESPONSE_OK(content) server_set_response(200, "OK", content)
#define SERVER_RESPONSE_BAD() server_set_response(400, "Bad Request", "")

struct RequestParams {
   char key[SERVER_BUFFER_SMALL];
   int value1;
   int value2;
};

int server_input_index;
char server_input;
char server_buffer[SERVER_BUFFER_BIG];
char server_response_template[] =
"HTTP/1.1 %d %s\r\n"
"Connection: close\r\n"
"Content-Type: application/json\r\n"
"Content-Length: %d\r\n"
"Access-Control-Allow-Origin: *\r\n"
"\r\n"
"%s";

int blink_last_state;
int blink_last_rate;
long blink_last_changed;

Servo move_servo_left;
Servo move_servo_right;

void setup()
{  
  blink_last_state = LOW;
  blink_last_rate = 0;
  blink_last_changed = millis();
  pinMode(13, OUTPUT);
  
  move_servo_left.attach(SERVO_LEFT_PIN);
  move_servo_left.write(SERVO_STOP);
  move_servo_right.attach(SERVO_RIGHT_PIN);
  move_servo_right.write(SERVO_STOP);
  
  server_input_index = 0;
  Serial.begin(SERVER_BAUD);
}

int server_is_header(char* line){
    return (strstr(line, "GET") != NULL);
}

struct RequestParams server_get_params(char* line) {
  char tkey[SERVER_BUFFER_SMALL];
  char tvalue1[SERVER_BUFFER_SMALL];
  char tvalue2[SERVER_BUFFER_SMALL];

  struct RequestParams request_params;
  strcpy(request_params.key, ACTION_UNKNOWN);
  request_params.value1 = -1;
  request_params.value2 = -1;

  int filled = sscanf(line, "%*[^/]%*c%[^/]%*c%[^/]%*c%[^/]", tkey, tvalue1, tvalue2);

  if (filled != EOF) {
    strcpy(request_params.key, tkey);
    request_params.value1 = atoi(tvalue1);
    request_params.value2 = atoi(tvalue2);
  }

  return request_params;
}

void server_set_response(int code, char* message, char* content) {
  char response[SERVER_BUFFER_BIG];
  int count = strlen(content);
  
  sprintf(response, server_response_template, code, message, count, content);
  
  Serial.print(response);
}

void blink_loop(){

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

void loop()
{
  if (Serial.available() > 0) {

    // Do not overflow buffer
    if (server_input_index >= SERVER_BUFFER_BIG) {
      server_input_index = 0;
    }

    server_input = Serial.read();

    if (server_input == '\n') {
      server_buffer[server_input_index] = '\0';
      server_input_index = 0;
    } else {
      server_buffer[server_input_index++] = server_input;
    }

    if (server_input == '\n' && server_is_header(server_buffer)) {
      struct RequestParams request_params = server_get_params(server_buffer);

      if (strcmp(request_params.key, ACTION_BLINK) == 0) {
        blink_last_rate = request_params.value1;
        SERVER_RESPONSE_OK("");

      } else if (strcmp(request_params.key, ACTION_SENSE) == 0) {
        int sensorLeftState = analogRead(SENSOR_LEFT_PIN);
        int sensorRightState = analogRead(SENSOR_RIGHT_PIN);

        char content[SERVER_BUFFER_SMALL];
        sprintf(content, "[%d, %d]", sensorLeftState, sensorRightState);
        SERVER_RESPONSE_OK(content);

      } else if (strcmp(request_params.key, ACTION_MOVE) == 0 ){
        move_servo_left.write(request_params.value1);
        move_servo_right.write(request_params.value2);
        SERVER_RESPONSE_OK("");

      } else {
        SERVER_RESPONSE_BAD();
      }
    }
  }

  blink_loop();
}
