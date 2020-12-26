// #############################################################################
// #              --- Infrared Remote Decoder (NEC Protocol) ---               #
// #############################################################################
// # main.c - Testcode for IR Library                                          #
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

 #include <avr/io.h>
 #include "libnecdecoder.h"
 #include "main.h"

 // Demo code description:
 // INT0   input for the IR receiver (i.e. TSOP 1736, TSOP 31236)
 // PORTB0 toggled on every valid packet
 // PORTB1 high if the current key is hold
 // UART   will output address and command at 9600 Baud 8N1


 int main( void )
  {
   // Initialize LED ports
   DDRB |= (1<<PB0) | (1<<PB1);
   PORTB &= ~( (1<<PB0) | (1<<PB1) );

   // Init UART (8N1)
   #if defined (__AVR_ATmega48__) || defined (__AVR_ATmega48A__) || defined (__AVR_ATmega48P__) || defined (__AVR_ATmega48PA__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega88A__) || defined (__AVR_ATmega88P__) || defined (__AVR_ATmega88PA__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega168PA__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
   UBRR0 = UBRR_VAL;
   UCSR0B |= (1<<TXEN0);
   UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);
   #elif defined (__AVR_ATtiny2313__) || defined (__AVR_ATtiny2313A__) || defined (__AVR_ATtiny4313__)
   UBRRH = (uint8_t) (UBRR_VAL>>8);
   UBRRL = (uint8_t) UBRR_VAL;
   UCSRB |= (1<<TXEN);
   UCSRC = (1<<UCSZ1)|(1<<UCSZ0);
   #else
   #error "MCU not supported"
   #endif

   // Initialize IR lib
   ir_init();

   uart_putstring("libnecdecoder v1.3 Demo\r\n");
   uart_putstring("(c) '13-'20 by Malte Pöggel - www.MALTEPOEGGEL.de - malte@poeggel.de\r\n");
   uart_putstring("Receiver ready, please press a key :-)\r\n");
   uart_putstring("\r\n");

   while(1)
    {
     // Check if new code is received
     if(ir.status & (1<<IR_RECEIVED))
      {
       // Toggle LED
       PORTB ^= (1<<PB0);

       // Send address and command to UART
       uart_putstring("Address: ");
       #ifdef PROTOCOL_NEC_EXTENDED
       uart_puthex(ir.address_l);
       uart_putchar(' ');
       uart_puthex(ir.address_h);
       #else
       uart_puthex(ir.address);
       #endif
       uart_putstring(", Command: ");
       uart_puthex(ir.command);
       uart_putstring("\r\n");

       // Reset state
       ir.status &= ~(1<<IR_RECEIVED);
      }

     // Check hold flag
     if(ir.status & (1<<IR_KEYHOLD))
      PORTB |= (1<<PB1);
       else PORTB &= ~(1<<PB1);
    }
  }


 // ###### Send single character via UART ######
 void uart_putchar( char x )
  {
   #if defined (__AVR_ATmega48__) || defined (__AVR_ATmega48A__) || defined (__AVR_ATmega48P__) || defined (__AVR_ATmega48PA__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega88A__) || defined (__AVR_ATmega88P__) || defined (__AVR_ATmega88PA__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega168PA__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
   while (!(UCSR0A & (1<<UDRE0)))
    {
    }
   UDR0 = x;
   #elif defined (__AVR_ATtiny2313__) || defined (__AVR_ATtiny2313A__) || defined (__AVR_ATtiny4313__)
   while (!(UCSRA & (1<<UDRE)))
    {
    }
   UDR = x;
   #else
   #error "MCU not supported"
   #endif
  }


 // ###### Send string via UART ######
 void uart_putstring( char* s )
  {
   while (*s) // while not \0
    {
     uart_putchar(*s);
     s++;
    }
  }


 // ###### Send 8bit as hex int via UART ######
 void uart_puthex( uint8_t i )
  {
   uint8_t tmp;
   tmp = ((i>>4)&0x0F);
   if(tmp>=10) tmp+=7;
   uart_putchar('0'+tmp);
   tmp = (i&0x0F);
   if(tmp>=10) tmp+=7;
   uart_putchar('0'+tmp);
  }
