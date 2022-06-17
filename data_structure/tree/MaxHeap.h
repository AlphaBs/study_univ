#pragma once
#include "TreeNode.h"

template <class T>
class MaxHeap
{
private:
    T* array;
    int length;
public:
    MaxHeap(int capacity);
    void insert(T data);
    T remove();
    void print();
};