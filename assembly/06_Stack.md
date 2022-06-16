# Stack

- Descending stack
- Full stack: stack pointer가 마지막에 넣은 값을 가리킨다
- Stack pointer: ESP
- Stack operand: 4byte

## IO

- PUSH: ESP -= 4, MOV
- POP: MOV, ESP += 4

- PUSHA, PUSHAD
- POPA, POPAD

- PUSHF, PUSHFD
- POPF, POPFD

- CALL
- RET
- RET x: pop한 후 ESP += x

## EFLAGS

- LAHF: AH <- EFLAGS
- SAHF: AH -> EFLAGS

# Function

- CALL addr: push EIP (fetch 이후), jmp addr
- RET: pop addr, jmp addr

## Prologue

1. 이전 EBP 값을 스택에 보관한다.
2. 현재 stack pointer로 frame pointer를 설정한다.
3. 지역변수를 위한 영역을 확보한다.

```
push ebp
mov ebp, esp
sub esp, 지역변수영역크기
```

## Epilogue

1. ESP 값을 EBP 값으로 되돌린다.
2. 이전 EBP 값을 스택에서 꺼내온다.
3. RET

```
mov esp, ebp
pop ebp
ret
```

## Example

```
// prologue
push ebp
mov ebp, esp
sub esp, 0x0C

mov dword ptr [ebp - 0x4], 0 // 지역변수
mov dword ptr [ebp - 0x8], 1
mov eax, [ebp - 0x4]
add eax, [ebp - 0x8]
mov [ebp - 0xC], eax // 지역변수

// epliogue
mov esp, ebp
pop ebp
ret
```

- ENTER, LEAVE

## Calling Convention

### PASCAL

- 피호출자 정리
- 인자를 왼쪽부터 스택에 push 한 후 call
- ret x: 인수 자리 지움
- 가변인자 사용 불가 (인수의 갯수를 모르면 첫번째 인수의 위치를 알 수 없음)

```
mov eax, [ebp + 0x14] // 인자 ebp+0x08 ~ ebp~0x14+04
add eax, [ebp + 0x10]
add eax, [ebp + 0x0C]
add eax, [ebp + 0x08]
ret 0x10 
```

### STDCALL

- 피호출자 정리
- win32 API
- 인자를 오른쪽부터 스택에 push 한 후 call, 첫번째 인자의 위치를 바로 알 수 있음
- EAX, ECX, EDX는 함수 내에서 사용하도록 지정됨
- 정수 리턴값은 EAX를 통해 전달
- 가변인자 사용 불가 (ret x 에서 x를 컴파일 타임에 결정할 수 없음)

```
mov eax, [ebp+0x08] // 인수 8부터 시작
add eax, [ebp+0x0C]
add eax, [ebp+0x10]
add eax, [ebp+0x14]
mov esp, ebp
pop ebp
ret 0x10
```

### FASTCALL

(x)
- 피호출자 정리
- 몇 개의 인자 전달에 레지스터를 사용하는 변형 

### CDECL

- 호출자 정리
- 대부분 x86 C언어 컴파일러에서 사용
- 인자를 오른쪽부터 스택에 넣어서 전달
- 호출한 쪽에서 스택을 정리
- 가변인자, 임의의 개수의 인자를 받을 수 있음, 인자 수를 ebp+0x08에 넣어줌

```
push 3
push 2
push 1
call f
add esp, 0x0C // unwind
```

```
push ebp
mov ebp, esp
mov eax, [ebp + 0x08]
add eax, [ebp + 0x0C]
add eax, [ebp + 0x10]
add eax, [ebp + 0x14]
mov esp, ebp
pop ebp
ret // 정리 안함
```

