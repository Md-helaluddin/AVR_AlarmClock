#define __SFR_OFFSET 0      // maps port to I/O ports instead of memory-mapped I/O
#include <avr/io.h>
.global main    


main:       ;init
            ldi   r16,0xff         ;output mask for port D (all outputs)
            out   DDRD,r16         ;write mask to DDRD                    
            ldi   r16,0            ;start counting at 0
loop:       ;control loop            
            call  showDigit        ;note: stack initialized by Arduino startup code
            ldi   r25, hi8(1000)
            ldi   r24, lo8(1000)
            call  delay           ;wait a second
            inc   r16
            cpi   r16,10
            brlo  loop
            ldi   r16,0
            jmp   loop


showDigit:  ;show digit in r16
            ldi   ZL,lo8(segments)
            ldi   ZH,hi8(segments)
            add   ZL,r16            ;.p2align ensures we do not cross a 256 byte boundary
            lpm                     ;read segments from program memory
            out   PORTD,r0          ;set segments
            ret

            .p2align  4             ;align by 2^4=16
segments:   .dc.b     0b0111111,0b110,0b1011011,0b1001111,0b1100110,0b1101101,0b1111101,0b0111,0b1111111,0b1101111  


delay:      ;delay a second
            ; Delay about (r25:r24)*ms. Clobbers r30, and r31.
            ; One millisecond is about 16000 cycles at 16MHz.
            ; The inner loop takes 4 cycles, so we repeat it 4000 times
            ldi   r31, hi8(4000)
            ldi   r30, lo8(4000)
1:          sbiw  r30, 1
            brne  1b
            sbiw  r24, 1
            brne  delay
            ret