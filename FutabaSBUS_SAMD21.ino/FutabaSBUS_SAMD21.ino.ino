/******************************************************************************
FutabaSBUS_ASMD21.ino
Pete Dokter @ SparkFun Electronics
original creation date: May 16, 2017

https://github.com/sparkfun/FutabaSBUS_SAMD21

This code interprets a Futaba SBUS data stream with a SAMD21 Mini Breakout by
SparkFun Electronics (https://www.sparkfun.com/products/13664). The specific
receiver used was a FrSky XSR, the transmitter being a FrSky Taranis X9D Plus.

Also requires the AdaFruit NeoPixel Library to run the WS2812B's

Development environment specifics:
Arduino 1.8.2
******************************************************************************/


//Setup up the WS2812B's===========================================================
#include <Adafruit_NeoPixel.h>

#define PIN            12
#define NUMPIXELS      16

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//=================================================================================
  

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  pixels.begin(); // This initializes the NeoPixel library.

  //Setup Serial1 to read the SBUS data. Don't forget - the signal is also inverted,
  //so you'll have to deal with that in hardware.
  Serial1.begin(100000, SERIAL_8E2); //100000 baud, 8E2
  
  SerialUSB.begin(115200);

}


void loop() {

  uint8_t x = 0;
  
  uint16_t bright = 250; //initial brightness level
  
  //SBUS gets read into this array...
  uint8_t RX_array[25]; 
  
  // ...then transferred into this one. I don't use 
  //channel 0, this is just for easier mental indexing
  uint16_t channel[17]; 
  
  uint8_t count1 = 0; //number of times through the loop before something changes
  uint8_t count2 = 0; //number of change-states
  uint8_t rate = 0;   //blinky speed
  uint8_t mode = 0;   //3 modes: 0 = off, 1 = green spinners, 2 = L&R flashing
  uint16_t hue1 = 0;   //Adds red to the blue in mode 2
  uint16_t hue2 = 0;   //Adds blue to the green in mode 2

  //status LED blink
  for (x = 0; x < 10; x++)
  {
    digitalWrite(13, HIGH);  
    delay(50);                 
    digitalWrite(13, LOW); 
    delay(50);
  }


  //zero  channel array
  for (x = 0; x < 17; x++) {channel[x] = 0;}
 
  while (1)
  {
    
    //zero the array
    for (x = 0; x < 25; x++)
    {
      RX_array[x] = 0;
    }
    
    x = 0;
    
    //Get the frame. This should ultimately be interrupt driven, but we'll cheat for now.
    while (x < 25)
    {
      if (Serial1.available()) 
      {
        RX_array[x] = Serial1.read();
        x++;
      }
    }

    //check to see if it's a good frame...sorta. Ideally, I should be looking for the
    //dead time before the data gets sent as well, but haven't implimented that yet.
    if ((RX_array[0] == 0x0F) && (RX_array[24] == 0))

    //Dump it into the channel array
    channel[1] = (((RX_array[2] & 0x0007) << 8) | RX_array[1]);
    channel[2] = (((RX_array[2] & 0x00F7) >> 3) | ((RX_array[3] & 0x003F) << 5));
    channel[3] = (((RX_array[3] & 0x00C0) >> 6) | (RX_array[4] << 2) | ((RX_array[5] & 0x0001) << 10));
    channel[4] = (((RX_array[5] & 0x00FE) >> 1) | ((RX_array[6] & 0x000F) << 7));
    channel[5] = (((RX_array[6] & 0x00F0) >> 4) | ((RX_array[7] & 0x007F) << 4));
    channel[6] = (((RX_array[7] & 0x0080) >> 7) | (RX_array[8] << 1) | ((RX_array[9] & 0x0003) << 9));
    channel[7] = (((RX_array[9] & 0x00FC) >> 2) | ((RX_array[10] & 0x001F) << 6));
    channel[8] = (((RX_array[10] & 0x00E0) >> 5) | (RX_array[11] << 3));
    channel[9] = ((RX_array[12] | ((RX_array[13] & 0x0007) << 8)));
    channel[10] = (((RX_array[13] & 0x00F8) >> 3) | ((RX_array[14] & 0x003F) << 5));
    channel[11] = (((RX_array[14] & 0x00C0) >> 6) | (RX_array[15] << 2) | ((RX_array[16] & 0x0001) << 10));
    channel[12] = (((RX_array[16] & 0x00FE) >> 1) | ((RX_array[17] & 0x000F) << 7));
    channel[13] = (((RX_array[17] & 0x00F0) >> 4) | ((RX_array[18] & 0x007F) << 4));
    channel[14] = (((RX_array[18] & 0x0080) >> 7) | (RX_array[19] << 1) | ((RX_array[20] & 0x0003) << 9));
    channel[15] = (((RX_array[20] & 0x00FC) >> 2) | ((RX_array[21] & 0x001F) << 6));
    channel[16] = (((RX_array[21] & 0x00E0) >> 5) | (RX_array[22] << 3));

    //set up light mode on channel 7
    if (channel[6] < 0x00FF) mode = 0;
    else if ((channel[6] > 0x00FF) && (channel[6] < 0x0400)) mode = 1;
    else if (channel[6] > 0x0700) mode = 2;

    //set up rate on channel 8
    rate = 30 - ((channel[8] - 170) / 55); //should give a rate between 0 and 30)
    if (rate < 0) rate = 0;
    
    //set up brightness on channel 9
    bright = (channel[9] - 170) / 6;
    if (bright < 0) bright = 0;
    else if (bright > 255) bright = 255;

    //set up hue1 on channel 10
    hue1 = (channel[10] - 170) / 6;
    if (hue1 < 0) hue1 = 0;
    else if (hue1 > 255) hue1 = 255;

    //set up hue2 on channel 11
    hue2 = (channel[11] - 170) / 6;
    if (hue2 < 0) hue2 = 0;
    else if (hue2 > 255) hue2 = 255;

    //It takes about 484uS to transmit to 16 WS2812B's, FYI...
    
    //no lights in mode 0
    if (mode == 0)
    {
      pixels.setPixelColor(0, pixels.Color(0,0,0));
      pixels.setPixelColor(1, pixels.Color(0,0,0));
      pixels.setPixelColor(2, pixels.Color(0,0,0));
      pixels.setPixelColor(3, pixels.Color(0,0,0));
      pixels.setPixelColor(4, pixels.Color(0,0,0));
      pixels.setPixelColor(5, pixels.Color(0,0,0));
      pixels.setPixelColor(6, pixels.Color(0,0,0));
      pixels.setPixelColor(7, pixels.Color(0,0,0));
      pixels.setPixelColor(8, pixels.Color(0,0,0));
      pixels.setPixelColor(9, pixels.Color(0,0,0));
      pixels.setPixelColor(10, pixels.Color(0,0,0));
      pixels.setPixelColor(11, pixels.Color(0,0,0));
      pixels.setPixelColor(12, pixels.Color(0,0,0));
      pixels.setPixelColor(13, pixels.Color(0,0,0));
      pixels.setPixelColor(14, pixels.Color(0,0,0));
      pixels.setPixelColor(15, pixels.Color(0,0,0));
      pixels.show();
    }


    else
    {
      if (count1 >= rate)
      {
        count1 = 0;
  
        if (count2 == 0)
        {
          if (mode == 1)  //green spinners
          {
            pixels.setPixelColor(0, pixels.Color(0,bright,0));
            pixels.setPixelColor(1, pixels.Color(0,0,0));
            pixels.setPixelColor(2, pixels.Color(0,0,0));
            pixels.setPixelColor(3, pixels.Color(0,0,0));
            pixels.setPixelColor(4, pixels.Color(0,bright,0));
            pixels.setPixelColor(5, pixels.Color(0,0,0));
            pixels.setPixelColor(6, pixels.Color(0,0,0));
            pixels.setPixelColor(7, pixels.Color(0,0,0));
            pixels.setPixelColor(8, pixels.Color(0,bright,0));
            pixels.setPixelColor(9, pixels.Color(0,0,0));
            pixels.setPixelColor(10, pixels.Color(0,0,0));
            pixels.setPixelColor(11, pixels.Color(0,0,0));
            pixels.setPixelColor(12, pixels.Color(0,bright,0));
            pixels.setPixelColor(13, pixels.Color(0,0,0));
            pixels.setPixelColor(14, pixels.Color(0,0,0));
            pixels.setPixelColor(15, pixels.Color(0,0,0));
            pixels.show();
          }

          else if (mode == 2)
          {
            //blue with red added
            pixels.setPixelColor(0, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(1, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(2, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(3, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(4, pixels.Color(0,0,0));
            pixels.setPixelColor(5, pixels.Color(0,0,0));
            pixels.setPixelColor(6, pixels.Color(0,0,0));
            pixels.setPixelColor(7, pixels.Color(0,0,0));
            pixels.setPixelColor(8, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(9, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(10, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(11, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(12, pixels.Color(0,0,0));
            pixels.setPixelColor(13, pixels.Color(0,0,0));
            pixels.setPixelColor(14, pixels.Color(0,0,0));
            pixels.setPixelColor(15, pixels.Color(0,0,0));
            pixels.show();
            
          }
  
        }
  
        else if (count2 == 1)
        {
          if (mode == 1)  //green spinners
          {
            pixels.setPixelColor(0, pixels.Color(0,0,0));
            pixels.setPixelColor(1, pixels.Color(0,0,0));
            pixels.setPixelColor(2, pixels.Color(0,0,0));
            pixels.setPixelColor(3, pixels.Color(0,bright,0));
            pixels.setPixelColor(4, pixels.Color(0,0,0));
            pixels.setPixelColor(5, pixels.Color(0,bright,0));
            pixels.setPixelColor(6, pixels.Color(0,0,0));
            pixels.setPixelColor(7, pixels.Color(0,0,0));
            pixels.setPixelColor(8, pixels.Color(0,0,0));
            pixels.setPixelColor(9, pixels.Color(0,0,0));
            pixels.setPixelColor(10, pixels.Color(0,0,0));
            pixels.setPixelColor(11, pixels.Color(0,bright,0));
            pixels.setPixelColor(12, pixels.Color(0,0,0));
            pixels.setPixelColor(13, pixels.Color(0,bright,0));
            pixels.setPixelColor(14, pixels.Color(0,0,0));
            pixels.setPixelColor(15, pixels.Color(0,0,0));
            pixels.show();
          }

          else if (mode == 2)
          {
            //green with blue added
            pixels.setPixelColor(0, pixels.Color(0,0,0));
            pixels.setPixelColor(1, pixels.Color(0,0,0));
            pixels.setPixelColor(2, pixels.Color(0,0,0));
            pixels.setPixelColor(3, pixels.Color(0,0,0));
            pixels.setPixelColor(4, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(5, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(6, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(7, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(8, pixels.Color(0,0,0));
            pixels.setPixelColor(9, pixels.Color(0,0,0));
            pixels.setPixelColor(10, pixels.Color(0,0,0));
            pixels.setPixelColor(11, pixels.Color(0,0,0));
            pixels.setPixelColor(12, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(13, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(14, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(15, pixels.Color(0,bright,hue2));
            pixels.show();
            
          }
        }
  
        else if (count2 == 2)
        {
          if (mode == 1)  //green spinners
          {
            pixels.setPixelColor(0, pixels.Color(0,0,0));
            pixels.setPixelColor(1, pixels.Color(0,0,0));
            pixels.setPixelColor(2, pixels.Color(0,bright,0));
            pixels.setPixelColor(3, pixels.Color(0,0,0));
            pixels.setPixelColor(4, pixels.Color(0,0,0));
            pixels.setPixelColor(5, pixels.Color(0,0,0));
            pixels.setPixelColor(6, pixels.Color(0,bright,0));
            pixels.setPixelColor(7, pixels.Color(0,0,0));
            pixels.setPixelColor(8, pixels.Color(0,0,0));
            pixels.setPixelColor(9, pixels.Color(0,0,0));
            pixels.setPixelColor(10, pixels.Color(0,bright,0));
            pixels.setPixelColor(11, pixels.Color(0,0,0));
            pixels.setPixelColor(12, pixels.Color(0,0,0));
            pixels.setPixelColor(13, pixels.Color(0,0,0));
            pixels.setPixelColor(14, pixels.Color(0,bright,0));
            pixels.setPixelColor(15, pixels.Color(0,0,0));
            pixels.show();
          }

          else if (mode == 2)
          {
            //blue with red added
            pixels.setPixelColor(0, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(1, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(2, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(3, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(4, pixels.Color(0,0,0));
            pixels.setPixelColor(5, pixels.Color(0,0,0));
            pixels.setPixelColor(6, pixels.Color(0,0,0));
            pixels.setPixelColor(7, pixels.Color(0,0,0));
            pixels.setPixelColor(8, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(9, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(10, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(11, pixels.Color(hue1,0,bright));
            pixels.setPixelColor(12, pixels.Color(0,0,0));
            pixels.setPixelColor(13, pixels.Color(0,0,0));
            pixels.setPixelColor(14, pixels.Color(0,0,0));
            pixels.setPixelColor(15, pixels.Color(0,0,0));
            pixels.show();
            
          }

        }
  
        else if (count2 == 3)
        {
          if (mode == 1)  //green spinners
          {
            pixels.setPixelColor(0, pixels.Color(0,0,0));
            pixels.setPixelColor(1, pixels.Color(0,bright,0));
            pixels.setPixelColor(2, pixels.Color(0,0,0));
            pixels.setPixelColor(3, pixels.Color(0,0,0));
            pixels.setPixelColor(4, pixels.Color(0,0,0));
            pixels.setPixelColor(5, pixels.Color(0,0,0));
            pixels.setPixelColor(6, pixels.Color(0,0,0));
            pixels.setPixelColor(7, pixels.Color(0,bright,0));
            pixels.setPixelColor(8, pixels.Color(0,0,0));
            pixels.setPixelColor(9, pixels.Color(0,bright,0));
            pixels.setPixelColor(10, pixels.Color(0,0,0));
            pixels.setPixelColor(11, pixels.Color(0,0,0));
            pixels.setPixelColor(12, pixels.Color(0,0,0));
            pixels.setPixelColor(13, pixels.Color(0,0,0));
            pixels.setPixelColor(14, pixels.Color(0,0,0));
            pixels.setPixelColor(15, pixels.Color(0,bright,0));
            pixels.show();
          }
          
          else if (mode == 2)
          {
            //green with blue added
            pixels.setPixelColor(0, pixels.Color(0,0,0));
            pixels.setPixelColor(1, pixels.Color(0,0,0));
            pixels.setPixelColor(2, pixels.Color(0,0,0));
            pixels.setPixelColor(3, pixels.Color(0,0,0));
            pixels.setPixelColor(4, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(5, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(6, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(7, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(8, pixels.Color(0,0,0));
            pixels.setPixelColor(9, pixels.Color(0,0,0));
            pixels.setPixelColor(10, pixels.Color(0,0,0));
            pixels.setPixelColor(11, pixels.Color(0,0,0));
            pixels.setPixelColor(12, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(13, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(14, pixels.Color(0,bright,hue2));
            pixels.setPixelColor(15, pixels.Color(0,bright,hue2));
            pixels.show();
            
          }

          
        }

        count2++;
        if (count2 == 4) count2 = 0;
        
      }
      
    }

    count1++;

  }
   

}
