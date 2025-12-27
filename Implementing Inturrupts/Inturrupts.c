#define __SFR_OFFSET 0      // maps port to I/O ports instead of memory-mapped I/O
#include <avr/io.h>

.global main    
.global TIMER1_COMPA_vect
.global INT0_vect
.global INT1_vect

main:       ;init
            ldi   r16,1
            out   DDRB,r16         ;we use PB0 for segment G
            ldi   r16,0x3f        
            out   DDRC,r16         ;PC0-PC5 for segments A-F
            ldi   r16,0b11110010   ;also init TX as output; INTs as input 
            out   DDRD,r16         ;write mask to DDRD                    
            ldi   r16,0x00
            out   PORTD,r16        ;turn all digits off
            ;init digits 
            ldi   r17,0
            sts   Second,r17
            ldi   r17,30
            sts   Minute,r17
            ldi   r17,12
            sts   Hour,r17
            ;init timer 1 (16-Bit) to 1Hz (CTC with Prescale /256 and TOP=62500)
            ldi   r16,hi8(62500)  ;16MHz/(62500*256)=1Hz 
            sts   OCR1AH,r16
            ldi   r16,lo8(62500)  ;16MHz/(62500*256)=1Hz 
            sts   OCR1AL,r16
            ldi   r16,0      
            sts   TCCR1A,r16
            ldi   r16,0b1100      ;WGM12(CTC1)|CS12 -> CTC with prescale /256
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
loop:       ;disable interrupts
            cli
            ;digit 1
            lds   r16,Hour
            call  R16Div10
            call  ShowDigit
            ldi   r22,0
            call  EnableDigit
            call  DisableDigit
            ;digit 2
            lds   r16,Hour
            call  R16Mod10
            call  ShowDigit
            ldi   r22,1
            call  EnableDigit
            call  DisableDigit
            ;digit 3
            lds   r16,Minute
            call  R16Div10
            call  ShowDigit
            ldi   r22,2
            call  EnableDigit
            call  DisableDigit
            ;digit 4
            lds   r16,Minute
            call  R16Mod10
            call  ShowDigit
            ldi   r22,3
            call  EnableDigit
            call  DisableDigit
            ;enable interrupts
            sei               ;will invoke pending ints
            jmp   loop

TIMER1_COMPA_vect:
            push  r16
            call  NextSecond
            pop   r16
            reti           

INT0_vect:  ;save regs
            push  r16
            ;increment minute
            lds   r16,Minute
            inc   r16
            cpi   r16,60
            brlo  1f
            clr   r16
1:          sts   Minute,r16
            clr   r16
            sts   Second,r16
            ;restore regs
            pop   r16
            reti

INT1_vect:  ;save regs
            push  r16
            ;increment minute
            lds   r16,Hour
            inc   r16
            cpi   r16,24
            brlo  1f
            clr   r16
1:          sts   Hour,r16
            clr   r16
            sts   Second,r16
            ;restore regs
            pop   r16
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
              add   ZL,r16            ;no risk of crossing a 256 byte boundary because table is aligned
              lpm                     ;read segments from program memory
              out   PORTC,r0          ;set segments
              rol   r0                ;bit 6 becomes 7
              rol   r0                ; ... carry
              rol   r0                ;... 0
              out   PORTB,r0
              ret

R16Div10:     ldi   ZL,lo8(div10)
              ldi   ZH,hi8(div10) 
              add   ZL,r16            ;no risk of crossing a 256 byte boundary because table is aligned
              lpm   r16,Z
              ret


R16Mod10:     ldi   ZL,lo8(mod10)
              ldi   ZH,hi8(mod10) 
              add   ZL,r16            ;no risk of crossing a 256 byte boundary because table is aligned
              lpm   r16,Z
              ret

              .p2align  4
segments:     .dc.b  0b0111111,0b110,0b1011011,0b1001111,0b1100110,0b1101101,0b1111101,0b0111,0b1111111,0b1101111  

              .p2align  6
div10:        .dc.b    0/10,  1/10,  2/10,  3/10,  4/10,  5/10,  6/10,  7/10,  8/10,  9/10 
              .dc.b   10/10, 11/10, 12/10, 13/10, 14/10, 15/10, 16/10, 17/10, 18/10, 19/10
              .dc.b   20/10, 21/10, 22/10, 23/10, 24/10, 25/10, 26/10, 27/10, 28/10, 29/10
              .dc.b   30/10, 31/10, 32/10, 33/10, 34/10, 35/10, 36/10, 37/10, 38/10, 39/10
              .dc.b   40/10, 41/10, 42/10, 43/10, 44/10, 45/10, 46/10, 47/10, 48/10, 49/10
              .dc.b   50/10, 51/10, 52/10, 53/10, 54/10, 55/10, 56/10, 57/10, 58/10, 59/10

              .p2align  6
mod10:        .dc.b    0%10,  1%10,  2%10,  3%10,  4%10,  5%10,  6%10,  7%10,  8%10,  9%10 
              .dc.b   10%10, 11%10, 12%10, 13%10, 14%10, 15%10, 16%10, 17%10, 18%10, 19%10
              .dc.b   20%10, 21%10, 22%10, 23%10, 24%10, 25%10, 26%10, 27%10, 28%10, 29%10
              .dc.b   30%10, 31%10, 32%10, 33%10, 34%10, 35%10, 36%10, 37%10, 38%10, 39%10
              .dc.b   40%10, 41%10, 42%10, 43%10, 44%10, 45%10, 46%10, 47%10, 48%10, 49%10
              .dc.b   50%10, 51%10, 52%10, 53%10, 54%10, 55%10, 56%10, 57%10, 58%10, 59%10


.data         ;this goes to SRAM

Second:       .dc.b 0
Minute:       .dc.b 0
Hour:         .dc.b 0
