# Floating Point

## 2진수 소수 표현

예시:  
`0011.1010 => 2^1 + 2^0 + 2^-1 + 2^-3`  
`3.625 => 11.101`

## 2진수 무한소수 반올림

X자리에서 반올림할 경우 최대 (X-1)자리 소수가 된다.

### Round to nearest (even)

정확한 값과 가까운 쪽으로 반올림, 양쪽 거리가 같은 경우 결과가 짝수인 쪽 선택. 이진수에서는 마지막 자리가 0

### Round down

정확한 값보다 크지 않은 가까운 값 선택

### Round up

정확한 값보다 작지 않은 가까운 값 선택

### Round toward zero

정확한 값의 절댓값보다 크지 않은 가까운 값 선택

## Data Types

### Single-Precision Floating-Point

- 부호 1bit, 지수 8bit, 가수 23bit
- 32bit, 4byte, dword

### Double-Precision Floating-Point

- 부호 1bit, 지수 11bit, 가수 52bit
- 64bit, 8byte, qword

### x86 Double Extended-Precision Floating-Point

- 부호 1bit, 지수 15bit, 정수 1bit, 가수 63bit
- 80bit, 10byte, tword

## 정규화

*m0.m1m2m3m4* 에서 m0 이 0 이 아닌 경우를 정규화 되었다고 함.  
예시: `1.xxx * 2^n`

## Floating Point

정규화 한 값을 sign bit, exponent bits, 로 나누어 저장

### Sign bit

### Exponent bits

지수부, biased exponent: 정규화한 지수값 + 지수편향값

지수편향값 계산: `2^(e-1) - 1`, e는 exponent bit 크기

### Fraction bits

가수부, 유효숫자의 첫번째 자리를 생략한 값

### Integer bit

정수 비트, 유효숫자의 첫 번째 수

- 확장정밀도 부동소수점에서 기계적 계산 과정 간단하게 하기 위함

## 특수 값

### signed zero

`(01) 00000000 0000000000000000000000`

+0: 부호비트가 0, 지수부와 가수부가 0  
-0: 부호비트가 1, 지수부와 가수부가 0

### Denormalized finite numbers

비정규화된 유한 수

`(01) 00000000 0101010101010101010101`

- 가장 작은 정규화된 값보다 0에 가까운 수를 표현하기 위해 사용
- 지수부의 비트는 전부 0, 가수부는 0이 아님

### signed infinities

`(01) 1111111 00000000000000000000000`

+8: 부호비트가 0, 지수부의 비트가 전부 1, 가수부의 비트가 전부 0  
-8: 부호비트가 1, 지수부의 비트가 전부 1, 가수부의 비트가 전부 0

### NaN (Not a Number)

숫자가 아님 (0으로 나눈 결과, 허수)

`0 11111111 010101010101010101010101`

부호비트가 0, 지수부가 전부 1, 가수부가 0이 아닌 값

