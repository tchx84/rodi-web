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

#define BUFFER_SIZE 1024

int index;
char inChar;
char inLine[BUFFER_SIZE];

char outResponse[] =
"HTTP/1.1 200 OK\r\n"
"Connection: close\r\n"
"Content-Type: application/json\r\n"
"Content-Length: 2\r\n"
"\r\n"
"{}";

void setup()
{
  index = 0;
  Serial.begin(57600);
}

int is_header(char line[]){
    return (strstr(line, "GET") != NULL || strstr(line, "POST") != NULL);
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
      // XXX parse_the_header(inLine);
      Serial.print(outResponse);
    }
  }
}
