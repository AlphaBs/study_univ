# 부호

## unsigned

모든 비트가 magnitude 를 가짐

## signed

MSB 는 부호비트, 음수 표현 가능  
2의 보수로 음수 표현

- NEG x: 2의 보수를 취해줌

# 확장

unsigned 로 빈 자리에는 0이 채워짐

- MOVZX r16, r/m8
- MOVZX r32, r/m8
- MOVZX r32, r/m16

signed 로 부호 값이 유지됨. EDX:EAX => 상위비트:하위비트

- CBW: AL => AX
- CWD: AX => DX:AX
- CWDE: AX => EAX
- CDQ: EAX => EDX:EAX

# EFLAGS

특수목적 레지스터, 상태 레지스터, 플래그의 집합

명령어마다 EFLAGS 사용 여부와 사용 목적도 다름

- CF: Carry Flag, carry 발생 여부 (unsigned 에서 확인)
- OF: Overflow Flag, overflow 발생 여부 (음수+음수 or 양수+양수) (signed 에서 확인)
- ZF: Zero Flag, 결과가 zero 이면 1
- SF: Sign Flag, 음수면 1

# 사칙연산

- INC
- DEC
- ADD x, y: [CF, OF, ZF, SF]
- ADC x, y: x += y += CF, [CF, OF, ZF, SF]
- SBB x, y: x -= y += CF, [CF, OF, ZF, SF], (뺄셈은 borrow를 가져옴)
- CMP x, y: 같으면 ZF = 1

## MUL

- MUL x: AX = AL * x, unsigned
- MUL x: DX:AX = AX * x, unsigned
- MUL x: EDX:EAX = EAX * x, unsigned

- IMUL x: signed
- IMUL x, y: x *= y
- IMUL x, y, z: x = y * z

## DIV

2byte / 1byte => 1byte, 1byte  
4byte / 2byte => 2byte, 2byte
8byte / 4byte => 4byte, 4byte

- DIV x: AH = AX / x, AX % x, unsigned
- DIV x: AX = DX:AX / x, DX = DX:AX % x, unsigned
- DIV x: EAX = EDX:EAX / x, EDX = EDX:EAX % x, unsigned
- IDIV x: signed

몫이 너무 커지면 예외 발생

나머지 음수 가능: `(a/b)*b + a%b = a`