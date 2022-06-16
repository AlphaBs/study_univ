# 분기명령어

EIP 를 조작하여 다음 실행할 명령어의 위치 설정

### Jump short

jump to relative address

Fetch (EIP증가) -> Decode -> Execute (EIP 계산 후 JUMP)

- JMP 0xFE

### Jump near

jump to address

- JMP 0x00402010
- JMP [EAX]
- JMP [0x00402010]

### Jump far

code segment 바꾸기, 잘 안씀, 못씀

## 조건 JMP

- JZ, JE: ZF == 1
- JNZ, JNE: ZF == 0

(unsigned: above, below)  
cmp x, y (x - y) 를 통해 carry flag 와 zero flag 값을 비교해 조건 비교

- JA: x > y
- JAE: x >= y
- JB: x < y
- JBE: x <= y

(signed: greater, less)  
cmp x, y (x - y) 해서 sign flag, overflow, zero flow 통해 조건 비교

- JG
- JNB

## Loop (X)

- LOOP 0x00000000: ECX - 1 한 뒤 ECX != 0 이면 JUMP
- LOOPZ = LOOPE 0x00000000: ECX - 1 한 뒤 ECX != 0 && EAX != 0 이면 JUMP

**signed, unsigned 연산 통일해서 사용하기**