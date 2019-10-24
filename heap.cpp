#include <unordered_map>
#include <vector>

#include "heap.h"

using namespace heap;

Node::Node(int x, int key) :
    // Содержимое узла.
    x(x),
    // Ключ
    key(key),
    // Предок узла
    parent(NULL),
    // Левый братский / сестринский узел
    left(NULL),
    // Правый братский / сестринский узел
    right(NULL),
    // Прямой потомок узла
    child(NULL),
    // Ранг узла = кол-во прямых потомков
    rank(0),
    // Перемещались ли ранее потомки этого узла
    marked(false) {}

void Node::_extract() {
    /*
    Удаление связей перед переносом узла.
    */
    parent = NULL;
    left = NULL;
    right = NULL;
}

FibonacciHeap::FibonacciHeap() :
    min_node(NULL) {}

FibonacciHeap::FibonacciHeap(Node* node) {
    /*
    Создание новой фибоначчиевой кучи

    Время работы: O(1)
    */
    min_node = node;
}

void FibonacciHeap::insert(Node* node) {
    /*
    Вставка узла node в список корневых узлов.

    Время работы: O(1)
    */
    FibonacciHeap* h2 = new FibonacciHeap();
    h2->_set_min(node);
    meld(h2);
    h2->_set_min(NULL);
    delete h2;
}

void FibonacciHeap::_set_min(Node* node) {
    /*
    Установка минимального узла.

    Время работы: O(1)
    */
    min_node = node;
}

void FibonacciHeap::_update_min(Node* node) {
    /*
    Обновление минимального узла, если ключ меньше.

    Время работы: O(1)
    */
    Node* current = find_min();
    if (current == NULL) {
        _set_min(node);
    }
    else if ((node != NULL) && (node->key <= current->key)) {
        _set_min(node);
    }
}

Node* FibonacciHeap::find_min() {
    /*
    Поиск минимального узла.

    Время работы: O(1)
    */
    return min_node;
}

void FibonacciHeap::meld(FibonacciHeap* h) {
    /*
    Объединение двух фибоначчиевых куч.

    Время работы: O(1)
    */
    Node* node1 = find_min();
    Node* node2 = h->find_min();
    //Склеивание двух двусвязных списков (колец)
    //x - удаляемая связь
    //left1 <-x node1 -> right1
    //      X
    //left2 <-x node2 -> right2

    //Добавляемая куча пуста
    if (node2 == NULL) {
        return;
    }

    //Исходная куча пуста
    if (node1 == NULL) {
        _set_min(node2);
        return;
    }

    //Поскольку список двусвязный кольцевой, то если есть левый узел,
    //то существует правый (равен левому или другому)
    //Если в списке 1 элемент, то он не указывает сам на себя = None
    Node* left1 = node1->left;
    Node* left2 = node2->left;

    //В исходной куче 1 корневой узел
    if (left1 == NULL) {
        if (left2 != NULL) {
            //По левому узлу второй кучи
            //     node1
            //  |        |
            //left2 <-x node2
            node1->left = left2;
            node1->right = node2;
            left2->right = node1;
            node2->left = node1;
        }
        else {
            //В обеих кучах 1 корневой узел
            //node1
            //  |
            //node2
            node1->left = node2;
            node1->right = node2;
            node2->left = node1;
            node2->right = node1;
        }
    }
    else {
        //Склеиваем через левый корневой узел второй кучи
        if (left2 != NULL) {
            //left1 <-x node1
            //       X
            //left2 <-x node2
            //наискосок
            left1->right = node2;
            node1->left = left2;
            left2->right = node1;
            node2->left = left1;
        }
        //Во второй куче 1 корневой узел
        else {
            //left1 <-x node1
            //  |        |
            //     node2
            node2->left = left1;
            node2->right = node1;
            left1->right = node2;
            node1->left = node2;
        }
    }

    //Если нужно, обновляем минимум
    _update_min(node2);
}

Node* FibonacciHeap::delete_min() {
    /*
    Извлечение минимального узла.

        x
        / | \
    c1 c2 c3
    Амортизированное время работы: O(log n)
    */
    Node* root = find_min();
    if (root == NULL) {
        throw "Куча пуста";
    }
    //Устанавливаем временно минимальный узел на левый
    _set_min(root->left);
    //Удаляем из списка минимальный узел
    _unlink(root);
    //Создаем новую кучу из потомков root (у них прежний parent)
    FibonacciHeap* h = new FibonacciHeap(root->child);
    meld(h);
    h->_set_min(NULL);
    delete h;
    _consolidate();
    root->_extract();
    root->child = NULL;
    return root;
}

Node* FibonacciHeap::_unlink(Node* node) {
    /*
    Извлечение узла из двухсвязного списка.

    Возвращает левый узел из оставшихся в списке, либо None
    left - node - right = left - right
    Время работы: O(1)
    */
    Node* left = node->left;
    Node* right = node->right;

    //В списке 1 элемент - удаляемый
    if (left == NULL) {
        return NULL;
    }

    if (left == right) {
        //В списке было 2 элемента
        left->left = NULL;
        left->right = NULL;
    }
    else {
        left->right = right;
        right->left = left;
    }

    return left;
}

