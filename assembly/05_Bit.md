# 비트연산

- NOT: 1의 보수
- AND x, y: x &= y
- OR x, y: x |= y
- XOR x, y: x ^= y

xor x, x: x = 0의 최적화

## 비트 조작

### SET BIT

or 연산 사용, 설정할 비트만 1로, 나머지는 0

### RESET BIT

and 연산 사용, 설정할 비트만 0로, 나머지는 1

### TOGLE BIT

xor 연산 사용, 설정할 비트만 1로, 나머지는 0

### TEST BIT

and 연산 사용, 확인할 비트만 1로, 나머지는 0

- TEST x, y: and x, y 한 뒤 결과값 버리고 상태값만 설정 (ZF), test eax, eax 으로 0 검사를 최적화할 수 있음

### SHIFT / ROTATE

CF는 밀려진 비트가 저장, 1-bit 나눗셈의 경우 나머지

OF는 1-bit shift에서만 설정, 부호 오류 발생한 건지

shift횟수는 5비트로 제한됨 (0~31 bit shift만 가능)

cl 레지스터 혹은 즉시값만 사용 가능

- SHL r/m, cl: **unsigned** left, 곱셈과 비슷, (SAL == SHL)
- SHR r/m, cl: **unsigned** right, 나눗셈과 비슷, 0으로 채움

- SAL r/m, cl: **signed** left, 곱셈과 비슷, (SAL == SHL)
- SAR r/m, cl: **signed** right, 나눗셈과 비슷, 부호에 맞는 비트 채움, 음수일 경우 다른 결과 발생 가능

- ROL r/m, cl
- ROR r/m, cl

- RCL r/m, cl: 빈칸에 CF 값을 넣음
- RCR r/m, cl: 빈칸에 CF 값을 넣음

- SHLD r/m, r, cl: r/m:r 을 cl 번 left shift, r에 값이 쓰이지 않기 때문에 한번 더 연산 해줘야함

```
mov edx, 0
mov eax, 0xFFFFFFFF
shld edx, eax, 1
shd eax, 1
```

- SHRD r/m, r, cl: r:r/m 을 cl 번 right shift, r에 값이 쓰이지 않음

```
mov edx, 0xFFFFFFFF
mov eax, 0
shrd eax, edx, 1
shr edx, 1
```