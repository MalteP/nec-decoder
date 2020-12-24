// #############################################################################
// #              --- Infrared Remote Decoder (NEC Protocol) ---               #
// #############################################################################
// # main.c - Testcode for IR Library                                          #
// #############################################################################
// #              Version: 1.1 - Compiler: AVR-GCC 4.5.3 (Linux)               #
// #      (c) 2013 by Malte Pöggel - All rights reserved. - License: BSD       #
// #               www.MALTEPOEGGEL.de - malte@maltepoeggel.de                 #
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
   UBRR0 = UBRR_VAL;
   UCSR0B |= (1<<TXEN0);
   UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

   // Initialize IR lib
   ir_init();
 
   uart_putstring("NEC Library Demo 1.0\r\n");
   uart_putstring("(c) 2011 by Malte Pöggel - www.maltepoeggel.de - malte@maltepoeggel.de\r\n");
   uart_putstring("Receiver is now ready to rock ;-)\r\n");
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
       uart_put8int(ir.address);
       uart_putstring(", Command: ");
       uart_put8int(ir.command);       
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
   while (!(UCSR0A & (1<<UDRE0)))
    {
    }
   UDR0 = x;
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


 // ###### Send 8bit int via UART ######
 void uart_put8int( uint8_t i )
  {
    uart_putchar('0'+((i / 100) % 10));
    uart_putchar('0'+((i / 10) % 10));
    uart_putchar('0'+(i % 10));
  }
