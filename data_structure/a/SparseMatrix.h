#include "iostream"

template <typename T>
class MatrixElement
{
public:
    int row;
    int col;
    T data;
};

template <typename T>
class SparseMatrix
{
public:
    int rows;
    int cols;
    int terms;
    MatrixElement<T>* elements;

    SparseMatrix(int rows, int cols, int terms);
    ~SparseMatrix();

    // to 스트림에 행렬을 순서대로 출력
    void Print(std::ostream& to) const;

    // rows*cols 크기의 이차원 배열 array 에서 값을 복사해옴
    void CopyFrom(int* array) const;

    // 전치 행렬을 나타내는 새로운 행렬 생성해서 반환
    SparseMatrix& Transpose() const;

    // FastTranspose 알고리즘
    SparseMatrix& FastTranspose() const;

    SparseMatrix& Multiply(SparseMatrix& b) const;
};