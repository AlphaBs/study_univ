#include "CircularChain.h"
#include "iostream"

template <class T>
ChainNode<T>* ChainList<T>::createNode(T data)
{
    ChainNode<T>* node = new ChainNode<T>();
    node->data = data;
    node->link = NULL;
    return node;
}

template <class T>
ChainList<T>::ChainList()
{
    // dummy node
    last = createNode(0);
    last->link = last;
}

template <class T>
int ChainList<T>::getSize() const
{
    return size;
}

template <class T>
void ChainList<T>::print() const
{
    ChainNode<T>* node = last;
    while ((node = node->link) != last)
    {
        std::cout << node->data << std::endl;
    }
}

template <class T>
void ChainList<T>::insertBack(T data)
{
    ChainNode<T> *node = createNode(data);
    node->link = last->link;
    last->link = node;
    size++;
}

template <class T>
void ChainList<T>::insertAfter(ChainNode<T>& from, T data)
{
    ChainNode<T> *link = from.link;
    ChainNode<T> *newNode = createNode(data);
    from.link = newNode;
    newNode->link = link;
    size++;
}

template <class T>
ChainNode<T>* ChainList<T>::getLast() const
{
    // last -> dummy
    if (last->link == last)
        return NULL;
    return last->link;
}

template <class T>
T ChainList<T>::getData(ChainNode<T>& node) const
{
    if (node == last)
        throw "it's dummy";
    return node.data;
}

template <class T>
void ChainList<T>::setData(ChainNode<T>& node, T data)
{
    if (node == last)
        throw "it's dummy";
    node.data = data;
}

template <class T>
void ChainList<T>::deleteNode(ChainNode<T>& node, ChainNode<T>& previous)
{
    if (previous.link != node)
        throw "not previous";
    previous.link = node.link;
    delete node;
    size--;
}

int main()
{
    ChainList<int>* list = new ChainList<int>();
    list->insertBack(1);
    list->insertBack(2);
    list->insertBack(3);
    list->insertBack(4);
    list->insertBack(5);
    list->print();

    return 0;
}