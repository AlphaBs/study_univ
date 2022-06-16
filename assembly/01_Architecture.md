# x86 Assembly

Intel문법 / AT&T 문법 존재 (이 수업은 Intel 문서 기준)

## Assembler

어셈블리어를 기계어로 번역 (MASM, GAS, NASM)

Assembly level analysing debugger: (OllyDBG, x64dbg)

# x86 Architecture

## Memory Byte Order

### Big Endian

- 상위 주소 -> 하위 주소 순으로 데이터 저장
- 읽기 쉬움
- 하위 바이트 값을 이용할 때 데이터 크기를 알아야 함

### Little Endian

- 하위 주소 -> 상위 주소 순으로 데이터 저장
- 하위 바이트 값을 이용할 때 데이터 크기를 몰라도 순차적 접근 가능
- 읽기 힘듬

## Memory Bit Order

신경쓰지 않음. 하드웨어에서 자체적으로 처리

## CPU

fetch -> decode -> execute

## Register

- 범용 목적 레지스터
    EAX, EBX, ECX, EDX, EBP, ESI, EDI, ESP

- 특수 목적 레지스터
    Program Counter(=EIP), Segment Register, EFLAGS register

- 부동소수점 레지스터

Byte order 신경쓰지 않음. 주소 개념 없음. 상위, 하위 개념 사용. (예시 AH, AL)

## Memory

메모리 주소는 byte 단위, CPU 에서 word 단위로 불러와 작업 (32bit, 64bit)

Little Endian 기본