#define __SFR_OFFSET 0      // maps port to I/O ports instead of memory-mapped I/O
#include <avr/io.h>
.global main    


.text
main:         ;init
              ldi   r16,0xff         ;output mask for port D (all outputs)
              out   PORTB,r16        ;turn all digits off
              out   DDRB,r16
              out   DDRD,r16         ;write mask to DDRD                    
              ldi   r16,0            ;start counting at 0
              ;init digits
              ldi   r17,0
              sts   SecOne,r17
              ldi   r17,0
              sts   SecTen,r17
              ldi   r17,0
              sts   MinOne,r17
              ldi   r17,0
              sts   MinTen,r17
              ldi   r17,3
              sts   HrOne,r17
              ldi   r17,2
              sts   HrTen,r17
resetCnt:     ;init counter for "seconds" (not at correc speed)
              ldi   r24,0
              ldi   r25,2            
loop:         ;digit 1
              lds   r16,HrTen
              call  ShowDigit
              ldi   r22,0
              call  EnableDigit
              ;call  DDelay
              call  DisableDigit
              ;digit 2
              lds   r16,HrOne
              call  ShowDigit
              ldi   r22,1
              call  EnableDigit
              ;call  DDelay
              call  DisableDigit
              ;digit 3
              lds   r16,MinTen
              call  ShowDigit
              ldi   r22,2
              call  EnableDigit
              ;call  DDelay
              call  DisableDigit
              ;digit 4
              lds   r16,MinOne
              call  ShowDigit
              ldi   r22,3
              call  EnableDigit
              ;call  DDelay
              call  DisableDigit
              ;next round
              sbiw  r24,1
              brne  loop
              call  NextSecond
              jmp   resetCnt


NextSecond:   ;increment time by one second
              lds   r16,SecOne
              inc   r16
              cpi   r16,10
              brlo  1f
              ldi   r16,0
              sts   SecOne,r16
              jmp   2f
1:            sts   SecOne,r16
              ret

2:            ;inc ten secs
              lds   r16,SecTen
              inc   r16
              cpi   r16,6
              brlo  1f
              ldi   r16,0
              sts   SecTen,r16
              jmp   2f
1:            sts   SecTen,r16
              ret

2:            ;inc minute
              lds   r16,MinOne
              inc   r16
              cpi   r16,10
              brlo  1f
              ldi   r16,0
              sts   MinOne,r16
              jmp   2f
1:            sts   MinOne,r16
              ret

2:           ;inc ten mins
              lds   r16,MinTen
              inc   r16
              cpi   r16,6
              brlo  1f
              ldi   r16,0
              sts   MinTen,r16
              jmp   2f
1:            sts   MinTen,r16
              ret

2:            ;inc hour
              lds   r16,HrOne
              inc   r16
              cpi   r16,4
              breq  3f
4:            cpi   r16,10
              brlo  1f
              ldi   r16,0
              sts   HrOne,r16
              jmp   2f
1:            sts   HrOne,r16
              ret

3:            ;check for 24 hour overlflow
              lds   r17,HrTen
              cpi   r17,2
              brne  4b
              ldi   r16,0
              sts   HrTen,r16
              jmp   1b
2:            ;inc ten hrs
              lds   r16,HrTen
              inc   r16
              cpi   r16,3
              brlo  1f
              ldi   r16,0
1:            sts   HrTen,r16
              ret


DisableDigit: ;all of them; changes r23
              ldi   r23,0xff
              out   PORTB,r23
              ret


EnableDigit:  ;r22=digit (0..3); changes Z,r0 
              ldi   ZL,lo8(Digits)
              ldi   ZH,hi8(Digits)
              add   ZL,r22            ;no risk of crossing a 256 byte boundary
              lpm                     ;read from table in program memory to r0
              out   PORTB,r0
              ret

              .p2align  2             ;align to 2^2=4 byte boundary (so table does not cross 256 byte boundary)
Digits:       .dc.b     0b11111110,0b11111101,0b11111011,0b11110111


ShowDigit:    ;show digit in r16
              ldi   ZL,lo8(Segments)
              ldi   ZH,hi8(Segments)
              add   ZL,r16            ;no risk of crossing a 256 byte boundary
              lpm                     ;read segments from program memory
              out   PORTD,r0          ;set segments
              ret

              .p2align  4             ;align to 2^4=16 byte boundary (so table does not cross 256 byte boundary)
Segments:     .dc.b     0b0111111,0b110,0b1011011,0b1001111,0b1100110,0b1101101,0b1111101,0b0111,0b1111111,0b1101111 


.data

SecOne:  .dc.b 0
SecTen:  .dc.b 0
MinOne:  .dc.b 0
MinTen:  .dc.b 0
HrOne:   .dc.b 0
HrTen:   .dc.b 0