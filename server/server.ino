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
#include <Adafruit_NeoPixel.h>

#define SERVER_BUFFER_BIG 256
#define SERVER_BUFFER_SMALL 32

#define SENSOR_RIGHT_PIN A6
#define SENSOR_LEFT_PIN A1
#define SENSOR_LIGHT_PIN A7

#define LED_PIN 13
#define PIXEL_PIN 11

#define SERVO_RIGHT_PIN 5
#define SERVO_LEFT_PIN 4
#define SERVO_STOP 0

#define SPEAKER_PIN 7

#define SEE_ECHO_PIN A0
#define SEE_TRIGGER_PIN 12
#define SEE_SHORT_DELAY 2
#define SEE_LONG_DELAY 10
#define SEE_MAX_DISTANCE 100
#define SEE_INTERNET_MAGIC_NUMBER 58.2

#define SERVER_BAUD 57600

#define ACTION_UNKNOWN 0
#define ACTION_BLINK 1
#define ACTION_SENSE 2
#define ACTION_MOVE 3
#define ACTION_SING 4
#define ACTION_SEE 5
#define ACTION_PIXEL 6
#define ACTION_SENSE_LIGHT 7
#define ACTION_TOGGLE 8
#define ACTION_TURN 9

#define SERVER_RESPONSE_OK(content) server_set_response(content)
#define SERVER_RESPONSE_BAD() Serial.print(server_response_template_bad)
#define SERVER_GOT_GET(line) (line[0] == 'G' && line[1] == 'E' && line[2] == 'T')

struct RequestParams {
   int action;
   int value1;
   int value2;
   int value3;
};

int server_input_index;
char server_input;
char server_buffer[SERVER_BUFFER_SMALL];
char server_response_content[SERVER_BUFFER_SMALL];

char server_response_template[] =
"HTTP/1.1 200 OK\n"
"Connection: close\n"
"Content-Type: application/json\n"
"Content-Length: %d\n"
"Access-Control-Allow-Origin: *\n"
"\n"
"%s";

char server_response_template_bad[] =
"HTTP/1.1 400 Bad Request\n"
"Connection: close\n"
"Content-Type: application/json\n"
"Content-Length: 0\n"
"Access-Control-Allow-Origin: *\n"
"\n"
"";

int blink_last_state;
int blink_last_rate;
int blink_is_off;
long blink_last_changed;
Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

Servo move_servo_left;
Servo move_servo_right;
bool servo_left_attached;
bool servo_right_attached;

long see_distance;
float see_duration;

void setup()
{
  blink_last_state = LOW;
  blink_last_rate = 0;
  blink_is_off = 1;
  blink_last_changed = millis();
  servo_left_attached = false;
  servo_right_attached = false;
  pinMode(LED_PIN, OUTPUT);

  pinMode(SPEAKER_PIN, OUTPUT);

  pinMode(SEE_TRIGGER_PIN, OUTPUT);
  pinMode(SEE_ECHO_PIN, INPUT);

  pixel.begin();
  pixel.setPixelColor(0, pixel.Color(0,0,0));
  pixel.show();

  server_input_index = 0;
  Serial.begin(SERVER_BAUD);
}

struct RequestParams server_get_params(char* line) {
  char taction[SERVER_BUFFER_SMALL];
  char tvalue1[SERVER_BUFFER_SMALL];
  char tvalue2[SERVER_BUFFER_SMALL];
  char tvalue3[SERVER_BUFFER_SMALL];

  struct RequestParams request_params;
  request_params.action = -1;
  request_params.value1 = -1;
  request_params.value2 = -1;
  request_params.value3 = -1;

  int filled = sscanf(line, "%*[^/]%*c%[^/]%*c%[^/]%*c%[^/]%*c%[^/]", taction, tvalue1, tvalue2, tvalue3);

  if (filled != EOF) {
    request_params.action = atoi(taction);
    request_params.value1 = atoi(tvalue1);
    request_params.value2 = atoi(tvalue2);
    request_params.value3 = atoi(tvalue3);
  }

  return request_params;
}

void server_set_response(char* content) {
  char response[SERVER_BUFFER_BIG];
  int count = strlen(content);

  sprintf(response, server_response_template, count, content);

  Serial.print(response);
}

