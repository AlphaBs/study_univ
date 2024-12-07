template <typename T>
class Stack
{
private:
    T *stack;
    int top;
    int capacity;
public:
    Stack(int capacity);
    ~Stack();

    void Push(const T& data);
    T& Pop();
    T& Peek();
    int Length();
};