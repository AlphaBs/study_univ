#include "Stack.h"
#include "iostream"

template <typename T>
Stack<T>::Stack(int capacity) : capacity(capacity)
{
    if (capacity < 1)
        throw "capacity must be > 0";
    stack = new T[capacity];
    top = -1;
}

template <typename T>
Stack<T>::~Stack()
{
    delete [] stack;
}

template <typename T>
void Stack<T>::Push(const T& data)
{
    if (top >= capacity - 1)
        throw "the stack is full";
    
    top++;
    stack[top] = data;
}

template <typename T>
T& Stack<T>::Pop()
{
    T& data = Peek();
    top--;
    return data;
}

template <typename T>
T& Stack<T>::Peek()
{
    if (top < 0)
        throw "the stack is empty";
    
    return stack[top];
}

template <typename T>
int Stack<T>::Length()
{
    return top + 1;
}