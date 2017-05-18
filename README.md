SparkFun SAMD21 Firmware for Futaba SBUS
===============================================================================

This example code is written in Arduino 1.8.2 for a SAMD21 Mini Breakout:
https://www.sparkfun.com/products/13664

...to interpret the Futaba SBUS 16-channel data stream. This data stream is
a UART signal running at 100K baud, 8E2, AND it's inverted. Yes, really. So
before you use this code, you need to invert the signal.

Included in this directory is a pic from my notebook denoting the structure of
the bit/channel order. Essentially, the data frame is 25 bytes long, with 0x0F
being the first you'll receive and 0x00 being the last. The data frame is
preceded by at least 4mS of quiet time on the UART line. Each channle is
represented by 11 bits, with those bits split between the bytes. The bit
pattern is such that the first 8 bits of channel 1 come first, but instead of
0-1-2-3-4-5-6-7, they come as 7-6-5-4-3-2-1-0. The byte after that contains the
last 3 bits of channel one plus the fist 5 bits of channel 2, but again the
order is reversed, so you'll first get 4-3-2-1-0 of channel 2, followed by
10-9-8 of channel 1. The rest of the frame follows the same patern, down to the
23rd byte (RX[22] in the code). I don't know what the 24th byte is used for,
but the 25th comes as 0x00. If none of that makes sense, try to reference it
against the included pic and the code. Good luck!

BTW, if you want to run the LEDs as I've set up, you'll need to install
FastLED.
