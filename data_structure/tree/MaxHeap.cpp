#include "TreeNode.h"
#include "MaxHeap.h"
#include <algorithm>
#include <iostream>

template <class T>
MaxHeap<T>::MaxHeap(int capacity)
{
    array = new T[capacity + 1];
    length = 0;
}

template <class T>
void MaxHeap<T>::insert(T data)
{
    length++;
    array[length] = data;

    int n = length;
    while (n != 1 && array[n / 2] < array[n])
    {
        std::swap(array[n / 2], array[n]);
        n /= 2;
    }
}

template <class T>
T MaxHeap<T>::remove()
{
    if (length == 0)
        throw "heap is empty";

    T top = array[1];
    std::swap(array[1], array[length]);
    length--;
    int current = 1;
    while (true)
    {
        int left = current * 2, right = left + 1;
        if (left > length || 
           (array[current] >= array[left] && array[current] >= array[right]))
        {
            break;
        }
        else if (array[left] > array[right] || right > length)
        {
            std::swap(array[left], array[current]);
            current = left;
        }
        else
        {
            std::swap(array[right], array[current]);
            current = right;
        }
    }
    return top;
}

template <class T>
void MaxHeap<T>::print()
{
    for (int i = 1; i <= length; i++)
    {
        std::cout << array[i] << " ";
    }
}

int main()
{
    MaxHeap<int> h(30);
    h.insert(3);
    h.insert(1);
    h.insert(1);
    h.insert(7);
    h.insert(5);
    h.insert(6);

    while (true)
    {
        h.print();
        std::cout << "\n" << h.remove() << "\n\n";
    }

    return 0;
}