template <class T>
class DoubleChainNode
{
public:
    T data;
    DoubleChainNode<T>* left;
    DoubleChainNode<T>* right;
};

template <class T>
class CircularDoubleChain
{
public:
    void push(T data);
    T pop();
private:
    DoubleChainNode<T>* first;
};