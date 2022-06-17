// circular linked list

template <class T>
class ChainNode 
{
    ChainNode<T>* link;
    T data;
};

template <class T>
class ChainList 
{
public:
    ChainList();

    void print() const;
    int getSize() const;

    // create
    void insertBack(T data);
    void insertAfter(ChainNode<T>& from, T data);

    // read
    ChainNode<T>* getLast() const;
    T getData(ChainNode<T>& node) const;

    // update
    void setData(ChainNode<T>& node, T data);

    // delete
    void deleteNode(ChainNode<T>& node, ChainNode<T>& previous);

private:
    int size;
    ChainNode<T>* last;
    ChainNode<T>* createNode(T data);
};
