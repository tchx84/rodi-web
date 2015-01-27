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

struct ServerParams {
   char key[1024];
   int value;
};

int server_input_index;
char server_input;
char server_buffer[1024];

int server_response_pending;
char server_response_buffer[1024];
char server_response[1024];
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

//Servo move_servo_left;
//Servo move_servo_right;
int move_servo_left_angle;
int move_servo_right_angle;
long move_last_changed;

void setup()
{
  server_input_index = 0;
  blink_last_state = LOW;
  blink_last_rate = 0;
  blink_last_changed = millis();
  server_response_pending = 0;
  //move_servo_left.attach(5);
  //move_servo_right.attach(6);
  move_servo_left_angle = 100;
  move_servo_right_angle = 100;
  move_last_changed = millis();
  Serial.begin(57600);
  pinMode(13, OUTPUT);  
}

int server_is_header(char line[]){
    return (strstr(line, "GET") != NULL || strstr(line, "POST") != NULL);
}

// ie., GET /led/500/ HTTP/1.1
struct ServerParams server_read_params(char* line) {
  struct ServerParams params;
  char tkey[1024];
  char tvalue[1024];

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

void move_do_loop() {
  long now = millis();
  if ((now - move_last_changed) < 100){
    return;
  }
  
  //move_servo_left.write(move_servo_left_angle);
  //move_servo_right.write(move_servo_right_angle);
  move_last_changed = now;
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
    if (server_input_index >= 1024) {
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
      struct ServerParams params = server_read_params(server_buffer);

      // first example is to make led to blink
      if (strcmp(params.key, "blink") == 0) {
        blink_last_rate = params.value;
        strcpy(server_response_buffer, "");
      }

      //second example is to read from light sensors
      if (strcmp(params.key, "sensors") == 0) {
        int sensorRightState = analogRead(A6);
        int sensorLeftState = analogRead(A3);
        sprintf(server_response_buffer, "[%d, %d]", sensorRightState, sensorLeftState);
      }

      if (strcmp(params.key, "move") == 0 ){
        move_servo_left_angle = params.value;
        move_servo_right_angle = params.value;
        strcpy(server_response_buffer, "");
      }

      server_response_pending = 1;
    }
  }

  blink_do_loop();
  //move_do_loop();
  
  server_do_response();
}
