#pragma once

template <class T>
class TreeNode
{
public:
    TreeNode(T data);
    TreeNode<T>* left;
    TreeNode<T>* right;
    T data;
};