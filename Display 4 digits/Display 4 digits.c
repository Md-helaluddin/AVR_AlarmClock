#define __SFR_OFFSET 0      // maps port to I/O ports instead of memory-mapped I/O
#include <avr/io.h>
.global main    


main:         ;init
              call  DisableDigit  ;make sure there is no garbage output at start
              ldi   r16,0xff      ;output mask for ports B+D (all outputs)
              out   DDRB,r16
              out   DDRD,r16                         
loop:         ;digit 1
              ldi   r16,1
              call  ShowDigit
              ldi   r22,0
              call  EnableDigit
              call  DisableDigit
              ;digit 2
              ldi   r16,2
              call  ShowDigit
              ldi   r22,1
              call  EnableDigit
              call  DisableDigit
              ;digit 3
              ldi   r16,3
              call  ShowDigit
              ldi   r22,2
              call  EnableDigit
              call  DisableDigit
              ;digit 4
              ldi   r16,4
              call  ShowDigit
              ldi   r22,3
              call  EnableDigit
              call  DisableDigit
              ;next round
              jmp   loop


DisableDigit: ;all of them; changes r23
              ldi   r23,0xff
              out   PORTB,r23
              ret


EnableDigit:  ;r22=digit (0..3); changes Z,r0 
              ldi   ZL,lo8(3f)
              ldi   ZH,hi8(3f)
              add   ZL,r22            ;no risk of crossing a 256 byte boundary
              lpm                     ;read from table in program memory to r0
              out   PORTB,r0
              ret

              .p2align  2             ;align to 2^2=4 byte boundary (so table does not cross 256 byte boundary)
3:            .dc.b     0b11111110,0b11111101,0b11111011,0b11110111


ShowDigit:    ;show digit in r16
              ldi   ZL,lo8(Segments)
              ldi   ZH,hi8(Segments)
              add   ZL,r16            ;no risk of crossing a 256 byte boundary
              lpm                     ;read segments from program memory
              out   PORTD,r0          ;set segments
              ret

              .p2align  4             ;align to 2^4=16 byte boundary (so table does not cross 256 byte boundary)
Segments:     .dc.b     0b0111111,0b110,0b1011011,0b1001111,0b1100110,0b1101101,0b1111101,0b0111,0b1111111,0b1101111 

