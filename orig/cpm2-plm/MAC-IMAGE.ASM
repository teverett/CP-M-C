; Control-flow disassembly of MAC.COM for Intel 8080.
; Reachable instructions are decoded; unreachable bytes remain DB data.
; Reassemble from the repository root with tools/asm80.py.
.cpu 8080
org 0100h
jmp 0128h                      ; 0100: C3 28 01
db " COPYRIGHT (C) 1977 DIGITAL RESEARCH " ; 0103: data
lxi sp, 3100h                  ; 0128: 31 00 31
xra a                          ; 012B: AF
sta 304Fh                      ; 012C: 32 4F 30
sta 305Ah                      ; 012F: 32 5A 30
call 2580h                     ; 0132: CD 80 25
call 1C03h                     ; 0135: CD 03 1C
lxi h, 0000h                   ; 0138: 21 00 00
shld 11D6h                     ; 013B: 22 D6 11
call 1C4Bh                     ; 013E: CD 4B 1C
xra a                          ; 0141: AF
sta 2EA3h                      ; 0142: 32 A3 2E
mvi a, 00h                     ; 0145: 3E 00
sta 2EA4h                      ; 0147: 32 A4 2E
lhld 304Dh                     ; 014A: 2A 4D 30
shld 2F24h                     ; 014D: 22 24 2F
call 1603h                     ; 0150: CD 03 16
call 2583h                     ; 0153: CD 83 25
lxi h, 0000h                   ; 0156: 21 00 00
shld 11C3h                     ; 0159: 22 C3 11
shld 11DEh                     ; 015C: 22 DE 11
lda 3067h                      ; 015F: 3A 67 30
ora a                          ; 0162: B7
jz 0169h                       ; 0163: CA 69 01
lxi h, 0100h                   ; 0166: 21 00 01
shld 3050h                     ; 0169: 22 50 30
shld 3052h                     ; 016C: 22 52 30
shld 11C7h                     ; 016F: 22 C7 11
xra a                          ; 0172: AF
sta 11CDh                      ; 0173: 32 CD 11
call 1606h                     ; 0176: CD 06 16
lda 3005h                      ; 0179: 3A 05 30
cpi 02h                        ; 017C: FE 02
jz 0176h                       ; 017E: CA 76 01
cpi 04h                        ; 0181: FE 04
jnz 01CBh                      ; 0183: C2 CB 01
lda 3009h                      ; 0186: 3A 09 30
cpi 24h                        ; 0189: FE 24
jnz 0DD5h                      ; 018B: C2 D5 0D
call 0EEAh                     ; 018E: CD EA 0E
jnz 0E20h                      ; 0191: C2 20 0E
lda 305Bh                      ; 0194: 3A 5B 30
mvi b, 00h                     ; 0197: 06 00
cpi 2Dh                        ; 0199: FE 2D
jz 01ACh                       ; 019B: CA AC 01
mvi b, 03h                     ; 019E: 06 03
cpi 2Bh                        ; 01A0: FE 2B
jz 01ACh                       ; 01A2: CA AC 01
mvi b, 07h                     ; 01A5: 06 07
cpi 2Ah                        ; 01A7: FE 2A
jnz 0E20h                      ; 01A9: C2 20 0E
push b                         ; 01AC: C5
call 1606h                     ; 01AD: CD 06 16
pop b                          ; 01B0: C1
lda 305Bh                      ; 01B1: 3A 5B 30
lxi h, 305Fh                   ; 01B4: 21 5F 30
cpi 4Dh                        ; 01B7: FE 4D
jz 01C4h                       ; 01B9: CA C4 01
lxi h, 3066h                   ; 01BC: 21 66 30
cpi 50h                        ; 01BF: FE 50
jnz 0E20h                      ; 01C1: C2 20 0E
mov m, b                       ; 01C4: 70
call 1606h                     ; 01C5: CD 06 16
jmp 0C09h                      ; 01C8: C3 09 0C
cpi 01h                        ; 01CB: FE 01
jnz 0E20h                      ; 01CD: C2 20 0E
call 2106h                     ; 01D0: CD 06 21
jz 040Dh                       ; 01D3: CA 0D 04
call 1C06h                     ; 01D6: CD 06 1C
call 1C09h                     ; 01D9: CD 09 1C
jnz 01ECh                      ; 01DC: C2 EC 01
call 1C0Ch                     ; 01DF: CD 0C 1C
lda 304Fh                      ; 01E2: 3A 4F 30
ora a                          ; 01E5: B7
cnz 119Ch                      ; 01E6: C4 9C 11
jmp 03E9h                      ; 01E9: C3 E9 03
call 1C12h                     ; 01EC: CD 12 1C
cpi 06h                        ; 01EF: FE 06
jnz 03E9h                      ; 01F1: C2 E9 03
lxi h, 0000h                   ; 01F4: 21 00 00
shld 11E0h                     ; 01F7: 22 E0 11
lda 304Fh                      ; 01FA: 3A 4F 30
ora a                          ; 01FD: B7
jz 020Fh                       ; 01FE: CA 0F 02
call 1C18h                     ; 0201: CD 18 1C
xchg                           ; 0204: EB
lhld 3056h                     ; 0205: 2A 56 30
mov a, l                       ; 0208: 7D
sub e                          ; 0209: 93
mov a, h                       ; 020A: 7C
sbb d                          ; 020B: 9A
jc 023Ch                       ; 020C: DA 3C 02
call 0272h                     ; 020F: CD 72 02
call 160Ch                     ; 0212: CD 0C 16
call 02AAh                     ; 0215: CD AA 02
jnz 02C1h                      ; 0218: C2 C1 02
lhld 11C3h                     ; 021B: 2A C3 11
mov a, h                       ; 021E: 7C
ora l                          ; 021F: B5
cnz 11B4h                      ; 0220: C4 B4 11
lda 304Fh                      ; 0223: 3A 4F 30
ora a                          ; 0226: B7
jnz 025Fh                      ; 0227: C2 5F 02
call 0291h                     ; 022A: CD 91 02
call 1C48h                     ; 022D: CD 48 1C
call 1C0Ch                     ; 0230: CD 0C 1C
lhld 3056h                     ; 0233: 2A 56 30
shld 11C3h                     ; 0236: 22 C3 11
jmp 0699h                      ; 0239: C3 99 06
lhld 3056h                     ; 023C: 2A 56 30
shld 11E0h                     ; 023F: 22 E0 11
call 1C45h                     ; 0242: CD 45 1C
call 1C09h                     ; 0245: CD 09 1C
jz 0256h                       ; 0248: CA 56 02
call 1C12h                     ; 024B: CD 12 1C
cpi 06h                        ; 024E: FE 06
jnz 02BBh                      ; 0250: C2 BB 02
jmp 0201h                      ; 0253: C3 01 02
call 160Ch                     ; 0256: CD 0C 16
call 02AAh                     ; 0259: CD AA 02
jnz 02BBh                      ; 025C: C2 BB 02
lhld 11E0h                     ; 025F: 2A E0 11
xchg                           ; 0262: EB
lhld 11D8h                     ; 0263: 2A D8 11
call 0EDDh                     ; 0266: CD DD 0E
jnz 02BBh                      ; 0269: C2 BB 02
shld 11C3h                     ; 026C: 22 C3 11
jmp 0699h                      ; 026F: C3 99 06
lhld 304Bh                     ; 0272: 2A 4B 30
push h                         ; 0275: E5
shld 3058h                     ; 0276: 22 58 30
lxi h, 3008h                   ; 0279: 21 08 30
mov c, m                       ; 027C: 4E
mov b, c                       ; 027D: 41
inx h                          ; 027E: 23
mov a, m                       ; 027F: 7E
push b                         ; 0280: C5
push h                         ; 0281: E5
call 1C27h                     ; 0282: CD 27 1C
pop h                          ; 0285: E1
pop b                          ; 0286: C1
dcr c                          ; 0287: 0D
jnz 027Eh                      ; 0288: C2 7E 02
pop h                          ; 028B: E1
mov m, b                       ; 028C: 70
shld 304Bh                     ; 028D: 22 4B 30
ret                            ; 0290: C9
lhld 304Bh                     ; 0291: 2A 4B 30
mov c, m                       ; 0294: 4E
shld 3058h                     ; 0295: 22 58 30
lxi h, 3008h                   ; 0298: 21 08 30
mov m, c                       ; 029B: 71
inx h                          ; 029C: 23
push b                         ; 029D: C5
push h                         ; 029E: E5
call 1C2Ah                     ; 029F: CD 2A 1C
pop h                          ; 02A2: E1
pop b                          ; 02A3: C1
mov m, a                       ; 02A4: 77
dcr c                          ; 02A5: 0D
jnz 029Ch                      ; 02A6: C2 9C 02
ret                            ; 02A9: C9
lda 3005h                      ; 02AA: 3A 05 30
cpi 05h                        ; 02AD: FE 05
rnz                            ; 02AF: C0
call 2106h                     ; 02B0: CD 06 21
rnz                            ; 02B3: C0
cpi 1Ah                        ; 02B4: FE 1A
rnz                            ; 02B6: C0
mov a, b                       ; 02B7: 78
cpi 09h                        ; 02B8: FE 09
ret                            ; 02BA: C9
call 119Ch                     ; 02BB: CD 9C 11
jmp 0DD5h                      ; 02BE: C3 D5 0D
lhld 3056h                     ; 02C1: 2A 56 30
push h                         ; 02C4: E5
call 0EF4h                     ; 02C5: CD F4 0E
lhld 3052h                     ; 02C8: 2A 52 30
lda 305Fh                      ; 02CB: 3A 5F 30
ora a                          ; 02CE: B7
cz 0F8Dh                       ; 02CF: CC 8D 0F
pop h                          ; 02D2: E1
shld 3056h                     ; 02D3: 22 56 30
call 1C1Eh                     ; 02D6: CD 1E 1C
sta 11C2h                      ; 02D9: 32 C2 11
lhld 304Dh                     ; 02DC: 2A 4D 30
push h                         ; 02DF: E5
ora a                          ; 02E0: B7
jz 0372h                       ; 02E1: CA 72 03
jmp 0300h                      ; 02E4: C3 00 03
cpi 3Bh                        ; 02E7: FE 3B
rz                             ; 02E9: C8
cpi 0Dh                        ; 02EA: FE 0D
rz                             ; 02EC: C8
cpi 0Ah                        ; 02ED: FE 0A
rz                             ; 02EF: C8
cpi 1Ah                        ; 02F0: FE 1A
rz                             ; 02F2: C8
cpi 21h                        ; 02F3: FE 21
ret                            ; 02F5: C9
lda 11C2h                      ; 02F6: 3A C2 11
ora a                          ; 02F9: B7
jz 0372h                       ; 02FA: CA 72 03
call 160Ch                     ; 02FD: CD 0C 16
lda 3005h                      ; 0300: 3A 05 30
cpi 04h                        ; 0303: FE 04
jnz 0346h                      ; 0305: C2 46 03
lda 3009h                      ; 0308: 3A 09 30
call 02E7h                     ; 030B: CD E7 02
jz 0365h                       ; 030E: CA 65 03
cpi 25h                        ; 0311: FE 25
jnz 033Bh                      ; 0313: C2 3B 03
call 0D6Dh                     ; 0316: CD 6D 0D
shld 11DCh                     ; 0319: 22 DC 11
mvi a, 0FFh                    ; 031C: 3E FF
sta 11DBh                      ; 031E: 32 DB 11
lda 3005h                      ; 0321: 3A 05 30
cpi 04h                        ; 0324: FE 04
jnz 0362h                      ; 0326: C2 62 03
lda 3009h                      ; 0329: 3A 09 30
push psw                       ; 032C: F5
xra a                          ; 032D: AF
sta 3008h                      ; 032E: 32 08 30
call 0D0Eh                     ; 0331: CD 0E 0D
call 03DBh                     ; 0334: CD DB 03
pop psw                        ; 0337: F1
jmp 0357h                      ; 0338: C3 57 03
cpi 2Ch                        ; 033B: FE 2C
jnz 0346h                      ; 033D: C2 46 03
call 03D7h                     ; 0340: CD D7 03
jmp 02F6h                      ; 0343: C3 F6 02
call 03DBh                     ; 0346: CD DB 03
call 1606h                     ; 0349: CD 06 16
lda 3005h                      ; 034C: 3A 05 30
cpi 04h                        ; 034F: FE 04
jnz 0362h                      ; 0351: C2 62 03
lda 3009h                      ; 0354: 3A 09 30
call 02E7h                     ; 0357: CD E7 02
jz 0365h                       ; 035A: CA 65 03
cpi 2Ch                        ; 035D: FE 2C
jz 02F6h                       ; 035F: CA F6 02
call 11B4h                     ; 0362: CD B4 11
lda 11C2h                      ; 0365: 3A C2 11
ora a                          ; 0368: B7
jz 0372h                       ; 0369: CA 72 03
call 03D7h                     ; 036C: CD D7 03
jmp 0365h                      ; 036F: C3 65 03
lhld 3058h                     ; 0372: 2A 58 30
inx h                          ; 0375: 23
push h                         ; 0376: E5
lxi h, 305Bh                   ; 0377: 21 5B 30
mov a, m                       ; 037A: 7E
call 02EDh                     ; 037B: CD ED 02
jz 0387h                       ; 037E: CA 87 03
call 1606h                     ; 0381: CD 06 16
jmp 0377h                      ; 0384: C3 77 03
xra a                          ; 0387: AF
mov m, a                       ; 0388: 77
sta 2F14h                      ; 0389: 32 14 2F
call 03ACh                     ; 038C: CD AC 03
lda 11CDh                      ; 038F: 3A CD 11
sta 2F54h                      ; 0392: 32 54 2F
call 1C2Dh                     ; 0395: CD 2D 1C
pop h                          ; 0398: E1
shld 2EF4h                     ; 0399: 22 F4 2E
pop h                          ; 039C: E1
shld 2F24h                     ; 039D: 22 24 2F
xra a                          ; 03A0: AF
sta 2F14h                      ; 03A1: 32 14 2F
mvi a, 01h                     ; 03A4: 3E 01
sta 2EA4h                      ; 03A6: 32 A4 2E
jmp 0176h                      ; 03A9: C3 76 01
lda 2EA3h                      ; 03AC: 3A A3 2E
ora a                          ; 03AF: B7
jz 03B8h                       ; 03B0: CA B8 03
lxi h, 2F91h                   ; 03B3: 21 91 2F
mvi m, 2Bh                     ; 03B6: 36 2B
call 2595h                     ; 03B8: CD 95 25
mvi a, 10h                     ; 03BB: 3E 10
sta 3004h                      ; 03BD: 32 04 30
ret                            ; 03C0: C9
db 3Ah, 0A3h, 2Eh, 0B7h, 0CAh, 0AEh, 11h, 3Ah, 0A4h, 2Eh, 0FEh, 03h ; 03C1: data
db 0D0h, 0FEh, 01h, 0C8h, 0CDh, 30h, 1Ch, 0C3h, 0C1h, 03h ; 03CD: data
xra a                          ; 03D7: AF
sta 3008h                      ; 03D8: 32 08 30
call 1C39h                     ; 03DB: CD 39 1C
call 1C24h                     ; 03DE: CD 24 1C
call 1C3Ch                     ; 03E1: CD 3C 1C
lxi h, 11C2h                   ; 03E4: 21 C2 11
dcr m                          ; 03E7: 35
ret                            ; 03E8: C9
lhld 11C3h                     ; 03E9: 2A C3 11
mov a, l                       ; 03EC: 7D
ora h                          ; 03ED: B4
cnz 11A2h                      ; 03EE: C4 A2 11
lhld 3056h                     ; 03F1: 2A 56 30
shld 11C3h                     ; 03F4: 22 C3 11
call 1606h                     ; 03F7: CD 06 16
lda 3005h                      ; 03FA: 3A 05 30
cpi 04h                        ; 03FD: FE 04
jnz 0179h                      ; 03FF: C2 79 01
lda 3009h                      ; 0402: 3A 09 30
cpi 3Ah                        ; 0405: FE 3A
jnz 0179h                      ; 0407: C2 79 01
jmp 0176h                      ; 040A: C3 76 01
cpi 1Ah                        ; 040D: FE 1A
jnz 0C21h                      ; 040F: C2 21 0C
mov e, b                       ; 0412: 58
mvi d, 00h                     ; 0413: 16 00
dcx d                          ; 0415: 1B
lxi h, 0420h                   ; 0416: 21 20 04
dad d                          ; 0419: 19
dad d                          ; 041A: 19
mov e, m                       ; 041B: 5E
inx h                          ; 041C: 23
mov h, m                       ; 041D: 66
mov l, e                       ; 041E: 6B
pchl                           ; 041F: E9
db 58h, 04h, 0A4h, 04h, 0BBh, 04h, 0D9h, 04h, 17h, 05h, 20h, 05h, 5Fh ; 0420: data
db 06h, 7Eh, 06h, 99h, 06h, 0Bh, 08h, 30h, 08h, 5Eh, 08h, 0A2h, 08h ; 042D: data
db 0C4h, 09h, 0C9h, 09h, 5Ah, 0Ah, 0F9h, 0Ah, 0FCh, 0Ah, 0FFh, 0Ah, 02h ; 043A: data
db 0Bh, 05h, 0Bh, 32h, 0Bh, 35h, 0Bh, 38h, 0Bh, 0A8h, 0Bh, 0ABh, 0Bh ; 0447: data
db 00h, 0Ch, 03h, 0Ch, 0CDh, 0F4h, 0Eh, 0CDh, 06h, 16h, 3Ah, 05h, 30h ; 0454: data
db 0FEh, 03h, 0C2h, 89h, 04h, 3Ah, 08h, 30h, 3Dh, 0CAh, 89h, 04h, 47h ; 0461: data
db 04h, 04h, 21h, 09h, 30h, 05h, 0CAh, 83h, 04h, 0C5h, 46h, 23h, 0E5h ; 046E: data
db 0CDh, 32h, 0Fh, 0E1h, 0C1h, 0C3h, 73h, 04h, 0CDh, 06h, 16h, 0C3h, 96h ; 047B: data
db 04h, 0CDh, 03h, 12h, 2Ah, 49h, 30h, 0CDh, 7Ch, 0Dh, 45h, 0CDh, 32h ; 0488: data
db 0Fh, 0CDh, 0E3h, 0Eh, 0CDh, 56h, 0Dh, 0FEh, 2Ch, 0CAh, 5Bh, 04h, 0C3h ; 0495: data
db 0D5h, 0Dh, 0CDh, 0F4h, 0Eh, 0CDh, 8Ah, 0Fh, 0CDh, 6Dh, 0Dh, 0EBh, 2Ah ; 04A2: data
db 52h, 30h, 19h, 22h, 52h, 30h, 22h, 50h, 30h, 0C3h, 0D5h, 0Dh, 0CDh ; 04AF: data
db 0F4h, 0Eh, 0CDh, 6Dh, 0Dh, 0E5h, 45h, 0CDh, 32h, 0Fh, 0E1h, 44h, 0CDh ; 04BC: data
db 32h, 0Fh, 0CDh, 0E3h, 0Eh, 0CDh, 56h, 0Dh, 0FEh, 2Ch, 0CAh, 0BEh, 04h ; 04C9: data
db 0C3h, 0D5h, 0Dh, 0CDh, 0F4h, 0Eh, 0CDh, 8Ah, 0Fh, 3Ah, 8Ch, 2Fh, 0FEh ; 04D6: data
db 20h, 0C2h, 0D5h, 0Dh, 0CDh, 6Dh, 0Dh, 3Ah, 8Ch, 2Fh, 0FEh, 20h, 0C2h ; 04E3: data
db 0F5h, 04h, 22h, 0C7h, 11h, 3Eh, 20h, 32h, 8Ch, 2Fh, 3Ah, 0CDh, 11h ; 04F0: data
db 0B7h, 0C4h, 0AEh, 11h, 0CDh, 06h, 16h, 3Ah, 05h, 30h, 0FEh, 04h, 0C2h ; 04FD: data
db 20h, 0Eh, 3Ah, 09h, 30h, 0FEh, 0Ah, 0C2h, 20h, 0Eh, 0C3h, 2Dh, 0Eh ; 050A: data
db 0CDh, 0F4h, 0Eh, 0CDh, 0B2h, 09h, 0C3h, 09h, 0Ch, 0C5h, 0CDh, 0F4h ; 0517: data
db 0Eh, 0CDh, 0C1h, 03h, 21h, 91h, "/6+:", 0A4h, 2Eh, 0FEh, 03h, 0D2h ; 0523: data
db 3Bh, 05h, 0C1h, 0CDh, 3Fh, 1Ch, 0C3h, 16h, 06h, 2Ah, 24h, 2Fh, 0E5h ; 0532: data
db 2Ah, 0B4h, 2Eh, 22h, 24h, 2Fh, 0CDh, 3Fh, 1Ch, 0E1h, 22h, 24h, 2Fh ; 053F: data
db 0F1h, 0FEh, 06h, 0C2h, 16h, 06h, 3Ah, 0A4h, 2Eh, 0FEh, 06h, 0C2h, 6Ch ; 054C: data
db 05h, 2Ah, 0B4h, ".^#V{", 0B2h, 0CAh, 16h, 06h, 1Bh, 72h, 2Bh, 73h ; 0559: data
db 0C3h, 35h, 06h, 2Ah, 0B4h, ".^#V", 1Ah, 0FEh, 0Dh, 0CAh, 16h, 06h ; 0569: data
db 0B7h, 0CAh, 93h, 05h, 3Ah, 0A4h, 2Eh, 0FEh, 03h, 0C2h, 0A0h, 05h, 1Ah ; 0578: data
db 13h, "r+s!", 08h, 30h, 36h, 01h, 23h, 77h, 0C3h, 9Ah, 05h, 3Eh, 0Dh ; 0585: data
db 12h, 0AFh, 32h, 08h, 30h, 0CDh, 39h, 1Ch, 0C3h, 06h, 06h, "!e/~" ; 0595: data
db 0F5h, 36h, 00h, "![0~", 0F5h, 36h, 00h, 0EBh, 22h, 0F4h, 2Eh, 7Eh ; 05A4: data
db 0D6h, 2Ch, 0C2h, 0C5h, 05h, 23h, 0E5h, 21h, 08h, 30h, 77h, 0CDh, 39h ; 05B3: data
db 1Ch, 0E1h, 0C3h, 0F0h, 05h, 0E5h, 0CDh, 0Ch, 16h, 0D1h, 0CDh, 0E4h ; 05C0: data
db 11h, 0C3h, 0D0h, 05h, 0CDh, 39h, 1Ch, 2Ah, 0F4h, 2Eh, 7Eh, 0B7h, 0C2h ; 05CC: data
db 0E0h, 05h, 36h, 0Dh, 0C3h, 0F7h, 05h, 2Ah, 0E2h, 11h, 0E5h, 0CDh, 06h ; 05D9: data
db 16h, 3Ah, 09h, 30h, 0FEh, 2Ch, 0C4h, 0B4h, 11h, 0E1h, 0CDh, 0EEh, 11h ; 05E6: data
db 0AFh, 32h, 66h, 2Fh, 0EBh, 2Ah, 0B4h, ".s#r", 0F1h, 32h, 5Bh, 30h ; 05F3: data
db 0F1h, "2e/*", 0B4h, 2Eh, 23h, 22h, 58h, 30h, 0CDh, 24h, 1Ch, 0CDh ; 0602: data
db 3Ch, 1Ch, 0C3h, 35h, 06h, 0CDh, 0ACh, 03h, 2Ah, 24h, 2Fh, 22h, 4Dh ; 0611: data
db 30h, 0CDh, 30h, 1Ch, ":T/2", 0CDh, 11h, 3Ah, 14h, "/2[0", 0B7h, 0C4h ; 061E: data
db 09h, 16h, 0C3h, 76h, 01h, 3Eh, 10h, 32h, 04h, 30h, 2Ah, 0D4h, 2Eh ; 0630: data
db 22h, 0F4h, 2Eh, 0AFh, 32h, 5Bh, 30h, 0C3h, 76h, 01h, 0F5h, 2Ah, 52h ; 063D: data
db 30h, 0E5h, 0CDh, 6Dh, 0Dh, 22h, 52h, 30h, 0CDh, 8Dh, 0Fh, 0E1h, 22h ; 064A: data
db 52h, 30h, 0F1h, 21h, 92h, 2Fh, 77h, 0C9h, 0CDh, 0EAh, 0Eh, 0CAh, 20h ; 0657: data
db 0Eh, 3Eh, 3Dh, 0CDh, 47h, 06h, 2Ah, 52h, 30h, 0E5h, 2Ah, 49h, 30h ; 0664: data
db 22h, 52h, 30h, 0CDh, 0F4h, 0Eh, 0E1h, 22h, 52h, 30h, 0C3h, 0D5h, 0Dh ; 0671: data
db 0CDh, 0F4h, 0Eh, 0CDh, 6Dh, 0Dh, 3Ah, 8Ch, 2Fh, 0FEh, 20h, 0C2h, 0C1h ; 067E: data
db 08h, 7Dh, 1Fh, 3Eh, 01h, 0D2h, 0C1h, 08h, 0CDh, 9Eh, 09h, 0C3h, 0D5h ; 068B: data
db 0Dh                         ; 0698: data
call 0EEAh                     ; 0699: CD EA 0E
jnz 06A5h                      ; 069C: C2 A5 06
call 11A2h                     ; 069F: CD A2 11
jmp 0DD5h                      ; 06A2: C3 D5 0D
lda 304Fh                      ; 06A5: 3A 4F 30
ora a                          ; 06A8: B7
jz 06C8h                       ; 06A9: CA C8 06
lhld 3056h                     ; 06AC: 2A 56 30
xchg                           ; 06AF: EB
lhld 11D8h                     ; 06B0: 2A D8 11
call 0EDDh                     ; 06B3: CD DD 0E
jz 06BFh                       ; 06B6: CA BF 06
call 119Ch                     ; 06B9: CD 9C 11
jmp 06CDh                      ; 06BC: C3 CD 06
call 1C18h                     ; 06BF: CD 18 1C
shld 11D8h                     ; 06C2: 22 D8 11
jmp 06CDh                      ; 06C5: C3 CD 06
mvi a, 06h                     ; 06C8: 3E 06
call 1C0Fh                     ; 06CA: CD 0F 1C
xra a                          ; 06CD: AF
sta 11DAh                      ; 06CE: 32 DA 11
lda 304Fh                      ; 06D1: 3A 4F 30
ora a                          ; 06D4: B7
cz 1C1Bh                       ; 06D5: CC 1B 1C
call 1606h                     ; 06D8: CD 06 16
lda 3005h                      ; 06DB: 3A 05 30
cpi 01h                        ; 06DE: FE 01
jnz 0701h                      ; 06E0: C2 01 07
lda 304Fh                      ; 06E3: 3A 4F 30
ora a                          ; 06E6: B7
cz 1C21h                       ; 06E7: CC 21 1C
lxi h, 11DAh                   ; 06EA: 21 DA 11
inr m                          ; 06ED: 34
call 1606h                     ; 06EE: CD 06 16
lda 3005h                      ; 06F1: 3A 05 30
cpi 04h                        ; 06F4: FE 04
jnz 0701h                      ; 06F6: C2 01 07
lda 3009h                      ; 06F9: 3A 09 30
cpi 2Ch                        ; 06FC: FE 2C
jz 06D8h                       ; 06FE: CA D8 06
mvi a, 01h                     ; 0701: 3E 01
call 0722h                     ; 0703: CD 22 07
jz 0E34h                       ; 0706: CA 34 0E
lda 304Fh                      ; 0709: 3A 4F 30
ora a                          ; 070C: B7
lda 11DAh                      ; 070D: 3A DA 11
cz 1C1Bh                       ; 0710: CC 1B 1C
jmp 0C09h                      ; 0713: C3 09 0C
cpi 09h                        ; 0716: FE 09
rz                             ; 0718: C8
cpi 10h                        ; 0719: FE 10
rz                             ; 071B: C8
cpi 0Eh                        ; 071C: FE 0E
rz                             ; 071E: C8
cpi 0Fh                        ; 071F: FE 0F
ret                            ; 0721: C9
sta 305Ch                      ; 0722: 32 5C 30
lda 3005h                      ; 0725: 3A 05 30
cpi 04h                        ; 0728: FE 04
jnz 073Dh                      ; 072A: C2 3D 07
lda 3009h                      ; 072D: 3A 09 30
cpi 0Dh                        ; 0730: FE 0D
jz 0746h                       ; 0732: CA 46 07
cpi 21h                        ; 0735: FE 21
jz 0746h                       ; 0737: CA 46 07
cpi 1Ah                        ; 073A: FE 1A
rz                             ; 073C: C8
call 11B4h                     ; 073D: CD B4 11
call 1606h                     ; 0740: CD 06 16
jmp 0725h                      ; 0743: C3 25 07
lhld 3058h                     ; 0746: 2A 58 30
shld 3060h                     ; 0749: 22 60 30
mvi a, 01h                     ; 074C: 3E 01
sta 305Ah                      ; 074E: 32 5A 30
call 1606h                     ; 0751: CD 06 16
lhld 3058h                     ; 0754: 2A 58 30
shld 11E0h                     ; 0757: 22 E0 11
call 1606h                     ; 075A: CD 06 16
lda 3005h                      ; 075D: 3A 05 30
cpi 04h                        ; 0760: FE 04
jnz 076Bh                      ; 0762: C2 6B 07
lda 3009h                      ; 0765: 3A 09 30
cpi 1Ah                        ; 0768: FE 1A
rz                             ; 076A: C8
cpi 01h                        ; 076B: FE 01
jnz 0754h                      ; 076D: C2 54 07
call 2106h                     ; 0770: CD 06 21
jnz 0754h                      ; 0773: C2 54 07
push psw                       ; 0776: F5
lda 305Ch                      ; 0777: 3A 5C 30
cpi 01h                        ; 077A: FE 01
jnz 07A6h                      ; 077C: C2 A6 07
lda 304Fh                      ; 077F: 3A 4F 30
ora a                          ; 0782: B7
jnz 07A6h                      ; 0783: C2 A6 07
lda 3008h                      ; 0786: 3A 08 30
dcr a                          ; 0789: 3D
jz 07A6h                       ; 078A: CA A6 07
dcr c                          ; 078D: 0D
jz 07A6h                       ; 078E: CA A6 07
push b                         ; 0791: C5
lhld 11E0h                     ; 0792: 2A E0 11
shld 3058h                     ; 0795: 22 58 30
call 2109h                     ; 0798: CD 09 21
call 1C27h                     ; 079B: CD 27 1C
lda 305Bh                      ; 079E: 3A 5B 30
ora a                          ; 07A1: B7
cnz 1C27h                      ; 07A2: C4 27 1C
pop b                          ; 07A5: C1
pop psw                        ; 07A6: F1
cpi 1Ah                        ; 07A7: FE 1A
jnz 0754h                      ; 07A9: C2 54 07
mov a, b                       ; 07AC: 78
call 0716h                     ; 07AD: CD 16 07
jnz 07BBh                      ; 07B0: C2 BB 07
lxi h, 305Ah                   ; 07B3: 21 5A 30
inr m                          ; 07B6: 34
rz                             ; 07B7: C8
jmp 0754h                      ; 07B8: C3 54 07
cpi 06h                        ; 07BB: FE 06
jnz 0754h                      ; 07BD: C2 54 07
lxi h, 305Ah                   ; 07C0: 21 5A 30
dcr m                          ; 07C3: 35
jnz 0754h                      ; 07C4: C2 54 07
lda 305Ch                      ; 07C7: 3A 5C 30
cpi 01h                        ; 07CA: FE 01
jnz 07F6h                      ; 07CC: C2 F6 07
lxi h, 0000h                   ; 07CF: 21 00 00
shld 11C3h                     ; 07D2: 22 C3 11
lda 305Dh                      ; 07D5: 3A 5D 30
ora a                          ; 07D8: B7
jz 07E5h                       ; 07D9: CA E5 07
lxi h, 0000h                   ; 07DC: 21 00 00
call 1C15h                     ; 07DF: CD 15 1C
jmp 07F1h                      ; 07E2: C3 F1 07
lhld 11D6h                     ; 07E5: 2A D6 11
call 1C15h                     ; 07E8: CD 15 1C
lhld 3056h                     ; 07EB: 2A 56 30
shld 11D6h                     ; 07EE: 22 D6 11
lda 304Fh                      ; 07F1: 3A 4F 30
ora a                          ; 07F4: B7
rnz                            ; 07F5: C0
lhld 3058h                     ; 07F6: 2A 58 30
mov a, m                       ; 07F9: 7E
cpi 0Dh                        ; 07FA: FE 0D
cnz 11B4h                      ; 07FC: C4 B4 11
lhld 3058h                     ; 07FF: 2A 58 30
mvi m, 0Dh                     ; 0802: 36 0D
xra a                          ; 0804: AF
call 1C27h                     ; 0805: CD 27 1C
xra a                          ; 0808: AF
inr a                          ; 0809: 3C
ret                            ; 080A: C9
db 0CDh, 6Dh, 0Dh, 3Ah, 8Ch, 2Fh, 0FEh, 20h, 0C2h, 0D5h, 0Dh, 3Ah, 67h ; 080B: data
db 30h, 0B7h, 0CAh, 21h, 08h, 11h, 00h, 01h, 19h, 22h, 52h, 30h, 22h ; 0818: data
db 50h, 30h, 0CDh, 0F4h, 0Eh, 0CDh, 8Ah, 0Fh, 0C3h, 0D5h, 0Dh, 0CDh ; 0825: data
db 0EAh, 0Eh, 0CAh, 20h, 0Eh, 0CDh, 12h, 1Ch, 0B7h, 0CAh, 42h, 08h, 0FEh ; 0831: data
db 05h, 0C4h, 0A2h, 11h, 3Eh, 05h, 0CDh, 0Fh, 1Ch, 3Eh, 23h, 0CDh, 47h ; 083E: data
db 06h, 0CDh, 0EAh, 0Eh, 2Ah, 49h, 30h, 0CDh, 15h, 1Ch, 21h, 00h, 00h ; 084B: data
db 22h, 0C3h, 11h, 0C3h, 0D5h, 0Dh, 0CDh, 0F4h, 0Eh, 0CDh, 06h, 16h, 3Ah ; 0858: data
db 05h, 30h, 0FEh, 03h, 0C2h, 20h, 0Eh, 3Ah, 5Ah, 30h, 0B7h, 0C2h, 20h ; 0865: data
db 0Eh, 21h, 08h, 30h, 4Eh, 0EBh, "*K0:O0", 0B7h, 0C2h, 9Fh, 08h, 22h ; 0872: data
db 62h, 30h, 2Bh, 22h, 58h, 30h, 79h, 0B7h, 0CAh, 9Bh, 08h, 13h, 1Ah ; 0883: data
db 0Dh, 0D5h, 0C5h, 0CDh, 27h, 1Ch, 0C1h, 0D1h, 0C3h, 89h, 08h, 0AFh ; 0890: data
db 0CDh, 27h, 1Ch, 0C3h, 09h, 0Ch, 0CDh, 0F4h, 0Eh, 0CDh, 0B2h, 09h ; 089C: data
db 0FEh, 01h, 3Eh, 02h, 0CAh, 0C1h, 08h, 0CDh, 0AEh, 11h, 0C3h, 09h, 0Ch ; 08A8: data
db 0FEh, 09h, 0C8h, 0FEh, 0Eh, 0C8h, 0FEh, 0Fh, 0C8h, 0FEh, 10h, 0C9h ; 08B5: data
db 32h, 0CAh, 11h, 0AFh, 32h, 0CBh, 11h, 32h, 0CCh, 11h, 3Ah, 05h, 30h ; 08C1: data
db 0FEh, 04h, 0C2h, 0F1h, 08h, 3Ah, 09h, 30h, 0FEh, 0Dh, 0C2h, 0E1h, 08h ; 08CE: data
db 0CDh, 06h, 16h, 0C3h, 0F7h, 08h, 0FEh, 21h, 0CAh, 0F7h, 08h, 0FEh ; 08DB: data
db 1Ah, 0C2h, 0F1h, 08h, 0CDh, 0AEh, 11h, 0C3h, 2Dh, 0Eh, 0CDh, 06h, 16h ; 08E7: data
db 0C3h, 0CBh, 08h, 0CDh, 06h, 16h, 3Ah, 05h, 30h, 0FEh, 02h, 0CCh, 06h ; 08F4: data
db 16h, 3Ah, 05h, 30h, 0FEh, 01h, 0C2h, 0CBh, 08h, 0CDh, 06h, 21h, 0CAh ; 0901: data
db 34h, 09h, 0CDh, 06h, 16h, 3Ah, 05h, 30h, 0FEh, 04h, 0C2h, 26h, 09h ; 090E: data
db 3Ah, 09h, 30h, 0FEh, 3Ah, 0C2h, 0CBh, 08h, 0CDh, 06h, 16h, 3Ah, 05h ; 091B: data
db 30h, 0FEh, 01h, 0C2h, 0CBh, 08h, 0CDh, 06h, 21h, 0C2h, 0CBh, 08h ; 0928: data
db 0FEh, 1Ah, 0C2h, 0CBh, 08h, 78h, 0FEh, 08h, 0C2h, 49h, 09h, 21h, 0CBh ; 0934: data
db 11h, 34h, 0CCh, 0A8h, 11h, 0C3h, 0CBh, 08h, 0FEh, 0Dh, 0C2h, 65h, 09h ; 0941: data
db 3Ah, 0CBh, 11h, 0B7h, 0C2h, 0CBh, 08h, 3Ah, 0CAh, 11h, 0FEh, 02h ; 094E: data
db 0CCh, 0AEh, 11h, 3Eh, 02h, 0CDh, 9Eh, 09h, 0C3h, 09h, 0Ch, 0FEh, 05h ; 095A: data
db 0C2h, 7Dh, 09h, 21h, 0CBh, 11h, 7Eh, 35h, 0B7h, 0C2h, 0CBh, 08h, 3Ah ; 0967: data
db 0CCh, 11h, 0B7h, 0C4h, 0AEh, 11h, 0C3h, 09h, 0Ch, 0CDh, 0B5h, 08h ; 0974: data
db 0C2h, 8Dh, 09h, 21h, 0CCh, 11h, 34h, 0CCh, 0A8h, 11h, 0C3h, 0CBh, 08h ; 0980: data
db 0FEh, 06h, 0C2h, 0CBh, 08h, 21h, 0CCh, 11h, 7Eh, 35h, 0B7h, 0C2h ; 098D: data
db 0CBh, 08h, 0C3h, 20h, 05h, 47h, 21h, 0CDh, 11h, 7Eh, 0FEh, 08h, 0D2h ; 0999: data
db 0A8h, 11h, 34h, 5Fh, 16h, 00h, 21h, 0CEh, 11h, 19h, 70h, 0C9h, 21h ; 09A6: data
db 0CDh, 11h, 7Eh, 0B7h, 0CAh, 0AEh, 11h, 35h, 5Eh, 16h, 00h, 21h, 0CEh ; 09B3: data
db 11h, 19h, 7Eh, 0C9h, 3Eh, 05h, 0C3h, 0CBh, 09h, 3Eh, 03h, 32h, 5Ch ; 09C0: data
db 30h, 0CDh, 0F4h, 0Eh, 0CDh, 06h, 16h, 3Ah, 05h, 30h, 0FEh, 01h, 0C2h ; 09CD: data
db 4Eh, 0Ah, 2Ah, 4Bh, 30h, 22h, 0C5h, 11h, 2Bh, 22h, 58h, 30h, 3Ah, 08h ; 09DA: data
db 30h, 0FEh, 10h, 0DAh, 0F0h, 09h, 3Eh, 10h, 0C6h, 03h, 0CDh, 27h, 1Ch ; 09E8: data
db 0AFh, 0CDh, 27h, 1Ch, 0CDh, 21h, 1Ch, 0CDh, 06h, 16h, 3Ah, 05h, 30h ; 09F5: data
db 0FEh, 04h, 0C2h, 4Eh, 0Ah, 3Ah, 09h, 30h, 0FEh, 2Ch, 0C2h, 4Eh, 0Ah ; 0A02: data
db 0CDh, 0Ch, 16h, 3Ah, 08h, 30h, 0B7h, 0C2h, 1Fh, 0Ah, 0CDh, 06h, 16h ; 0A0F: data
db 0C3h, 3Eh, 0Ah, 0CDh, 0Fh, 0Ch, 0CAh, 3Eh, 0Ah, 21h, 08h, "0N#~" ; 0A1C: data
db 0C5h, 0E5h, 0CDh, 27h, 1Ch, 0E1h, 0C1h, 0Dh, 0C2h, 29h, 0Ah, 3Eh, 0Dh ; 0A2B: data
db 0CDh, 27h, 1Ch, 0CDh, 06h, 16h, 0AFh, 0CDh, 27h, 1Ch, 2Ah, 4Bh, 30h ; 0A38: data
db 22h, "V0:\0", 0C3h, 78h, 0Ah, 0CDh, 0B4h, 11h, 3Ah, 5Ch, 30h, 0CDh ; 0A45: data
db 22h, 07h, 0C3h, 09h, 0Ch, 0CDh, 6Dh, 0Dh, 0E5h, "}*K0", 22h, 0C5h ; 0A55: data
db 11h, 2Bh, 22h, 58h, 30h, 0CDh, 27h, 1Ch, 0F1h, 0CDh, 27h, 1Ch, 2Ah ; 0A64: data
db 4Bh, 30h, 22h, 56h, 30h, 3Eh, 06h, 0CDh, 22h, 07h, 0CAh, 34h, 0Eh ; 0A71: data
db 0CDh, 0ACh, 03h, 0CDh, 06h, 16h, 3Ah, 0CDh, 11h, "2T/:[0", 0FEh, 0Ah ; 0A7E: data
db 0C2h, 93h, 0Ah, 0AFh, 32h, 14h, 2Fh, 0CDh, 2Dh, 1Ch, 2Ah, 4Dh, 30h ; 0A8F: data
db 22h, "$/*X0", 0EBh, 21h, 0C5h, 11h, 7Bh, 96h, 23h, 7Ah, 9Eh, 0EBh ; 0A9C: data
db 0DAh, 0BFh, 0Ah, 7Eh, 2Bh, 22h, "X0*M0+", 22h, 4Dh, 30h, 77h, 0C3h ; 0AAC: data
db 9Fh, 0Ah, 23h, 22h, "K0*M0", 22h, 0B4h, 2Eh, 00h, 3Ah, 5Ch, 30h, 0FEh ; 0ABD: data
db 06h, 0CAh, 0DDh, 0Ah, 4Eh, 06h, 00h, 5Dh, 54h, 09h, 0EBh, "s#r+" ; 0ACE: data
db 0E5h, 2Ah, 56h, 30h, 0EBh, 2Ah, 0C5h, 11h, 7Bh, 95h, 5Fh, 7Ah, 9Ch ; 0ADD: data
db 57h, 0E1h, 19h, 22h, 0D4h, ".:\02", 0A4h, 2Eh, 0C3h, 52h, 05h, 0C3h ; 0AEA: data
db 06h, 0Ch, 0C3h, 06h, 0Ch, 0C3h, 06h, 0Ch, 0C3h, 06h, 0Ch, 0CDh, 0F4h ; 0AFA: data
db 0Eh, 0CDh, 06h, 16h, 0CDh, 0Fh, 0Ch, 0CAh, 25h, 0Bh, 0CDh, 03h, 12h ; 0B07: data
db "*I0:", 8Ch, 2Fh, 0FEh, 20h, 0C2h, 0D5h, 0Dh, 0CDh, 0AAh, 25h, 0C3h ; 0B14: data
db 0D5h, 0Dh, 0CDh, 0ACh, 03h, 3Ah, 4Fh, 30h, 0B7h, 0C4h, 0ADh, 25h ; 0B23: data
db 0C3h, 0D5h, 0Dh, 0C3h, 20h, 05h, 0C3h, 06h, 0Ch, 3Ah, 0A3h, 2Eh, 0B7h ; 0B2F: data
db 0CAh, 0A2h, 0Bh, 0CDh, 06h, 16h, 3Ah, 05h, 30h, 0FEh, 01h, 0C2h, 0A2h ; 0B3C: data
db 0Bh, 2Ah, 4Bh, 30h, 0E5h, 2Bh, 22h, 58h, 30h, 0CDh, 21h, 1Ch, 0AFh ; 0B49: data
db 32h, 0DBh, 11h, 3Ch, 32h, 08h, 30h, 2Ah, 0DEh, 11h, 23h, 22h, 0DEh ; 0B56: data
db 11h, 22h, 0DCh, 11h, 0CDh, 0Eh, 0Dh, 3Ah, 0Ah, 30h, 0FEh, 30h, 0C4h ; 0B63: data
db 0A8h, 11h, 21h, 3Fh, 3Fh, 22h, 09h, 30h, 0CDh, 39h, 1Ch, 0E1h, 22h ; 0B70: data
db 4Bh, 30h, 2Bh, 22h, 58h, 30h, 0CDh, 24h, 1Ch, 0CDh, 3Ch, 1Ch, 0CDh ; 0B7D: data
db 06h, 16h, 0CDh, 0Fh, 0Ch, 0CAh, 0D5h, 0Dh, 3Ah, 05h, 30h, 0FEh, 04h ; 0B8A: data
db 0C2h, 0A2h, 0Bh, 3Ah, 09h, 30h, 0FEh, 2Ch, 0CAh, 3Fh, 0Bh, 0CDh, 0B4h ; 0B97: data
db 11h, 0C3h, 0D5h, 0Dh, 0C3h, 06h, 0Ch, 0CDh, 0F4h, 0Eh, 2Ah, 0D6h, 11h ; 0BA4: data
db 7Dh, 0B4h, 0C2h, 0FAh, 0Bh, 3Ah, 5Dh, 30h, 0B7h, 0C2h, 0FAh, 0Bh ; 0BB1: data
db 0CDh, 06h, 16h, 3Ah, 4Fh, 30h, 0B7h, 0C2h, 09h, 0Ch, 3Ah, 05h, 30h ; 0BBD: data
db 0FEh, 01h, 0C2h, 0FAh, 0Bh, 0CDh, 0A4h, "%:e0", 0B7h, 0C4h, 0ADh, 25h ; 0BCA: data
db 0CDh, 06h, 16h, 3Ah, 05h, 30h, 0FEh, 04h, 0C2h, 0D9h, 0Bh, 3Ah, 09h ; 0BD9: data
db 30h, 0FEh, 0Dh, 0CAh, 0F1h, 0Bh, 0FEh, 1Ah, 0C2h, 0D9h, 0Bh, 0CDh ; 0BE6: data
db 0ACh, 03h, 0CDh, 0A7h, 25h, 0C3h, 76h, 01h, 0CDh, 0B4h, 11h, 0C3h ; 0BF2: data
db 0D5h, 0Dh, 0C3h, 06h, 0Ch, 0C3h, 06h, 0Ch, 0CDh, 0BAh, 11h ; 0BFE: data
call 1606h                     ; 0C09: CD 06 16
jmp 0DD5h                      ; 0C0C: C3 D5 0D
db 3Ah, 05h, 30h, 0FEh, 04h, 0C0h, 3Ah, 09h, 30h, 0FEh, 0Dh, 0C8h, 0FEh ; 0C0F: data
db 21h, 0C8h, 0FEh, 3Bh, 0C9h  ; 0C1C: data
sui 1Ch                        ; 0C21: D6 1C
cpi 2Ah                        ; 0C23: FE 2A
jnc 0E20h                      ; 0C25: D2 20 0E
mov e, a                       ; 0C28: 5F
mvi d, 00h                     ; 0C29: 16 00
lxi h, 0C35h                   ; 0C2B: 21 35 0C
dad d                          ; 0C2E: 19
dad d                          ; 0C2F: 19
mov e, m                       ; 0C30: 5E
inx h                          ; 0C31: 23
mov h, m                       ; 0C32: 66
mov l, e                       ; 0C33: 6B
pchl                           ; 0C34: E9
db 53h, 0Ch, 5Ch, 0Ch, 68h, 0Ch, 6Eh, 0Ch, 82h, 0Ch, 8Bh, 0Ch, 9Ah, 0Ch ; 0C35: data
db 0AAh, 0Ch, 0B3h, 0Ch, 0C2h, 0Ch, 0CBh, 0Ch, 0D2h, 0Ch, 0D9h, 0Ch ; 0C43: data
db 0E8h, 0Ch, 0EFh, 0Ch, 0CDh, 32h, 0Fh, 0CDh, 06h, 16h, 0C3h, 0FBh, 0Ch ; 0C4F: data
db 0CDh, 0A0h, 0Dh, 0CDh, 0BBh, 0Dh, 0CDh, 0B5h, 0Dh, 0C3h, 0FBh, 0Ch ; 0C5C: data
db 0CDh, 0A0h, 0Dh, 0C3h, 0FBh, 0Ch, 0CDh, 96h, 0Dh, 0FEh, 38h, 0CAh ; 0C68: data
db 7Bh, 0Ch, 0E6h, 08h, 0C4h, 82h, 11h, 79h, 0E6h, 30h, 0B0h, 0C3h, 0F8h ; 0C74: data
db 0Ch, 0CDh, 32h, 0Fh, 0CDh, 0B5h, 0Dh, 0C3h, 0FBh, 0Ch, 0CDh, 96h, 0Dh ; 0C81: data
db 0B0h, 47h, 0CDh, 0BBh, 0Dh, 0CDh, 8Bh, 0Dh, 0B0h, 0C3h, 0F8h, 0Ch ; 0C8E: data
db 0CDh, 96h, 0Dh, 0B0h, 0CDh, 31h, 0Fh, 0CDh, 0BBh, 0Dh, 0CDh, 0AFh ; 0C9A: data
db 0Dh, 0C3h, 0FBh, 0Ch, 0CDh, 32h, 0Fh, 0CDh, 0AFh, 0Dh, 0C3h, 0FBh ; 0CA6: data
db 0Ch, 0CDh, 96h, 0Dh, 0E6h, 28h, 0C4h, 82h, 11h, 79h, 0E6h, 10h, 0B0h ; 0CB2: data
db 0C3h, 0F8h, 0Ch, 0CDh, 32h, 0Fh, 0CDh, 0B5h, 0Dh, 0C3h, 0FBh, 0Ch ; 0CBF: data
db 0CDh, 8Bh, 0Dh, 0B0h, 0C3h, 0F8h, 0Ch, 0CDh, 96h, 0Dh, 0B0h, 0C3h ; 0CCB: data
db 0F8h, 0Ch, 0CDh, 96h, 0Dh, 0E6h, 08h, 0C4h, 82h, 11h, 79h, 0E6h, 30h ; 0CD7: data
db 0B0h, 0C3h, 0F8h, 0Ch, 0CDh, 96h, 0Dh, 0B0h, 0C3h, 0F8h, 0Ch, 0CDh ; 0CE4: data
db 32h, 0Fh, 0CDh, 0AFh, 0Dh, 0C3h, 0FBh, 0Ch, 0CDh, 31h, 0Fh, 0CDh ; 0CF0: data
db 0F4h, 0Eh, 0CDh, 0E3h, 0Eh, 0C3h, 0D5h, 0Dh, 10h, 27h, 0E8h, 03h, 64h ; 0CFC: data
db 00h, 0Ah, 00h, 01h, 00h     ; 0D09: data
mvi b, 05h                     ; 0D0E: 06 05
lxi h, 0D04h                   ; 0D10: 21 04 0D
mov e, m                       ; 0D13: 5E
inx h                          ; 0D14: 23
mov d, m                       ; 0D15: 56
inx h                          ; 0D16: 23
push h                         ; 0D17: E5
lhld 11DCh                     ; 0D18: 2A DC 11
mvi c, 30h                     ; 0D1B: 0E 30
mov a, l                       ; 0D1D: 7D
sub e                          ; 0D1E: 93
mov l, a                       ; 0D1F: 6F
mov a, h                       ; 0D20: 7C
sbb d                          ; 0D21: 9A
mov h, a                       ; 0D22: 67
jc 0D2Ah                       ; 0D23: DA 2A 0D
inr c                          ; 0D26: 0C
jmp 0D1Dh                      ; 0D27: C3 1D 0D
dad d                          ; 0D2A: 19
shld 11DCh                     ; 0D2B: 22 DC 11
lda 11DBh                      ; 0D2E: 3A DB 11
ora a                          ; 0D31: B7
jz 0D44h                       ; 0D32: CA 44 0D
mov a, b                       ; 0D35: 78
dcr a                          ; 0D36: 3D
jz 0D44h                       ; 0D37: CA 44 0D
mov a, c                       ; 0D3A: 79
cpi 30h                        ; 0D3B: FE 30
jz 0D50h                       ; 0D3D: CA 50 0D
xra a                          ; 0D40: AF
sta 11DBh                      ; 0D41: 32 DB 11
lxi h, 3008h                   ; 0D44: 21 08 30
mov e, m                       ; 0D47: 5E
inr m                          ; 0D48: 34
mvi d, 00h                     ; 0D49: 16 00
lxi h, 3009h                   ; 0D4B: 21 09 30
dad d                          ; 0D4E: 19
mov m, c                       ; 0D4F: 71
pop h                          ; 0D50: E1
dcr b                          ; 0D51: 05
jnz 0D13h                      ; 0D52: C2 13 0D
ret                            ; 0D55: C9
db 3Ah, 05h, 30h, 0FEh, 04h, 0C4h, 96h, 11h, 3Ah, 09h, 30h, 0FEh, 2Ch ; 0D56: data
db 0C8h, 0FEh, 3Bh, 0C8h, 0FEh, 0Dh, 0C4h, 96h, 11h, 0C9h ; 0D63: data
push b                         ; 0D6D: C5
call 1606h                     ; 0D6E: CD 06 16
call 1203h                     ; 0D71: CD 03 12
lhld 3049h                     ; 0D74: 2A 49 30
pop b                          ; 0D77: C1
ret                            ; 0D78: C9
db 0CDh, 6Dh, 0Dh, 7Ch, 0B7h, 7Dh, 0C8h, 24h, 0C2h, 86h, 0Dh, 0B7h, 0F8h ; 0D79: data
db 0CDh, 8Ch, 11h, 6Fh, 0C9h, 0CDh, 79h, 0Dh, 0FEh, 08h, 0D4h, 8Ch, 11h ; 0D86: data
db 0E6h, 07h, 0C9h, 0CDh, 8Bh, 0Dh, 17h, 17h, 17h, 0E6h, 38h, 4Fh, 0C9h ; 0D93: data
db 0CDh, 96h, 0Dh, 0E6h, 08h, 0C4h, 82h, 11h, 79h, 0E6h, 30h, 0B0h, 0C3h ; 0DA0: data
db 31h, 0Fh, 0CDh, 79h, 0Dh, 0C3h, 31h, 0Fh, 0CDh, 6Dh, 0Dh, 0C3h, 58h ; 0DAD: data
db 0Fh, 0F5h, 0C5h, 3Ah, 05h, 30h, 0FEh, 04h, 0C2h, 0CDh, 0Dh, 3Ah, 09h ; 0DBA: data
db 30h, 0FEh, 2Ch, 0CAh, 0D2h, 0Dh, 3Eh, 43h, 0CDh, 98h, 25h, 0C1h, 0F1h ; 0DC7: data
db 0C9h                        ; 0DD4: data
call 0EF4h                     ; 0DD5: CD F4 0E
lda 3005h                      ; 0DD8: 3A 05 30
cpi 04h                        ; 0DDB: FE 04
jnz 0E20h                      ; 0DDD: C2 20 0E
lda 3009h                      ; 0DE0: 3A 09 30
cpi 0Dh                        ; 0DE3: FE 0D
jnz 0DEEh                      ; 0DE5: C2 EE 0D
call 1606h                     ; 0DE8: CD 06 16
jmp 0176h                      ; 0DEB: C3 76 01
cpi 3Bh                        ; 0DEE: FE 3B
jnz 0E16h                      ; 0DF0: C2 16 0E
call 0EF4h                     ; 0DF3: CD F4 0E
call 1606h                     ; 0DF6: CD 06 16
lda 3005h                      ; 0DF9: 3A 05 30
cpi 04h                        ; 0DFC: FE 04
jnz 0DF6h                      ; 0DFE: C2 F6 0D
lda 3009h                      ; 0E01: 3A 09 30
cpi 0Ah                        ; 0E04: FE 0A
jz 0176h                       ; 0E06: CA 76 01
cpi 1Ah                        ; 0E09: FE 1A
jz 0E2Dh                       ; 0E0B: CA 2D 0E
cpi 21h                        ; 0E0E: FE 21
jz 0176h                       ; 0E10: CA 76 01
jmp 0DF6h                      ; 0E13: C3 F6 0D
cpi 21h                        ; 0E16: FE 21
jz 0176h                       ; 0E18: CA 76 01
cpi 1Ah                        ; 0E1B: FE 1A
jz 0E2Dh                       ; 0E1D: CA 2D 0E
call 11B4h                     ; 0E20: CD B4 11
jmp 0DF6h                      ; 0E23: C3 F6 0D
mov a, e                       ; 0E26: 7B
sub l                          ; 0E27: 95
mov l, a                       ; 0E28: 6F
mov a, d                       ; 0E29: 7A
sbb h                          ; 0E2A: 9C
mov h, a                       ; 0E2B: 67
ret                            ; 0E2C: C9
lda 2EA3h                      ; 0E2D: 3A A3 2E
ora a                          ; 0E30: B7
jz 0E37h                       ; 0E31: CA 37 0E
call 11AEh                     ; 0E34: CD AE 11
xra a                          ; 0E37: AF
sta 305Ah                      ; 0E38: 32 5A 30
lxi h, 304Fh                   ; 0E3B: 21 4F 30
mov a, m                       ; 0E3E: 7E
inr m                          ; 0E3F: 34
ora a                          ; 0E40: B7
jnz 0E6Ch                      ; 0E41: C2 6C 0E
lxi h, 0FFFFh                  ; 0E44: 21 FF FF
shld 11D8h                     ; 0E47: 22 D8 11
lhld 11D6h                     ; 0E4A: 2A D6 11
mov a, h                       ; 0E4D: 7C
ora l                          ; 0E4E: B5
jz 013Eh                       ; 0E4F: CA 3E 01
shld 3056h                     ; 0E52: 22 56 30
push h                         ; 0E55: E5
call 1C18h                     ; 0E56: CD 18 1C
xthl                           ; 0E59: E3
push h                         ; 0E5A: E5
lhld 11D8h                     ; 0E5B: 2A D8 11
call 1C15h                     ; 0E5E: CD 15 1C
pop h                          ; 0E61: E1
shld 11D8h                     ; 0E62: 22 D8 11
pop h                          ; 0E65: E1
shld 11D6h                     ; 0E66: 22 D6 11
jmp 0E4Ah                      ; 0E69: C3 4A 0E
call 1606h                     ; 0E6C: CD 06 16
call 0F8Ah                     ; 0E6F: CD 8A 0F
lxi h, 2F91h                   ; 0E72: 21 91 2F
mvi m, 0Dh                     ; 0E75: 36 0D
lxi h, 2F8Dh                   ; 0E77: 21 8D 2F
call 2592h                     ; 0E7A: CD 92 25
lda 305Eh                      ; 0E7D: 3A 5E 30
ora a                          ; 0E80: B7
jz 0E8Fh                       ; 0E81: CA 8F 0E
mvi a, 01h                     ; 0E84: 3E 01
sta 2EA4h                      ; 0E86: 32 A4 2E
call 25A1h                     ; 0E89: CD A1 25
call 0FB1h                     ; 0E8C: CD B1 0F
lhld 304Bh                     ; 0E8F: 2A 4B 30
xchg                           ; 0E92: EB
lhld 3054h                     ; 0E93: 2A 54 30
call 0E26h                     ; 0E96: CD 26 0E
push h                         ; 0E99: E5
lhld 304Dh                     ; 0E9A: 2A 4D 30
xchg                           ; 0E9D: EB
lhld 3054h                     ; 0E9E: 2A 54 30
call 0E26h                     ; 0EA1: CD 26 0E
mov e, h                       ; 0EA4: 5C
mvi d, 00h                     ; 0EA5: 16 00
pop h                          ; 0EA7: E1
call 1209h                     ; 0EA8: CD 09 12
xchg                           ; 0EAB: EB
call 0F8Dh                     ; 0EAC: CD 8D 0F
lxi h, 2F91h                   ; 0EAF: 21 91 2F
lxi d, 0EC0h                   ; 0EB2: 11 C0 0E
ldax d                         ; 0EB5: 1A
ora a                          ; 0EB6: B7
jz 0ECEh                       ; 0EB7: CA CE 0E
mov m, a                       ; 0EBA: 77
inx h                          ; 0EBB: 23
inx d                          ; 0EBC: 13
jmp 0EB5h                      ; 0EBD: C3 B5 0E
db "H USE FACTOR", 0Dh, 00h    ; 0EC0: data
lxi h, 2F8Eh                   ; 0ECE: 21 8E 2F
call 2592h                     ; 0ED1: CD 92 25
lhld 11C7h                     ; 0ED4: 2A C7 11
shld 3050h                     ; 0ED7: 22 50 30
jmp 259Eh                      ; 0EDA: C3 9E 25
mov a, d                       ; 0EDD: 7A
cmp h                          ; 0EDE: BC
rnz                            ; 0EDF: C0
mov a, e                       ; 0EE0: 7B
cmp l                          ; 0EE1: BD
ret                            ; 0EE2: C9
db 2Ah, 50h, 30h, 22h, 52h, 30h, 0C9h ; 0EE3: data
lhld 11C3h                     ; 0EEA: 2A C3 11
shld 3056h                     ; 0EED: 22 56 30
call 1C09h                     ; 0EF0: CD 09 1C
ret                            ; 0EF3: C9
call 0EEAh                     ; 0EF4: CD EA 0E
rz                             ; 0EF7: C8
lxi h, 0000h                   ; 0EF8: 21 00 00
shld 11C3h                     ; 0EFB: 22 C3 11
lda 304Fh                      ; 0EFE: 3A 4F 30
ora a                          ; 0F01: B7
jnz 0F1Bh                      ; 0F02: C2 1B 0F
call 1C12h                     ; 0F05: CD 12 1C
push psw                       ; 0F08: F5
ani 07h                        ; 0F09: E6 07
cnz 11A2h                      ; 0F0B: C4 A2 11
pop psw                        ; 0F0E: F1
ori 01h                        ; 0F0F: F6 01
call 1C0Fh                     ; 0F11: CD 0F 1C
lhld 3052h                     ; 0F14: 2A 52 30
call 1C15h                     ; 0F17: CD 15 1C
ret                            ; 0F1A: C9
call 1C12h                     ; 0F1B: CD 12 1C
ani 07h                        ; 0F1E: E6 07
cz 119Ch                       ; 0F20: CC 9C 11
call 1C18h                     ; 0F23: CD 18 1C
xchg                           ; 0F26: EB
lhld 3052h                     ; 0F27: 2A 52 30
call 0EDDh                     ; 0F2A: CD DD 0E
cnz 119Ch                      ; 0F2D: C4 9C 11
ret                            ; 0F30: C9
db 47h, 0C5h, 3Ah, 4Fh, 30h, 0B7h, 78h, 0C4h, 9Bh, 25h, 3Ah, 8Dh, 2Fh ; 0F31: data
db 0FEh, " *R0", 0CCh, 8Dh, 0Fh, 3Ah, 0C9h, 11h, 0FEh, 10h, 0C1h, 78h ; 0F3E: data
db 0DCh, 7Ah, 0Fh, "*P0#", 22h, 50h, 30h, 0C9h, 0E5h, 45h, 0CDh, 32h ; 0F4D: data
db 0Fh, 0E1h, 44h, 0C3h, 32h, 0Fh ; 0F5C: data
adi 30h                        ; 0F62: C6 30
cpi 3Ah                        ; 0F64: FE 3A
rc                             ; 0F66: D8
adi 07h                        ; 0F67: C6 07
ret                            ; 0F69: C9
call 0F62h                     ; 0F6A: CD 62 0F
lxi h, 11C9h                   ; 0F6D: 21 C9 11
mov e, m                       ; 0F70: 5E
mvi d, 00h                     ; 0F71: 16 00
inr m                          ; 0F73: 34
lxi h, 2F8Ch                   ; 0F74: 21 8C 2F
dad d                          ; 0F77: 19
mov m, a                       ; 0F78: 77
ret                            ; 0F79: C9
push psw                       ; 0F7A: F5
rar                            ; 0F7B: 1F
rar                            ; 0F7C: 1F
rar                            ; 0F7D: 1F
rar                            ; 0F7E: 1F
ani 0Fh                        ; 0F7F: E6 0F
call 0F6Ah                     ; 0F81: CD 6A 0F
pop psw                        ; 0F84: F1
ani 0Fh                        ; 0F85: E6 0F
jmp 0F6Ah                      ; 0F87: C3 6A 0F
lhld 3052h                     ; 0F8A: 2A 52 30
xchg                           ; 0F8D: EB
lxi h, 11C9h                   ; 0F8E: 21 C9 11
push h                         ; 0F91: E5
mvi m, 01h                     ; 0F92: 36 01
mov a, d                       ; 0F94: 7A
push d                         ; 0F95: D5
call 0F7Ah                     ; 0F96: CD 7A 0F
pop d                          ; 0F99: D1
mov a, e                       ; 0F9A: 7B
call 0F7Ah                     ; 0F9B: CD 7A 0F
pop h                          ; 0F9E: E1
inr m                          ; 0F9F: 34
ret                            ; 0FA0: C9
sui 41h                        ; 0FA1: D6 41
cpi 1Ah                        ; 0FA3: FE 1A
mov e, a                       ; 0FA5: 5F
rc                             ; 0FA6: D8
adi 41h                        ; 0FA7: C6 41
cpi 3Fh                        ; 0FA9: FE 3F
mvi e, 1Ah                     ; 0FAB: 1E 1A
rz                             ; 0FAD: C8
mvi e, 1Bh                     ; 0FAE: 1E 1B
ret                            ; 0FB0: C9
xra a                          ; 0FB1: AF
sta 2EA5h                      ; 0FB2: 32 A5 2E
sta 11C9h                      ; 0FB5: 32 C9 11
lhld 3054h                     ; 0FB8: 2A 54 30
shld 3056h                     ; 0FBB: 22 56 30
lxi h, 2EACh                   ; 0FBE: 21 AC 2E
mvi c, 38h                     ; 0FC1: 0E 38
xra a                          ; 0FC3: AF
mov m, a                       ; 0FC4: 77
inx h                          ; 0FC5: 23
dcr c                          ; 0FC6: 0D
jnz 0FC4h                      ; 0FC7: C2 C4 0F
lhld 3056h                     ; 0FCA: 2A 56 30
xchg                           ; 0FCD: EB
lhld 304Bh                     ; 0FCE: 2A 4B 30
mov a, e                       ; 0FD1: 7B
sub l                          ; 0FD2: 95
mov a, d                       ; 0FD3: 7A
sbb h                          ; 0FD4: 9C
jnc 10A4h                      ; 0FD5: D2 A4 10
lhld 3062h                     ; 0FD8: 2A 62 30
call 0EDDh                     ; 0FDB: CD DD 0E
dcx h                          ; 0FDE: 2B
shld 3058h                     ; 0FDF: 22 58 30
jz 0FFDh                       ; 0FE2: CA FD 0F
call 1C12h                     ; 0FE5: CD 12 1C
cpi 06h                        ; 0FE8: FE 06
jnz 100Bh                      ; 0FEA: C2 0B 10
call 1C1Eh                     ; 0FED: CD 1E 1C
ora a                          ; 0FF0: B7
jz 0FFDh                       ; 0FF1: CA FD 0F
dcr a                          ; 0FF4: 3D
push psw                       ; 0FF5: F5
call 1C24h                     ; 0FF6: CD 24 1C
pop psw                        ; 0FF9: F1
jmp 0FF0h                      ; 0FFA: C3 F0 0F
call 1C2Ah                     ; 0FFD: CD 2A 1C
ora a                          ; 1000: B7
jnz 0FFDh                      ; 1001: C2 FD 0F
lhld 3058h                     ; 1004: 2A 58 30
inx h                          ; 1007: 23
jmp 109Eh                      ; 1008: C3 9E 10
lxi h, 2EA4h                   ; 100B: 21 A4 2E
cmp m                          ; 100E: BE
jnz 1090h                      ; 100F: C2 90 10
lhld 3056h                     ; 1012: 2A 56 30
shld 2EAAh                     ; 1015: 22 AA 2E
inx h                          ; 1018: 23
shld 3058h                     ; 1019: 22 58 30
call 1C24h                     ; 101C: CD 24 1C
lda 3064h                      ; 101F: 3A 64 30
ora a                          ; 1022: B7
jnz 103Ch                      ; 1023: C2 3C 10
lda 3008h                      ; 1026: 3A 08 30
cpi 02h                        ; 1029: FE 02
jc 103Ch                       ; 102B: DA 3C 10
lxi h, 3009h                   ; 102E: 21 09 30
mov a, m                       ; 1031: 7E
cpi 3Fh                        ; 1032: FE 3F
jnz 103Ch                      ; 1034: C2 3C 10
inx h                          ; 1037: 23
cmp m                          ; 1038: BE
jz 1090h                       ; 1039: CA 90 10
lda 3009h                      ; 103C: 3A 09 30
call 0FA1h                     ; 103F: CD A1 0F
lxi h, 2EACh                   ; 1042: 21 AC 2E
mvi d, 00h                     ; 1045: 16 00
dad d                          ; 1047: 19
dad d                          ; 1048: 19
shld 2EA8h                     ; 1049: 22 A8 2E
mov e, m                       ; 104C: 5E
inx h                          ; 104D: 23
mov d, m                       ; 104E: 56
xchg                           ; 104F: EB
shld 3056h                     ; 1050: 22 56 30
mov a, l                       ; 1053: 7D
ora h                          ; 1054: B4
jz 107Bh                       ; 1055: CA 7B 10
inx h                          ; 1058: 23
inx h                          ; 1059: 23
mov a, m                       ; 105A: 7E
ani 0Fh                        ; 105B: E6 0F
inr a                          ; 105D: 3C
mov c, a                       ; 105E: 4F
lxi d, 3008h                   ; 105F: 11 08 30
mov b, m                       ; 1062: 46
inx d                          ; 1063: 13
inx h                          ; 1064: 23
ldax d                         ; 1065: 1A
cmp m                          ; 1066: BE
jc 107Bh                       ; 1067: DA 7B 10
jnz 1075h                      ; 106A: C2 75 10
dcr b                          ; 106D: 05
jz 107Bh                       ; 106E: CA 7B 10
dcr c                          ; 1071: 0D
jnz 1063h                      ; 1072: C2 63 10
lhld 3056h                     ; 1075: 2A 56 30
jmp 1049h                      ; 1078: C3 49 10
lhld 3056h                     ; 107B: 2A 56 30
xchg                           ; 107E: EB
lhld 2EAAh                     ; 107F: 2A AA 2E
shld 3056h                     ; 1082: 22 56 30
mov m, e                       ; 1085: 73
inx h                          ; 1086: 23
mov m, d                       ; 1087: 72
dcx h                          ; 1088: 2B
xchg                           ; 1089: EB
lhld 2EA8h                     ; 108A: 2A A8 2E
mov m, e                       ; 108D: 73
inx h                          ; 108E: 23
mov m, d                       ; 108F: 72
lhld 3056h                     ; 1090: 2A 56 30
inx h                          ; 1093: 23
inx h                          ; 1094: 23
mov a, m                       ; 1095: 7E
ani 0Fh                        ; 1096: E6 0F
adi 04h                        ; 1098: C6 04
mov e, a                       ; 109A: 5F
mvi d, 00h                     ; 109B: 16 00
dad d                          ; 109D: 19
shld 3056h                     ; 109E: 22 56 30
jmp 0FCAh                      ; 10A1: C3 CA 0F
lxi h, 2EACh                   ; 10A4: 21 AC 2E
shld 2EA8h                     ; 10A7: 22 A8 2E
mvi a, 1Ch                     ; 10AA: 3E 1C
sta 2EA7h                      ; 10AC: 32 A7 2E
lhld 2EA8h                     ; 10AF: 2A A8 2E
mov e, m                       ; 10B2: 5E
inx h                          ; 10B3: 23
mov d, m                       ; 10B4: 56
inx h                          ; 10B5: 23
shld 2EA8h                     ; 10B6: 22 A8 2E
xchg                           ; 10B9: EB
shld 3056h                     ; 10BA: 22 56 30
lhld 3056h                     ; 10BD: 2A 56 30
mov a, l                       ; 10C0: 7D
ora h                          ; 10C1: B4
jz 1164h                       ; 10C2: CA 64 11
inx h                          ; 10C5: 23
inx h                          ; 10C6: 23
mov a, m                       ; 10C7: 7E
ani 0Fh                        ; 10C8: E6 0F
inr a                          ; 10CA: 3C
sta 2EA6h                      ; 10CB: 32 A6 2E
mov b, a                       ; 10CE: 47
lhld 3056h                     ; 10CF: 2A 56 30
inx h                          ; 10D2: 23
inx h                          ; 10D3: 23
shld 3058h                     ; 10D4: 22 58 30
lda 2EA5h                      ; 10D7: 3A A5 2E
ora a                          ; 10DA: B7
jz 10FAh                       ; 10DB: CA FA 10
mvi a, 09h                     ; 10DE: 3E 09
call 1175h                     ; 10E0: CD 75 11
lxi h, 2EA5h                   ; 10E3: 21 A5 2E
mov a, m                       ; 10E6: 7E
ani 0F8h                       ; 10E7: E6 F8
adi 08h                        ; 10E9: C6 08
mov m, a                       ; 10EB: 77
ani 0Fh                        ; 10EC: E6 0F
jz 10FAh                       ; 10EE: CA FA 10
mvi a, 08h                     ; 10F1: 3E 08
add m                          ; 10F3: 86
mov m, a                       ; 10F4: 77
mvi a, 09h                     ; 10F5: 3E 09
call 1175h                     ; 10F7: CD 75 11
lda 2EA5h                      ; 10FA: 3A A5 2E
add b                          ; 10FD: 80
adi 05h                        ; 10FE: C6 05
cpi 50h                        ; 1100: FE 50
jc 1127h                       ; 1102: DA 27 11
lxi h, 11C9h                   ; 1105: 21 C9 11
dcr m                          ; 1108: 35
mov e, m                       ; 1109: 5E
mvi d, 00h                     ; 110A: 16 00
dcx d                          ; 110C: 1B
lxi h, 2F8Ch                   ; 110D: 21 8C 2F
dad d                          ; 1110: 19
mov a, m                       ; 1111: 7E
cpi 09h                        ; 1112: FE 09
jz 1105h                       ; 1114: CA 05 11
lxi h, 11C9h                   ; 1117: 21 C9 11
mov a, m                       ; 111A: 7E
mvi m, 00h                     ; 111B: 36 00
sta 3004h                      ; 111D: 32 04 30
call 2595h                     ; 1120: CD 95 25
xra a                          ; 1123: AF
sta 2EA5h                      ; 1124: 32 A5 2E
call 1C18h                     ; 1127: CD 18 1C
push h                         ; 112A: E5
mov a, h                       ; 112B: 7C
call 0F7Ah                     ; 112C: CD 7A 0F
pop h                          ; 112F: E1
mov a, l                       ; 1130: 7D
call 0F7Ah                     ; 1131: CD 7A 0F
mvi a, 20h                     ; 1134: 3E 20
call 1175h                     ; 1136: CD 75 11
lxi h, 2EA5h                   ; 1139: 21 A5 2E
mov a, m                       ; 113C: 7E
adi 05h                        ; 113D: C6 05
mov m, a                       ; 113F: 77
lda 2EA6h                      ; 1140: 3A A6 2E
ora a                          ; 1143: B7
jz 1157h                       ; 1144: CA 57 11
dcr a                          ; 1147: 3D
push psw                       ; 1148: F5
call 1C2Ah                     ; 1149: CD 2A 1C
call 1175h                     ; 114C: CD 75 11
lxi h, 2EA5h                   ; 114F: 21 A5 2E
inr m                          ; 1152: 34
pop psw                        ; 1153: F1
jmp 1143h                      ; 1154: C3 43 11
lhld 3056h                     ; 1157: 2A 56 30
mov e, m                       ; 115A: 5E
inx h                          ; 115B: 23
mov d, m                       ; 115C: 56
xchg                           ; 115D: EB
shld 3056h                     ; 115E: 22 56 30
jmp 10BDh                      ; 1161: C3 BD 10
lxi h, 2EA7h                   ; 1164: 21 A7 2E
dcr m                          ; 1167: 35
jnz 10AFh                      ; 1168: C2 AF 10
lda 11C9h                      ; 116B: 3A C9 11
sta 3004h                      ; 116E: 32 04 30
call 2595h                     ; 1171: CD 95 25
ret                            ; 1174: C9
lxi h, 11C9h                   ; 1175: 21 C9 11
mov e, m                       ; 1178: 5E
mvi d, 00h                     ; 1179: 16 00
inr m                          ; 117B: 34
lxi h, 2F8Ch                   ; 117C: 21 8C 2F
dad d                          ; 117F: 19
mov m, a                       ; 1180: 77
ret                            ; 1181: C9
db 0F5h, 0C5h, 3Eh, 52h, 0CDh, 98h, 25h, 0C1h, 0F1h, 0C9h, 0F5h, 0E5h ; 1182: data
db 3Eh, 56h, 0CDh, 98h, 25h, 0E1h, 0F1h, 0C9h, 0F5h, 3Eh, 44h, 0C3h ; 118E: data
db 0BDh, 11h                   ; 119A: data
push psw                       ; 119C: F5
mvi a, 50h                     ; 119D: 3E 50
jmp 11BDh                      ; 119F: C3 BD 11
push psw                       ; 11A2: F5
mvi a, 4Ch                     ; 11A3: 3E 4C
jmp 11BDh                      ; 11A5: C3 BD 11
db 0F5h, 3Eh, 4Fh, 0C3h, 0BDh, 11h ; 11A8: data
push psw                       ; 11AE: F5
mvi a, 42h                     ; 11AF: 3E 42
jmp 11BDh                      ; 11B1: C3 BD 11
push psw                       ; 11B4: F5
mvi a, 53h                     ; 11B5: 3E 53
jmp 11BDh                      ; 11B7: C3 BD 11
db 0F5h, 3Eh, 4Eh              ; 11BA: data
call 2598h                     ; 11BD: CD 98 25
pop psw                        ; 11C0: F1
ret                            ; 11C1: C9
db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h ; 11C2: data
db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h ; 11D0: data
db 00h, 00h, 00h, 00h, 00h, 00h, 2Ah, 0E2h, 11h, 0CDh, 0DDh, 0Eh, 0C0h ; 11DE: data
db 36h, 00h, 0C9h, 22h, 0F4h, 2Eh, 7Eh, 0FEh, 0Dh, 0C0h, 36h, 00h, 0C9h ; 11EB: data
db 00h, 00h, "654321", 0C3h, 00h, 16h ; 11F8: data
jmp 142Dh                      ; 1203: C3 2D 14
db 0C3h, 1Eh, 13h              ; 1206: data
jmp 12E8h                      ; 1209: C3 E8 12
db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h ; 120C: data
db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h ; 121A: data
db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h ; 1228: data
xchg                           ; 1233: EB
lxi h, 1232h                   ; 1234: 21 32 12
mov a, m                       ; 1237: 7E
cpi 10h                        ; 1238: FE 10
jc 1242h                       ; 123A: DA 42 12
call 15E4h                     ; 123D: CD E4 15
mvi m, 00h                     ; 1240: 36 00
mov a, m                       ; 1242: 7E
inr m                          ; 1243: 34
inr m                          ; 1244: 34
mov c, a                       ; 1245: 4F
mvi b, 00h                     ; 1246: 06 00
lxi h, 1221h                   ; 1248: 21 21 12
dad b                          ; 124B: 09
mov m, e                       ; 124C: 73
inx h                          ; 124D: 23
mov m, d                       ; 124E: 72
ret                            ; 124F: C9
push psw                       ; 1250: F5
lxi h, 1231h                   ; 1251: 21 31 12
mov a, m                       ; 1254: 7E
cpi 0Ah                        ; 1255: FE 0A
jc 125Fh                       ; 1257: DA 5F 12
mvi m, 00h                     ; 125A: 36 00
call 15E4h                     ; 125C: CD E4 15
mov e, m                       ; 125F: 5E
mvi d, 00h                     ; 1260: 16 00
inr m                          ; 1262: 34
pop psw                        ; 1263: F1
lxi h, 120Dh                   ; 1264: 21 0D 12
dad d                          ; 1267: 19
mov m, a                       ; 1268: 77
lxi h, 1217h                   ; 1269: 21 17 12
dad d                          ; 126C: 19
mov m, b                       ; 126D: 70
ret                            ; 126E: C9
db 21h, 32h, 12h, 7Eh, 0B7h, 0C2h, 7Eh, 12h, 0CDh, 0E4h, 15h, 21h, 00h ; 126F: data
db 00h, 0C9h, 35h, 35h, 4Eh, 06h, 00h, 21h, 21h, 12h, 09h, "N#fi", 0C9h ; 127C: data
db 0CDh, 6Fh, 12h, 0EBh, 0CDh, 6Fh, 12h, 0C9h ; 128C: data
mov l, a                       ; 1294: 6F
mvi h, 00h                     ; 1295: 26 00
dad h                          ; 1297: 29
lxi d, 12A1h                   ; 1298: 11 A1 12
dad d                          ; 129B: 19
mov e, m                       ; 129C: 5E
inx h                          ; 129D: 23
mov h, m                       ; 129E: 66
mov l, e                       ; 129F: 6B
pchl                           ; 12A0: E9
db 39h, 13h, 42h, 13h, 49h, 13h, 4Fh, 13h, 5Bh, 13h, 6Fh, 13h, 76h, 13h ; 12A1: data
db 80h, 13h, 8Fh, 13h, 9Bh, 13h, 0A8h, 13h, 0B4h, 13h, 0BBh, 13h, 0C2h ; 12AF: data
db 13h, 0DAh, 13h, 0E1h, 13h, 0EDh, 13h, 0F9h, 13h, 05h, 14h, 0Ch, 14h ; 12BC: data
db 0E4h, 15h, 0CDh, 8Ch, 12h, 7Ah, 0B7h, 0C2h, 0D7h, 12h, 7Bh, 0FEh, 11h ; 12C9: data
db 0D8h, 0CDh, 0E4h, 15h, 3Eh, 10h, 0C9h, 0AFh, 95h, 6Fh, 3Eh, 00h, 9Ch ; 12D6: data
db 67h, 0C9h, 0CDh, 8Ch, 12h   ; 12E3: data
xchg                           ; 12E8: EB
shld 131Bh                     ; 12E9: 22 1B 13
lxi h, 131Dh                   ; 12EC: 21 1D 13
mvi m, 11h                     ; 12EF: 36 11
lxi b, 0000h                   ; 12F1: 01 00 00
push b                         ; 12F4: C5
xra a                          ; 12F5: AF
mov a, e                       ; 12F6: 7B
ral                            ; 12F7: 17
mov e, a                       ; 12F8: 5F
mov a, d                       ; 12F9: 7A
ral                            ; 12FA: 17
mov d, a                       ; 12FB: 57
dcr m                          ; 12FC: 35
pop h                          ; 12FD: E1
rz                             ; 12FE: C8
mvi a, 00h                     ; 12FF: 3E 00
aci 00h                        ; 1301: CE 00
dad h                          ; 1303: 29
mov b, h                       ; 1304: 44
add l                          ; 1305: 85
lhld 131Bh                     ; 1306: 2A 1B 13
sub l                          ; 1309: 95
mov c, a                       ; 130A: 4F
mov a, b                       ; 130B: 78
sbb h                          ; 130C: 9C
mov b, a                       ; 130D: 47
push b                         ; 130E: C5
jnc 1314h                      ; 130F: D2 14 13
dad b                          ; 1312: 09
xthl                           ; 1313: E3
lxi h, 131Dh                   ; 1314: 21 1D 13
cmc                            ; 1317: 3F
jmp 12F6h                      ; 1318: C3 F6 12
db 00h, 00h, 00h, 44h, 4Dh, 21h, 00h, 00h, 0AFh, 78h, 1Fh, 47h, 79h, 1Fh ; 131B: data
db 4Fh, 0DAh, 32h, 13h, 0B0h, 0C8h, 0C3h, 33h, 13h, 19h, 0EBh, 29h, 0EBh ; 1329: data
db 0C3h, 23h, 13h, 0CDh, 8Ch, 12h, 0CDh, 1Eh, 13h, 0C3h, 11h, 14h, 0CDh ; 1336: data
db 0E5h, 12h, 0EBh, 0C3h, 11h, 14h, 0CDh, 0E5h, 12h, 0C3h, 11h, 14h ; 1343: data
db 0CDh, 0CBh, 12h, 0B7h, 0CAh, 11h, 14h, 29h, 3Dh, 0C3h, 52h, 13h, 0CDh ; 134F: data
db 0CBh, 12h, 0B7h, 0CAh, 11h, 14h, 0F5h, 0AFh, 7Ch, 1Fh, 67h, 7Dh, 1Fh ; 135C: data
db 6Fh, 0F1h, 3Dh, 0C3h, 5Eh, 13h, 0CDh, 8Ch, 12h, 19h, 0C3h, 11h, 14h ; 1369: data
db 0CDh, 8Ch, 12h, 0EBh, 0CDh, 0DDh, 12h, 0C3h, 72h, 13h, 0CDh, 6Fh, 12h ; 1376: data
db 0CDh, 0DDh, 12h, 0C3h, 11h, 14h, 7Ah, 0BCh, 0C0h, 7Bh, 0BDh, 0C9h ; 1383: data
db 0CDh, 8Ch, 12h, 0CDh, 89h, 13h, 0C2h, 0D4h, 13h, 0C3h, 0CEh, 13h ; 138F: data
db 0CDh, 8Ch, 12h, 7Dh, 93h, 7Ch, 9Ah, 0DAh, 0CEh, 13h, 0C3h, 0D4h, 13h ; 139B: data
db 0CDh, 8Ch, 12h, 0CDh, 89h, 13h, 0CAh, 0CEh, 13h, 0C3h, 9Eh, 13h, 0CDh ; 13A8: data
db 8Ch, 12h, 0EBh, 0C3h, 9Eh, 13h, 0CDh, 8Ch, 12h, 0EBh, 0C3h, 0ABh, 13h ; 13B5: data
db 0CDh, 8Ch, 12h, 0CDh, 89h, 13h, 0C2h, 0CEh, 13h, 0C3h, 0D4h, 13h, 21h ; 13C2: data
db 0FFh, 0FFh, 0C3h, 11h, 14h, 21h, 00h, 00h, 0C3h, 11h, 14h, 0CDh, 6Fh ; 13CF: data
db 12h, 23h, 0C3h, 83h, 13h, 0CDh, 8Ch, 12h, 7Ah, 0A4h, 67h, 7Bh, 0A5h ; 13DC: data
db 6Fh, 0C3h, 11h, 14h, 0CDh, 8Ch, 12h, 7Ah, 0B4h, 67h, 7Bh, 0B5h, 6Fh ; 13E9: data
db 0C3h, 11h, 14h, 0CDh, 8Ch, 12h, 7Ah, 0ACh, 67h, 7Bh, 0ADh, 6Fh, 0C3h ; 13F6: data
db 11h, 14h, 0CDh, 6Fh, 12h, 6Ch, 0C3h, 0Fh, 14h, 0CDh, 6Fh, 12h, 26h ; 1403: data
db 00h, 0C3h, 33h, 12h         ; 1410: data
lda 3005h                      ; 1414: 3A 05 30
cpi 04h                        ; 1417: FE 04
rnz                            ; 1419: C0
lda 3009h                      ; 141A: 3A 09 30
cpi 0Dh                        ; 141D: FE 0D
rz                             ; 141F: C8
cpi 3Bh                        ; 1420: FE 3B
rz                             ; 1422: C8
cpi 21h                        ; 1423: FE 21
ret                            ; 1425: C9
call 1414h                     ; 1426: CD 14 14
rz                             ; 1429: C8
cpi 2Ch                        ; 142A: FE 2C
ret                            ; 142C: C9
xra a                          ; 142D: AF
sta 1231h                      ; 142E: 32 31 12
sta 1232h                      ; 1431: 32 32 12
dcr a                          ; 1434: 3D
sta 120Ch                      ; 1435: 32 0C 12
lxi h, 0000h                   ; 1438: 21 00 00
shld 3049h                     ; 143B: 22 49 30
call 1426h                     ; 143E: CD 26 14
jnz 1471h                      ; 1441: C2 71 14
lxi h, 1231h                   ; 1444: 21 31 12
mov a, m                       ; 1447: 7E
ora a                          ; 1448: B7
jz 145Ch                       ; 1449: CA 5C 14
dcr m                          ; 144C: 35
mov e, a                       ; 144D: 5F
dcr e                          ; 144E: 1D
mvi d, 00h                     ; 144F: 16 00
lxi h, 120Dh                   ; 1451: 21 0D 12
dad d                          ; 1454: 19
mov a, m                       ; 1455: 7E
call 1294h                     ; 1456: CD 94 12
jmp 1444h                      ; 1459: C3 44 14
lda 1232h                      ; 145C: 3A 32 12
cpi 02h                        ; 145F: FE 02
cnz 15E4h                      ; 1461: C4 E4 15
lda 2F8Ch                      ; 1464: 3A 8C 2F
cpi 20h                        ; 1467: FE 20
rnz                            ; 1469: C0
lhld 1221h                     ; 146A: 2A 21 12
shld 3049h                     ; 146D: 22 49 30
ret                            ; 1470: C9
lda 2F8Ch                      ; 1471: 3A 8C 2F
cpi 20h                        ; 1474: FE 20
jnz 15D0h                      ; 1476: C2 D0 15
lda 3005h                      ; 1479: 3A 05 30
cpi 03h                        ; 147C: FE 03
jnz 149Dh                      ; 147E: C2 9D 14
lda 3008h                      ; 1481: 3A 08 30
ora a                          ; 1484: B7
cz 15E4h                       ; 1485: CC E4 15
cpi 03h                        ; 1488: FE 03
cnc 15E4h                      ; 148A: D4 E4 15
mvi d, 00h                     ; 148D: 16 00
lxi h, 3009h                   ; 148F: 21 09 30
mov e, m                       ; 1492: 5E
inx h                          ; 1493: 23
dcr a                          ; 1494: 3D
jz 1499h                       ; 1495: CA 99 14
mov d, m                       ; 1498: 56
xchg                           ; 1499: EB
jmp 15CDh                      ; 149A: C3 CD 15
cpi 02h                        ; 149D: FE 02
jnz 14A8h                      ; 149F: C2 A8 14
lhld 3006h                     ; 14A2: 2A 06 30
jmp 15CDh                      ; 14A5: C3 CD 15
call 2106h                     ; 14A8: CD 06 21
jnz 158Dh                      ; 14AB: C2 8D 15
cpi 19h                        ; 14AE: FE 19
jnc 1582h                      ; 14B0: D2 82 15
cpi 18h                        ; 14B3: FE 18
jnz 14F1h                      ; 14B5: C2 F1 14
call 160Ch                     ; 14B8: CD 0C 16
call 1414h                     ; 14BB: CD 14 14
jz 14E8h                       ; 14BE: CA E8 14
lda 3005h                      ; 14C1: 3A 05 30
cpi 03h                        ; 14C4: FE 03
jnz 14D9h                      ; 14C6: C2 D9 14
lda 3008h                      ; 14C9: 3A 08 30
ora a                          ; 14CC: B7
jnz 14D9h                      ; 14CD: C2 D9 14
call 1606h                     ; 14D0: CD 06 16
call 1426h                     ; 14D3: CD 26 14
jz 14E8h                       ; 14D6: CA E8 14
call 160Ch                     ; 14D9: CD 0C 16
call 1414h                     ; 14DC: CD 14 14
jnz 14D9h                      ; 14DF: C2 D9 14
lxi h, 0000h                   ; 14E2: 21 00 00
jmp 14EBh                      ; 14E5: C3 EB 14
lxi h, 0FFFFh                  ; 14E8: 21 FF FF
call 15D6h                     ; 14EB: CD D6 15
jmp 143Eh                      ; 14EE: C3 3E 14
cpi 14h                        ; 14F1: FE 14
mov c, a                       ; 14F3: 4F
lda 120Ch                      ; 14F4: 3A 0C 12
jnz 1507h                      ; 14F7: C2 07 15
ora a                          ; 14FA: B7
cz 15E4h                       ; 14FB: CC E4 15
mvi a, 0FFh                    ; 14FE: 3E FF
sta 120Ch                      ; 1500: 32 0C 12
mov a, c                       ; 1503: 79
jmp 1555h                      ; 1504: C3 55 15
ora a                          ; 1507: B7
jnz 1560h                      ; 1508: C2 60 15
push b                         ; 150B: C5
lda 1231h                      ; 150C: 3A 31 12
ora a                          ; 150F: B7
jz 1530h                       ; 1510: CA 30 15
mov e, a                       ; 1513: 5F
dcr e                          ; 1514: 1D
mvi d, 00h                     ; 1515: 16 00
lxi h, 1217h                   ; 1517: 21 17 12
dad d                          ; 151A: 19
mov a, m                       ; 151B: 7E
cmp b                          ; 151C: B8
jc 1530h                       ; 151D: DA 30 15
lxi h, 1231h                   ; 1520: 21 31 12
mov m, e                       ; 1523: 73
lxi h, 120Dh                   ; 1524: 21 0D 12
dad d                          ; 1527: 19
mov a, m                       ; 1528: 7E
call 1294h                     ; 1529: CD 94 12
pop b                          ; 152C: C1
jmp 150Bh                      ; 152D: C3 0B 15
pop b                          ; 1530: C1
mov a, c                       ; 1531: 79
cpi 15h                        ; 1532: FE 15
jnz 1555h                      ; 1534: C2 55 15
lxi h, 1231h                   ; 1537: 21 31 12
mov a, m                       ; 153A: 7E
ora a                          ; 153B: B7
jz 154Eh                       ; 153C: CA 4E 15
dcr a                          ; 153F: 3D
mov m, a                       ; 1540: 77
mov e, a                       ; 1541: 5F
mvi d, 00h                     ; 1542: 16 00
lxi h, 120Dh                   ; 1544: 21 0D 12
dad d                          ; 1547: 19
mov a, m                       ; 1548: 7E
cpi 14h                        ; 1549: FE 14
jz 1551h                       ; 154B: CA 51 15
call 15E4h                     ; 154E: CD E4 15
xra a                          ; 1551: AF
jmp 155Ah                      ; 1552: C3 5A 15
call 1250h                     ; 1555: CD 50 12
mvi a, 0FFh                    ; 1558: 3E FF
sta 120Ch                      ; 155A: 32 0C 12
jmp 15D0h                      ; 155D: C3 D0 15
mov a, c                       ; 1560: 79
cpi 05h                        ; 1561: FE 05
jz 15D0h                       ; 1563: CA D0 15
cpi 06h                        ; 1566: FE 06
jnz 1570h                      ; 1568: C2 70 15
inr a                          ; 156B: 3C
mov c, a                       ; 156C: 4F
jmp 150Bh                      ; 156D: C3 0B 15
cpi 0Eh                        ; 1570: FE 0E
jz 150Bh                       ; 1572: CA 0B 15
cpi 12h                        ; 1575: FE 12
jz 150Bh                       ; 1577: CA 0B 15
cpi 13h                        ; 157A: FE 13
cnz 15E4h                      ; 157C: C4 E4 15
jmp 150Bh                      ; 157F: C3 0B 15
cpi 1Ah                        ; 1582: FE 1A
cz 15E4h                       ; 1584: CC E4 15
mov l, b                       ; 1587: 68
mvi h, 00h                     ; 1588: 26 00
jmp 15CDh                      ; 158A: C3 CD 15
lda 3005h                      ; 158D: 3A 05 30
cpi 04h                        ; 1590: FE 04
jnz 15ACh                      ; 1592: C2 AC 15
lda 3009h                      ; 1595: 3A 09 30
cpi 24h                        ; 1598: FE 24
jz 15A6h                       ; 159A: CA A6 15
call 15E4h                     ; 159D: CD E4 15
lxi h, 0000h                   ; 15A0: 21 00 00
jmp 15CDh                      ; 15A3: C3 CD 15
lhld 3052h                     ; 15A6: 2A 52 30
jmp 15CDh                      ; 15A9: C3 CD 15
call 1C06h                     ; 15AC: CD 06 1C
call 1C09h                     ; 15AF: CD 09 1C
jnz 15C0h                      ; 15B2: C2 C0 15
mvi a, 50h                     ; 15B5: 3E 50
call 2598h                     ; 15B7: CD 98 25
call 1C0Ch                     ; 15BA: CD 0C 1C
jmp 15CAh                      ; 15BD: C3 CA 15
call 1C12h                     ; 15C0: CD 12 1C
ani 07h                        ; 15C3: E6 07
mvi a, 55h                     ; 15C5: 3E 55
cz 2598h                       ; 15C7: CC 98 25
call 1C18h                     ; 15CA: CD 18 1C
call 15D6h                     ; 15CD: CD D6 15
call 1606h                     ; 15D0: CD 06 16
jmp 143Eh                      ; 15D3: C3 3E 14
lda 120Ch                      ; 15D6: 3A 0C 12
ora a                          ; 15D9: B7
cz 15E4h                       ; 15DA: CC E4 15
xra a                          ; 15DD: AF
sta 120Ch                      ; 15DE: 32 0C 12
jmp 1233h                      ; 15E1: C3 33 12
push h                         ; 15E4: E5
mvi a, 45h                     ; 15E5: 3E 45
call 2598h                     ; 15E7: CD 98 25
pop h                          ; 15EA: E1
ret                            ; 15EB: C9
db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h ; 15EC: data
db 00h, 00h, 00h, 00h, 00h, 00h, 0C3h, 00h, 1Ch ; 15FA: data
jmp 17F1h                      ; 1603: C3 F1 17
jmp 18B3h                      ; 1606: C3 B3 18
db 0C3h, 66h, 16h              ; 1609: data
jmp 1AFCh                      ; 160C: C3 FC 1A
db 1Ah, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 01h, 1Ah ; 160F: data
lda 2EA3h                      ; 1619: 3A A3 2E
ora a                          ; 161C: B7
jz 164Fh                       ; 161D: CA 4F 16
lhld 2EF4h                     ; 1620: 2A F4 2E
mov a, m                       ; 1623: 7E
ora a                          ; 1624: B7
jnz 1648h                      ; 1625: C2 48 16
lda 2EA4h                      ; 1628: 3A A4 2E
cpi 02h                        ; 162B: FE 02
jz 163Dh                       ; 162D: CA 3D 16
lxi h, 1618h                   ; 1630: 21 18 16
inr m                          ; 1633: 34
mvi a, 00h                     ; 1634: 3E 00
rnz                            ; 1636: C0
call 1BE7h                     ; 1637: CD E7 1B
call 2595h                     ; 163A: CD 95 25
call 1C30h                     ; 163D: CD 30 1C
lda 2F14h                      ; 1640: 3A 14 2F
ora a                          ; 1643: B7
rnz                            ; 1644: C0
jmp 1619h                      ; 1645: C3 19 16
inx h                          ; 1648: 23
shld 2EF4h                     ; 1649: 22 F4 2E
jmp 20D8h                      ; 164C: C3 D8 20
call 2586h                     ; 164F: CD 86 25
sta 1618h                      ; 1652: 32 18 16
mov b, a                       ; 1655: 47
lda 3005h                      ; 1656: 3A 05 30
cpi 03h                        ; 1659: FE 03
mov a, b                       ; 165B: 78
rz                             ; 165C: C8
cpi 61h                        ; 165D: FE 61
rc                             ; 165F: D8
cpi 7Bh                        ; 1660: FE 7B
rnc                            ; 1662: D0
ani 5Fh                        ; 1663: E6 5F
ret                            ; 1665: C9
push psw                       ; 1666: F5
cpi 0Dh                        ; 1667: FE 0D
jz 1687h                       ; 1669: CA 87 16
cpi 0Ah                        ; 166C: FE 0A
jz 1687h                       ; 166E: CA 87 16
lda 3004h                      ; 1671: 3A 04 30
cpi 78h                        ; 1674: FE 78
jnc 1687h                      ; 1676: D2 87 16
mov e, a                       ; 1679: 5F
mvi d, 00h                     ; 167A: 16 00
inr a                          ; 167C: 3C
sta 3004h                      ; 167D: 32 04 30
lxi h, 2F8Ch                   ; 1680: 21 8C 2F
dad d                          ; 1683: 19
pop psw                        ; 1684: F1
mov m, a                       ; 1685: 77
ret                            ; 1686: C9
pop psw                        ; 1687: F1
ret                            ; 1688: C9
lda 2F65h                      ; 1689: 3A 65 2F
call 1853h                     ; 168C: CD 53 18
rnz                            ; 168F: C0
lda 2F65h                      ; 1690: 3A 65 2F
call 1839h                     ; 1693: CD 39 18
ret                            ; 1696: C9
xra a                          ; 1697: AF
sta 2F66h                      ; 1698: 32 66 2F
sta 2F64h                      ; 169B: 32 64 2F
call 1619h                     ; 169E: CD 19 16
sta 2F65h                      ; 16A1: 32 65 2F
lda 3005h                      ; 16A4: 3A 05 30
cpi 06h                        ; 16A7: FE 06
rz                             ; 16A9: C8
lda 2F65h                      ; 16AA: 3A 65 2F
cpi 80h                        ; 16AD: FE 80
jc 16C6h                       ; 16AF: DA C6 16
call 210Ch                     ; 16B2: CD 0C 21
sta 2F66h                      ; 16B5: 32 66 2F
lxi d, 2F67h                   ; 16B8: 11 67 2F
mov a, m                       ; 16BB: 7E
stax d                         ; 16BC: 12
inx h                          ; 16BD: 23
inx d                          ; 16BE: 13
dcr b                          ; 16BF: 05
jnz 16BBh                      ; 16C0: C2 BB 16
jmp 16E5h                      ; 16C3: C3 E5 16
call 1853h                     ; 16C6: CD 53 18
rz                             ; 16C9: C8
call 1689h                     ; 16CA: CD 89 16
jz 16F0h                       ; 16CD: CA F0 16
lxi h, 2F66h                   ; 16D0: 21 66 2F
mov a, m                       ; 16D3: 7E
cpi 0Fh                        ; 16D4: FE 0F
jnc 16EEh                      ; 16D6: D2 EE 16
inr m                          ; 16D9: 34
lxi h, 2F67h                   ; 16DA: 21 67 2F
mov e, a                       ; 16DD: 5F
mvi d, 00h                     ; 16DE: 16 00
dad d                          ; 16E0: 19
lda 2F65h                      ; 16E1: 3A 65 2F
mov m, a                       ; 16E4: 77
call 1619h                     ; 16E5: CD 19 16
sta 2F65h                      ; 16E8: 32 65 2F
jmp 16CAh                      ; 16EB: C3 CA 16
xra a                          ; 16EE: AF
ret                            ; 16EF: C9
xra a                          ; 16F0: AF
inr a                          ; 16F1: 3C
ret                            ; 16F2: C9
lhld 3056h                     ; 16F3: 2A 56 30
shld 1613h                     ; 16F6: 22 13 16
call 1C33h                     ; 16F9: CD 33 1C
call 1C36h                     ; 16FC: CD 36 1C
rnz                            ; 16FF: C0
lhld 1613h                     ; 1700: 2A 13 16
shld 3056h                     ; 1703: 22 56 30
ret                            ; 1706: C9
xra a                          ; 1707: AF
sta 1617h                      ; 1708: 32 17 16
lxi h, 1617h                   ; 170B: 21 17 16
inr m                          ; 170E: 34
jnz 171Dh                      ; 170F: C2 1D 17
call 1BDBh                     ; 1712: CD DB 1B
lxi h, 2F66h                   ; 1715: 21 66 2F
mvi m, 00h                     ; 1718: 36 00
shld 2EF4h                     ; 171A: 22 F4 2E
lxi h, 2F66h                   ; 171D: 21 66 2F
mov a, m                       ; 1720: 7E
ora a                          ; 1721: B7
jz 1735h                       ; 1722: CA 35 17
dcr m                          ; 1725: 35
lxi h, 2F64h                   ; 1726: 21 64 2F
mov e, m                       ; 1729: 5E
inr m                          ; 172A: 34
mvi d, 00h                     ; 172B: 16 00
lxi h, 2F67h                   ; 172D: 21 67 2F
dad d                          ; 1730: 19
mov a, m                       ; 1731: 7E
jmp 1666h                      ; 1732: C3 66 16
lda 2EA3h                      ; 1735: 3A A3 2E
ora a                          ; 1738: B7
lda 2F65h                      ; 1739: 3A 65 2F
jnz 174Ah                      ; 173C: C2 4A 17
mov b, a                       ; 173F: 47
ora a                          ; 1740: B7
jnz 1777h                      ; 1741: C2 77 17
call 1619h                     ; 1744: CD 19 16
jmp 1666h                      ; 1747: C3 66 16
ora a                          ; 174A: B7
jz 177Fh                       ; 174B: CA 7F 17
cpi 5Eh                        ; 174E: FE 5E
jnz 176Ch                      ; 1750: C2 6C 17
call 1697h                     ; 1753: CD 97 16
mvi b, 5Eh                     ; 1756: 06 5E
jnz 177Bh                      ; 1758: C2 7B 17
lda 2F65h                      ; 175B: 3A 65 2F
cpi 26h                        ; 175E: FE 26
jnz 177Bh                      ; 1760: C2 7B 17
lxi h, 2F66h                   ; 1763: 21 66 2F
inr m                          ; 1766: 34
inx h                          ; 1767: 23
mov m, a                       ; 1768: 77
jmp 1777h                      ; 1769: C3 77 17
cpi 26h                        ; 176C: FE 26
jz 179Eh                       ; 176E: CA 9E 17
mov b, a                       ; 1771: 47
cpi 7Fh                        ; 1772: FE 7F
jz 17B1h                       ; 1774: CA B1 17
xra a                          ; 1777: AF
sta 2F65h                      ; 1778: 32 65 2F
mov a, b                       ; 177B: 78
jmp 1666h                      ; 177C: C3 66 16
call 1697h                     ; 177F: CD 97 16
jz 170Bh                       ; 1782: CA 0B 17
lda 2F65h                      ; 1785: 3A 65 2F
cpi 26h                        ; 1788: FE 26
jz 1795h                       ; 178A: CA 95 17
lda 3005h                      ; 178D: 3A 05 30
cpi 03h                        ; 1790: FE 03
jz 170Bh                       ; 1792: CA 0B 17
call 16F3h                     ; 1795: CD F3 16
jz 170Bh                       ; 1798: CA 0B 17
jmp 17BDh                      ; 179B: C3 BD 17
call 1697h                     ; 179E: CD 97 16
mvi b, 26h                     ; 17A1: 06 26
jz 177Bh                       ; 17A3: CA 7B 17
call 16F3h                     ; 17A6: CD F3 16
mvi b, 26h                     ; 17A9: 06 26
jz 177Bh                       ; 17AB: CA 7B 17
jmp 17BDh                      ; 17AE: C3 BD 17
call 1697h                     ; 17B1: CD 97 16
jz 170Bh                       ; 17B4: CA 0B 17
call 16F3h                     ; 17B7: CD F3 16
jz 170Bh                       ; 17BA: CA 0B 17
lxi h, 2F65h                   ; 17BD: 21 65 2F
mov a, m                       ; 17C0: 7E
cpi 26h                        ; 17C1: FE 26
jnz 17C8h                      ; 17C3: C2 C8 17
mvi a, 7Fh                     ; 17C6: 3E 7F
mvi m, 00h                     ; 17C8: 36 00
sta 2F14h                      ; 17CA: 32 14 2F
call 1C2Dh                     ; 17CD: CD 2D 1C
lxi h, 2EA4h                   ; 17D0: 21 A4 2E
mvi m, 02h                     ; 17D3: 36 02
lhld 304Dh                     ; 17D5: 2A 4D 30
shld 2F24h                     ; 17D8: 22 24 2F
call 1C42h                     ; 17DB: CD 42 1C
shld 2EF4h                     ; 17DE: 22 F4 2E
xra a                          ; 17E1: AF
sta 2F66h                      ; 17E2: 32 66 2F
lhld 1613h                     ; 17E5: 2A 13 16
shld 3056h                     ; 17E8: 22 56 30
call 1697h                     ; 17EB: CD 97 16
jmp 170Bh                      ; 17EE: C3 0B 17
call 180Eh                     ; 17F1: CD 0E 18
sta 2F66h                      ; 17F4: 32 66 2F
sta 2F65h                      ; 17F7: 32 65 2F
sta 305Bh                      ; 17FA: 32 5B 30
sta 3004h                      ; 17FD: 32 04 30
mvi a, 0Ah                     ; 1800: 3E 0A
sta 160Fh                      ; 1802: 32 0F 16
call 2595h                     ; 1805: CD 95 25
mvi a, 10h                     ; 1808: 3E 10
sta 3004h                      ; 180A: 32 04 30
ret                            ; 180D: C9
xra a                          ; 180E: AF
sta 3008h                      ; 180F: 32 08 30
sta 1610h                      ; 1812: 32 10 16
ret                            ; 1815: C9
lxi h, 3008h                   ; 1816: 21 08 30
mov a, m                       ; 1819: 7E
cpi 40h                        ; 181A: FE 40
jc 1824h                       ; 181C: DA 24 18
mvi m, 00h                     ; 181F: 36 00
call 1BDBh                     ; 1821: CD DB 1B
mov e, m                       ; 1824: 5E
mvi d, 00h                     ; 1825: 16 00
inr m                          ; 1827: 34
inx h                          ; 1828: 23
dad d                          ; 1829: 19
lda 305Bh                      ; 182A: 3A 5B 30
mov m, a                       ; 182D: 77
ret                            ; 182E: C9
mov a, m                       ; 182F: 7E
cpi 24h                        ; 1830: FE 24
rnz                            ; 1832: C0
xra a                          ; 1833: AF
mov m, a                       ; 1834: 77
ret                            ; 1835: C9
lda 305Bh                      ; 1836: 3A 5B 30
sui 30h                        ; 1839: D6 30
cpi 0Ah                        ; 183B: FE 0A
ral                            ; 183D: 17
ani 01h                        ; 183E: E6 01
ret                            ; 1840: C9
call 1836h                     ; 1841: CD 36 18
rnz                            ; 1844: C0
lda 305Bh                      ; 1845: 3A 5B 30
sui 41h                        ; 1848: D6 41
cpi 06h                        ; 184A: FE 06
ral                            ; 184C: 17
ani 01h                        ; 184D: E6 01
ret                            ; 184F: C9
lda 305Bh                      ; 1850: 3A 5B 30
cpi 3Fh                        ; 1853: FE 3F
jz 1865h                       ; 1855: CA 65 18
cpi 40h                        ; 1858: FE 40
jz 1865h                       ; 185A: CA 65 18
sui 41h                        ; 185D: D6 41
cpi 1Ah                        ; 185F: FE 1A
ral                            ; 1861: 17
ani 01h                        ; 1862: E6 01
ret                            ; 1864: C9
ora a                          ; 1865: B7
ret                            ; 1866: C9
call 1850h                     ; 1867: CD 50 18
rnz                            ; 186A: C0
call 1836h                     ; 186B: CD 36 18
ret                            ; 186E: C9
cpi 20h                        ; 186F: FE 20
rnc                            ; 1871: D0
cpi 09h                        ; 1872: FE 09
rz                             ; 1874: C8
cpi 0Dh                        ; 1875: FE 0D
rz                             ; 1877: C8
cpi 0Ah                        ; 1878: FE 0A
rz                             ; 187A: C8
cpi 1Ah                        ; 187B: FE 1A
rz                             ; 187D: C8
jmp 1BE1h                      ; 187E: C3 E1 1B
call 1707h                     ; 1881: CD 07 17
call 186Fh                     ; 1884: CD 6F 18
sta 305Bh                      ; 1887: 32 5B 30
lda 305Ah                      ; 188A: 3A 5A 30
ora a                          ; 188D: B7
jz 18A6h                       ; 188E: CA A6 18
lda 305Ch                      ; 1891: 3A 5C 30
cpi 01h                        ; 1894: FE 01
jnz 18A0h                      ; 1896: C2 A0 18
lda 304Fh                      ; 1899: 3A 4F 30
ora a                          ; 189C: B7
jnz 18A6h                      ; 189D: C2 A6 18
lda 305Bh                      ; 18A0: 3A 5B 30
call 1C27h                     ; 18A3: CD 27 1C
lda 305Bh                      ; 18A6: 3A 5B 30
ret                            ; 18A9: C9
cpi 0Dh                        ; 18AA: FE 0D
rz                             ; 18AC: C8
cpi 1Ah                        ; 18AD: FE 1A
rz                             ; 18AF: C8
cpi 21h                        ; 18B0: FE 21
ret                            ; 18B2: C9
call 180Eh                     ; 18B3: CD 0E 18
xra a                          ; 18B6: AF
sta 3005h                      ; 18B7: 32 05 30
lda 305Bh                      ; 18BA: 3A 5B 30
cpi 09h                        ; 18BD: FE 09
jz 1952h                       ; 18BF: CA 52 19
cpi 3Bh                        ; 18C2: FE 3B
jnz 192Fh                      ; 18C4: C2 2F 19
mvi a, 06h                     ; 18C7: 3E 06
sta 3005h                      ; 18C9: 32 05 30
lda 305Ah                      ; 18CC: 3A 5A 30
ora a                          ; 18CF: B7
jz 193Fh                       ; 18D0: CA 3F 19
lda 305Ch                      ; 18D3: 3A 5C 30
cpi 01h                        ; 18D6: FE 01
jnz 18E2h                      ; 18D8: C2 E2 18
lda 304Fh                      ; 18DB: 3A 4F 30
ora a                          ; 18DE: B7
jnz 193Fh                      ; 18DF: C2 3F 19
call 1881h                     ; 18E2: CD 81 18
cpi 3Bh                        ; 18E5: FE 3B
jnz 1942h                      ; 18E7: C2 42 19
lhld 3060h                     ; 18EA: 2A 60 30
xchg                           ; 18ED: EB
lhld 3058h                     ; 18EE: 2A 58 30
dcx h                          ; 18F1: 2B
dcx h                          ; 18F2: 2B
mov a, e                       ; 18F3: 7B
cmp l                          ; 18F4: BD
jnz 18FDh                      ; 18F5: C2 FD 18
mov a, d                       ; 18F8: 7A
cmp h                          ; 18F9: BC
jz 1911h                       ; 18FA: CA 11 19
mov a, m                       ; 18FD: 7E
cpi 0Ah                        ; 18FE: FE 0A
jnz 1908h                      ; 1900: C2 08 19
dcx h                          ; 1903: 2B
dcx h                          ; 1904: 2B
jmp 1911h                      ; 1905: C3 11 19
cpi 21h                        ; 1908: FE 21
jnc 1911h                      ; 190A: D2 11 19
dcx h                          ; 190D: 2B
jmp 18F3h                      ; 190E: C3 F3 18
shld 3058h                     ; 1911: 22 58 30
lda 305Ah                      ; 1914: 3A 5A 30
push psw                       ; 1917: F5
xra a                          ; 1918: AF
sta 305Ah                      ; 1919: 32 5A 30
call 1881h                     ; 191C: CD 81 18
call 18AAh                     ; 191F: CD AA 18
jnz 191Ch                      ; 1922: C2 1C 19
call 1C27h                     ; 1925: CD 27 1C
pop psw                        ; 1928: F1
sta 305Ah                      ; 1929: 32 5A 30
jmp 1958h                      ; 192C: C3 58 19
lda 305Bh                      ; 192F: 3A 5B 30
cpi 2Ah                        ; 1932: FE 2A
jnz 194Bh                      ; 1934: C2 4B 19
lda 160Fh                      ; 1937: 3A 0F 16
cpi 0Ah                        ; 193A: FE 0A
jnz 194Bh                      ; 193C: C2 4B 19
call 1881h                     ; 193F: CD 81 18
call 18AAh                     ; 1942: CD AA 18
jz 1958h                       ; 1945: CA 58 19
jmp 193Fh                      ; 1948: C3 3F 19
ori 20h                        ; 194B: F6 20
cpi 20h                        ; 194D: FE 20
jnz 1958h                      ; 194F: C2 58 19
call 1881h                     ; 1952: CD 81 18
jmp 18B6h                      ; 1955: C3 B6 18
xra a                          ; 1958: AF
sta 3005h                      ; 1959: 32 05 30
call 1850h                     ; 195C: CD 50 18
jz 1967h                       ; 195F: CA 67 19
mvi a, 01h                     ; 1962: 3E 01
jmp 19A3h                      ; 1964: C3 A3 19
call 1836h                     ; 1967: CD 36 18
jz 1972h                       ; 196A: CA 72 19
mvi a, 02h                     ; 196D: 3E 02
jmp 19A3h                      ; 196F: C3 A3 19
lda 305Bh                      ; 1972: 3A 5B 30
cpi 27h                        ; 1975: FE 27
jnz 1983h                      ; 1977: C2 83 19
xra a                          ; 197A: AF
sta 305Bh                      ; 197B: 32 5B 30
mvi a, 03h                     ; 197E: 3E 03
jmp 19A3h                      ; 1980: C3 A3 19
cpi 0Ah                        ; 1983: FE 0A
jnz 19A1h                      ; 1985: C2 A1 19
lda 2EA3h                      ; 1988: 3A A3 2E
ora a                          ; 198B: B7
jz 1994h                       ; 198C: CA 94 19
mvi a, 2Bh                     ; 198F: 3E 2B
sta 2F91h                      ; 1991: 32 91 2F
call 2595h                     ; 1994: CD 95 25
lxi h, 2F8Ch                   ; 1997: 21 8C 2F
mvi m, 20h                     ; 199A: 36 20
mvi a, 10h                     ; 199C: 3E 10
sta 3004h                      ; 199E: 32 04 30
mvi a, 04h                     ; 19A1: 3E 04
sta 3005h                      ; 19A3: 32 05 30
lda 305Bh                      ; 19A6: 3A 5B 30
sta 160Fh                      ; 19A9: 32 0F 16
ora a                          ; 19AC: B7
cnz 1816h                      ; 19AD: C4 16 18
call 1881h                     ; 19B0: CD 81 18
lda 3005h                      ; 19B3: 3A 05 30
cpi 04h                        ; 19B6: FE 04
jnz 1A06h                      ; 19B8: C2 06 1A
lda 305Ah                      ; 19BB: 3A 5A 30
ora a                          ; 19BE: B7
rnz                            ; 19BF: C0
lda 3009h                      ; 19C0: 3A 09 30
cpi 3Dh                        ; 19C3: FE 3D
jnz 19CEh                      ; 19C5: C2 CE 19
lxi h, 5145h                   ; 19C8: 21 45 51
jmp 19F9h                      ; 19CB: C3 F9 19
cpi 3Ch                        ; 19CE: FE 3C
jnz 19E4h                      ; 19D0: C2 E4 19
lxi h, 544Ch                   ; 19D3: 21 4C 54
lda 305Bh                      ; 19D6: 3A 5B 30
cpi 3Dh                        ; 19D9: FE 3D
jnz 19F9h                      ; 19DB: C2 F9 19
lxi h, 454Ch                   ; 19DE: 21 4C 45
jmp 19F5h                      ; 19E1: C3 F5 19
cpi 3Eh                        ; 19E4: FE 3E
rnz                            ; 19E6: C0
lxi h, 5447h                   ; 19E7: 21 47 54
lda 305Bh                      ; 19EA: 3A 5B 30
cpi 3Dh                        ; 19ED: FE 3D
jnz 19F9h                      ; 19EF: C2 F9 19
lxi h, 4547h                   ; 19F2: 21 47 45
xra a                          ; 19F5: AF
sta 305Bh                      ; 19F6: 32 5B 30
shld 3009h                     ; 19F9: 22 09 30
lxi h, 3008h                   ; 19FC: 21 08 30
inr m                          ; 19FF: 34
mvi a, 01h                     ; 1A00: 3E 01
sta 3005h                      ; 1A02: 32 05 30
ret                            ; 1A05: C9
lxi h, 305Bh                   ; 1A06: 21 5B 30
lda 3005h                      ; 1A09: 3A 05 30
cpi 01h                        ; 1A0C: FE 01
jnz 1A1Eh                      ; 1A0E: C2 1E 1A
call 182Fh                     ; 1A11: CD 2F 18
jz 19A6h                       ; 1A14: CA A6 19
call 1867h                     ; 1A17: CD 67 18
jnz 19A6h                      ; 1A1A: C2 A6 19
ret                            ; 1A1D: C9
cpi 02h                        ; 1A1E: FE 02
jnz 1AB4h                      ; 1A20: C2 B4 1A
call 182Fh                     ; 1A23: CD 2F 18
jz 19A6h                       ; 1A26: CA A6 19
call 1841h                     ; 1A29: CD 41 18
jnz 19A6h                      ; 1A2C: C2 A6 19
lda 305Bh                      ; 1A2F: 3A 5B 30
cpi 4Fh                        ; 1A32: FE 4F
jz 1A3Ch                       ; 1A34: CA 3C 1A
cpi 51h                        ; 1A37: FE 51
jnz 1A41h                      ; 1A39: C2 41 1A
mvi a, 08h                     ; 1A3C: 3E 08
jmp 1A48h                      ; 1A3E: C3 48 1A
cpi 48h                        ; 1A41: FE 48
jnz 1A52h                      ; 1A43: C2 52 1A
mvi a, 10h                     ; 1A46: 3E 10
sta 1610h                      ; 1A48: 32 10 16
xra a                          ; 1A4B: AF
sta 305Bh                      ; 1A4C: 32 5B 30
jmp 1A6Dh                      ; 1A4F: C3 6D 1A
lda 160Fh                      ; 1A52: 3A 0F 16
cpi 42h                        ; 1A55: FE 42
jnz 1A5Fh                      ; 1A57: C2 5F 1A
mvi a, 02h                     ; 1A5A: 3E 02
jmp 1A66h                      ; 1A5C: C3 66 1A
cpi 44h                        ; 1A5F: FE 44
mvi a, 0Ah                     ; 1A61: 3E 0A
jnz 1A6Ah                      ; 1A63: C2 6A 1A
lxi h, 3008h                   ; 1A66: 21 08 30
dcr m                          ; 1A69: 35
sta 1610h                      ; 1A6A: 32 10 16
lxi h, 0000h                   ; 1A6D: 21 00 00
shld 3006h                     ; 1A70: 22 06 30
lxi h, 3008h                   ; 1A73: 21 08 30
mov c, m                       ; 1A76: 4E
inx h                          ; 1A77: 23
mov a, m                       ; 1A78: 7E
inx h                          ; 1A79: 23
cpi 41h                        ; 1A7A: FE 41
jnc 1A84h                      ; 1A7C: D2 84 1A
sui 30h                        ; 1A7F: D6 30
jmp 1A86h                      ; 1A81: C3 86 1A
sui 37h                        ; 1A84: D6 37
push h                         ; 1A86: E5
push b                         ; 1A87: C5
mov c, a                       ; 1A88: 4F
lxi h, 1610h                   ; 1A89: 21 10 16
cmp m                          ; 1A8C: BE
cnc 1BD5h                      ; 1A8D: D4 D5 1B
mvi b, 00h                     ; 1A90: 06 00
mov a, m                       ; 1A92: 7E
lhld 3006h                     ; 1A93: 2A 06 30
xchg                           ; 1A96: EB
lxi h, 0000h                   ; 1A97: 21 00 00
ora a                          ; 1A9A: B7
jz 1AA9h                       ; 1A9B: CA A9 1A
rar                            ; 1A9E: 1F
jnc 1AA3h                      ; 1A9F: D2 A3 1A
dad d                          ; 1AA2: 19
xchg                           ; 1AA3: EB
dad h                          ; 1AA4: 29
xchg                           ; 1AA5: EB
jmp 1A9Ah                      ; 1AA6: C3 9A 1A
dad b                          ; 1AA9: 09
shld 3006h                     ; 1AAA: 22 06 30
pop b                          ; 1AAD: C1
pop h                          ; 1AAE: E1
dcr c                          ; 1AAF: 0D
jnz 1A78h                      ; 1AB0: C2 78 1A
ret                            ; 1AB3: C9
lda 305Bh                      ; 1AB4: 3A 5B 30
cpi 0Dh                        ; 1AB7: FE 0D
jz 1BDBh                       ; 1AB9: CA DB 1B
cpi 27h                        ; 1ABC: FE 27
jnz 19A6h                      ; 1ABE: C2 A6 19
call 1881h                     ; 1AC1: CD 81 18
cpi 27h                        ; 1AC4: FE 27
rnz                            ; 1AC6: C0
jmp 19A6h                      ; 1AC7: C3 A6 19
lda 305Bh                      ; 1ACA: 3A 5B 30
ora a                          ; 1ACD: B7
rz                             ; 1ACE: C8
cpi 20h                        ; 1ACF: FE 20
rz                             ; 1AD1: C8
cpi 09h                        ; 1AD2: FE 09
ret                            ; 1AD4: C9
lda 305Bh                      ; 1AD5: 3A 5B 30
cpi 2Ch                        ; 1AD8: FE 2C
rz                             ; 1ADA: C8
cpi 3Bh                        ; 1ADB: FE 3B
rz                             ; 1ADD: C8
cpi 25h                        ; 1ADE: FE 25
rz                             ; 1AE0: C8
lda 305Bh                      ; 1AE1: 3A 5B 30
cpi 0Dh                        ; 1AE4: FE 0D
rz                             ; 1AE6: C8
cpi 1Ah                        ; 1AE7: FE 1A
rz                             ; 1AE9: C8
cpi 21h                        ; 1AEA: FE 21
ret                            ; 1AEC: C9
lda 305Bh                      ; 1AED: 3A 5B 30
cpi 3Bh                        ; 1AF0: FE 3B
rz                             ; 1AF2: C8
cpi 20h                        ; 1AF3: FE 20
rz                             ; 1AF5: C8
cpi 09h                        ; 1AF6: FE 09
rz                             ; 1AF8: C8
cpi 2Ch                        ; 1AF9: FE 2C
ret                            ; 1AFB: C9
call 180Eh                     ; 1AFC: CD 0E 18
xra a                          ; 1AFF: AF
sta 3005h                      ; 1B00: 32 05 30
sta 1616h                      ; 1B03: 32 16 16
call 1ACAh                     ; 1B06: CD CA 1A
jnz 1B12h                      ; 1B09: C2 12 1B
call 1881h                     ; 1B0C: CD 81 18
jmp 1B06h                      ; 1B0F: C3 06 1B
call 1AD5h                     ; 1B12: CD D5 1A
jnz 1B2Fh                      ; 1B15: C2 2F 1B
mvi a, 04h                     ; 1B18: 3E 04
sta 3005h                      ; 1B1A: 32 05 30
jmp 1BC9h                      ; 1B1D: C3 C9 1B
lda 305Bh                      ; 1B20: 3A 5B 30
sta 160Fh                      ; 1B23: 32 0F 16
call 1881h                     ; 1B26: CD 81 18
lda 3005h                      ; 1B29: 3A 05 30
cpi 04h                        ; 1B2C: FE 04
rz                             ; 1B2E: C8
call 1AE1h                     ; 1B2F: CD E1 1A
jnz 1B47h                      ; 1B32: C2 47 1B
lda 3005h                      ; 1B35: 3A 05 30
cpi 03h                        ; 1B38: FE 03
cz 1BD5h                       ; 1B3A: CC D5 1B
lda 1616h                      ; 1B3D: 3A 16 16
ora a                          ; 1B40: B7
cnz 1BD5h                      ; 1B41: C4 D5 1B
jmp 1BCFh                      ; 1B44: C3 CF 1B
lda 3005h                      ; 1B47: 3A 05 30
cpi 03h                        ; 1B4A: FE 03
jnz 1B6Ch                      ; 1B4C: C2 6C 1B
lda 305Bh                      ; 1B4F: 3A 5B 30
cpi 27h                        ; 1B52: FE 27
jnz 1BC9h                      ; 1B54: C2 C9 1B
call 1816h                     ; 1B57: CD 16 18
call 1881h                     ; 1B5A: CD 81 18
lda 305Bh                      ; 1B5D: 3A 5B 30
cpi 27h                        ; 1B60: FE 27
jz 1B20h                       ; 1B62: CA 20 1B
xra a                          ; 1B65: AF
sta 3005h                      ; 1B66: 32 05 30
jmp 1B2Fh                      ; 1B69: C3 2F 1B
lda 305Bh                      ; 1B6C: 3A 5B 30
cpi 27h                        ; 1B6F: FE 27
jnz 1B7Ch                      ; 1B71: C2 7C 1B
mvi a, 03h                     ; 1B74: 3E 03
sta 3005h                      ; 1B76: 32 05 30
jmp 1BC9h                      ; 1B79: C3 C9 1B
cpi 5Eh                        ; 1B7C: FE 5E
jnz 1B97h                      ; 1B7E: C2 97 1B
call 1881h                     ; 1B81: CD 81 18
lda 305Bh                      ; 1B84: 3A 5B 30
cpi 09h                        ; 1B87: FE 09
jz 1BC9h                       ; 1B89: CA C9 1B
cpi 20h                        ; 1B8C: FE 20
jnc 1BC9h                      ; 1B8E: D2 C9 1B
call 1BE1h                     ; 1B91: CD E1 1B
jmp 1BCFh                      ; 1B94: C3 CF 1B
cpi 3Ch                        ; 1B97: FE 3C
jnz 1BA8h                      ; 1B99: C2 A8 1B
lxi h, 1616h                   ; 1B9C: 21 16 16
mov a, m                       ; 1B9F: 7E
inr m                          ; 1BA0: 34
ora a                          ; 1BA1: B7
jz 1B20h                       ; 1BA2: CA 20 1B
jmp 1BC9h                      ; 1BA5: C3 C9 1B
cpi 3Eh                        ; 1BA8: FE 3E
jnz 1BBCh                      ; 1BAA: C2 BC 1B
lxi h, 1616h                   ; 1BAD: 21 16 16
mov a, m                       ; 1BB0: 7E
ora a                          ; 1BB1: B7
jz 1BC9h                       ; 1BB2: CA C9 1B
dcr m                          ; 1BB5: 35
jz 1B20h                       ; 1BB6: CA 20 1B
jmp 1BC9h                      ; 1BB9: C3 C9 1B
lda 1616h                      ; 1BBC: 3A 16 16
ora a                          ; 1BBF: B7
jnz 1BC9h                      ; 1BC0: C2 C9 1B
call 1AEDh                     ; 1BC3: CD ED 1A
jz 1BCFh                       ; 1BC6: CA CF 1B
call 1816h                     ; 1BC9: CD 16 18
jmp 1B20h                      ; 1BCC: C3 20 1B
mvi a, 05h                     ; 1BCF: 3E 05
sta 3005h                      ; 1BD1: 32 05 30
ret                            ; 1BD4: C9
push psw                       ; 1BD5: F5
mvi a, 56h                     ; 1BD6: 3E 56
jmp 1BEDh                      ; 1BD8: C3 ED 1B
push psw                       ; 1BDB: F5
mvi a, 4Fh                     ; 1BDC: 3E 4F
jmp 1BEDh                      ; 1BDE: C3 ED 1B
push psw                       ; 1BE1: F5
mvi a, 49h                     ; 1BE2: 3E 49
jmp 1BEDh                      ; 1BE4: C3 ED 1B
push psw                       ; 1BE7: F5
mvi a, 42h                     ; 1BE8: 3E 42
jmp 1BEDh                      ; 1BEA: C3 ED 1B
push b                         ; 1BED: C5
push h                         ; 1BEE: E5
call 2598h                     ; 1BEF: CD 98 25
pop h                          ; 1BF2: E1
pop b                          ; 1BF3: C1
pop psw                        ; 1BF4: F1
ret                            ; 1BF5: C9
db 0F1h, 0C9h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 0C3h, 00h, 21h ; 1BF6: data
jmp 1D51h                      ; 1C03: C3 51 1D
jmp 1EA9h                      ; 1C06: C3 A9 1E
jmp 1E89h                      ; 1C09: C3 89 1E
jmp 1F02h                      ; 1C0C: C3 02 1F
jmp 2012h                      ; 1C0F: C3 12 20
jmp 2024h                      ; 1C12: C3 24 20
jmp 203Fh                      ; 1C15: C3 3F 20
jmp 2048h                      ; 1C18: C3 48 20
jmp 2059h                      ; 1C1B: C3 59 20
jmp 2060h                      ; 1C1E: C3 60 20
jmp 2065h                      ; 1C21: C3 65 20
jmp 2092h                      ; 1C24: C3 92 20
jmp 20BCh                      ; 1C27: C3 BC 20
jmp 20B2h                      ; 1C2A: C3 B2 20
jmp 1D7Eh                      ; 1C2D: C3 7E 1D
jmp 1DC5h                      ; 1C30: C3 C5 1D
jmp 1E8Fh                      ; 1C33: C3 8F 1E
jmp 1E47h                      ; 1C36: C3 47 1E
jmp 1F87h                      ; 1C39: C3 87 1F
jmp 1FA5h                      ; 1C3C: C3 A5 1F
db 0C3h, 0BBh, 1Fh             ; 1C3F: data
jmp 1FF0h                      ; 1C42: C3 F0 1F
jmp 1EF8h                      ; 1C45: C3 F8 1E
jmp 1E1Ah                      ; 1C48: C3 1A 1E
jmp 1D66h                      ; 1C4B: C3 66 1D
db 44h, 3Fh, 0B2h, 3Ch, 1Eh, 3Dh, 0C7h, 3Dh, 0A1h, 3Ah, 00h, 00h, 65h ; 1C4E: data
db 3Eh, 00h, 00h, "?; >", 00h, 00h, 3Ch, 3Eh, 89h, "; <Y=2=", 03h, 3Eh ; 1C5B: data
db 0E8h, 3Bh, 0FAh, 3Ch, 00h, 00h, 00h, 00h, 0B7h, 3Dh, 00h, 00h, 52h ; 1C70: data
db 3Eh, 0DCh, 3Eh, 0E5h, 3Eh, 00h, 00h, 00h, 00h, 46h, 3Eh, 0DCh, 3Bh ; 1C7D: data
db 00h, 00h, 9Ch, 3Eh, 47h, 3Dh, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h ; 1C8A: data
db 93h, 3Dh, 0C6h, 3Ch, 15h, 3Dh, 00h, 00h, 0AEh, 3Dh, 91h, 3Eh, 0BEh ; 1C98: data
db 3Dh, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 73h, 3Bh, 0A5h, 3Dh, 50h ; 1CA5: data
db 3Dh, 00h, 00h, 00h, 00h, 00h, 00h, 2Ah, 3Eh, 04h, "=$?P:-?]>*<", 15h ; 1CB3: data
db 3Ch, 00h, 00h, 81h, 3Dh, 8Ah, 3Dh, 00h, 00h, 0E5h, 3Ch, 0FFh, 3Bh ; 1CC9: data
db 00h, 00h, 9Ch, 3Dh, 81h, 3Eh, 89h, 3Eh, 79h, 3Eh, 0Ah, 3Ch, 55h, 3Ch ; 1CD6: data
db 0FEh, ":?<k<", 0DDh, 3Ah, 0F4h, 3Bh, 00h, 00h, 00h, 00h, 00h, 00h ; 1CE4: data
db 0C6h, 3Eh, 00h, 00h, 00h, 00h, 00h, 00h, 09h, 3Bh, 0E8h, 3Ah, 0Dh ; 1CF4: data
db 3Dh, 93h, 3Bh, 0FBh, 3Eh, 05h, 3Fh, 00h, 00h, 00h, 00h, 0EEh, 3Eh ; 1D01: data
db 50h, 3Fh, 00h, 00h, 0AAh, 3Ch, 0F9h, 3Dh, 1Ah, 3Fh, 0B2h, 3Eh, 0BCh ; 1D0E: data
db 3Eh, 0EFh, 3Dh, 00h, 00h, 0DBh, 3Ch, 0D1h, 3Ch, 00h, 00h, 69h, 3Bh ; 1D1B: data
db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 6Dh, 3Dh ; 1D28: data
db 16h, 3Eh, 00h, 00h, 00h, 00h, 63h, 3Dh, 0E5h, 3Dh, 0A8h, ">o>v<" ; 1D36: data
db 0DBh, 3Dh, 0BCh, 3Ch, 0A8h, 3Bh, 38h, 3Fh, 01h, 4Eh, 1Ch ; 1D46: data
lxi h, 1C4Eh                   ; 1D51: 21 4E 1C
mvi b, 80h                     ; 1D54: 06 80
xra a                          ; 1D56: AF
mov m, a                       ; 1D57: 77
inx h                          ; 1D58: 23
mov m, a                       ; 1D59: 77
inx h                          ; 1D5A: 23
dcr b                          ; 1D5B: 05
jnz 1D57h                      ; 1D5C: C2 57 1D
lxi h, 0000h                   ; 1D5F: 21 00 00
shld 3056h                     ; 1D62: 22 56 30
ret                            ; 1D65: C9
lxi h, 2E83h                   ; 1D66: 21 83 2E
mvi b, 10h                     ; 1D69: 06 10
xra a                          ; 1D6B: AF
mov m, a                       ; 1D6C: 77
inx h                          ; 1D6D: 23
mov m, a                       ; 1D6E: 77
inx h                          ; 1D6F: 23
dcr b                          ; 1D70: 05
jnz 1D6Ch                      ; 1D71: C2 6C 1D
ret                            ; 1D74: C9
db 0CDh, 1Ah, 1Eh, 0E6h, 0Fh, 32h, 4Eh, 1Dh, 0C9h ; 1D75: data
lxi h, 2EA3h                   ; 1D7E: 21 A3 2E
mov a, m                       ; 1D81: 7E
cpi 0Fh                        ; 1D82: FE 0F
jnc 1E15h                      ; 1D84: D2 15 1E
inr m                          ; 1D87: 34
mov e, m                       ; 1D88: 5E
mvi d, 00h                     ; 1D89: 16 00
lxi h, 2EA4h                   ; 1D8B: 21 A4 2E
mov a, m                       ; 1D8E: 7E
dad d                          ; 1D8F: 19
mov m, a                       ; 1D90: 77
lxi h, 2ED4h                   ; 1D91: 21 D4 2E
call 1DBCh                     ; 1D94: CD BC 1D
lxi h, 2EB4h                   ; 1D97: 21 B4 2E
call 1DBCh                     ; 1D9A: CD BC 1D
lxi h, 2EF4h                   ; 1D9D: 21 F4 2E
call 1DBCh                     ; 1DA0: CD BC 1D
lxi h, 2F14h                   ; 1DA3: 21 14 2F
mov a, m                       ; 1DA6: 7E
dad d                          ; 1DA7: 19
mov m, a                       ; 1DA8: 77
lxi h, 2F24h                   ; 1DA9: 21 24 2F
call 1DBCh                     ; 1DAC: CD BC 1D
lxi h, 2F44h                   ; 1DAF: 21 44 2F
mov a, m                       ; 1DB2: 7E
dad d                          ; 1DB3: 19
mov m, a                       ; 1DB4: 77
lxi h, 2F54h                   ; 1DB5: 21 54 2F
mov a, m                       ; 1DB8: 7E
dad d                          ; 1DB9: 19
mov m, a                       ; 1DBA: 77
ret                            ; 1DBB: C9
mov c, m                       ; 1DBC: 4E
inx h                          ; 1DBD: 23
mov b, m                       ; 1DBE: 46
dad d                          ; 1DBF: 19
dad d                          ; 1DC0: 19
mov m, b                       ; 1DC1: 70
dcx h                          ; 1DC2: 2B
mov m, c                       ; 1DC3: 71
ret                            ; 1DC4: C9
lxi h, 2EA3h                   ; 1DC5: 21 A3 2E
mov a, m                       ; 1DC8: 7E
ora a                          ; 1DC9: B7
jz 1E15h                       ; 1DCA: CA 15 1E
push h                         ; 1DCD: E5
mov e, m                       ; 1DCE: 5E
mvi d, 00h                     ; 1DCF: 16 00
lxi h, 2EA4h                   ; 1DD1: 21 A4 2E
call 1E04h                     ; 1DD4: CD 04 1E
lxi h, 2ED4h                   ; 1DD7: 21 D4 2E
call 1E0Ah                     ; 1DDA: CD 0A 1E
lxi h, 2EB4h                   ; 1DDD: 21 B4 2E
call 1E0Ah                     ; 1DE0: CD 0A 1E
lxi h, 2EF4h                   ; 1DE3: 21 F4 2E
call 1E0Ah                     ; 1DE6: CD 0A 1E
lxi h, 2F14h                   ; 1DE9: 21 14 2F
call 1E04h                     ; 1DEC: CD 04 1E
lxi h, 2F24h                   ; 1DEF: 21 24 2F
call 1E0Ah                     ; 1DF2: CD 0A 1E
lxi h, 2F44h                   ; 1DF5: 21 44 2F
call 1E04h                     ; 1DF8: CD 04 1E
lxi h, 2F54h                   ; 1DFB: 21 54 2F
call 1E04h                     ; 1DFE: CD 04 1E
pop h                          ; 1E01: E1
dcr m                          ; 1E02: 35
ret                            ; 1E03: C9
push h                         ; 1E04: E5
dad d                          ; 1E05: 19
mov a, m                       ; 1E06: 7E
pop h                          ; 1E07: E1
mov m, a                       ; 1E08: 77
ret                            ; 1E09: C9
push h                         ; 1E0A: E5
dad d                          ; 1E0B: 19
dad d                          ; 1E0C: 19
mov c, m                       ; 1E0D: 4E
inx h                          ; 1E0E: 23
mov b, m                       ; 1E0F: 46
pop h                          ; 1E10: E1
mov m, c                       ; 1E11: 71
inx h                          ; 1E12: 23
mov m, b                       ; 1E13: 70
ret                            ; 1E14: C9
mvi a, 42h                     ; 1E15: 3E 42
jmp 2598h                      ; 1E17: C3 98 25
lxi h, 3008h                   ; 1E1A: 21 08 30
shld 20D6h                     ; 1E1D: 22 D6 20
lhld 20D6h                     ; 1E20: 2A D6 20
mov b, m                       ; 1E23: 46
xra a                          ; 1E24: AF
inx h                          ; 1E25: 23
add m                          ; 1E26: 86
dcr b                          ; 1E27: 05
jnz 1E25h                      ; 1E28: C2 25 1E
ani 7Fh                        ; 1E2B: E6 7F
sta 1D4Eh                      ; 1E2D: 32 4E 1D
ret                            ; 1E30: C9
db "G*V0##~", 0E6h, 0F0h, 0B0h, 77h, 0C9h ; 1E31: data
lhld 3056h                     ; 1E3D: 2A 56 30
inx h                          ; 1E40: 23
inx h                          ; 1E41: 23
mov a, m                       ; 1E42: 7E
ani 0Fh                        ; 1E43: E6 0F
inr a                          ; 1E45: 3C
ret                            ; 1E46: C9
call 1E89h                     ; 1E47: CD 89 1E
rz                             ; 1E4A: C8
xchg                           ; 1E4B: EB
lxi b, 0000h                   ; 1E4C: 01 00 00
lda 2EA4h                      ; 1E4F: 3A A4 2E
cpi 01h                        ; 1E52: FE 01
jz 1E74h                       ; 1E54: CA 74 1E
lxi h, 2EA3h                   ; 1E57: 21 A3 2E
mov c, m                       ; 1E5A: 4E
mvi b, 00h                     ; 1E5B: 06 00
lxi h, 2EA4h                   ; 1E5D: 21 A4 2E
dad b                          ; 1E60: 09
mov a, c                       ; 1E61: 79
ora a                          ; 1E62: B7
jz 1E71h                       ; 1E63: CA 71 1E
mov a, m                       ; 1E66: 7E
cpi 01h                        ; 1E67: FE 01
jz 1E74h                       ; 1E69: CA 74 1E
dcx b                          ; 1E6C: 0B
dcx h                          ; 1E6D: 2B
jmp 1E61h                      ; 1E6E: C3 61 1E
inr a                          ; 1E71: 3C
xchg                           ; 1E72: EB
ret                            ; 1E73: C9
lxi h, 2F24h                   ; 1E74: 21 24 2F
dad b                          ; 1E77: 09
dad b                          ; 1E78: 09
mov a, e                       ; 1E79: 7B
sub m                          ; 1E7A: 96
mov a, d                       ; 1E7B: 7A
inx h                          ; 1E7C: 23
sbb m                          ; 1E7D: 9E
jc 1E89h                       ; 1E7E: DA 89 1E
lxi h, 0000h                   ; 1E81: 21 00 00
shld 3056h                     ; 1E84: 22 56 30
xra a                          ; 1E87: AF
ret                            ; 1E88: C9
lhld 3056h                     ; 1E89: 2A 56 30
mov a, l                       ; 1E8C: 7D
ora h                          ; 1E8D: B4
ret                            ; 1E8E: C9
lxi h, 2F66h                   ; 1E8F: 21 66 2F
shld 20D6h                     ; 1E92: 22 D6 20
call 1E20h                     ; 1E95: CD 20 1E
lda 1D4Eh                      ; 1E98: 3A 4E 1D
ani 0Fh                        ; 1E9B: E6 0F
sta 1D4Eh                      ; 1E9D: 32 4E 1D
lxi h, 2E83h                   ; 1EA0: 21 83 2E
shld 1D4Fh                     ; 1EA3: 22 4F 1D
jmp 1EB8h                      ; 1EA6: C3 B8 1E
call 1E1Ah                     ; 1EA9: CD 1A 1E
lxi h, 1C4Eh                   ; 1EAC: 21 4E 1C
shld 1D4Fh                     ; 1EAF: 22 4F 1D
lxi h, 3008h                   ; 1EB2: 21 08 30
shld 20D6h                     ; 1EB5: 22 D6 20
lhld 20D6h                     ; 1EB8: 2A D6 20
mov a, m                       ; 1EBB: 7E
cpi 11h                        ; 1EBC: FE 11
jc 1EC3h                       ; 1EBE: DA C3 1E
mvi m, 10h                     ; 1EC1: 36 10
lxi h, 1D4Eh                   ; 1EC3: 21 4E 1D
mov e, m                       ; 1EC6: 5E
mvi d, 00h                     ; 1EC7: 16 00
lhld 1D4Fh                     ; 1EC9: 2A 4F 1D
dad d                          ; 1ECC: 19
dad d                          ; 1ECD: 19
mov e, m                       ; 1ECE: 5E
inx h                          ; 1ECF: 23
mov h, m                       ; 1ED0: 66
mov l, e                       ; 1ED1: 6B
shld 3056h                     ; 1ED2: 22 56 30
call 1E89h                     ; 1ED5: CD 89 1E
rz                             ; 1ED8: C8
call 1E3Dh                     ; 1ED9: CD 3D 1E
lhld 20D6h                     ; 1EDC: 2A D6 20
cmp m                          ; 1EDF: BE
jnz 1EF8h                      ; 1EE0: C2 F8 1E
mov b, a                       ; 1EE3: 47
inx h                          ; 1EE4: 23
xchg                           ; 1EE5: EB
lhld 3056h                     ; 1EE6: 2A 56 30
inx h                          ; 1EE9: 23
inx h                          ; 1EEA: 23
inx h                          ; 1EEB: 23
ldax d                         ; 1EEC: 1A
cmp m                          ; 1EED: BE
jnz 1EF8h                      ; 1EEE: C2 F8 1E
inx d                          ; 1EF1: 13
inx h                          ; 1EF2: 23
dcr b                          ; 1EF3: 05
jnz 1EECh                      ; 1EF4: C2 EC 1E
ret                            ; 1EF7: C9
lhld 3056h                     ; 1EF8: 2A 56 30
mov e, m                       ; 1EFB: 5E
inx h                          ; 1EFC: 23
mov d, m                       ; 1EFD: 56
xchg                           ; 1EFE: EB
jmp 1ED2h                      ; 1EFF: C3 D2 1E
lxi h, 3008h                   ; 1F02: 21 08 30
mov e, m                       ; 1F05: 5E
mvi d, 00h                     ; 1F06: 16 00
lhld 304Bh                     ; 1F08: 2A 4B 30
shld 3056h                     ; 1F0B: 22 56 30
dad d                          ; 1F0E: 19
lxi d, 0005h                   ; 1F0F: 11 05 00
dad d                          ; 1F12: 19
xchg                           ; 1F13: EB
lhld 304Dh                     ; 1F14: 2A 4D 30
mov a, e                       ; 1F17: 7B
sub l                          ; 1F18: 95
mov a, d                       ; 1F19: 7A
sbb h                          ; 1F1A: 9C
xchg                           ; 1F1B: EB
jnc 1FF3h                      ; 1F1C: D2 F3 1F
shld 304Bh                     ; 1F1F: 22 4B 30
lxi h, 1C4Eh                   ; 1F22: 21 4E 1C
shld 1D4Fh                     ; 1F25: 22 4F 1D
call 1F31h                     ; 1F28: CD 31 1F
xra a                          ; 1F2B: AF
inx h                          ; 1F2C: 23
mov m, a                       ; 1F2D: 77
inx h                          ; 1F2E: 23
mov m, a                       ; 1F2F: 77
ret                            ; 1F30: C9
lhld 3056h                     ; 1F31: 2A 56 30
xchg                           ; 1F34: EB
lxi h, 1D4Eh                   ; 1F35: 21 4E 1D
mov c, m                       ; 1F38: 4E
mvi b, 00h                     ; 1F39: 06 00
lhld 1D4Fh                     ; 1F3B: 2A 4F 1D
dad b                          ; 1F3E: 09
dad b                          ; 1F3F: 09
mov c, m                       ; 1F40: 4E
inx h                          ; 1F41: 23
mov b, m                       ; 1F42: 46
mov m, d                       ; 1F43: 72
dcx h                          ; 1F44: 2B
mov m, e                       ; 1F45: 73
xchg                           ; 1F46: EB
mov m, c                       ; 1F47: 71
inx h                          ; 1F48: 23
mov m, b                       ; 1F49: 70
lxi d, 3008h                   ; 1F4A: 11 08 30
ldax d                         ; 1F4D: 1A
cpi 11h                        ; 1F4E: FE 11
jc 1F55h                       ; 1F50: DA 55 1F
mvi a, 10h                     ; 1F53: 3E 10
mov b, a                       ; 1F55: 47
dcr a                          ; 1F56: 3D
inx h                          ; 1F57: 23
mov m, a                       ; 1F58: 77
inx h                          ; 1F59: 23
inx d                          ; 1F5A: 13
ldax d                         ; 1F5B: 1A
mov m, a                       ; 1F5C: 77
dcr b                          ; 1F5D: 05
jnz 1F59h                      ; 1F5E: C2 59 1F
ret                            ; 1F61: C9
lhld 304Dh                     ; 1F62: 2A 4D 30
xchg                           ; 1F65: EB
lxi h, 3008h                   ; 1F66: 21 08 30
mov l, m                       ; 1F69: 6E
mvi h, 00h                     ; 1F6A: 26 00
dad b                          ; 1F6C: 09
mov a, e                       ; 1F6D: 7B
sub l                          ; 1F6E: 95
mov l, a                       ; 1F6F: 6F
mov a, d                       ; 1F70: 7A
sbb h                          ; 1F71: 9C
mov h, a                       ; 1F72: 67
shld 3056h                     ; 1F73: 22 56 30
xchg                           ; 1F76: EB
lxi h, 304Bh                   ; 1F77: 21 4B 30
mov a, e                       ; 1F7A: 7B
sub m                          ; 1F7B: 96
inx h                          ; 1F7C: 23
mov a, d                       ; 1F7D: 7A
sbb m                          ; 1F7E: 9E
jc 1FF3h                       ; 1F7F: DA F3 1F
xchg                           ; 1F82: EB
shld 304Dh                     ; 1F83: 22 4D 30
ret                            ; 1F86: C9
lxi b, 0001h                   ; 1F87: 01 01 00
call 1F62h                     ; 1F8A: CD 62 1F
lhld 304Dh                     ; 1F8D: 2A 4D 30
xchg                           ; 1F90: EB
lxi h, 3008h                   ; 1F91: 21 08 30
mov c, m                       ; 1F94: 4E
inx h                          ; 1F95: 23
mov a, c                       ; 1F96: 79
ora a                          ; 1F97: B7
jz 1FA2h                       ; 1F98: CA A2 1F
dcr c                          ; 1F9B: 0D
mov a, m                       ; 1F9C: 7E
stax d                         ; 1F9D: 12
inx d                          ; 1F9E: 13
jmp 1F95h                      ; 1F9F: C3 95 1F
xra a                          ; 1FA2: AF
stax d                         ; 1FA3: 12
ret                            ; 1FA4: C9
lxi b, 0003h                   ; 1FA5: 01 03 00
call 1F62h                     ; 1FA8: CD 62 1F
lxi h, 2E83h                   ; 1FAB: 21 83 2E
shld 1D4Fh                     ; 1FAE: 22 4F 1D
call 1F31h                     ; 1FB1: CD 31 1F
lda 1D4Eh                      ; 1FB4: 3A 4E 1D
call 2012h                     ; 1FB7: CD 12 20
ret                            ; 1FBA: C9
db 2Ah, 4Dh, 30h, 0EBh, "!$/{", 96h, 23h, 7Ah, 9Eh, 0D0h, 0EBh, 22h, 56h ; 1FBB: data
db 30h, 0CDh, 24h, 20h, 5Fh, 16h, 00h, 21h, 83h, 2Eh, 19h, 19h, 0EBh ; 1FCB: data
db "*V0~", 12h, 23h, 7Eh, 13h, 12h, 0CDh, 31h, 20h, 7Eh, 0B7h, 23h, 0C2h ; 1FD8: data
db 0E4h, 1Fh, 22h, 4Dh, 30h, 0C3h, 0BBh, 1Fh ; 1FE8: data
jmp 2031h                      ; 1FF0: C3 31 20
lxi h, 1FFCh                   ; 1FF3: 21 FC 1F
call 2592h                     ; 1FF6: CD 92 25
jmp 259Eh                      ; 1FF9: C3 9E 25
db "SYMBOL TABLE OVERFLOW", 0Dh ; 1FFC: data
ral                            ; 2012: 17
ral                            ; 2013: 17
ral                            ; 2014: 17
ral                            ; 2015: 17
ani 0F0h                       ; 2016: E6 F0
mov b, a                       ; 2018: 47
lhld 3056h                     ; 2019: 2A 56 30
inx h                          ; 201C: 23
inx h                          ; 201D: 23
mov a, m                       ; 201E: 7E
ani 0Fh                        ; 201F: E6 0F
ora b                          ; 2021: B0
mov m, a                       ; 2022: 77
ret                            ; 2023: C9
lhld 3056h                     ; 2024: 2A 56 30
inx h                          ; 2027: 23
inx h                          ; 2028: 23
mov a, m                       ; 2029: 7E
rar                            ; 202A: 1F
rar                            ; 202B: 1F
rar                            ; 202C: 1F
rar                            ; 202D: 1F
ani 0Fh                        ; 202E: E6 0F
ret                            ; 2030: C9
call 1E3Dh                     ; 2031: CD 3D 1E
lhld 3056h                     ; 2034: 2A 56 30
mov e, a                       ; 2037: 5F
mvi d, 00h                     ; 2038: 16 00
dad d                          ; 203A: 19
inx h                          ; 203B: 23
inx h                          ; 203C: 23
inx h                          ; 203D: 23
ret                            ; 203E: C9
push h                         ; 203F: E5
call 2031h                     ; 2040: CD 31 20
pop d                          ; 2043: D1
mov m, e                       ; 2044: 73
inx h                          ; 2045: 23
mov m, d                       ; 2046: 72
ret                            ; 2047: C9
call 2031h                     ; 2048: CD 31 20
mov e, m                       ; 204B: 5E
inx h                          ; 204C: 23
mov d, m                       ; 204D: 56
xchg                           ; 204E: EB
ret                            ; 204F: C9
call 2031h                     ; 2050: CD 31 20
inx h                          ; 2053: 23
inx h                          ; 2054: 23
shld 3058h                     ; 2055: 22 58 30
ret                            ; 2058: C9
push psw                       ; 2059: F5
call 2050h                     ; 205A: CD 50 20
pop psw                        ; 205D: F1
mov m, a                       ; 205E: 77
ret                            ; 205F: C9
call 2050h                     ; 2060: CD 50 20
mov a, m                       ; 2063: 7E
ret                            ; 2064: C9
call 1E1Ah                     ; 2065: CD 1A 1E
ani 0Fh                        ; 2068: E6 0F
add a                          ; 206A: 87
add a                          ; 206B: 87
add a                          ; 206C: 87
add a                          ; 206D: 87
mov c, a                       ; 206E: 4F
lxi h, 3008h                   ; 206F: 21 08 30
mov a, m                       ; 2072: 7E
cpi 11h                        ; 2073: FE 11
jc 207Ah                       ; 2075: DA 7A 20
mvi m, 10h                     ; 2078: 36 10
mov a, m                       ; 207A: 7E
dcr a                          ; 207B: 3D
ora c                          ; 207C: B1
call 20BCh                     ; 207D: CD BC 20
lxi h, 3008h                   ; 2080: 21 08 30
mov c, m                       ; 2083: 4E
inx h                          ; 2084: 23
mov a, m                       ; 2085: 7E
push b                         ; 2086: C5
push h                         ; 2087: E5
call 20BCh                     ; 2088: CD BC 20
pop h                          ; 208B: E1
pop b                          ; 208C: C1
dcr c                          ; 208D: 0D
jnz 2084h                      ; 208E: C2 84 20
ret                            ; 2091: C9
call 20B2h                     ; 2092: CD B2 20
mov c, a                       ; 2095: 4F
rlc                            ; 2096: 07
rlc                            ; 2097: 07
rlc                            ; 2098: 07
rlc                            ; 2099: 07
ani 0Fh                        ; 209A: E6 0F
sta 1D4Eh                      ; 209C: 32 4E 1D
mov a, c                       ; 209F: 79
ani 0Fh                        ; 20A0: E6 0F
inr a                          ; 20A2: 3C
mov c, a                       ; 20A3: 4F
lxi d, 3008h                   ; 20A4: 11 08 30
stax d                         ; 20A7: 12
call 20B2h                     ; 20A8: CD B2 20
inx d                          ; 20AB: 13
stax d                         ; 20AC: 12
dcr c                          ; 20AD: 0D
jnz 20A8h                      ; 20AE: C2 A8 20
ret                            ; 20B1: C9
lhld 3058h                     ; 20B2: 2A 58 30
inx h                          ; 20B5: 23
shld 3058h                     ; 20B6: 22 58 30
mov a, m                       ; 20B9: 7E
ret                            ; 20BA: C9
db 0C9h                        ; 20BB: data
mov c, a                       ; 20BC: 4F
lhld 3058h                     ; 20BD: 2A 58 30
inx h                          ; 20C0: 23
xchg                           ; 20C1: EB
lhld 304Dh                     ; 20C2: 2A 4D 30
mov a, e                       ; 20C5: 7B
sub l                          ; 20C6: 95
mov a, d                       ; 20C7: 7A
sbb h                          ; 20C8: 9C
jnc 1FF3h                      ; 20C9: D2 F3 1F
xchg                           ; 20CC: EB
shld 3058h                     ; 20CD: 22 58 30
mov m, c                       ; 20D0: 71
inx h                          ; 20D1: 23
shld 304Bh                     ; 20D2: 22 4B 30
ret                            ; 20D5: C9
db 0EBh, 22h                   ; 20D6: data
cpi 2Ch                        ; 20D8: FE 2C
jnz 1652h                      ; 20DA: C2 52 16
shld 11E2h                     ; 20DD: 22 E2 11
jmp 1652h                      ; 20E0: C3 52 16
db 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 0DAh ; 20E3: data
db 96h, 07h, 0Dh, 0CAh, 96h, 07h, 0C3h, 80h, 07h, 00h, 00h, 00h, 00h ; 20F1: data
db 00h, 00h, 0C3h, 00h, 26h, 0C3h, 80h, 23h ; 20FE: data
jmp 240Dh                      ; 2106: C3 0D 24
jmp 2462h                      ; 2109: C3 62 24
jmp 247Bh                      ; 210C: C3 7B 24
db 0FCh, "*!:!X!", 0FDh, 21h, 4Dh, 22h, 70h, 22h, 82h, 22h, 88h, 22h ; 210F: data
db 0A8h, 22h, 0C6h, 22h, "4#\#j#", 0Dh ; 2120: data
db "()*+,-/ABCDEHLMDBDIDSDWEIEQGEGTIFINLELTNEORSPACIADCADDADIANAANDANICMACMCCMPCPIDAADADDCRDCXENDEQUHLTINRINXIRPJMPLDALOWLXIMODMOVMVINOPNOTNULORAORGORIOUTPOPPSWRALRARRETRLCRRCRSTSBBSBISETSHLSHRSTASTCSUBSUIXORXRAXRIASEGCALLCSEGDSEGELSEENDMHIGHIRPCLDAXLHLDNAMEPAGEPCHLPUSHREPTSHLDSPHLSTAXXCHGXTHLENDIFEXITMEXTRNLOCALMACROSTKLNTITLEINPAGEMACLIBPUBLIC" ; 212B: data
db 10h, 0Fh, 37h, 14h, 07h, 03h, 17h, 0Ah, 14h, 14h, 15h, 1Eh, 00h, 50h ; 2282: data
db 05h, 46h, 16h, 0Ah, 06h, 46h, 01h, 50h, 19h, 07h, 19h, 00h, 19h, 01h ; 2290: data
db 19h, 02h, 19h, 03h, 19h, 04h, 19h, 05h, 19h, 06h, 1Ah, 01h, 1Ch, 0F3h ; 229E: data
db 1Ah, 02h, 1Ah, 03h, 1Ch, 0FBh, 08h, 41h, 0Ch, 41h, 0Bh, 41h, 1Ah, 08h ; 22AC: data
db 2Ah, 0DBh, 0Ah, 41h, 09h, 41h, 0Dh, 41h, 10h, 28h, 19h, 06h, 23h ; 22BA: data
db 0CEh, 26h, 88h, 26h, 80h, 23h, 0C6h, 26h, 0A0h, 0Fh, 32h, 23h, 0E6h ; 22C7: data
db 1Ch, 2Fh, 1Ch, 3Fh, 26h, 0B8h, 23h, 0FEh, 1Ch, 27h, 1Eh, 09h, 27h ; 22D4: data
db 05h, 28h, 0Bh, 1Ah, 04h, 1Ah, 07h, 1Ch, 76h, 27h, 04h, 28h, 03h, 1Ah ; 22E1: data
db 0Eh, 20h, 0C3h, 25h, 3Ah, 13h, 1Eh, 1Dh, 01h, 02h, 50h, 21h, 40h, 22h ; 22EF: data
db 06h, 1Ch, 00h, 0Eh, 3Ch, 18h, 00h, 26h, 0B0h, 1Ah, 0Ah, 23h, 0F6h ; 22FD: data
db 2Ah, 0D3h, 1Fh, 0C1h, 19h, 06h, 1Ch, 17h, 1Ch, 1Fh, 1Ch, 0C9h, 1Ch ; 230A: data
db 07h, 1Ch, 0Fh, 29h, 0C7h, 26h, 98h, 23h, 0DEh, 1Ah, 0Bh, 03h, 50h ; 2317: data
db 04h, 50h, 25h, 32h, 1Ch, 37h, 26h, 90h, 23h, 0D6h, 11h, 28h, 26h ; 2324: data
db 0A8h, 23h, 0EEh, 1Ah, 0Dh, 20h, 0CDh, 1Ah, 12h, 1Ah, 13h, 1Ah, 0Dh ; 2331: data
db 1Ah, 06h, 12h, 1Eh, 1Ah, 0Fh, 24h, 0Ah, 25h, 2Ah, 1Ah, 14h, 1Ah, 15h ; 233E: data
db 1Ch, 0E9h, 1Fh, 0C5h, 1Ah, 10h, 25h, 22h, 1Ch, 0F9h, 24h, 02h, 1Ch ; 234C: data
db 0EBh, 1Ch, 0E3h, 1Ah, 05h, 1Ah, 16h, 1Ah, 17h, 1Ah, 18h, 1Ah, 09h ; 2359: data
db 1Ah, 1Ch, 1Ah, 0Ch, 1Ah, 19h, 1Ah, 1Ah, 1Ah, 1Bh, "NZZ NCC POPEP M " ; 2366: data
mvi e, 0FFh                    ; 2380: 1E FF
inr b                          ; 2382: 04
mvi c, 00h                     ; 2383: 0E 00
xra a                          ; 2385: AF
mov a, b                       ; 2386: 78
add c                          ; 2387: 81
rar                            ; 2388: 1F
cmp e                          ; 2389: BB
jz 23C1h                       ; 238A: CA C1 23
mov e, a                       ; 238D: 5F
push h                         ; 238E: E5
push d                         ; 238F: D5
push b                         ; 2390: C5
push h                         ; 2391: E5
mov b, d                       ; 2392: 42
mov c, b                       ; 2393: 48
mvi d, 00h                     ; 2394: 16 00
lxi h, 0000h                   ; 2396: 21 00 00
dad d                          ; 2399: 19
dcr b                          ; 239A: 05
jnz 2399h                      ; 239B: C2 99 23
pop d                          ; 239E: D1
dad d                          ; 239F: 19
lxi d, 3009h                   ; 23A0: 11 09 30
ldax d                         ; 23A3: 1A
cmp m                          ; 23A4: BE
inx d                          ; 23A5: 13
inx h                          ; 23A6: 23
jnz 23B3h                      ; 23A7: C2 B3 23
dcr c                          ; 23AA: 0D
jnz 23A3h                      ; 23AB: C2 A3 23
pop b                          ; 23AE: C1
pop d                          ; 23AF: D1
pop h                          ; 23B0: E1
mov a, e                       ; 23B1: 7B
ret                            ; 23B2: C9
pop b                          ; 23B3: C1
pop d                          ; 23B4: D1
pop h                          ; 23B5: E1
jc 23BDh                       ; 23B6: DA BD 23
mov c, e                       ; 23B9: 4B
jmp 2385h                      ; 23BA: C3 85 23
mov b, e                       ; 23BD: 43
jmp 2385h                      ; 23BE: C3 85 23
xra a                          ; 23C1: AF
inr a                          ; 23C2: 3C
ret                            ; 23C3: C9
lda 3009h                      ; 23C4: 3A 09 30
lxi b, 0C220h                  ; 23C7: 01 20 C2
cpi 4Ah                        ; 23CA: FE 4A
rz                             ; 23CC: C8
mvi b, 0C4h                    ; 23CD: 06 C4
cpi 43h                        ; 23CF: FE 43
rz                             ; 23D1: C8
lxi b, 0C01Ch                  ; 23D2: 01 1C C0
cpi 52h                        ; 23D5: FE 52
ret                            ; 23D7: C9
lda 3008h                      ; 23D8: 3A 08 30
cpi 04h                        ; 23DB: FE 04
jnc 240Ah                      ; 23DD: D2 0A 24
cpi 03h                        ; 23E0: FE 03
jz 23EFh                       ; 23E2: CA EF 23
cpi 02h                        ; 23E5: FE 02
jnz 240Ah                      ; 23E7: C2 0A 24
lxi h, 300Bh                   ; 23EA: 21 0B 30
mvi m, 20h                     ; 23ED: 36 20
lxi b, 0008h                   ; 23EF: 01 08 00
lxi d, 2370h                   ; 23F2: 11 70 23
lxi h, 300Ah                   ; 23F5: 21 0A 30
ldax d                         ; 23F8: 1A
cmp m                          ; 23F9: BE
inx d                          ; 23FA: 13
jnz 2402h                      ; 23FB: C2 02 24
ldax d                         ; 23FE: 1A
inx h                          ; 23FF: 23
cmp m                          ; 2400: BE
rz                             ; 2401: C8
inx d                          ; 2402: 13
inr b                          ; 2403: 04
dcr c                          ; 2404: 0D
jnz 23F5h                      ; 2405: C2 F5 23
inr c                          ; 2408: 0C
ret                            ; 2409: C9
xra a                          ; 240A: AF
inr a                          ; 240B: 3C
ret                            ; 240C: C9
lda 3008h                      ; 240D: 3A 08 30
mov c, a                       ; 2410: 4F
dcr a                          ; 2411: 3D
mov e, a                       ; 2412: 5F
mvi d, 00h                     ; 2413: 16 00
push d                         ; 2415: D5
cpi 06h                        ; 2416: FE 06
jnc 245Eh                      ; 2418: D2 5E 24
lxi h, 2282h                   ; 241B: 21 82 22
dad d                          ; 241E: 19
mov b, m                       ; 241F: 46
lxi h, 2110h                   ; 2420: 21 10 21
dad d                          ; 2423: 19
dad d                          ; 2424: 19
mov d, m                       ; 2425: 56
inx h                          ; 2426: 23
mov h, m                       ; 2427: 66
mov l, d                       ; 2428: 6A
mov d, c                       ; 2429: 51
call 2380h                     ; 242A: CD 80 23
jnz 2447h                      ; 242D: C2 47 24
sta 210Fh                      ; 2430: 32 0F 21
pop d                          ; 2433: D1
lxi h, 211Eh                   ; 2434: 21 1E 21
dad d                          ; 2437: 19
dad d                          ; 2438: 19
mov e, m                       ; 2439: 5E
inx h                          ; 243A: 23
mov d, m                       ; 243B: 56
mov l, a                       ; 243C: 6F
mvi h, 00h                     ; 243D: 26 00
dad h                          ; 243F: 29
dad d                          ; 2440: 19
xra a                          ; 2441: AF
mov c, a                       ; 2442: 4F
mov a, m                       ; 2443: 7E
inx h                          ; 2444: 23
mov b, m                       ; 2445: 46
ret                            ; 2446: C9
pop d                          ; 2447: D1
call 23C4h                     ; 2448: CD C4 23
rnz                            ; 244B: C0
push b                         ; 244C: C5
call 23D8h                     ; 244D: CD D8 23
mov a, b                       ; 2450: 78
pop b                          ; 2451: C1
rnz                            ; 2452: C0
ora a                          ; 2453: B7
ral                            ; 2454: 17
ral                            ; 2455: 17
ral                            ; 2456: 17
ora b                          ; 2457: B0
mov b, a                       ; 2458: 47
mov a, c                       ; 2459: 79
cmp a                          ; 245A: BF
mvi c, 01h                     ; 245B: 0E 01
ret                            ; 245D: C9
pop d                          ; 245E: D1
xra a                          ; 245F: AF
inr a                          ; 2460: 3C
ret                            ; 2461: C9
lxi h, 3008h                   ; 2462: 21 08 30
mov c, m                       ; 2465: 4E
dcr c                          ; 2466: 0D
lxi h, 2283h                   ; 2467: 21 83 22
xra a                          ; 246A: AF
dcr c                          ; 246B: 0D
jz 2474h                       ; 246C: CA 74 24
add m                          ; 246F: 86
inx h                          ; 2470: 23
jmp 246Bh                      ; 2471: C3 6B 24
lxi h, 210Fh                   ; 2474: 21 0F 21
add m                          ; 2477: 86
ori 80h                        ; 2478: F6 80
ret                            ; 247A: C9
ani 7Fh                        ; 247B: E6 7F
lxi h, 2499h                   ; 247D: 21 99 24
mov e, a                       ; 2480: 5F
mvi d, 00h                     ; 2481: 16 00
dad d                          ; 2483: 19
dad d                          ; 2484: 19
mov e, m                       ; 2485: 5E
inx h                          ; 2486: 23
mov a, m                       ; 2487: 7E
rar                            ; 2488: 1F
rar                            ; 2489: 1F
rar                            ; 248A: 1F
rar                            ; 248B: 1F
ani 0Fh                        ; 248C: E6 0F
mov b, a                       ; 248E: 47
mov a, m                       ; 248F: 7E
ani 0Fh                        ; 2490: E6 0F
mov d, a                       ; 2492: 57
lxi h, 213Ah                   ; 2493: 21 3A 21
dad d                          ; 2496: 19
mov a, b                       ; 2497: 78
ret                            ; 2498: C9
db 00h, 20h, 02h, 20h, 04h, 20h, 06h, 20h, 08h, 20h, 0Ah, 20h, 0Ch, 20h ; 2499: data
db 0Eh, 20h, 10h, 20h, 12h, 20h, 14h, 20h, 16h, 20h, 18h, 20h, 1Ah, 20h ; 24A7: data
db 1Ch, 20h, 1Eh, "0!0$0", 27h ; 24B5: data
db "0*0-000306090<0?0B0E0H0K0N0Q0T0W0Z0]0`0c0f0i0l0o0r0u0x0{0~0", 81h ; 24BE: data
db 30h, 84h, 30h, 87h, 30h, 8Ah, 30h, 8Dh, 30h, 90h, 30h, 93h, 30h, 96h ; 24FA: data
db 30h, 99h, 30h, 9Ch, 30h, 9Fh, 30h, 0A2h, 30h, 0A5h, 30h, 0A8h, 30h ; 2508: data
db 0ABh, 30h, 0AEh, 30h, 0B1h, 30h, 0B4h, 30h, 0B7h, 30h, 0BAh, 30h ; 2515: data
db 0BDh, 30h, 0C0h, 30h, 0C3h, 40h, 0C7h, 40h, 0CBh, 40h, 0CFh, 40h ; 2521: data
db 0D3h, 40h, 0D7h, 40h, 0DBh, 40h, 0DFh, 40h, 0E3h, 40h, 0E7h, 40h ; 252D: data
db 0EBh, 40h, 0EFh, 40h, 0F3h, 40h, 0F7h, 40h, 0FBh, 40h, 0FFh, 40h, 03h ; 2539: data
db 41h, 07h, 41h, 0Bh, 41h, 0Fh, 41h, 13h, 51h, 18h, 51h, 1Dh, 51h, 22h ; 2546: data
db 51h, 27h, "Q,Q1Q6a<aBaX0", 22h, 0BDh, 11h, 0CDh, 06h, 16h, 0C3h, 57h ; 2554: data
db 07h, "z{|}", 0AFh, "O~#F", 0C9h, 79h, 0Eh, 01h, 0BFh, 0C9h, "SEAR " ; 256B: data
jmp 26F6h                      ; 2580: C3 F6 26
jmp 2905h                      ; 2583: C3 05 29
jmp 294Ch                      ; 2586: C3 4C 29
db 0C3h, 0Eh, 2Ah, 0C3h, 95h, 2Ah, 0C3h, 0C9h, 2Ah ; 2589: data
jmp 2678h                      ; 2592: C3 78 26
jmp 2B74h                      ; 2595: C3 74 2B
jmp 2C21h                      ; 2598: C3 21 2C
db 0C3h, 0A3h, 2Dh             ; 259B: data
jmp 2C8Bh                      ; 259E: C3 8B 2C
jmp 2C49h                      ; 25A1: C3 49 2C
db 0C3h, 0A1h, 26h, 0C3h, 0E1h, 26h, 0C3h, 4Dh, 2Bh, 0C3h, 0F9h, 2Ah ; 25A4: data
db 80h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 19h ; 25B0: data
db 00h, 00h, 00h, 37h, 0Eh, 1Ah, 0C3h, 05h, 00h, 11h, 80h, 00h, 0C3h ; 25BE: data
db 0C2h, "%!I%", 0BEh, 0C8h, 77h, 5Fh, 0Eh, 0Eh, 0CDh, 05h, 00h, 0C9h ; 25CB: data
db ":JASM", 00h, 50h, 45h, 00h, 0C3h, 0CDh, "%:L%", 0C3h, 0CDh, "%:M%" ; 25DA: data
db 0C3h, 0CDh, 25h, 3Ah, 00h, 00h, 04h, 00h, 00h, 04h, 0CDh, "I*~#" ; 25EF: data
db 0FEh, 0Dh, 0FFh, 50h, 52h, 4Eh, 00h, "FTYPE", 00h, 0FFh, 0FFh, 0FFh ; 25FE: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 73h, 00h, 00h ; 260E: data
db 00h, 0FFh, 0FFh, 0FFh, 00h, 00h, 00h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 261A: data
db 48h, 45h, 58h, 00h, 0FFh, 0FFh, "&FTYPE", 00h, 0FFh, 0FFh, 0FFh, 0FFh ; 2626: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 00h, 00h, 00h, 0FFh, 0FFh ; 2637: data
mvi c, 1Ah                     ; 2642: 0E 1A
jmp 0005h                      ; 2644: C3 05 00
lxi d, 0080h                   ; 2647: 11 80 00
jmp 2642h                      ; 264A: C3 42 26
lxi h, 25C9h                   ; 264D: 21 C9 25
cmp m                          ; 2650: BE
rz                             ; 2651: C8
mov m, a                       ; 2652: 77
mov e, a                       ; 2653: 5F
mvi c, 0Eh                     ; 2654: 0E 0E
call 0005h                     ; 2656: CD 05 00
ret                            ; 2659: C9
lda 25CAh                      ; 265A: 3A CA 25
jmp 264Dh                      ; 265D: C3 4D 26
lda 25CBh                      ; 2660: 3A CB 25
jmp 264Dh                      ; 2663: C3 4D 26
db 3Ah, 0CCh, 25h, 0C3h, 4Dh, 26h ; 2666: data
lda 25CDh                      ; 266C: 3A CD 25
jmp 264Dh                      ; 266F: C3 4D 26
lda 25CEh                      ; 2672: 3A CE 25
jmp 264Dh                      ; 2675: C3 4D 26
mov a, m                       ; 2678: 7E
call 2AC9h                     ; 2679: CD C9 2A
mov a, m                       ; 267C: 7E
inx h                          ; 267D: 23
cpi 0Dh                        ; 267E: FE 0D
jnz 2678h                      ; 2680: C2 78 26
mvi a, 0Ah                     ; 2683: 3E 0A
call 2AC9h                     ; 2685: CD C9 2A
ret                            ; 2688: C9
lxi d, 005Ch                   ; 2689: 11 5C 00
mvi b, 09h                     ; 268C: 06 09
ldax d                         ; 268E: 1A
cpi 3Fh                        ; 268F: FE 3F
jz 293Dh                       ; 2691: CA 3D 29
cpi 24h                        ; 2694: FE 24
jz 293Dh                       ; 2696: CA 3D 29
mov m, a                       ; 2699: 77
inx h                          ; 269A: 23
inx d                          ; 269B: 13
dcr b                          ; 269C: 05
jnz 268Eh                      ; 269D: C2 8E 26
ret                            ; 26A0: C9
db 21h, 5Ch, 00h, 36h, 00h, 11h, 08h, 30h, 1Ah, 0FEh, 09h, 0DAh, 0B1h ; 26A1: data
db 26h, 3Eh, 08h, 47h, 4Fh, 13h, 23h, 1Ah, 77h, 0Dh, 0C2h, 0B3h, 26h ; 26AE: data
db 3Eh, 08h, 90h, 4Fh, 0Ch, 23h, 0Dh, 0CAh, 0CAh, 26h, 36h, 20h, 0C3h ; 26BB: data
db 0C0h, "&6L#6I#6B#", 0AFh, 77h, 32h, 7Ch, 00h, 0CDh, 72h, 26h, 11h ; 26C8: data
db 5Ch, 00h, 0C3h, 75h, 27h, 3Eh, 0FFh, "2]0!", 80h, 00h, 22h, 0D0h ; 26DC: data
db "%![0~2", 0CFh, 25h, 0AFh, 77h, 0C9h ; 26EB: data
call 2647h                     ; 26F6: CD 47 26
lxi h, 2CD3h                   ; 26F9: 21 D3 2C
call 2678h                     ; 26FC: CD 78 26
mvi a, 38h                     ; 26FF: 3E 38
sta 25B3h                      ; 2701: 32 B3 25
xra a                          ; 2704: AF
sta 25B4h                      ; 2705: 32 B4 25
lxi h, 0000h                   ; 2708: 21 00 00
shld 3062h                     ; 270B: 22 62 30
lhld 0006h                     ; 270E: 2A 06 00
shld 304Dh                     ; 2711: 22 4D 30
lxi h, 3100h                   ; 2714: 21 00 31
shld 25F6h                     ; 2717: 22 F6 25
lxi d, 0400h                   ; 271A: 11 00 04
dad d                          ; 271D: 19
shld 261Bh                     ; 271E: 22 1B 26
lxi d, 0300h                   ; 2721: 11 00 03
dad d                          ; 2724: 19
shld 2640h                     ; 2725: 22 40 26
lxi d, 0300h                   ; 2728: 11 00 03
dad d                          ; 272B: 19
inx h                          ; 272C: 23
shld 304Bh                     ; 272D: 22 4B 30
shld 3054h                     ; 2730: 22 54 30
jmp 27EAh                      ; 2733: C3 EA 27
cpi 20h                        ; 2736: FE 20
rz                             ; 2738: C8
push b                         ; 2739: C5
push h                         ; 273A: E5
mov e, a                       ; 273B: 5F
mvi c, 02h                     ; 273C: 0E 02
call 0005h                     ; 273E: CD 05 00
pop h                          ; 2741: E1
pop b                          ; 2742: C1
ret                            ; 2743: C9
inx h                          ; 2744: 23
mov a, m                       ; 2745: 7E
call 2736h                     ; 2746: CD 36 27
dcr c                          ; 2749: 0D
jnz 2744h                      ; 274A: C2 44 27
ret                            ; 274D: C9
push h                         ; 274E: E5
xchg                           ; 274F: EB
lda 25C9h                      ; 2750: 3A C9 25
adi 41h                        ; 2753: C6 41
call 2736h                     ; 2755: CD 36 27
mvi a, 3Ah                     ; 2758: 3E 3A
call 2736h                     ; 275A: CD 36 27
mvi c, 08h                     ; 275D: 0E 08
call 2744h                     ; 275F: CD 44 27
mvi a, 2Eh                     ; 2762: 3E 2E
call 2736h                     ; 2764: CD 36 27
mvi c, 03h                     ; 2767: 0E 03
call 2744h                     ; 2769: CD 44 27
mvi a, 2Dh                     ; 276C: 3E 2D
call 2736h                     ; 276E: CD 36 27
pop h                          ; 2771: E1
jmp 2678h                      ; 2772: C3 78 26
mvi c, 0Fh                     ; 2775: 0E 0F
push d                         ; 2777: D5
call 0005h                     ; 2778: CD 05 00
cpi 0FFh                       ; 277B: FE FF
pop d                          ; 277D: D1
rnz                            ; 277E: C0
lxi h, 2CE8h                   ; 277F: 21 E8 2C
call 274Eh                     ; 2782: CD 4E 27
jmp 0000h                      ; 2785: C3 00 00
mvi c, 10h                     ; 2788: 0E 10
push d                         ; 278A: D5
call 0005h                     ; 278B: CD 05 00
cpi 0FFh                       ; 278E: FE FF
pop d                          ; 2790: D1
rnz                            ; 2791: C0
lxi h, 2D6Bh                   ; 2792: 21 6B 2D
call 2678h                     ; 2795: CD 78 26
jmp 0000h                      ; 2798: C3 00 00
mvi c, 13h                     ; 279B: 0E 13
jmp 0005h                      ; 279D: C3 05 00
mvi c, 16h                     ; 27A0: 0E 16
push d                         ; 27A2: D5
call 0005h                     ; 27A3: CD 05 00
cpi 0FFh                       ; 27A6: FE FF
pop d                          ; 27A8: D1
rnz                            ; 27A9: C0
lxi h, 2CFFh                   ; 27AA: 21 FF 2C
call 274Eh                     ; 27AD: CD 4E 27
jmp 0000h                      ; 27B0: C3 00 00
lda 25CBh                      ; 27B3: 3A CB 25
cpi 19h                        ; 27B6: FE 19
rz                             ; 27B8: C8
cpi 17h                        ; 27B9: FE 17
rz                             ; 27BB: C8
cpi 0Fh                        ; 27BC: FE 0F
ret                            ; 27BE: C9
cpi 09h                        ; 27BF: FE 09
jnz 27D2h                      ; 27C1: C2 D2 27
mvi a, 20h                     ; 27C4: 3E 20
call 27D2h                     ; 27C6: CD D2 27
lda 25D2h                      ; 27C9: 3A D2 25
ani 07h                        ; 27CC: E6 07
jnz 27C4h                      ; 27CE: C2 C4 27
ret                            ; 27D1: C9
push psw                       ; 27D2: F5
mov e, a                       ; 27D3: 5F
mvi c, 05h                     ; 27D4: 0E 05
call 0005h                     ; 27D6: CD 05 00
pop psw                        ; 27D9: F1
lxi h, 25D2h                   ; 27DA: 21 D2 25
cpi 0Ah                        ; 27DD: FE 0A
jnz 27E5h                      ; 27DF: C2 E5 27
mvi m, 00h                     ; 27E2: 36 00
ret                            ; 27E4: C9
cpi 20h                        ; 27E5: FE 20
rc                             ; 27E7: D8
inr m                          ; 27E8: 34
ret                            ; 27E9: C9
xra a                          ; 27EA: AF
sta 25D2h                      ; 27EB: 32 D2 25
sta 305Dh                      ; 27EE: 32 5D 30
sta 3065h                      ; 27F1: 32 65 30
sta 3064h                      ; 27F4: 32 64 30
sta 3067h                      ; 27F7: 32 67 30
sta 25B5h                      ; 27FA: 32 B5 25
lda 005Ch                      ; 27FD: 3A 5C 00
cpi 20h                        ; 2800: FE 20
jz 293Dh                       ; 2802: CA 3D 29
mvi c, 19h                     ; 2805: 0E 19
call 0005h                     ; 2807: CD 05 00
lxi h, 25C9h                   ; 280A: 21 C9 25
mov m, a                       ; 280D: 77
inx h                          ; 280E: 23
mov m, a                       ; 280F: 77
inx h                          ; 2810: 23
mov m, a                       ; 2811: 77
inx h                          ; 2812: 23
mov m, a                       ; 2813: 77
inx h                          ; 2814: 23
mov m, a                       ; 2815: 77
inx h                          ; 2816: 23
mov m, a                       ; 2817: 77
inx h                          ; 2818: 23
mvi a, 01h                     ; 2819: 3E 01
sta 305Eh                      ; 281B: 32 5E 30
sta 305Fh                      ; 281E: 32 5F 30
lda 006Dh                      ; 2821: 3A 6D 00
cpi 24h                        ; 2824: FE 24
jnz 28C8h                      ; 2826: C2 C8 28
lxi h, 0081h                   ; 2829: 21 81 00
mov a, m                       ; 282C: 7E
inx h                          ; 282D: 23
cpi 24h                        ; 282E: FE 24
jnz 282Ch                      ; 2830: C2 2C 28
mov a, m                       ; 2833: 7E
ora a                          ; 2834: B7
jz 28C8h                       ; 2835: CA C8 28
inx h                          ; 2838: 23
cpi 20h                        ; 2839: FE 20
jz 2833h                       ; 283B: CA 33 28
lxi d, 25CAh                   ; 283E: 11 CA 25
cpi 41h                        ; 2841: FE 41
jz 28A9h                       ; 2843: CA A9 28
inx d                          ; 2846: 13
cpi 50h                        ; 2847: FE 50
jz 28A9h                       ; 2849: CA A9 28
inx d                          ; 284C: 13
cpi 53h                        ; 284D: FE 53
jz 28A9h                       ; 284F: CA A9 28
inx d                          ; 2852: 13
cpi 48h                        ; 2853: FE 48
jz 28A9h                       ; 2855: CA A9 28
inx d                          ; 2858: 13
cpi 4Ch                        ; 2859: FE 4C
jz 28A9h                       ; 285B: CA A9 28
inx d                          ; 285E: 13
mvi b, 07h                     ; 285F: 06 07
cpi 2Ah                        ; 2861: FE 2A
jz 2874h                       ; 2863: CA 74 28
mvi b, 03h                     ; 2866: 06 03
cpi 2Bh                        ; 2868: FE 2B
jz 2874h                       ; 286A: CA 74 28
mvi b, 00h                     ; 286D: 06 00
cpi 2Dh                        ; 286F: FE 2D
jnz 28B6h                      ; 2871: C2 B6 28
lxi d, 305Eh                   ; 2874: 11 5E 30
mov a, m                       ; 2877: 7E
cpi 53h                        ; 2878: FE 53
jz 28A3h                       ; 287A: CA A3 28
inx d                          ; 287D: 13
cpi 4Dh                        ; 287E: FE 4D
jz 28A3h                       ; 2880: CA A3 28
lxi d, 3065h                   ; 2883: 11 65 30
cpi 4Ch                        ; 2886: FE 4C
jz 28A3h                       ; 2888: CA A3 28
lxi d, 3064h                   ; 288B: 11 64 30
cpi 51h                        ; 288E: FE 51
jz 28A3h                       ; 2890: CA A3 28
lxi d, 3067h                   ; 2893: 11 67 30
cpi 52h                        ; 2896: FE 52
jz 28A3h                       ; 2898: CA A3 28
lxi d, 25B5h                   ; 289B: 11 B5 25
cpi 31h                        ; 289E: FE 31
jnz 28B6h                      ; 28A0: C2 B6 28
mov a, b                       ; 28A3: 78
stax d                         ; 28A4: 12
inx h                          ; 28A5: 23
jmp 2833h                      ; 28A6: C3 33 28
mov a, m                       ; 28A9: 7E
sui 41h                        ; 28AA: D6 41
cpi 1Ah                        ; 28AC: FE 1A
jnc 28B6h                      ; 28AE: D2 B6 28
stax d                         ; 28B1: 12
inx h                          ; 28B2: 23
jmp 2833h                      ; 28B3: C3 33 28
inx h                          ; 28B6: 23
mvi m, 0Dh                     ; 28B7: 36 0D
lxi h, 2D29h                   ; 28B9: 21 29 2D
call 2678h                     ; 28BC: CD 78 26
lxi h, 0081h                   ; 28BF: 21 81 00
call 2678h                     ; 28C2: CD 78 26
jmp 0000h                      ; 28C5: C3 00 00
lxi h, 25D3h                   ; 28C8: 21 D3 25
call 2689h                     ; 28CB: CD 89 26
lxi h, 25F8h                   ; 28CE: 21 F8 25
push h                         ; 28D1: E5
call 2689h                     ; 28D2: CD 89 26
pop h                          ; 28D5: E1
call 27B3h                     ; 28D6: CD B3 27
jz 28E9h                       ; 28D9: CA E9 28
push h                         ; 28DC: E5
push h                         ; 28DD: E5
call 2660h                     ; 28DE: CD 60 26
pop d                          ; 28E1: D1
call 279Bh                     ; 28E2: CD 9B 27
pop d                          ; 28E5: D1
call 27A0h                     ; 28E6: CD A0 27
lda 25CDh                      ; 28E9: 3A CD 25
cpi 19h                        ; 28EC: FE 19
jz 2904h                       ; 28EE: CA 04 29
lxi h, 261Dh                   ; 28F1: 21 1D 26
push h                         ; 28F4: E5
push h                         ; 28F5: E5
call 2689h                     ; 28F6: CD 89 26
call 266Ch                     ; 28F9: CD 6C 26
pop d                          ; 28FC: D1
call 279Bh                     ; 28FD: CD 9B 27
pop d                          ; 2900: D1
call 27A0h                     ; 2901: CD A0 27
ret                            ; 2904: C9
lxi h, 25B0h                   ; 2905: 21 B0 25
mvi m, 30h                     ; 2908: 36 30
inx h                          ; 290A: 23
mvi m, 30h                     ; 290B: 36 30
inx h                          ; 290D: 23
mvi m, 30h                     ; 290E: 36 30
inx h                          ; 2910: 23
mvi a, 0FFh                    ; 2911: 3E FF
sta 3066h                      ; 2913: 32 66 30
lxi h, 0000h                   ; 2916: 21 00 00
shld 2619h                     ; 2919: 22 19 26
lda 304Fh                      ; 291C: 3A 4F 30
ora a                          ; 291F: B7
cnz 2AF9h                      ; 2920: C4 F9 2A
lxi h, 0400h                   ; 2923: 21 00 04
shld 25F4h                     ; 2926: 22 F4 25
xra a                          ; 2929: AF
sta 25DFh                      ; 292A: 32 DF 25
sta 25F3h                      ; 292D: 32 F3 25
sta 25B8h                      ; 2930: 32 B8 25
call 265Ah                     ; 2933: CD 5A 26
lxi d, 25D3h                   ; 2936: 11 D3 25
call 2775h                     ; 2939: CD 75 27
ret                            ; 293C: C9
lxi h, 2D12h                   ; 293D: 21 12 2D
call 2678h                     ; 2940: CD 78 26
jmp 0000h                      ; 2943: C3 00 00
mov a, d                       ; 2946: 7A
cmp h                          ; 2947: BC
rnz                            ; 2948: C0
mov a, e                       ; 2949: 7B
cmp l                          ; 294A: BD
ret                            ; 294B: C9
push b                         ; 294C: C5
push d                         ; 294D: D5
push h                         ; 294E: E5
lda 305Dh                      ; 294F: 3A 5D 30
ora a                          ; 2952: B7
jz 29A2h                       ; 2953: CA A2 29
lhld 25D0h                     ; 2956: 2A D0 25
lxi d, 0080h                   ; 2959: 11 80 00
call 2946h                     ; 295C: CD 46 29
jnz 2977h                      ; 295F: C2 77 29
lxi h, 0000h                   ; 2962: 21 00 00
shld 25D0h                     ; 2965: 22 D0 25
call 2672h                     ; 2968: CD 72 26
mvi c, 14h                     ; 296B: 0E 14
lxi d, 005Ch                   ; 296D: 11 5C 00
call 0005h                     ; 2970: CD 05 00
ora a                          ; 2973: B7
jnz 2989h                      ; 2974: C2 89 29
lhld 25D0h                     ; 2977: 2A D0 25
inx h                          ; 297A: 23
shld 25D0h                     ; 297B: 22 D0 25
dcx h                          ; 297E: 2B
lxi d, 0080h                   ; 297F: 11 80 00
dad d                          ; 2982: 19
mov a, m                       ; 2983: 7E
cpi 1Ah                        ; 2984: FE 1A
jnz 29FFh                      ; 2986: C2 FF 29
lda 2EA3h                      ; 2989: 3A A3 2E
ora a                          ; 298C: B7
sta 305Dh                      ; 298D: 32 5D 30
jz 29A2h                       ; 2990: CA A2 29
call 2672h                     ; 2993: CD 72 26
lxi d, 005Ch                   ; 2996: 11 5C 00
lxi h, 2D7Eh                   ; 2999: 21 7E 2D
call 274Eh                     ; 299C: CD 4E 27
jmp 0000h                      ; 299F: C3 00 00
lhld 25F4h                     ; 29A2: 2A F4 25
lxi d, 0400h                   ; 29A5: 11 00 04
call 2946h                     ; 29A8: CD 46 29
jnz 29F0h                      ; 29AB: C2 F0 29
call 265Ah                     ; 29AE: CD 5A 26
lxi h, 0000h                   ; 29B1: 21 00 00
shld 25F4h                     ; 29B4: 22 F4 25
mvi b, 08h                     ; 29B7: 06 08
lhld 25F6h                     ; 29B9: 2A F6 25
push b                         ; 29BC: C5
push h                         ; 29BD: E5
xchg                           ; 29BE: EB
call 2642h                     ; 29BF: CD 42 26
mvi c, 14h                     ; 29C2: 0E 14
lxi d, 25D3h                   ; 29C4: 11 D3 25
call 0005h                     ; 29C7: CD 05 00
pop h                          ; 29CA: E1
lxi d, 0080h                   ; 29CB: 11 80 00
dad d                          ; 29CE: 19
pop b                          ; 29CF: C1
ora a                          ; 29D0: B7
jnz 29DBh                      ; 29D1: C2 DB 29
dcr b                          ; 29D4: 05
jnz 29BCh                      ; 29D5: C2 BC 29
jmp 29EDh                      ; 29D8: C3 ED 29
cpi 03h                        ; 29DB: FE 03
jnc 2A05h                      ; 29DD: D2 05 2A
dcr b                          ; 29E0: 05
jz 29EDh                       ; 29E1: CA ED 29
mvi c, 80h                     ; 29E4: 0E 80
mvi m, 1Ah                     ; 29E6: 36 1A
inx h                          ; 29E8: 23
dcr c                          ; 29E9: 0D
jnz 29E6h                      ; 29EA: C2 E6 29
call 2647h                     ; 29ED: CD 47 26
lhld 25F6h                     ; 29F0: 2A F6 25
xchg                           ; 29F3: EB
lhld 25F4h                     ; 29F4: 2A F4 25
push h                         ; 29F7: E5
inx h                          ; 29F8: 23
shld 25F4h                     ; 29F9: 22 F4 25
pop h                          ; 29FC: E1
dad d                          ; 29FD: 19
mov a, m                       ; 29FE: 7E
pop h                          ; 29FF: E1
pop d                          ; 2A00: D1
pop b                          ; 2A01: C1
ani 7Fh                        ; 2A02: E6 7F
ret                            ; 2A04: C9
lxi h, 2D3Ch                   ; 2A05: 21 3C 2D
call 2678h                     ; 2A08: CD 78 26
jmp 0000h                      ; 2A0B: C3 00 00
push b                         ; 2A0E: C5
mov b, a                       ; 2A0F: 47
lda 25CBh                      ; 2A10: 3A CB 25
cpi 19h                        ; 2A13: FE 19
jz 2A37h                       ; 2A15: CA 37 2A
cpi 17h                        ; 2A18: FE 17
jnz 2A24h                      ; 2A1A: C2 24 2A
mov a, b                       ; 2A1D: 78
call 2AC9h                     ; 2A1E: CD C9 2A
jmp 2A37h                      ; 2A21: C3 37 2A
push d                         ; 2A24: D5
push h                         ; 2A25: E5
cpi 0Fh                        ; 2A26: FE 0F
mov a, b                       ; 2A28: 78
jnz 2A32h                      ; 2A29: C2 32 2A
call 27BFh                     ; 2A2C: CD BF 27
jmp 2A35h                      ; 2A2F: C3 35 2A
call 2A39h                     ; 2A32: CD 39 2A
pop h                          ; 2A35: E1
pop d                          ; 2A36: D1
pop b                          ; 2A37: C1
ret                            ; 2A38: C9
lhld 2619h                     ; 2A39: 2A 19 26
xchg                           ; 2A3C: EB
lhld 261Bh                     ; 2A3D: 2A 1B 26
dad d                          ; 2A40: 19
mov m, a                       ; 2A41: 77
xchg                           ; 2A42: EB
inx h                          ; 2A43: 23
shld 2619h                     ; 2A44: 22 19 26
xchg                           ; 2A47: EB
lxi h, 0300h                   ; 2A48: 21 00 03
call 2946h                     ; 2A4B: CD 46 29
rnz                            ; 2A4E: C0
call 2660h                     ; 2A4F: CD 60 26
lxi h, 0000h                   ; 2A52: 21 00 00
shld 2619h                     ; 2A55: 22 19 26
lhld 261Bh                     ; 2A58: 2A 1B 26
lxi d, 25F8h                   ; 2A5B: 11 F8 25
mvi b, 06h                     ; 2A5E: 06 06
mov a, m                       ; 2A60: 7E
cpi 1Ah                        ; 2A61: FE 1A
jz 2A85h                       ; 2A63: CA 85 2A
push b                         ; 2A66: C5
push d                         ; 2A67: D5
push h                         ; 2A68: E5
xchg                           ; 2A69: EB
call 2642h                     ; 2A6A: CD 42 26
pop h                          ; 2A6D: E1
lxi d, 0080h                   ; 2A6E: 11 80 00
dad d                          ; 2A71: 19
pop d                          ; 2A72: D1
push d                         ; 2A73: D5
push h                         ; 2A74: E5
mvi c, 15h                     ; 2A75: 0E 15
call 0005h                     ; 2A77: CD 05 00
pop h                          ; 2A7A: E1
pop d                          ; 2A7B: D1
pop b                          ; 2A7C: C1
ora a                          ; 2A7D: B7
jnz 2A8Ch                      ; 2A7E: C2 8C 2A
dcr b                          ; 2A81: 05
jnz 2A60h                      ; 2A82: C2 60 2A
call 2647h                     ; 2A85: CD 47 26
ret                            ; 2A88: C9
db 0C3h, 60h, 2Ah              ; 2A89: data
lxi h, 2D53h                   ; 2A8C: 21 53 2D
call 2678h                     ; 2A8F: CD 78 26
jmp 2CB6h                      ; 2A92: C3 B6 2C
push b                         ; 2A95: C5
push d                         ; 2A96: D5
push h                         ; 2A97: E5
call 2A9Fh                     ; 2A98: CD 9F 2A
pop h                          ; 2A9B: E1
pop d                          ; 2A9C: D1
pop b                          ; 2A9D: C1
ret                            ; 2A9E: C9
lhld 263Eh                     ; 2A9F: 2A 3E 26
xchg                           ; 2AA2: EB
lhld 2640h                     ; 2AA3: 2A 40 26
dad d                          ; 2AA6: 19
mov m, a                       ; 2AA7: 77
xchg                           ; 2AA8: EB
inx h                          ; 2AA9: 23
shld 263Eh                     ; 2AAA: 22 3E 26
xchg                           ; 2AAD: EB
lxi h, 0300h                   ; 2AAE: 21 00 03
call 2946h                     ; 2AB1: CD 46 29
rnz                            ; 2AB4: C0
call 266Ch                     ; 2AB5: CD 6C 26
lxi h, 0000h                   ; 2AB8: 21 00 00
shld 263Eh                     ; 2ABB: 22 3E 26
lhld 2640h                     ; 2ABE: 2A 40 26
lxi d, 261Dh                   ; 2AC1: 11 1D 26
mvi b, 06h                     ; 2AC4: 06 06
jmp 2A60h                      ; 2AC6: C3 60 2A
push b                         ; 2AC9: C5
push d                         ; 2ACA: D5
push h                         ; 2ACB: E5
mvi c, 02h                     ; 2ACC: 0E 02
mov e, a                       ; 2ACE: 5F
call 0005h                     ; 2ACF: CD 05 00
pop h                          ; 2AD2: E1
pop d                          ; 2AD3: D1
pop b                          ; 2AD4: C1
ret                            ; 2AD5: C9
lxi h, 25B2h                   ; 2AD6: 21 B2 25
mvi c, 03h                     ; 2AD9: 0E 03
mov a, m                       ; 2ADB: 7E
inr a                          ; 2ADC: 3C
mov m, a                       ; 2ADD: 77
cpi 3Ah                        ; 2ADE: FE 3A
jc 2AEAh                       ; 2AE0: DA EA 2A
mvi m, 30h                     ; 2AE3: 36 30
dcx h                          ; 2AE5: 2B
dcr c                          ; 2AE6: 0D
jnz 2ADBh                      ; 2AE7: C2 DB 2A
lxi h, 25B0h                   ; 2AEA: 21 B0 25
mvi c, 03h                     ; 2AED: 0E 03
mov a, m                       ; 2AEF: 7E
call 2A0Eh                     ; 2AF0: CD 0E 2A
inx h                          ; 2AF3: 23
dcr c                          ; 2AF4: 0D
jnz 2AEFh                      ; 2AF5: C2 EF 2A
ret                            ; 2AF8: C9
lda 25B3h                      ; 2AF9: 3A B3 25
ora a                          ; 2AFC: B7
rz                             ; 2AFD: C8
mvi a, 0Ch                     ; 2AFE: 3E 0C
call 2A0Eh                     ; 2B00: CD 0E 2A
xra a                          ; 2B03: AF
sta 25B4h                      ; 2B04: 32 B4 25
lhld 3062h                     ; 2B07: 2A 62 30
mov a, l                       ; 2B0A: 7D
ora h                          ; 2B0B: B4
rz                             ; 2B0C: C8
lxi h, 2CD3h                   ; 2B0D: 21 D3 2C
mov a, m                       ; 2B10: 7E
cpi 0Dh                        ; 2B11: FE 0D
jz 2B1Dh                       ; 2B13: CA 1D 2B
call 2A0Eh                     ; 2B16: CD 0E 2A
inx h                          ; 2B19: 23
jmp 2B10h                      ; 2B1A: C3 10 2B
mvi a, 09h                     ; 2B1D: 3E 09
call 2A0Eh                     ; 2B1F: CD 0E 2A
mvi a, 23h                     ; 2B22: 3E 23
call 2A0Eh                     ; 2B24: CD 0E 2A
call 2AD6h                     ; 2B27: CD D6 2A
mvi a, 09h                     ; 2B2A: 3E 09
call 2A0Eh                     ; 2B2C: CD 0E 2A
lhld 3062h                     ; 2B2F: 2A 62 30
mov a, m                       ; 2B32: 7E
ora a                          ; 2B33: B7
jz 2B3Eh                       ; 2B34: CA 3E 2B
call 2A0Eh                     ; 2B37: CD 0E 2A
inx h                          ; 2B3A: 23
jmp 2B32h                      ; 2B3B: C3 32 2B
mvi a, 0Dh                     ; 2B3E: 3E 0D
call 2A0Eh                     ; 2B40: CD 0E 2A
mvi a, 0Ah                     ; 2B43: 3E 0A
call 2A0Eh                     ; 2B45: CD 0E 2A
mvi a, 0Ah                     ; 2B48: 3E 0A
jmp 2A0Eh                      ; 2B4A: C3 0E 2A
db 7Dh, 32h, 0B3h, 25h, 21h, 0B4h, 25h, 96h, 0D0h, 0C3h, 0F9h, 2Ah ; 2B4D: data
mov c, a                       ; 2B59: 4F
call 2A0Eh                     ; 2B5A: CD 0E 2A
lda 2F8Ch                      ; 2B5D: 3A 8C 2F
cpi 20h                        ; 2B60: FE 20
rz                             ; 2B62: C8
lda 304Fh                      ; 2B63: 3A 4F 30
cpi 02h                        ; 2B66: FE 02
rz                             ; 2B68: C8
lda 25CBh                      ; 2B69: 3A CB 25
cpi 17h                        ; 2B6C: FE 17
rz                             ; 2B6E: C8
mov a, c                       ; 2B6F: 79
call 2AC9h                     ; 2B70: CD C9 2A
ret                            ; 2B73: C9
lda 25B5h                      ; 2B74: 3A B5 25
lxi h, 304Fh                   ; 2B77: 21 4F 30
ora m                          ; 2B7A: B6
jnz 2B95h                      ; 2B7B: C2 95 2B
lda 3065h                      ; 2B7E: 3A 65 30
lxi h, 305Dh                   ; 2B81: 21 5D 30
ana m                          ; 2B84: A6
jnz 2BE3h                      ; 2B85: C2 E3 2B
mov a, m                       ; 2B88: 7E
ora a                          ; 2B89: B7
jz 2C10h                       ; 2B8A: CA 10 2C
lda 2F8Ch                      ; 2B8D: 3A 8C 2F
cpi 20h                        ; 2B90: FE 20
jz 2C10h                       ; 2B92: CA 10 2C
lxi h, 2F8Ch                   ; 2B95: 21 8C 2F
mov a, m                       ; 2B98: 7E
cpi 20h                        ; 2B99: FE 20
jnz 2BE3h                      ; 2B9B: C2 E3 2B
lda 3066h                      ; 2B9E: 3A 66 30
ora a                          ; 2BA1: B7
jz 2C10h                       ; 2BA2: CA 10 2C
lda 2F91h                      ; 2BA5: 3A 91 2F
cpi 2Bh                        ; 2BA8: FE 2B
jnz 2BE3h                      ; 2BAA: C2 E3 2B
lda 305Fh                      ; 2BAD: 3A 5F 30
ora a                          ; 2BB0: B7
jz 2C10h                       ; 2BB1: CA 10 2C
cpi 03h                        ; 2BB4: FE 03
jz 2BE3h                       ; 2BB6: CA E3 2B
lda 2F92h                      ; 2BB9: 3A 92 2F
cpi 23h                        ; 2BBC: FE 23
jz 2C10h                       ; 2BBE: CA 10 2C
lda 2F8Dh                      ; 2BC1: 3A 8D 2F
cpi 20h                        ; 2BC4: FE 20
jz 2C10h                       ; 2BC6: CA 10 2C
lda 305Fh                      ; 2BC9: 3A 5F 30
dcr a                          ; 2BCC: 3D
jz 2BE3h                       ; 2BCD: CA E3 2B
lxi d, 0010h                   ; 2BD0: 11 10 00
dcx d                          ; 2BD3: 1B
lxi h, 2F8Ch                   ; 2BD4: 21 8C 2F
dad d                          ; 2BD7: 19
mov a, m                       ; 2BD8: 7E
cpi 20h                        ; 2BD9: FE 20
jz 2BD3h                       ; 2BDB: CA D3 2B
inx d                          ; 2BDE: 13
lxi h, 3004h                   ; 2BDF: 21 04 30
mov m, e                       ; 2BE2: 73
lxi h, 25B4h                   ; 2BE3: 21 B4 25
push h                         ; 2BE6: E5
mov a, m                       ; 2BE7: 7E
lxi h, 25B3h                   ; 2BE8: 21 B3 25
sub m                          ; 2BEB: 96
cnc 2AF9h                      ; 2BEC: D4 F9 2A
pop h                          ; 2BEF: E1
inr m                          ; 2BF0: 34
lda 3004h                      ; 2BF1: 3A 04 30
lxi h, 2F8Ch                   ; 2BF4: 21 8C 2F
ora a                          ; 2BF7: B7
jz 2C06h                       ; 2BF8: CA 06 2C
mov b, a                       ; 2BFB: 47
mov a, m                       ; 2BFC: 7E
call 2B59h                     ; 2BFD: CD 59 2B
inx h                          ; 2C00: 23
mov a, b                       ; 2C01: 78
dcr a                          ; 2C02: 3D
jmp 2BF7h                      ; 2C03: C3 F7 2B
mvi a, 0Dh                     ; 2C06: 3E 0D
call 2B59h                     ; 2C08: CD 59 2B
mvi a, 0Ah                     ; 2C0B: 3E 0A
call 2B59h                     ; 2C0D: CD 59 2B
xra a                          ; 2C10: AF
sta 3004h                      ; 2C11: 32 04 30
lxi h, 2F8Ch                   ; 2C14: 21 8C 2F
mvi a, 78h                     ; 2C17: 3E 78
mvi m, 20h                     ; 2C19: 36 20
inx h                          ; 2C1B: 23
dcr a                          ; 2C1C: 3D
jnz 2C19h                      ; 2C1D: C2 19 2C
ret                            ; 2C20: C9
mov b, a                       ; 2C21: 47
lxi h, 2F8Ch                   ; 2C22: 21 8C 2F
mov a, m                       ; 2C25: 7E
cpi 20h                        ; 2C26: FE 20
rnz                            ; 2C28: C0
mov m, b                       ; 2C29: 70
ret                            ; 2C2A: C9
call 27B3h                     ; 2C2B: CD B3 27
rz                             ; 2C2E: C8
lhld 2619h                     ; 2C2F: 2A 19 26
mov a, l                       ; 2C32: 7D
ora h                          ; 2C33: B4
jz 2C3Fh                       ; 2C34: CA 3F 2C
mvi a, 1Ah                     ; 2C37: 3E 1A
call 2A0Eh                     ; 2C39: CD 0E 2A
jmp 2C2Fh                      ; 2C3C: C3 2F 2C
call 2660h                     ; 2C3F: CD 60 26
lxi d, 25F8h                   ; 2C42: 11 F8 25
call 2788h                     ; 2C45: CD 88 27
ret                            ; 2C48: C9
lda 305Eh                      ; 2C49: 3A 5E 30
cpi 03h                        ; 2C4C: FE 03
jz 2AF9h                       ; 2C4E: CA F9 2A
call 2C2Bh                     ; 2C51: CD 2B 2C
lxi h, 2601h                   ; 2C54: 21 01 26
mvi m, 53h                     ; 2C57: 36 53
inx h                          ; 2C59: 23
mvi m, 59h                     ; 2C5A: 36 59
inx h                          ; 2C5C: 23
mvi m, 4Dh                     ; 2C5D: 36 4D
inx h                          ; 2C5F: 23
xra a                          ; 2C60: AF
mov m, a                       ; 2C61: 77
lxi h, 2618h                   ; 2C62: 21 18 26
mov m, a                       ; 2C65: 77
lda 25CCh                      ; 2C66: 3A CC 25
sta 25CBh                      ; 2C69: 32 CB 25
lxi h, 0000h                   ; 2C6C: 21 00 00
shld 2619h                     ; 2C6F: 22 19 26
call 27B3h                     ; 2C72: CD B3 27
jz 2AF9h                       ; 2C75: CA F9 2A
xra a                          ; 2C78: AF
sta 25B3h                      ; 2C79: 32 B3 25
call 2660h                     ; 2C7C: CD 60 26
lxi d, 25F8h                   ; 2C7F: 11 F8 25
push d                         ; 2C82: D5
call 279Bh                     ; 2C83: CD 9B 27
pop d                          ; 2C86: D1
call 27A0h                     ; 2C87: CD A0 27
ret                            ; 2C8A: C9
call 2C2Bh                     ; 2C8B: CD 2B 2C
lda 25CDh                      ; 2C8E: 3A CD 25
cpi 19h                        ; 2C91: FE 19
jz 2CB6h                       ; 2C93: CA B6 2C
lda 25B8h                      ; 2C96: 3A B8 25
ora a                          ; 2C99: B7
cnz 2E0Fh                      ; 2C9A: C4 0F 2E
lhld 3050h                     ; 2C9D: 2A 50 30
shld 25B6h                     ; 2CA0: 22 B6 25
call 2E0Fh                     ; 2CA3: CD 0F 2E
lhld 263Eh                     ; 2CA6: 2A 3E 26
mov a, l                       ; 2CA9: 7D
ora h                          ; 2CAA: B4
jz 2CB6h                       ; 2CAB: CA B6 2C
mvi a, 1Ah                     ; 2CAE: 3E 1A
call 2A95h                     ; 2CB0: CD 95 2A
jmp 2CA6h                      ; 2CB3: C3 A6 2C
nop                            ; 2CB6: 00
nop                            ; 2CB7: 00
nop                            ; 2CB8: 00
lda 25CDh                      ; 2CB9: 3A CD 25
cpi 19h                        ; 2CBC: FE 19
jz 2CCAh                       ; 2CBE: CA CA 2C
call 266Ch                     ; 2CC1: CD 6C 26
lxi d, 261Dh                   ; 2CC4: 11 1D 26
call 2788h                     ; 2CC7: CD 88 27
lxi h, 2D93h                   ; 2CCA: 21 93 2D
call 2678h                     ; 2CCD: CD 78 26
jmp 0000h                      ; 2CD0: C3 00 00
db "CP/M MACRO ASSEM 2.0", 0Dh, "NO SOURCE FILE PRESENT", 0Dh ; 2CD3: data
db "NO DIRECTORY SPACE", 0Dh, "SOURCE FILE NAME ERROR", 0Dh ; 2CFF: data
db "INVALID PARAMETER:", 0Dh, "SOURCE FILE READ ERROR", 0Dh ; 2D29: data
db "OUTPUT FILE WRITE ERROR", 0Dh, "CANNOT CLOSE FILES", 0Dh ; 2D53: data
db "UNBALANCED MACRO LIB", 0Dh, "END OF ASSEMBLY", 0Dh, 0C5h, 47h, 3Ah ; 2D7E: data
db 0CDh, 25h, 0FEh, 19h, 78h, 0CAh, 0EFh, 2Dh, 0D5h, 0F5h, 21h, 0B8h ; 2DA6: data
db 25h, 7Eh, 0B7h, 0CAh, 0DBh, 2Dh, 0FEh, 10h, 0DAh, 0C3h, 2Dh, 0CDh ; 2DB2: data
db 0Fh, 2Eh, 0C3h, 0DBh, "-*P0", 0EBh, 2Ah, 0B6h, 25h, 4Fh, 06h, 00h ; 2DBE: data
db 09h, 7Bh, 0BDh, 0C2h, 0D8h, 2Dh, 7Ah, 0BCh, 0CAh, 0E1h, 2Dh, 0CDh ; 2DCD: data
db 0Fh, ".*P0", 22h, 0B6h, 25h, 21h, 0B8h, 25h, 5Eh, 34h, 16h, 00h, 21h ; 2DD9: data
db 0B9h, 25h, 19h, 0F1h, 77h, 0D1h, 0C1h, 0C9h ; 2DE9: data
push psw                       ; 2DF1: F5
rrc                            ; 2DF2: 0F
rrc                            ; 2DF3: 0F
rrc                            ; 2DF4: 0F
rrc                            ; 2DF5: 0F
ani 0Fh                        ; 2DF6: E6 0F
call 2E06h                     ; 2DF8: CD 06 2E
pop psw                        ; 2DFB: F1
push psw                       ; 2DFC: F5
ani 0Fh                        ; 2DFD: E6 0F
call 2E06h                     ; 2DFF: CD 06 2E
pop psw                        ; 2E02: F1
add d                          ; 2E03: 82
mov d, a                       ; 2E04: 57
ret                            ; 2E05: C9
adi 90h                        ; 2E06: C6 90
daa                            ; 2E08: 27
aci 40h                        ; 2E09: CE 40
daa                            ; 2E0B: 27
jmp 2A95h                      ; 2E0C: C3 95 2A
mvi a, 3Ah                     ; 2E0F: 3E 3A
call 2A95h                     ; 2E11: CD 95 2A
lxi h, 25B8h                   ; 2E14: 21 B8 25
mov e, m                       ; 2E17: 5E
xra a                          ; 2E18: AF
mov d, a                       ; 2E19: 57
mov m, a                       ; 2E1A: 77
lhld 25B6h                     ; 2E1B: 2A B6 25
mov a, e                       ; 2E1E: 7B
call 2DF1h                     ; 2E1F: CD F1 2D
mov a, h                       ; 2E22: 7C
call 2DF1h                     ; 2E23: CD F1 2D
mov a, l                       ; 2E26: 7D
call 2DF1h                     ; 2E27: CD F1 2D
xra a                          ; 2E2A: AF
call 2DF1h                     ; 2E2B: CD F1 2D
mov a, e                       ; 2E2E: 7B
ora a                          ; 2E2F: B7
jz 2E3Fh                       ; 2E30: CA 3F 2E
lxi h, 25B9h                   ; 2E33: 21 B9 25
mov a, m                       ; 2E36: 7E
inx h                          ; 2E37: 23
call 2DF1h                     ; 2E38: CD F1 2D
dcr e                          ; 2E3B: 1D
jnz 2E36h                      ; 2E3C: C2 36 2E
xra a                          ; 2E3F: AF
sub d                          ; 2E40: 92
call 2DF1h                     ; 2E41: CD F1 2D
mvi a, 0Dh                     ; 2E44: 3E 0D
call 2A95h                     ; 2E46: CD 95 2A
mvi a, 0Ah                     ; 2E49: 3E 0A
call 2A95h                     ; 2E4B: CD 95 2A
ret                            ; 2E4E: C9
db 0CDh, 9Eh, 2Ah, 3Eh, 0Ah, 0CDh, 9Eh, 2Ah, 0C9h, 0F7h, 0FFh, 0FFh ; 2E4F: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0EFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2E5B: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2E66: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2E71: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2E7C: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2E87: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 77h, 0F7h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2E92: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2E9E: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2EA9: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2EB4: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2EBF: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2ECA: data
db 0FFh, 0FFh, 73h, 0F7h, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2ED5: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2EE1: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2EEC: data
db 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh, 0FFh ; 2EF7: data
