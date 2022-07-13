
; external functions declaration
.global BCD_Decode
.global BCD_Encode

; u08 BCD_Decode(u08 data)
BCD_Decode:
    mov   r25, r24
    swap  r24
    andi  r24, 0x0F ; 15
    ldi   r22, 0x0A ; 10
    rcall __mulqi3
    andi  r25, 0x0F ; 15
    add   r24, r25
    ret

; u08 BCD_Encode(u08 data)
BCD_Encode:
    ldi   r22, 0x0A ; 10
    rcall __udivmodqi4
    swap  r24
    andi  r24, 0xF0 ; 240
    add   r24, r25
    ret
