template <typename T>
class Queue
{
private:
    int front;
    int rear;
    int capacity;
    T* queue;
    void Expand();

public:
    Queue(int capacity);
    ~Queue();

    void Print();
    T& Front();
    T& Rear();
    void Enqueue(const T& data);
    T& Dequeue();
    bool IsEmpty();
    int Size();
};