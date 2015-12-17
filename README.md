rodi-web
========
A web-service interface for Gary Servin's RoDi (Robot Didáctico inalámbrico) wireless robot.
Allows programmers to control RoDi by simply using HTTP requests, ie., from a web browser,
a bash script, and basically any programming language.

API
----

* Blink

    ```
    GET /1/<rate-in-milliseconds>/
    ```

    ie.,
    ```
    curl --get http://192.168.4.1:1234/1/1000/ // blink each second
    curl --get http://192.168.4.1:1234/1/0/    // stop blinking
    ```

* Sense

    ```
    GET /2/
    ```

    Returns a JSON formatted array of two integers, for left and right sensor values.

    ie.,
    ```
    $curl --get http://192.168.4.1:1234/2/
    [62, 66]
    ```

* Move

    ```
    GET /3/<left-wheel-speed[-100:100]>/<right-wheel-speed[-100:100]>/
    ```

    ie.,
    ```
    $curl --get http://192.168.4.1:1234/3/0/0/          // stop
    $curl --get http://192.168.4.1:1234/3/100/-100/     // turn right
    $curl --get http://192.168.4.1:1234/3/-100/100/     // turn left
    $curl --get http://192.168.4.1:1234/3/100/100/      // forward
    $curl --get http://192.168.4.1:1234/3/-100/-100/    // backward
    ```

    100 moves wheel forward at full speed
    -100 moves wheel backward at full speed

* Sing

    ```
    GET /4/<note>/<duration>
    ```

    ie.,
    ```
    curl --get http://192.168.4.1:1234/4/31/250/ // blink each second
    ```

    Notes can be found in http://arduino.cc/en/tutorial/tone

* See

    ```
    GET /5/
    ```

    Returns a JSON formatted integer, for the distance in centimeters.

    ie.,
    ```
    $curl --get http://192.168.4.1:1234/5/
    13
    ```

* Pixel

    ```
    GET /6/<red>/<green>/<blue>
    ```

    Set the color of the onboard RGB led (pixel)

    ie.,
    ```
    $curl --get http://192.168.4.1:1234/6/0/0/0/        // off
    $curl --get http://192.168.4.1:1234/6/255/0/0/      // red
    $curl --get http://192.168.4.1:1234/6/0/255/0/      // green
    $curl --get http://192.168.4.1:1234/6/0/0/255/      // blue
    $curl --get http://192.168.4.1:1234/6/255/255/255/  // white
    ```

* Sense Light

    ```
    GET /7/
    ```

    Returns a JSON formatted integer, for the light intensity level (0 - 1023).

    ie.,
    ```
    $curl --get http://192.168.4.1:1234/7/
    66
    ```

* Led

    ```
    GET /8/<state[0|1]>
    ```

    Turns the onboard led on or off.

    ie.,
    ```
    curl --get http://192.168.4.1:1234/8/0/    // turns the onboard led off
    curl --get http://192.168.4.1:1234/8/1/    // turns the onbboard led on
    ```

* IMU

    ```
    GET /9/
    ```

    Returns a JSON formatted array of seven integers, for IMU sensor values.

    All values are from in the range [-32768:32767]:
        - X-axis acceleration (~16384 = 1g)
        - Y-axis acceleration (~16384 = 1g)
        - Z-axis acceleration (~16384 = 1g)
        - X-axis angular velocity
        - Y-axis angular velocity
        - Z-axis angular velocity
        - Temperature       (degrees C * 10)

    ie.,
    ```
    $curl --get http://192.168.4.1:1234/9/
    [-268, 8, -268, -371, -449, -51, 284]
    ```

Authors
----------
* Martin Abente Lahaye - tch@sugarlabs.org
* Gary Servin - garyservin@gmail.com

Cool Stuff
-------------------
I recommend Turtle Blocks Javascript as a client for RoDi-web. Grab the latest
bits from https://github.com/walterbender/turtleblocksjs. Don't forget to load
the RoDi plugin!

