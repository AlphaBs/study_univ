# 알고리즘
특정 작업을 수행하는 명령어들의 유한 집합

## 알고리즘의 요건
- 입력: 데이터가 0개 이상
- 출력: 적어도 한 개 이상의 결과 생성
- 명확성: 각 명령은 명확하고 모호하지 않아야 함
- 유한성: 어떤 경우에도 반드시 유한 시간 내에 종료
- 유효성: 반드시 실행 가능해야 함

## 시간 복잡도
O(1) < O(logN) < O(N) < O(nlogn) < O(n^2) < O(2^n) < O(n!)

상수 < 로그 < 다항 < 지수 < 팩토리얼

- 선언문 0
- 산술식 및 지정문 1

## 정렬

### 선택 정렬

최대 원소를 찾아서 교환

시간복잡도: O(n^2)

## 탐색

### 선형 탐색

인덱스 처음부터 끝까지 탐색

시간복잡도: O(N)

### 이진 탐색 (바이너리 서치)

```py
left = 0, right = n - 1
while left <= max:
    mid = (left + right) / 2
    if arr[mid] == target:
        return mid
    elif arr[mid] > right:
        right = mid - 1
    else:
        left = mid + 1
return -1
```

시간복잡도: O(logN)

## 순열 생성기

```cpp
void perm(char *a, const int k, const int m)
{
    if (k == m) {
        for (int i= 0; i <= m; i++) cout << a[i] << " ";
        cout << endl;
    }
    else {
        for (i = k; i <= m; i++) {
            swap(a[k], a[i]);
            perm(a, k+1, m);
            swap(a[k], a[i]);
        }
    }
}
```

O(n!)

## 문자열

### KMP 알고리즘

실패 함수 계산: O(LengthP)
전체 시간: O(LengthP + LengthS)