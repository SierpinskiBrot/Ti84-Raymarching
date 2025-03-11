### Raymarching on a Ti-84

Using CE/C++ Toolchain:

https://ce-programming.github.io/toolchain/index.html

Must download the required libraries:

https://github.com/CE-Programming/libraries


v1.1.0 - Takes 451s (~7.5 mins) to draw

-Found some more assembly optimizations

-Reduced from 500 to 200 ray marches

![Screenshot](/bin/23.png)


v1.0.3 - Takes 972s (~16 mins) to draw

-Less floating point math

-Implemented fast inverse square root from quake

![Screenshot](/bin/19.png)


v1.0.2 - Takes 1027s (~17 mins) to draw

-Stole fixedpoint and vector libraries from https://github.com/TheScienceElf/TI-84-CE-Raytracing

-Using mainly fixed points instead of floats now

![Screenshot](/bin/17.png)

v1.0.1 - Takes 1240s (~21 mins) to draw

![Screenshot](/bin/16.png)

v1.0.0 - Takes 1381s (~23 mins) to draw

![Screenshot](/bin/15.png)


