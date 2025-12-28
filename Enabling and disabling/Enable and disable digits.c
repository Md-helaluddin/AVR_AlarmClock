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