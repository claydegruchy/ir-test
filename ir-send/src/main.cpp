/*
 * TinySender.cpp
 *
 *  Example for sending using TinyIR. By default sends simultaneously using all supported protocols
 *  To use a single protocol, simply delete or comment out all unneeded protocols in the main loop
 *  Program size is significantly reduced when using a single protocol
 *  For example, sending only 8 bit address and command NEC codes saves 780 bytes program memory and 26 bytes RAM compared to SimpleSender,
 *  which does the same, but uses the IRRemote library (and is therefore much more flexible).
 *
 *
 * The FAST protocol is a proprietary modified JVC protocol without address, with parity and with a shorter header.
 *  FAST Protocol characteristics:
 * - Bit timing is like NEC or JVC
 * - The header is shorter, 3156 vs. 12500
 * - No address and 16 bit data, interpreted as 8 bit command and 8 bit inverted command,
 *     leading to a fixed protocol length of (6 + (16 * 3) + 1) * 526 = 55 * 526 = 28930 microseconds or 29 ms.
 * - Repeats are sent as complete frames but in a 50 ms period / with a 21 ms distance.
 *
 *
 *  This file is part of IRMP https://github.com/IRMP-org/IRMP.
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2022-2024 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */
#include <Arduino.h>

#define IR_SEND_PIN 18
int LED_BUILTIN = 2;

// #include "TinyIRSender.hpp"

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.
}

/*
 * Set up the data to be sent.
 * The compiler is intelligent and removes the code for 16 bit address handling if we call it with an uint8_t address :-).
 * Using an uint16_t address or data requires additional 28 bytes program memory for NEC and 56 bytes program memory for FAST.
 */
uint8_t sAddress = 0x02;
// uint16_t sAddress = 0x02;
uint8_t sCommand = 0x34;
// uint16_t sCommand = 0x34;
uint8_t sRepeats = 0;

void loop()
{
  Serial.println("loop");
  delay(1000); // delay must be greater than 5 ms (RECORD_GAP_MICROS), otherwise the receiver sees it as one long signal
}