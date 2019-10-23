namespace heap {
    class Node;
    class FibonacciHeap;
}

class heap::Node {
 public:
    Node(int x, int key);
    void _extract();

    int x;
    int key;
    Node* parent;
    Node* left;
    Node* right;
    Node* child;
    int rank;
    bool marked;
};

class heap::FibonacciHeap {
    /*
    Фибоначчиева куча.
    */
 public:
    FibonacciHeap();
    FibonacciHeap(Node* node);
    void insert(Node* node);
    Node* find_min();
    void meld(FibonacciHeap* h);
    Node* delete_min();
    void decrease_key(Node* node, int newkey);
    Node* delete_node(Node* node);

    bool empty() { return min_node == NULL; }

 protected:
    void _set_min(Node* node);
    void _update_min(Node* node);
    Node* _unlink(Node* node);
    void _consolidate();
    Node* _link(Node* node1, Node* node2);
    void _cut(Node* node);
    void _cascading_cut(Node* node);

 private:
    Node* min_node;
};