void blink_loop(){
    if (blink_is_off) {
      return;
    }

    if (blink_last_rate == 0) {
      blink_last_state = LOW;
      blink_is_off = 1;
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
    if (server_input_index >= SERVER_BUFFER_SMALL) {
      server_input_index = 0;
    }

    server_input = Serial.read();

    // start condition
    if (server_input == 'G') {
      server_input_index = 0;
    }

    // end condition
    if (server_input == '\n') {
      server_buffer[server_input_index] = '\0';
      server_input_index = 0;
    } else {
      server_buffer[server_input_index++] = server_input;
    }

    if (server_input == '\n' && SERVER_GOT_GET(server_buffer)) {
      struct RequestParams request_params = server_get_params(server_buffer);

      switch (request_params.action) {
        case ACTION_BLINK: {
          blink_last_rate = request_params.value1;
          blink_is_off = 0;
          SERVER_RESPONSE_OK("");
          break;
        }
        case ACTION_SENSE: {
          int sensorLeftState = analogRead(SENSOR_LEFT_PIN);
          int sensorRightState = analogRead(SENSOR_RIGHT_PIN);

          sprintf(server_response_content, "[%d, %d]", sensorLeftState, sensorRightState);
          SERVER_RESPONSE_OK(server_response_content);
          break;
        }
        case ACTION_MOVE: {
          if(request_params.value1 == SERVO_STOP){
            move_servo_left.detach();
            servo_left_attached = false;
          }else{
            if(!servo_left_attached){
              move_servo_left.attach(SERVO_LEFT_PIN);
              servo_left_attached = true;
            }
            int tmp = map(request_params.value1, -100, 100, 0, 180);
            move_servo_left.write(constrain(tmp, 0, 180));
          }

          if(request_params.value2 == SERVO_STOP){
            move_servo_right.detach();
            servo_right_attached = false;
          }else{
            if(!servo_right_attached){
              move_servo_right.attach(SERVO_RIGHT_PIN);
              servo_right_attached = true;
            }
            int tmp = map(request_params.value2, 100, -100, 0, 180);
            move_servo_right.write(constrain(tmp, 0, 180));
          }

          SERVER_RESPONSE_OK("");
          break;
        }
        case ACTION_SING: {
          tone(SPEAKER_PIN, request_params.value1, request_params.value2);

          SERVER_RESPONSE_OK("");
          break;
        }
        case ACTION_SEE: {
          digitalWrite(SEE_TRIGGER_PIN, LOW);
          delayMicroseconds(SEE_SHORT_DELAY);

          digitalWrite(SEE_TRIGGER_PIN, HIGH);
          delayMicroseconds(SEE_LONG_DELAY);

          digitalWrite(SEE_TRIGGER_PIN, LOW);
          see_duration = (float) pulseIn(SEE_ECHO_PIN, HIGH);
          see_distance = see_duration / SEE_INTERNET_MAGIC_NUMBER;

          if (see_distance > SEE_MAX_DISTANCE) {
            see_distance = SEE_MAX_DISTANCE;
          }

          sprintf(server_response_content, "%ld", see_distance);
          SERVER_RESPONSE_OK(server_response_content);
          break;
        }
        case ACTION_PIXEL: {
          pixel.setPixelColor(0, pixel.Color(request_params.value1,request_params.value2,request_params.value3));
          pixel.show();
          SERVER_RESPONSE_OK("");
          break;
        }
        case ACTION_SENSE_LIGHT: {
          int sensorLightState = analogRead(SENSOR_LIGHT_PIN);

          sprintf(server_response_content, "%d", sensorLightState);
          SERVER_RESPONSE_OK(server_response_content);
          break;
        }
        case ACTION_TOGGLE: {
          digitalWrite(LED_PIN, HIGH-digitalRead(LED_PIN));
          SERVER_RESPONSE_OK("");
          break;
        }
        case ACTION_TURN: {
          digitalWrite(LED_PIN, request_params.value1);
          SERVER_RESPONSE_OK("");
          break;
        }
        default: {
          SERVER_RESPONSE_BAD();
        }
      }
    }
  }

  blink_loop();
}
