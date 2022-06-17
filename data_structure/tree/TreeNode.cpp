#include "TreeNode.h"

template <class T>
TreeNode<T>::TreeNode(T data)
{
    this->data = data;
    this->left = nullptr;
    this->right = nullptr;
}