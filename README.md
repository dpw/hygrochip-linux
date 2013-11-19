# A tool for reading from Hygrochip humidity sensors under Linux

This is a simple program that queries
[Hygrochip](http://www.hygrochip.com/) humidity sensors from Linux.
It was developed and tested with
the HYT-271, but it should work with the other models that use the
same sample code.  See [my blog
post](http://david.wragg.org/blog/2013/01/rpi-humidity.html) for more
details.

To build, just do:

    $ make

And then to run it:

    $ ./hyt-read
    Either the -d or -b option must be present

    Usage: hyt-read [ -b I2C bus name | -d device file ] [ -a I2C slave address ]
                    [ -i seconds ] [ -T ] [ -H ]
    Options:
            -b X    Open the I2C bus named X (e.g. bcm2708_i2c.1)
            -d X    Open the I2C device named X (e.g. /dev/i2c-0)
            -a X    Target I2C slave address X (default 0x28)
            -i X    Read data every X seconds
            -T      Print only temperature
            -H      Print only humidity
            -h      Show this message

    $ ./hyt-read -d /dev/i2c-1
    36.916317 27.065556

The first value returned is the relative humidity as a percentage, the
second is the temperature in degrees C.

You can also specify use the I2C bus name (as reported by `i2cdetect
-l`), i.e. on the Raspberry Pi revision 2:

    ./hyt-read -b bcm2708_i2c.1

The file `munin_plugin.sh` contains a script contributed by Florian
Heiderich to use this program with the munin monitoring tool.
