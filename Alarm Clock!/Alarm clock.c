#define __SFR_OFFSET 0      // maps port to I/O ports instead of memory-mapped I/O
#include <avr/io.h>

.global main    
.global TIMER1_COMPA_vect
.global INT0_vect
.global INT1_vect

main:       ;--- init --
            ldi   r16,1
            out   DDRB,r16         ;we use PB0 for segment G
            ldi   r16,0x3f        
            out   DDRC,r16         ;PC0-PC5 for segments A-F
            ldi   r16,0b11110010   ;also init TX as output; INTs as input 
            out   DDRD,r16         ;write mask to DDRD                    
            ldi   r16,0x00
            out   PORTD,r16        ;turn all digits off
            ;init tim and alarm digits 
            ldi   r17,0
            sts   Second,r17
            sts   Ticks,r17
            ldi   r17,30
            sts   Minute,r17
            ldi   r17,12
            sts   Hour,r17
            ldi   r17,45
            sts   AMinute,r17
            ldi   r17,23
            sts   AHour,r17
            ;init timer 1 (16-Bit) to 1Hz (CTC with Prescale /256 and TOP=62500)
            ldi   r16,hi8(62500)  ;16MHz/(62500*256)=1Hz 
            sts   OCR1AH,r16
            ldi   r16,lo8(62500)  ;16MHz/(62500*256)=1Hz 
            sts   OCR1AL,r16
            ldi   r16,0      
            sts   TCCR1A,r16
            ldi   r16,0b1011      ;WGM12(CTC1)|CS11|CS10 -> CTC with prescale /64 gets us 4 ticks per second
            sts   TCCR1B,r16
            ldi   r16,0
            sts   TCNT1H,r16
            sts   TCNT1L,r16
            ;enable interrupts
            ldi   r16,0b00001111  ;interrupt on rising edge (both INT0 and INT1)
            sts   EICRA, R16     
            ldi   r16,0b00000011
            out   EIMSK, R16      ;enable INT0 and INT1
            ldi   r16,2
            sts   TIMSK1,R16      ;set OCIE1A (Timer1 Output Compare A Interrupt Enable) in Timer 1 Interrupt Mask Register
            sei                                       
loop:       ;--- Main Loop ---
            ;see if alarm enabled
            sbis  PINB, 1         ;skip next instruction if bit Set
            rjmp  1f 
            ;see if normal time display (we only blink that)
            sbic  PINB, 2         ;skip next instruction if bit clear
            rjmp  1f 
            ;see if alarm minute
            lds   r16,Hour
            lds   r17,AHour
            cp    r16,17
            brne  1f
            lds   r16,Minute
            lds   r17,AMinute
            cp    r16,17
            brne  1f
            ;okay, its alarm time. We do blink by doing nothing every other second
            lds   r16,Ticks
            andi  r16,1
            breq  loop          ;skip display on every even second that minute
1:          ;display    
            call  UpdDisplay
            jmp   loop

UpdDisplay: ;check if time or alarm mode
            sbic  PINB, 2        ;skip next instruction if bit clear
            rjmp  1f
            ;display current time
            lds   r17,Hour
            lds   r18,Minute
            jmp   Display
1:          ;display alarm time
            lds   r17,AHour
            lds   r18,AMinute
            jmp   Display

Display:    ;*********************************
            ;Display Time value on 4x7 segment
            ;In:  r17 = Hour
            ;     r18 = Minute
            ;*********************************
            cli
            ;digit 1
            mov   r16,r17
            call  R16Div10
            call  ShowDigit
            ldi   r22,0
            call  EnableDigit
            call  DisableDigit
            ;digit 2
            mov   r16,r17
            call  R16Mod10
            call  ShowDigit
            ldi   r22,1
            call  EnableDigit
            call  DisableDigit
            ;digit 3
            mov   r16,r18
            call  R16Div10
            call  ShowDigit
            ldi   r22,2
            call  EnableDigit
            call  DisableDigit
            ;digit 4
            mov   r16,r18
            call  R16Mod10
            call  ShowDigit
            ldi   r22,3
            call  EnableDigit
            call  DisableDigit
            ;enable interrupts
            sei
            ;return
            ret

INT0_vect:  ;--- INT0 handler (Minute Button) ---
            ;save regs
            push  r16
            push  r30   ;ZL
            push  r31   ;ZH            
            ;see which minute to increment
            sbic  PINB, 2        ;skip next instruction if bit clear
            rjmp  3f 
            ;use time minute                       
            ldi   ZL,lo8(Minute)
            ldi   ZH,hi8(Minute)
            rjmp  2f
3:          ;use alarm minute
            ldi   ZL,lo8(AMinute)
            ldi   ZH,hi8(AMinute)
2:          ;increment minute
            ld    r16,Z
            inc   r16
            cpi   r16,60
            brlo  1f
            clr   r16
1:          ;store minute
            st    Z,R16
            ;clear seconds
            clr   r16
            std   Z+1,r16     
            ;restore regs
            pop   r31
            pop   r30
            pop   r16
            reti

INT1_vect:  ;--- INT1 handler (houre Button) ---
            ;save regs
            push  r16
            push  r30   ;ZL
            push  r31   ;ZH            
            ;see which minute to increment
            sbic  PINB, 2         ;skip next instruction if bit clear
            rjmp  3f 
            ;use time hour
            ldi   ZL,lo8(Hour)
            ldi   ZH,hi8(Hour)
            rjmp  2f
