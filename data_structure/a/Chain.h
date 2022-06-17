template <class T>
class Chain
{
public:
    Chain();

private:
    class Node 
    {
        friend class Chain;
        private:
        T data;
        Node* left;
        Node* right;
    };

    Node* head;
    Node* tail;
};