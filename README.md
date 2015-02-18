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
    GET /3/<left-orientation>/<right-orientation>/
    ```

    ie.,
    ```
    $curl --get http://192.168.4.1:1234/3/94/94/   // stop
    $curl --get http://192.168.4.1:1234/3/0/0/     // turn right
    $curl --get http://192.168.4.1:1234/3/180/180/ // turn left
    $curl --get http://192.168.4.1:1234/3/0/180/   // forward
    $curl --get http://192.168.4.1:1234/3/180/0/   // backward
    ```

    As in Arduino's ServoWrite http://arduino.cc/en/Reference/ServoWrite

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

Authors
----------
* Martin Abente Lahaye - tch@sugarlabs.org

Cool Stuff
-------------------
I recommend Turtle Blocks Javascript as a client for RoDi-web. Grab the latest
bits from https://github.com/walterbender/turtleblocksjs. Don't forget to load
the RoDi plugin!

