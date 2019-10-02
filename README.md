# A clock based on Arduino Pro Mini, an MTK3339 GPS and an OLED display

Shows the time and date, both local time and well as UTC. CET is provided with
automatic summer/winter time. You can change this in the source code. See the
[Time](https://github.com/PaulStoffregen/Time) library for details.

Uses the GPZDA field from the GPS. Although this is not officially supported or
mentioned in the datasheet for MTK3339, it works very well. The program will
not work on GPSs that don't emit GPZDA.

The program has been made for a 3.3V Arduino Pro Mini (Atmega 328P), but it's
generic enough that it should work with many other microcontrollers. It only
needs a serial port and i2c.

Make sure the voltage is compatible. Pro Minis seem to come in both 3.3V and 5V
versions, while the GPS and display are for 3.3V.

## Connections

| GPS | Pro Mini |
|-----|----------|
| VCC | VCC      |
| GND | GND      |
| TX  | RX       |
| RX  | TX       |

| OLED | Pro Mini |
|------|----------|
| VCC  | VCC      |
| GND  | GND      |
| SCL  | A5       |
| SDA  | A4       |

Unfortunately the i2c pins (A4/SDA and A5/SCL) are not exposed on breadboard
friendly pins. I suggest soldering a pair of headers on top of it.

Make sure to disconnect the GPS TX/RX when programming, otherwise data from the
GPS will interfere with the programming process.

## Why GPZDA only?

Because GPZDA gives us everything we need for time. Nothing less, nothing more.
That makes the clock more power efficient and easy to work with.

And to my understanding the time is provided from the internal RTC clock,
regardless of satellite availability. Even if the GPS is turned off, its
internal clock keeps ticking, powered by the backup battery. That said, this
retention doesn't seem to last for long, only a few hours or days.
