#include "Queue.h"
#include "iostream"

template <typename T>
Queue<T>::Queue(int capacity) : capacity(capacity)
{
    queue = new T[capacity];
    front = rear = 0;
}

template <typename T>
Queue<T>::~Queue()
{
    delete [] queue;
}

template <typename T>
T& Queue<T>::Front()
{
    if (IsEmpty())
        throw "Queue is empty";

    return queue[(front+1) % capacity];
}

template <typename T>
T& Queue<T>::Rear()
{
    if (IsEmpty())
        throw "Queue is empty";

    return queue[rear];
}

template <typename T>
void Queue<T>::Enqueue(const T& data)
{
    if (Size() == capacity - 1)
        Expand();

    rear = (rear + 1) % capacity;
    queue[rear] = data;
}

template <typename T>
T& Queue<T>::Dequeue()
{
    if (IsEmpty())
        throw "Queue is empty";
    front = (front + 1) % capacity;
    return queue[front];
}

template <typename T>
bool Queue<T>::IsEmpty()
{
    return front == rear;
}

template <typename T>
int Queue<T>::Size()
{
    if (front > rear)
        return rear + capacity - front;
    else
        return rear - front;
}

template <typename T>
void Queue<T>::Expand()
{
    std::cout << "expanded" << std::endl;
    if (IsEmpty())
        return;

    T* newArray = new int[capacity * 2];
    int newIndex = 0;
    if (front < rear)
    {
        for (int i = front; i < rear; i++)
        {
            newArray[newIndex++] = queue[i];
        }
    }
    else
    {
        for (int i = front; i < capacity; i++)
        {
            newArray[newIndex++] = queue[i];
        }
        for (int i = 0; i < rear; i++)
        {
            newArray[newIndex++] = queue[i];
        }
    }

    capacity = capacity * 2;
    delete [] queue;
    queue = newArray;

    front = 0;
    rear = newIndex;
}

int main()
{
    Queue<int> q(1);
    q.Enqueue(1);
    q.Enqueue(2);
    std::cout << q.Dequeue() << std::endl;
    std::cout <<q.Dequeue() << std::endl;
    for (int i = 0; i < 10; i++)
    {
        q.Enqueue(i);
        std::cout << q.Dequeue() << std::endl;
    }

    return 0;
}