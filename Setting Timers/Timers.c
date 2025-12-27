#define __SFR_OFFSET 0      // maps port to I/O ports instead of memory-mapped I/O
#include <avr/io.h>
.global main    

main:       ;init
            ldi   r16,0xff         ;output mask for port D (all outputs)
            out   DDRB,r16
            out   DDRD,r16         ;write mask to DDRD                    
            out   PORTB,r16        ;turn all digits off
            ldi   r16,0            ;start counting at 0
            ;init digits 
            ldi   r17,0
            sts   Second,r17
            ldi   r17,0
            sts   Minute,r17
            ldi   r17,0
            sts   Hour,r17
            ;init timer 
            ldi   r16,249        ;16MHz/(250*64)=1000Hz -> 1ms 
            sts   OCR2A,r16
            ldi   r16,0b010      ;CTC 
            sts   TCCR2A,r16
            ldi   r16,0b100      ;CS22 -> prescale /64
            sts   TCCR2B,r16
            ldi   r16,0
            sts   TCNT2,r16
            sts   LastCnt,r16                                       
resetms:    ;reset millis
            ldi   r16,hi8(1000)
            sts   MillisH,r16            
            ldi   r16,lo8(1000)
            sts   MillisL,r16            
loop:       ;digit 1
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
            ;check timer
            lds   r17,TCNT2       ;get current timer value (high byte)
            lds   r16,LastCnt     ;get last timer value
            sts   LastCnt,r17     ;store current timer value            
            cp    r17,r16
            brsh  loop            ;no timer wraparound if current value is Same or Higher (brsh) than old value
            lds   r24,MillisL
            lds   r25,MillisH
            sbiw  r24,1           ;sets flags for conditional branch below
            sts   MillisL,r24     ;does not affect flags
            sts   MillisH,r25     ;does not affect flags
            brne  loop
            call  NextSecond      ;increment time
            jmp   resetms

NextSecond: ;increment time by one second
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

DisableDigit: ;all of them; changes r23
              ldi r23,0xff
              out PORTB,r23
              ret

EnableDigit:  ;r22=digit (0..3); changes r23                   
              ldi r23,1
1:            cpi r22,0
              breq  2f
              lsl   r23
              dec   r22
              jmp   1b
2:            com r23                 ;invert r16
              out PORTB,r23
              ret

ShowDigit:    ;show digit in r16; modifies r16
              ldi   ZL,lo8(segments)
              ldi   ZH,hi8(segments)
              add   ZL,r16            ;no risk of crossing a 256 byte boundary so far
              clr   r16
              adc   ZH,r16
              lpm                     ;read segments from program memory
              out   PORTD,r0          ;set segments
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

Second:       .dc.b 0
Minute:       .dc.b 0
Hour:         .dc.b 0
LastCnt:      .dc.b 0
MillisL:      .dc.b 0
MillisH:      .dc.b 0