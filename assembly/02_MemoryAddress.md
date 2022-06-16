# Addressing Memory

## 주소 지정 방식

**크기 ptr 세그먼트:[베이스+인덱스*스케일+변위]**

- 크기: byte, word, dword, etc (data-types.md 파일 참고)
- 세그먼트 레지스터: 메모리 영역
- 베이스: 범용 레지스터 혹은 없음
- 인덱스: 범용 레지스터 (ESP 제외) 또는 없음
- 스케일: 1, 2, 4, 8 또는 없음
- 변위: 1, 2, 4 바이트 또는 없음

### 세그먼트

16비트: 접근할 수 있는 영역의 크기가 레지스터 크기보다 크기 때문에 세그먼티 레지스터와 주소를 조합해 최종 접근할 주소를 계산

32비트: 페이지 선택 용도

- CS: 코드 영역
- DS: 데이터 영역 (기본)
- ES, FS, GS, SS 등등

### 예시

변위만 사용 & 세그먼트 생략: 

`mov dword ptr ds:[0x00402000], 1`   
`mov dword ptr [0x00402000], 1`   
`mov dword [0x00402000], 1`

모두 사용

`mov eax, dword ptr [ebx + ecx*4 + 0x10]`

## Opcode

`| Prefixes | Opcode | ModR/M | SIB | Displacement | Immediate |`

mov 명령어의 두 인자로는 (레지스터 r), (레지스터 혹은 메모리 주소 r/m)가 들어간다. 두 인자의 순서, r/m이 무엇을 가르키는 지 명령어에 담아내기 위해 ModR/M 과 SIB 사용

Opcode 에 + 가 있는 경우, EAX, ECX, EDX, EBX 순으로 확장 (정확한 건 공식문서 참고)

Opcode 에 /r 이 아닌 /0 같은 숫자가 있으면 표 윗부분 REG = 을 참고. (Reg = 0 이라는 뜻)

세그먼트 레지스터에 값 넣을수는 있지만 운영체제 보호모드로 인해 오류

### ModR/M (Mod, Reg, R/M)

mod: r/m 부분이 레지스터를 의미하는지, 메모리 주소를 의미하는지 나타냄

표 윗부분에는 r부분을, 표 왼쪽에는 r/m에 해당하는 주소를 찾아 ModR/M byte 를 찾으면 된다.

Scale, Index 를 사용하는 경우, r/m은 [--][--] 이고, 뒤에 SIB 바이트가 따라온다는 것을 알려줌. 

### SIB (Scale, Index, Base)

표 윗부분에는 베이스 레지스터, 왼쪽에는 적절한 scale, index 을 찾아 SIB byte 를 찾으면 된다. 

base 가 없는 경우 [*]

scale*index 가 없는 경우 none

### Prefix: Operand-size Override

r32, r/m32 -> r16, r/m16 으로 크기 변경

### Prefix: Address-size Override

32비트 모드에서 16비트 주소 지정 방식 사용

(잘 안쓰임)

## LEA

`lea eax, [ebx + ecx*4 + 0x10]`  
eax에 주소 계산한 값을 저장