// #############################################################################
// #              --- Infrared Remote Decoder (NEC Protocol) ---               #
// #############################################################################
// # libnecdecoder.c - Header: NEC IR Library                                  #
// #############################################################################
// #              Version: 1.3 - Compiler: AVR-GCC 10.2.0 (Linux)              #
// #    (c) '13-'20 by Malte Pöggel - All rights reserved. - License: BSD      #
// #                  www.MALTEPOEGGEL.de - malte@poeggel.de                   #
// #############################################################################
// #   Redistribution and use in source and binary forms, with or without mo-  #
// # dification, are permitted provided that the following conditions are met: #
// #                                                                           #
// # * Redistributions of source code must retain the above copyright notice,  #
// #   this list of conditions and the following disclaimer.                   #
// # * Redistributions in binary form must reproduce the above copyright       #
// #   notice, this list of conditions and the following disclaimer in the     #
// #   documentation and/or other materials provided with the distribution.    #
// #                                                                           #
// #    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    #
// # "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED #
// #      TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A      #
// #     PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT    #
// #   HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  #
// # SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  #
// #    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,    #
// #  OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY   #
// #  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING  #
// #    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS     #
// #      SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.         #
// #############################################################################

#ifndef LIBNECDECODER_H
 #define LIBNECDECODER_H

 // Uncomment this to enable extended NEC protocol support.
 //#define PROTOCOL_NEC_EXTENDED

 // Uncomment this to use 16bit TIMER1. A preload value is used in order to act as 8 bit timer
 //#define USE_16BIT_TIMER1

 // Uncomment this to use INT1 instead of INT0
 //#define USE_INT1

 // Configure timer prescaler: 1024 (default) or 256 (might be useful for slow MCU frequencies)
 #define TIMER_PRESCALER 1024

 // Calculation of timer values:
 // Tick [ms] = 1 / (MCU frequency [kHz] / Prescaler)
 // Overflow [ms] = Tick [ms] * 256
 //
 // Example for 8MHz clock:
 // 1 / (8000kHz / 1024) = 0.128ms per timer increment
 //
 // For the AGC Burst just divide 9ms / 0.128ms = 70.3125 (~70) timer ticks.
 // Min/Max values are used because remote controls are using ceramic resonators
 // which might deviate due temperature or battery voltage changes.
 // Remember that both timers are handled as 8 bit, so their values may not exceed 255.
 // The timer overflow is used to detect a missing infrared signal, and also to detect if a key is still held down.

 // AGC Burst, 9ms typ
 #define TIME_BURST_MIN 65
 #define TIME_BURST_MAX 75

 // Gap after AGC Burst, 4.5ms typ
 #define TIME_GAP_MIN   30
 #define TIME_GAP_MAX   40

 // Gap (key hold) after AGC Burst, 2.25ms typ
 #define TIME_HOLD_MIN   15
 #define TIME_HOLD_MAX   20

 // Short pulse for each bit, 560us typ
 #define TIME_PULSE_MIN  2
 #define TIME_PULSE_MAX  7

 // Gap for logical 0, 560us typ
 #define TIME_ZERO_MIN   2
 #define TIME_ZERO_MAX   7

 // Gap for logical 1, 1.69ms typ
 #define TIME_ONE_MIN    9
 #define TIME_ONE_MAX   19

 // Definition for state machine
 enum ir_state_t { IR_BURST, IR_GAP, IR_ADDRESS, IR_ADDRESS_INV, IR_COMMAND, IR_COMMAND_INV };

 // Definition for status bits
 #define IR_RECEIVED 0 // Received new command
 #define IR_KEYHOLD  1 // Key hold
 #define IR_SIGVALID 2 // Valid signal (Internal used)

 // Timer Overflows till keyhold flag is cleared
 #define IR_HOLD_OVF 5

 // Struct definition
 struct ir_struct
  {
   #ifdef PROTOCOL_NEC_EXTENDED
   uint8_t address_l;
   uint8_t address_h;
   #else
   uint8_t address;
   #endif
   uint8_t command;
   uint8_t status;
  };

 // Global status structure
 extern volatile struct ir_struct ir;

 // Functions
 void ir_init( void );
 void ir_stop( void );

#endif
