// #############################################################################
// #              --- Infrared Remote Decoder (NEC Protocol) ---               #
// #############################################################################
// # main.c - Header: Testcode for IR Library                                  #
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

#ifndef MAIN_H
 #define MAIN_H

 #ifndef F_CPU
  #warning "F_CPU undefined, set to 8MHz"
  #define F_CPU 8000000UL
 #endif

 // Baudrate
 #define BAUD 9600UL

 // Calculations
 #define UBRR_VAL ((F_CPU+BAUD*8)/(BAUD*16)-1)
 #define BAUD_REAL (F_CPU/(16*(UBRR_VAL+1)))
 #define BAUD_ERROR ((BAUD_REAL*1000)/BAUD)

 #if ((BAUD_ERROR<990) || (BAUD_ERROR>1010))
  #error Baudrate error higher than 1%!
 #endif

 // Functions
 void uart_putchar( char x );
 void uart_putstring( char* x );
 void uart_puthex( uint8_t i );

#endif
