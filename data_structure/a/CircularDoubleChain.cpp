#include "iostream"
#include "CircularDoubleChain.h"

template <class T>
void CircularDoubleChain<T>::push(T data)
{
    DoubleChainNode* node = new DoubleChainNode();
    node->data = data;

    if (first == NULL)
    {
        first = node;
        node->left = node;
        node->right = node;
    }
    else
    {
        
    }
}

template <class T>
T CircularDoubleChain<T>::pop()
{

}

int main()
{
    return 0;
}