void FibonacciHeap::_consolidate() {
    /*
    Уплотнение списка корней - склеивание деревьев с одинаковым рангом.

    Обновляет минимальный узел
    и устанавливает parent=None для всех корневых узлов
    Время работы: O(log n)
    */
    //временный минимальный узел
    Node* root = find_min();
    if (root == NULL) {
        return;
    }

    //Словарь корневых узлов вида ранг -> узел
    std::unordered_map<int, Node*> ranked;
    ranked[root->rank] = root;
    root->parent = NULL;
    Node* node = root->right;

    while (node != NULL) {
        //У корня нет предков
        node->parent = NULL;
        //Текущий узел
        Node* melded = node;
        //Следующий просматриваемый узел
        node = node->right;
        auto got = ranked.find(node->rank);
        if ((got != ranked.end()) && (got->second == node)) {
            //Мы там уже были, поэтому эта итерация последняя
            node = NULL;
        }

        while (ranked.find(melded->rank) != ranked.end()) {
            //В списке корней есть дерево с таким же рангом.
            int rank = melded->rank;
            //Склеиваем
            melded = _link(melded, ranked[rank]);
            //и удаляем из словаря прежний ранг
            ranked.erase(rank);
        }
        //обновляем с новым значением ранга получившееся дерево
        ranked[melded->rank] = melded;
        //Обновляем минимальный узел
        _update_min(melded);
    }
}

Node* FibonacciHeap::_link(Node* _node1, Node* _node2) {
    /*
    Склеивание двух корней.

    Корнем становится узел с меньшим ключом, второй - его потомком
    Возвращает получившийся корень
    Время работы: O(1)
    */
    Node* node1 = _node1;
    Node* node2 = _node2;
    if (_node1->key > _node2->key) {
        node1 = _node2;
        node2 = _node1;
    }
    //node1              node1
    //  |    ->            |
    //child      node2 - child

    //node2 извлекается из списка корней
    _unlink(node2);
    node2->_extract();
    //убирается отметка
    node2->marked = false;
    //и он становится потомком node1
    node2->parent = node1;
    //Обновляем ранг получившегося дерева
    node1->rank++;

    //Потомок первого корня
    Node* child = node1->child;
    if (child == NULL) {
        //Если нет потомков
        node1->child = node2;
    }
    else {
        Node* left = child->left;
        if (left == NULL) {
            //Один потомок
            //child - node2
            child->left = node2;
            child->right = node2;
            node2->left = child;
            node2->right = child;
        }
        else {
            //left <-x child
            //  |        |
            //     node2
            node2->left = left;
            node2->right = child;
            left->right = node2;
            child->left = node2;
        }
    }

    return node1;
}

void FibonacciHeap::decrease_key(Node* node, int newkey) {
    /*
    Уменьшение ключа узла node до значения newkey.

    Время работы: O(1)
    */
    if (newkey >= node->key) {
        throw "Новый ключ не меньше";
    }
    node->key = newkey;

    if (node->parent == NULL) {
        //Узел - корневой
        _update_min(node);
        return;
    }

    Node* parent = node->parent;
    parent->rank--;
    parent->child = _unlink(node);
    _cascading_cut(parent);
    node->_extract();
    insert(node);
}

void FibonacciHeap::_cut(Node* node) {
    /*
    Подрезка дерева - перенос node в список корней.

    Время работы: O(1)
    */
    if (node == NULL) {
        throw "Пустой узел";
    }
    Node* parent = node->parent;
    if (parent == NULL) {
        //Узел уже корневой
        return;
    }
    parent->rank--;
    parent->child = _unlink(node);
    node->_extract();
    insert(node);
}

void FibonacciHeap::_cascading_cut(Node* node) {
    /*
    Каскадная подрезка дерева.

    Начиная от узла node, и пока предшествующий узел имеет отметку
    о перемещении (marked = True), все они становятся корневыми.

    Время работы: O(log n)
    */
    Node* parent = node;
    while (parent != NULL) {
        if (!parent->marked) {
            parent->marked = true;
            return;
        }
        else {
            node = parent;
            parent = node->parent;
            _cut(node);
        }
    }
}

Node* FibonacciHeap::delete_node(Node* node) {
    /*
    Удаление узла node

    Амортизированное время работы: O(log n)
    */
    if (node == find_min()) {
        //Узел - минимальный
        return delete_min();
    }
    Node* parent = node->parent;
    if (parent == NULL) {
        //Узел - корневой
        _unlink(node);
    }
    else {
        parent->rank--;
        parent->child = _unlink(node);
        _cascading_cut(parent);
    }

    FibonacciHeap* h = new FibonacciHeap(node->child);
    meld(h);
    h->_set_min(NULL);
    delete h;
    _consolidate();
    node->_extract();
    node->child = NULL;
    return node;
}

void FibonacciHeap::push(int x, int key) {
    Node* node = new Node(x, key);
    insert(node);
}

void FibonacciHeap::pop() {
    Node* node = delete_min();
    if (node == NULL) {
        throw "Куча пуста";
    }
    delete node;
}

int FibonacciHeap::top() {
    Node* node = find_min();
    if (node == NULL) {
        throw "Куча пуста";
    }
    return node->x;
}

FibonacciHeap::~FibonacciHeap() {
    if (min_node == NULL) {
        return;
    }
    std::vector<Node*> v {min_node};

    while (!v.empty()) {
        Node* node = v.back();
        v.pop_back();
        if (node == NULL) {
            continue;
        }
        Node* right = node->right;
        while (right != node) {
            v.push_back(right->child);
            Node* node_next = right->right;
            delete right;
            right = node_next;
        }
        v.push_back(node->child);
        delete node;
    }
}