3:          ;use alarm hour
            ldi   ZL,lo8(AHour)
            ldi   ZH,hi8(AHour)            
2:          ;increment hour
            ld    r16,Z
            inc   r16
            cpi   r16,24
            brlo  1f
            clr   r16
1:          ;store hour
            st    Z,r16
            ;clear second
            clr   r16
            std   Z+2,r16     
            ;restore regs
            pop   r31
            pop   r30
            pop   r16
            reti

TIMER1_COMPA_vect:
            push  r16
            lds   r16,Ticks
            inc   r16
            cpi   r16,4
            brne  1f             ;does not change flags!
            clr   r16
1:          sts   Ticks,r16      ;does not change flags!
            brne  2f             ;flags from cpi/clr are still set
            call  NextSecond
2:          pop   r16
            reti           

NextSecond: ;* increment time by one second
            ;* modifies r16            
            ;inc second
            lds   r16,Second
            inc   r16
            cpi   r16,60
            brlo  1f
            ldi   r16,0
            sts   Second,r16
            jmp   2f
1:          sts   Second,r16
            ret
2:          ;inc minute
            lds   r16,Minute
            inc   r16
            cpi   r16,60
            brlo  1f
            ldi   r16,0
            sts   Minute,r16
            jmp   2f
1:          sts   Minute,r16
            ret
2:          ;inc hour
            lds   r16,Hour
            inc   r16
4:          cpi   r16,24
            brlo  1f
            ldi   r16,0
1:          sts   Hour,r16
            ret

DisableDigit: ;* all of them; changes r23
              ldi r23,0xf0
              out PORTD,r23
              ret

EnableDigit:  ;r22=digit (0..3); changes r23                 
              cpi   r22,3
              breq  3f
              cpi   r22,2
              breq  2f
              cpi   r22,1
              breq  1f
              cbi   PORTD,4
              ret               
1:            cbi   PORTD,5
              ret  
2:            cbi   PORTD,6
              ret  
3:            cbi   PORTD,7
              ret  

ShowDigit:    ;show digit in r16; modifies r16
              ldi   ZL,lo8(segments)
              ldi   ZH,hi8(segments)
              add   ZL,r16            ;no risk of crossing a 256 byte boundary so far
              clr   r16
              adc   ZH,r16
              lpm                     ;read segments from program memory
              out   PORTC,r0          ;set segments
              rol   r0                ;bit 6 becomes 7
              rol   r0                ; ... carry
              rol   r0                ;... 0
              out   PORTB,r0
              ret

R16Div10:     ldi   ZL,lo8(div10)
              ldi   ZH,hi8(div10) 
              add   ZL,r16
              clr   r16
              adc   ZH,r16
              lpm   r16,Z
              ret


R16Mod10:     ldi   ZL,lo8(mod10)
              ldi   ZH,hi8(mod10) 
              add   ZL,r16
              clr   r16
              adc   ZH,r16
              lpm   r16,Z
              ret

segments:     .dc.b  0b0111111,0b110,0b1011011,0b1001111,0b1100110,0b1101101,0b1111101,0b0111,0b1111111,0b1101111  

div10:        .dc.b    0/10,  1/10,  2/10,  3/10,  4/10,  5/10,  6/10,  7/10,  8/10,  9/10 
              .dc.b   10/10, 11/10, 12/10, 13/10, 14/10, 15/10, 16/10, 17/10, 18/10, 19/10
              .dc.b   20/10, 21/10, 22/10, 23/10, 24/10, 25/10, 26/10, 27/10, 28/10, 29/10
              .dc.b   30/10, 31/10, 32/10, 33/10, 34/10, 35/10, 36/10, 37/10, 38/10, 39/10
              .dc.b   40/10, 41/10, 42/10, 43/10, 44/10, 45/10, 46/10, 47/10, 48/10, 49/10
              .dc.b   50/10, 51/10, 52/10, 53/10, 54/10, 55/10, 56/10, 57/10, 58/10, 59/10

mod10:        .dc.b    0%10,  1%10,  2%10,  3%10,  4%10,  5%10,  6%10,  7%10,  8%10,  9%10 
              .dc.b   10%10, 11%10, 12%10, 13%10, 14%10, 15%10, 16%10, 17%10, 18%10, 19%10
              .dc.b   20%10, 21%10, 22%10, 23%10, 24%10, 25%10, 26%10, 27%10, 28%10, 29%10
              .dc.b   30%10, 31%10, 32%10, 33%10, 34%10, 35%10, 36%10, 37%10, 38%10, 39%10
              .dc.b   40%10, 41%10, 42%10, 43%10, 44%10, 45%10, 46%10, 47%10, 48%10, 49%10
              .dc.b   50%10, 51%10, 52%10, 53%10, 54%10, 55%10, 56%10, 57%10, 58%10, 59%10


.data         ;this goes to SRAM

Hour:         .dc.b 0
Minute:       .dc.b 0
Second:       .dc.b 0
Ticks:        .dc.b 0
AHour:        .dc.b 0
AMinute:      .dc.b 0
ASecond:      .dc.b 0     ;always zero, needed to unify handling of buttons
