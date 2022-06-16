# SIMD

## SISD vs SIMD

- SISD (Single Instruction Single Data)  
하나의 명령어로 데이터를 하나씩 처리  
예시: add, mul, div, sub 등등 기존에 쓰던 사칙연산

- SIMD (Single Instruction Multiple Data)  
하나의 명령어로 데이터를 여러개씩 처리  
예시: 벡터, 행렬 처리

## x86 SIMD

각 명령어셋마다 지원하는 레지스터의 종류, 갯수, 크기가 점점 확장된다.  동시에 계산할 수 있는 데이터의 크기 또한 점점 커진다.

- MMX (펜티엄 MMX 부터)
- SSE (펜티엄 3 부터)
- SSE2 (펜티엄 4 부터)
- SSE3 (펜티엄 프레스캇 부터)
- AVX (i시리즈 2세대 부터)
- AVX2 (i시리즈 4세대 부터)

(이 문서는 SSE3 기준으로 작성)

## 데이터 타입

xmm0 ~ xmm7 레지스터: 128 비트 데이터 저장 가능. packed type, scalar type 으로 표현

packed type: 128 비트 안에 여러 데이터 저장 가능. qword 데이터 두개 혹은 dword 데이터 4개 혹은 word 데이터 8개 혹은 byte 데이터 16개 저장 가능

scalar type: 128 비트 안에 하나의 데이터만 저장 가능

## 데이터 이동

### Scalar type

하위 바이트 이동, 나머지는 0으로 채움

movss: mov scalar single-precision floating-point value

movsd: mov scalar double-precision floating-point value

### Packed type

movaps: mov aligned packed (single-precision, or double-precision) floating-point values

16-byte 정렬된 주소만 사용 가능, 안되면 예외

movups: mov unaligned packed single-precision floating-point values

movups: mov unaligned packed double-precision floating-point values

movhps/movhpd: 상위 qword에 mov, 하위 변경 안함

movlps/movlpd: 하위 qword에 mov, 상위 변경 안함

movhlps dest, src: src의 상위 qword를 dest의 하위 qword로 mov, 상위 qword 유지

movlhps dest, src: src의 하위 qword를 dest의 상위 qword로 mov, 하위 qword 유지

movshdup: mov packed single-fp high and duplicate, src을 0 1 2 3으로 나누었을 때, dest를 3 3 1 1로 채우기

movsldup: mov packed single-fp low and duplicate, src을 0 1 2 3으로 나누었을 때, dest를 2 2 0 0로 채우기

## 기본 연산

addss: 최하위 단정밀도 부동소수점 값끼리 더한다

addsd: 하위 배정밀도 부동소수점 값끼리 더한다.

addps: 각 자리의 단정밀도 부동소수점 값끼리 더한다.

addpd: 각 자리의 배정밀도 부동소수점 값끼리 더한다.

subss, subsd: scalar 뺄셈  
subps, subpd: packed 뺄셈  

mulss, mulsd: scalar 곱셈  
mulps, mulpd: packed 곱셈  

divss, divsd: scalar 나눗셈  
divps, divpd: packed 나눗셈  

shufps dest, src, select: select 하위 2비트는 dest 의 위치, 상위 2비트는 src 의 위치로 선택해서 dest 에 넣기

shufpd dest, src, select: select 하위 1비트는 dest 의 위치, 상위 2비트는 src 의 위치로 선택해서 dest 에 넣기

## 16-byte align 연산

```
mov ebp, esp
and esp, 0xFFFFFFF0
sub esp, 16
```

`esp -= (esp % 16)`

## Horizontal 연산 (SSE1)

덧셈 예시:

```
movaps xmm1, xmm0
shufps xmm1, xmm0, 0xB1
addps xmm0, xmm1
movhlps xmm1, xmm0
addss xmm0, xmm1
```

## Horizontal 연산 (SSE3)

덧셈 예시:

```
movshdup xmm1, xmm0
addps xmm0, xmm1
movhlps xmm1, xmm0
addss xmm0, xmm1
```

## Horizontal 연산 (SSE3)

HADDPS dest, src
```
dest(0) = dest(0) + dest(1)
dest(1) = dest(2) + dest(3)
dest(2) = src(0) + src(1)
dest(3) = src(2) + src(3)
```