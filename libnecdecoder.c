// #############################################################################
// #              --- Infrared Remote Decoder (NEC Protocol) ---               #
// #############################################################################
// # libnecdecoder.c - NEC IR Library                                          #
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
 #include <avr/interrupt.h>
 #include "libnecdecoder.h"

 void ir_reset_counter( void );

 volatile uint8_t ir_state;
 volatile uint8_t ir_bitctr;
 #ifdef PROTOCOL_NEC_EXTENDED
 volatile uint8_t ir_tmp_address_l;
 volatile uint8_t ir_tmp_address_h;
 #else
 volatile uint8_t ir_tmp_address;
 #endif
 volatile uint8_t ir_tmp_command;
 volatile uint8_t ir_tmp_keyhold;
 volatile uint8_t ir_tmp_ovf;
 volatile struct ir_struct ir;


 // ###### Initializes IR function ######
 void ir_init( void )
  {
   #if defined (__AVR_ATmega48__) || defined (__AVR_ATmega48A__) || defined (__AVR_ATmega48P__) || defined (__AVR_ATmega48PA__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega88A__) || defined (__AVR_ATmega88P__) || defined (__AVR_ATmega88PA__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega168PA__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)

   #ifndef USE_16BIT_TIMER1
   TCCR0A &= ~( (1<<COM0A0) | (1<<COM0A1) | (1<<COM0B0) | (1<<COM0B1) | (1<<WGM00) | (1<<WGM01) );
   TCCR0B |= (TIMER_PRESCALER==1024?(1<<CS00):0) | (1<<CS02);
   TIMSK0 |= (1<<TOIE0);
   #else
   TCCR1A &= ~( (1<<COM0A0) | (1<<COM0A1) | (1<<COM0B0) | (1<<COM0B1) | (1<<WGM00) | (1<<WGM01) );
   TCCR1B |= (TIMER_PRESCALER==1024?(1<<CS10):0) | (1<<CS12);
   #endif
   #ifndef USE_INT1
   // Interrupt 0 (PD2): Inverted signal input, triggered by logical change
   DDRD   &= ~(1<<PD2);
   EICRA  |= (1<<ISC00);
   EIMSK  |= (1<<INT0);
   #else
   // Interrupt 1 (PD3): Inverted signal input, triggered by logical change
   DDRD   &= ~(1<<PD3);
   EICRA  |= (1<<ISC10);
   EIMSK  |= (1<<INT1);
   #endif

   #elif defined (__AVR_ATtiny2313__) || defined (__AVR_ATtiny2313A__) || defined (__AVR_ATtiny4313__)

   #ifndef USE_16BIT_TIMER1
   TCCR0A &= ~( (1<<COM0A0) | (1<<COM0A1) | (1<<COM0B0) | (1<<COM0B1) | (1<<WGM00) | (1<<WGM01) );
   TCCR0B |= (TIMER_PRESCALER==1024?(1<<CS00):0) | (1<<CS02);
   TIMSK  |= (1<<TOIE0);
   #else
   TCCR1A &= ~( (1<<COM0A0) | (1<<COM0A1) | (1<<COM0B0) | (1<<COM0B1) | (1<<WGM00) | (1<<WGM01) );
   TCCR1B |= (TIMER_PRESCALER==1024?(1<<CS10):0) | (1<<CS12);
   TIMSK  |= (1<<TOIE1);
   #endif
   #ifndef USE_INT1
   // Interrupt 0 (PD2): Inverted signal input, triggered by logical change
   DDRD   &= ~(1<<PD2);
   MCUCR  |= (1<<ISC00);
   GIMSK  |= (1<<INT0);
   #else
   // Interrupt 1 (PD3): Inverted signal input, triggered by logical change
   DDRD   &= ~(1<<PD3);
   MCUCR  |= (1<<ISC10);
   GIMSK  |= (1<<INT1);
   #endif

   #else
   #error "MCU not supported"
   #endif

   // Reset state
   ir_state = IR_BURST;

   // Reset global variables
   ir_tmp_keyhold = 0;
   ir_tmp_ovf = 0;

   // Initialize timer
   ir_reset_counter();

   // Global interrupt enable
   sei();
  }


 // ###### Stops ir function ######
 void ir_stop( void )
  {
   // Stop timer and disable interrupt
   #if defined (__AVR_ATmega48__) || defined (__AVR_ATmega48A__) || defined (__AVR_ATmega48P__) || defined (__AVR_ATmega48PA__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega88A__) || defined (__AVR_ATmega88P__) || defined (__AVR_ATmega88PA__) || defined (__AVR_ATmega168__) || defined (__AVR_ATmega168A__) || defined (__AVR_ATmega168P__) || defined (__AVR_ATmega168PA__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)

   TCCR0B &= ~((1<<CS00) | (1<<CS02));
   #ifndef USE_16BIT_TIMER1
   TIMSK0 &= ~(1<<TOIE0);
   #else
   TIMSK1 &= ~(1<<TOIE1);
   #endif
   #ifndef USE_INT1
   EIMSK  &= ~(1<<INT0);
   #else
   EIMSK  &= ~(1<<INT1);
   #endif

   #elif defined (__AVR_ATtiny2313__) || defined (__AVR_ATtiny2313A__) || defined (__AVR_ATtiny4313__)

   TCCR0B &= ~((1<<CS00) | (1<<CS02));
   #ifndef USE_16BIT_TIMER1
   TIMSK  &= ~(1<<TOIE0);
   #else
   TIMSK  &= ~(1<<TOIE1);
   #endif
   #ifndef USE_INT1
   GIMSK  &= ~(1<<INT0);
   #else
   GIMSK  &= ~(1<<INT1);
   #endif

   #else
   #error "MCU not supported"
   #endif
  }


 // ###### INT0/1 for decoding ######
 #ifndef USE_INT1
 ISR( INT0_vect )
 #else
 ISR( INT1_vect )
 #endif
  {
   // Get current port state to check if we triggered on rising or falling edge
   #ifndef USE_INT1
   uint8_t port_state = ( PIND & (1<<PD2) );
   #else
   uint8_t port_state = ( PIND & (1<<PD3) );
   #endif

   #ifndef USE_16BIT_TIMER1
   uint8_t cnt_state = TCNT0;
   #else
   uint8_t cnt_state = TCNT1L;
   #endif

   if(ir_tmp_ovf!=0)
    {
     // Overflow, so reset and ignore.
     ir_tmp_ovf = 0;
     ir_state = IR_BURST;
     ir_reset_counter();
     return;
    }

   switch(ir_state)
    {
     case IR_BURST:
      if(!port_state)
       {
        ir_reset_counter();
       } else {
        if((cnt_state>TIME_BURST_MIN)&&(cnt_state<TIME_BURST_MAX))
         {
          ir_state = IR_GAP; // Next state
          ir_reset_counter();
         }
       }
      break;
     case IR_GAP:
      if(!port_state)
       {
        if((cnt_state>TIME_GAP_MIN)&&(cnt_state<TIME_GAP_MAX))
         {
          ir_reset_counter();
          ir_state = IR_ADDRESS; // Next state
          ir_bitctr = 0; // Reset bitcounter
          ir.status &= ~(1<<IR_KEYHOLD);
          break;
         } else
          if((cnt_state>TIME_HOLD_MIN)&&(cnt_state<TIME_HOLD_MAX))
           {
            if(ir.status & (1<<IR_SIGVALID))
             {
              ir.status |= (1<<IR_KEYHOLD);
              ir_tmp_keyhold = IR_HOLD_OVF;
             }
            ir_state = IR_BURST;
            break;
           }
       }
      // Should not happen, must be invalid. Reset.
      ir_state = IR_BURST;
      break;
     case IR_ADDRESS:
      if(port_state)
       {
        // Must be short pulse
        if((cnt_state>TIME_PULSE_MIN)&&(cnt_state<TIME_PULSE_MAX))
         {
          ir_reset_counter();
          break;
         }
        // Should not happen, must be invalid. Reset.
        ir_state = IR_BURST;
       } else {
        if((cnt_state>TIME_ZERO_MIN)&&(cnt_state<TIME_ZERO_MAX))
         {
          // 0
          #ifdef PROTOCOL_NEC_EXTENDED
          ir_tmp_address_l &= ~(1<<ir_bitctr++);
          #else
          ir_tmp_address &= ~(1<<ir_bitctr++);
          #endif
          ir_reset_counter();
          if(ir_bitctr>=8)
           {
            ir_state = IR_ADDRESS_INV; // Next state
            ir_bitctr = 0; // Reset bitcounter
           }
          break;
         } else {
          if((cnt_state>TIME_ONE_MIN)&&(cnt_state<TIME_ONE_MAX))
           {
            // 1
            #ifdef PROTOCOL_NEC_EXTENDED
            ir_tmp_address_l |= (1<<ir_bitctr++);
            #else
            ir_tmp_address |= (1<<ir_bitctr++);
            #endif
            ir_reset_counter();
            if(ir_bitctr>=8)
             {
              ir_state = IR_ADDRESS_INV; // Next state
              ir_bitctr = 0; // Reset bitcounter
             }
            break;
           }
         }
        // Should not happen, must be invalid. Reset.
        ir_state = IR_BURST;
        break;
       }
      break;
     case IR_ADDRESS_INV:
      if(port_state)
       {
        // Must be short pulse
        if((cnt_state>TIME_PULSE_MIN)&&(cnt_state<TIME_PULSE_MAX))
         {
          ir_reset_counter();
          break;
         }
        // Should not happen, must be invalid. Reset.
        ir_state = IR_BURST;
       } else {
        if((cnt_state>TIME_ZERO_MIN)&&(cnt_state<TIME_ZERO_MAX))
         {
          // 0 (inverted) or high address
          #ifdef PROTOCOL_NEC_EXTENDED
          ir_tmp_address_h &= ~(1<<ir_bitctr++);
          #else
          if(!(ir_tmp_address&(1<<ir_bitctr++)))
           {
            // Should not happen, must be invalid. Reset.
            ir_state = IR_BURST;
            break;
           }
          #endif
          ir_reset_counter();
          if(ir_bitctr>=8)
           {
            ir_state = IR_COMMAND; // Next state
            ir_bitctr = 0; // Reset bitcounter
           }
          break;
         } else {
          if((cnt_state>TIME_ONE_MIN)&&(cnt_state<TIME_ONE_MAX))
           {
            // 1 (inverted) or high address
            #ifdef PROTOCOL_NEC_EXTENDED
            ir_tmp_address_h |= (1<<ir_bitctr++);
            #else
            if(ir_tmp_address&(1<<ir_bitctr++))
             {
              // Should not happen, must be invalid. Reset.
              ir_state = IR_BURST;
              break;
             }
            #endif
            ir_reset_counter();
            if(ir_bitctr>=8)
             {
              ir_state = IR_COMMAND; // Next state
              ir_bitctr = 0; // Reset bitcounter
             }
            break;
           }
         }
        // Should not happen, must be invalid. Reset.
        ir_state = IR_BURST;
        break;
       }
      break;
     case IR_COMMAND:
      if(port_state)
       {
        // Must be short pulse
        if((cnt_state>TIME_PULSE_MIN)&&(cnt_state<TIME_PULSE_MAX))
         {
          ir_reset_counter();
          break;
         }
        // Should not happen, must be invalid. Reset.
        ir_state = IR_BURST;
       } else {
        if((cnt_state>TIME_ZERO_MIN)&&(cnt_state<TIME_ZERO_MAX))
         {
          // 0
          ir_tmp_command &= ~(1<<ir_bitctr++);
          ir_reset_counter();
          if(ir_bitctr>=8)
           {
            ir_state = IR_COMMAND_INV; // Next state
            ir_bitctr = 0; // Reset bitcounter
           }
          break;
         } else {
          if((cnt_state>TIME_ONE_MIN)&&(cnt_state<TIME_ONE_MAX))
           {
            // 1
            ir_tmp_command |= (1<<ir_bitctr++);
            ir_reset_counter();
            if(ir_bitctr>=8)
             {
              ir_state = IR_COMMAND_INV; // Next state
              ir_bitctr = 0; // Reset bitcounter
             }
            break;
           }
         }
        // Should not happen, must be invalid. Reset.
        ir_state = IR_BURST;
        break;
       }
      break;
     case IR_COMMAND_INV:
      if(port_state)
       {
        // Must be short pulse
        if((cnt_state>TIME_PULSE_MIN)&&(cnt_state<TIME_PULSE_MAX))
         {
          ir_reset_counter();
          break;
         }
        // Should not happen, must be invalid. Reset.
        ir_state = IR_BURST;
       } else {
        if((cnt_state>TIME_ZERO_MIN)&&(cnt_state<TIME_ZERO_MAX))
         {
          // 0 (inverted)
          if(!(ir_tmp_command&(1<<ir_bitctr++)))
           {
            // Should not happen, must be invalid. Reset.
            ir_state = IR_BURST;
            break;
           }
          ir_reset_counter();
          if(ir_bitctr>=8)
           {
            ir_state = IR_BURST; // Decoding finished.
            // Only apply if received flag is not set, must be done
            // by the main program after reading address and command
            if(!(ir.status & (1<<IR_RECEIVED)))
             {
              #ifdef PROTOCOL_NEC_EXTENDED
              ir.address_l = ir_tmp_address_l;
              ir.address_h = ir_tmp_address_h;
              #else
              ir.address = ir_tmp_address;
              #endif
              ir.command = ir_tmp_command;
              ir.status |= (1<<IR_RECEIVED) | (1<<IR_SIGVALID);
              ir_tmp_keyhold = IR_HOLD_OVF; // To make sure that valid flag is cleared
             }
            ir_bitctr = 0; // Reset bitcounter
           }
          break;
         } else {
          if((cnt_state>TIME_ONE_MIN)&&(cnt_state<TIME_ONE_MAX))
           {
            // 1 (inverted)
            if(ir_tmp_command&(1<<ir_bitctr++))
             {
              // Should not happen, must be invalid. Reset.
              ir_state = IR_BURST;
              break;
             }
            ir_reset_counter();
            if(ir_bitctr>=8)
             {
              ir_state = IR_BURST; // Decoding finished.
              // Only apply if received flag is not set, must be done
              // by the main program after reading address and command
              if(!(ir.status & (1<<IR_RECEIVED)))
               {
                #ifdef PROTOCOL_NEC_EXTENDED
                ir.address_l = ir_tmp_address_l;
                ir.address_h = ir_tmp_address_h;
                #else
                ir.address = ir_tmp_address;
                #endif
                ir.command = ir_tmp_command;
                ir.status |= (1<<IR_RECEIVED) | (1<<IR_SIGVALID);
                ir_tmp_keyhold = IR_HOLD_OVF; // To make sure that valid flag is cleared
               }
              ir_bitctr = 0; // Reset bitcounter
             }
            break;
           }
         }
        // Should not happen, must be invalid. Reset.
        ir_state = IR_BURST;
        break;
       }
      break;
    }
  }


 // ###### Timer 0 Overflow for hold flag clear ######
 #ifndef USE_16BIT_TIMER1
 ISR (TIMER0_OVF_vect)
 #else
 ISR (TIMER1_OVF_vect)
 #endif
  {
   // Reset timer
   ir_reset_counter();
   // Handle overflow
   ir_tmp_ovf = 1;
   if(ir_tmp_keyhold>0)
    {
     ir_tmp_keyhold--;
     if(ir_tmp_keyhold==0) ir.status &= ~((1<<IR_KEYHOLD) | (1<<IR_SIGVALID));
    }
  }


 // ###### Reset counter register value ######
 void ir_reset_counter( void )
  {
   #ifndef USE_16BIT_TIMER1
   TCNT0 = 0;
   #else
   TCNT1H = 0xFF;
   TCNT1L = 0x00;
   #endif
  }
