# serial_ltc_shomi

This program is uploaded to an Arduino.
The Arduino then receives commands over the serial port (USB).
Then it sets parameters in multiple LTC2668 chips, which are
sent using the SPI protocol, using different chip-select
channels on the Arduino.