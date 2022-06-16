# String

- MOVSB: [ESI] -> [EDI], 1바이트 이동, DF가 0이면 증가, DF가 1이면 감소
- MOVSW: [ESI] -> [EDI], 2바이트 이동, 2바이트 증감
- MOVSD: [ESI] -> [EDI], 4바이트 이동, 4바이트 증감
- CLD: DF = 0, 증가방향
- STD: DF = 1, 역방향
- STOSB, STOSW, STOSD: EAX,AX,AL -> ES:[EDI], EDI 증감
- LODSB, LODSW, LODSD: DS:[EDI] -> EAX,AX,AL, EDI 증감
- SCASB, SCASW, SCASD: compare EAX,AX,AL <-> ES:[EDI], EDI 증감
- CMPSB, CMPSW, CMPSD: compare DS:[ESI] <-> ES:[EDI]

(DS와 ES는 사실상 같은 곳)

## 반복

1. ECX가 0이 아니면 실행
2. ECX 감소
3. ZF 비교 혹은 반복

- REP: ECX가 0이 될때까지 반복
- REPE, REPZ: ECX가 0이거나 ZF가 1이면 반복
- REPNE, REPNZ: ECX가 0이거나 ZF가 0이면 반복

## Examples

NULL 종료 문자열 길이 알아내기

```
cld
xor al, al
mov ecx, -1
mov edi, 0x402000
repnz scasb
neg ecx
sub ecx, 2
```

길이를 알고 길이가 같은 두 문자열 비교

```
cld
mov ecx, 4
mov esi, 0x402010
mov edi, 0x402000
repe compsb
mov eax, [edi-1]
sub eax, [esi-1] // eax가 0인지 비교
```

길이를 알고 있고 특정 문자의 첫 위치 찾기

```
cld
mov ecx, 4
mov al, 0x40
mov edi, 0x00402010
repne scasb
// ecx가 0인 경우 못찾음, edi-1에 찾는 문자 존재
```