# 행렬

## 행렬의 표현

### DenseMatrix

a*b 크기의 행렬을 이차원 배열에 저장  
`matrix[a][b]`

### SparseMatrix

0이 아닌 행렬의 값을 (x, y) 으로 저장  
`vector<(x, y)>`

## 행렬의 전치

### DensMatrix Transpose

O(rows*cols)

### SparseMatrix Transpose

O(terms * cols)

### SparseMatrix FastTranspose

`2 * terms + cols`  
O(terms + cols)

## 행렬의 곱

$O(\sum_r(b.cols\cdot t_r + b.terms))$ = 
$O(b.cols\cdot + a.rows\cdot b.terms)$

