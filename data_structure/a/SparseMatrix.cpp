#include "SparseMatrix.h"
#include "iostream"

template <class T>
SparseMatrix<T>::SparseMatrix(int rows, int cols, int terms)
    : rows(rows), cols(cols), terms(terms)
{
    elements = new MatrixElement[terms];
}

template <class T> 
SparseMatrix<T>::~SparseMatrix()
{
    delete[] elements;
}

template <class T>
void SparseMatrix<T>::CopyFrom(int* array) const
{
    int index = 0;
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            if (array[cols * y + x] == 0)
                continue;

            elements[index].data = array[cols * y + x];
            elements[index].row = y;
            elements[index].col = x;
            index++;
        }
    }
}

template <class T>
SparseMatrix<T>& SparseMatrix<T>::Transpose() const
{
    SparseMatrix<T> *newMatrix = new SparseMatrix<T>(cols, rows, terms);
    int newIndex = 0;

    for (int c = 0; c < cols; c++)
    {
        for (int t = 0; t < terms; t++)
        {
            if (elements[t].col == c)
            {
                newMatrix->elements[newIndex].row = c;
                newMatrix->elements[newIndex].col = elements[t].row;
                newMatrix->elements[newIndex].data = elements[t].data;
                newIndex++;
            }
        }
    }

    return *newMatrix;
}

template <class T>
SparseMatrix<T>& SparseMatrix<T>::FastTranspose() const
{
    SparseMatrix<T>* newMatrix = new SparseMatrix<T>(cols, rows, terms);

    int* newRowSize = new int[cols];
    int* newRowStart = new int[cols];
    std::fill(newRowSize, newRowSize + cols, 0);

    for (int i = 0; i < terms; i++)
        newRowSize[elements[i].col]++;

    newRowStart[0] = 0;
    for (int i = 1; i < cols; i++)
        newRowStart[i] = newRowStart[i - 1] + newRowSize[i - 1];

    for (int i = 0; i < terms; i++)
    {
        int j = newRowStart[elements[i].col];
        newMatrix->elements[j].row = elements[i].col;
        newMatrix->elements[j].col = elements[i].row;
        newMatrix->elements[j].data = elements[i].data;
        newRowStart[elements[i].col]++;
    }

    delete [] newRowSize;
    delete [] newRowStart;

    return *newMatrix;
}

// 미구현
SparseMatrix& SparseMatrix::Multiply(SparseMatrix& b) const 
{
    if (cols != b.rows)
        throw "InvalidOperation: cannot multiply matrix with different cols, rows";

    SparseMatrix transposed = b.Transpose();

    int i = 0;
    SparseMatrix *newMatrix = new SparseMatrix(rows, b.cols, 100);

    int sum = 0;
    while (true)
    {
        
    }
}

template <class T>
void SparseMatrix<T>::Print(std::ostream& to) const
{
    for (int y = 0; y < rows; y++)
    {
        for (int x = 0; x < cols; x++)
        {
            bool found = false;
            for (int t = 0; t < terms; t++)
            {
                if (elements[t].row == y && elements[t].col == x)
                {
                    found = true;
                    to << elements[t].data;
                }
            }

            if (!found)
                to << 0;
            to << " ";
        }
        to << std::endl;
    }
}

int main()
{
    const int rows = 3;
    const int cols = 5;

    const int matrix[rows][cols] =
        {
            {1, 2, 0, 4, 0},
            {0, 4, 5, 0, 0},
            {0, 0, 6, 8, 3}};

    SparseMatrix<int> a(rows, cols, 8);
    a.CopyFrom((int*)matrix);

    std::cout << "Original" << std::endl;
    a.Print(std::cout);

    std::cout << "Transpose" << std::endl;
    SparseMatrix<int>& newMatrix1 = a.Transpose();
    newMatrix1.Print(std::cout);

    std::cout << "FastTranspose" << std::endl;
    SparseMatrix<int>& newMatrix2 = a.FastTranspose();
    newMatrix2.Print(std::cout);

    return 0;
}