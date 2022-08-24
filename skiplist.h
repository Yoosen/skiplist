#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;		// 临界区互斥锁
std::string delimiter = ":";

// 节点类
template<typename K, typename V>
class Node {
public:
    Node();

    Node(K k, V v, int);

    ~Node();

    K get_key() const;	// 不允许更改 class 内成员

    V get_value() const;

    void set_value(V);

    // 保存指向下一个不同级别节点的指针的线性数组
    Node<K, V> **forward;

    int node_level;

private:
    K key;
    V value;
};


// 节点构造函数，level是该节点再跳表中有多少层，创建节点时进行随机分配
template<typename K, typename V>
Node<K, V>::Node(const K k, const V v, int level) {
    this->key = k;
    this->value = v;
    this->node_level = level;

    this->forward = new Node<K, V>*[level + 1];
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
};

// 析构
template<typename K, typename V>
Node<K, V>::~Node() {
    delete []forward;
};

// 获取 key
template<typename K, typename V>
K Node<K, V>::get_key() const {
    return key;
};

// 获取 value
template<typename K, typename V>
K Node<K, V>::get_value() const {
    return value;
};

// 设置 value
template<typename K, typename V>
void Node<K, V>::set_value(V value) {
    thie->value = value;
}

// 跳表类定义
template<typename K, typename V>
class Skiplist {
public:
    Skiplist(int);
    ~Skiplist();

    int get_random_level();
    Node<K, V>* create_node(K, V, int);
    int insert_element(K, V);
    void display_list();
    bool search_element(K);
    void delete_element(K);
    void dump_file();
    void load_file();
    int size();

private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string* value);
    bool is_valid_string(const std::string& str);

private:
    int _max_level;	// 最大层数

    int _skip_list_level;	// 当前层数

    Node<K, V>* _header;	// 头节点

    std::ofstream _file_writer;
    std::ifstream _file_reader;

    int _element_count;		// 跳表当前元素数
};

// 跳表类构造函数
template<typename K, typename V>
Skiplist<K, V>::Skiplist(int max_level) {
    this->_max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;

    // 创建跳表的头节点初始化 key 和 value 为 null
    K k;
    V v;
    this->_header = new Node<K, V>(k, v, max_level);
};

// 直接 new 一个 node ，返回指针
template<typename K, typename V>
Node<K, V>* Skiplist<K, V>::create_node(const K k, const V v, int level) {
    Node<K, V>* n = new Node<K, V>(k, v, level);
    return n;
}

// 插入节点
// 如在以下跳表中插入 key 为 50 的节点：
//                            +------------+
//                            |  insert 50 |
//                            +------------+
// level 4     +-->1+                                                      100
//                  |
//                  |                      insert +----+
// level 3         1+-------->10+---------------> | 50 |          70       100
//                                                |    |
//                                                |    |
// level 2         1          10         30       | 50 |          70       100
//                                                |    |
//                                                |    |
// level 1         1    4     10         30       | 50 |          70       100
//                                                |    |
//                                                |    |
// level 0         1    4   9 10         30   40  | 50 |  60      70       100

// 需要再每层中找到插入的位置，即每层插入位置的上一个节点，
// 该节点的 key 小于插入节点，下一个节点的 key 大于插入节点
// 这里定义了一个数组 update 存放插入位置的上一个节点

template<typename K, typename V>
int Skiplist<K, V>::insert_element(const K key, const V value) {
    mtx.lock();

    Node<K, V> *current = this->_header;

    // update[i]是第 i 层中 key 最后一个比插入 key 小的 node*
    Node<K, V> *update[_max_level + 1];

    //从最高层搜索填补update
    for(int i = _skip_list_level; i >= 0; --i) {
        which(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }

    //在上图示例中，如插入key 50, 在每层中，得到它的上一节点的key依次为40,30,30,10,1
    // 第 0 层， current->forward[0]为应该插入的位置
    current = current->forward[0];

    // 该 key 已存在，解锁后直接返回
    if(current != NULL && current->get_key() == key) {
        std::cout << "key: " << key << ", exists" << std::endl;
        mtx.unlock();
        return 1;
    }

    // current 为空，表示到达了该层的末尾
    // 不为空则要在 update[0] 和 current 之间插入
    if(current == NULL || current->get_key() != key) {

        // 随机层数
        int random_level = get_random_level();

        // 如果随即层数比当前的层数高时，比当前层高的前一个节点就是 _header
        if(random_level > _skip_list_level) {
            for(int i = _skip_list_level + 1; i < random_level + 1; ++i) {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }

        // 创建节点
        Node<K, V>* inserted_node = create_node(key, value, random_level);

        // 插入节点
        // 1.对每一层，插入节点的下一个节点为 update[i] 的下一个节点
        // 2.update[i] 的下一节点更新为插入节点
        for(int i = 0; i < random_level; ++i) {
            inserted_node->forward[i] = update[i]->forward[i];
            update[i].forward[i] = inserted_node;
        }

        std::cout << "Successfully inserted key:" << key << ", value:" << std::endl;
        _element_count++;	// 增加节点数
    }

    mtx.unlock();
    return 0;
}

/* 搜索节点
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/

// 从顶层的 header 开始， 从上而下， 从左到右，依次遍历每层的节点 key，直到 0层


template<typename K, typename V>
bool Skiplist<K, V>::search_element(K, key) {
    std::cout << "search_element------------" << std::endl;

    Node<K, V> *current = _header;

    // 从最高层开始
    for(int i = _skip_list_level; i >= 0; --i) {
        while(current->forward[i] && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
    }

    current = current->forward[0];

    if(current and current->get_key() == key) {
        std::cout << "Found key: " << key << ", value: " << current->get_value() << std::endl;
        return true;
    }

    std::cout << "Not Found Key:" << key << std::endl;
    return false;
}

// 删除元素
// 先找到每层中该节点位置的前一节点
// 若某层中该key存在，前一节点的下一节点指向该元素的下一节点
template<typename K, typename V>
void Skiplist<K, V>::delete_element(K key) {
    mtx.lock();
    Node<K, V> *current = this->_header;
    Node<K, V> *update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    for(int i = _skip_list_level; i >= 0; --i) {
        // 找到当前层最后一个小于需删除的 key 的 key
        while(current->forward[i] != NULL && current->forward[i]->get_key() < key) {
            current = current->forward[i];
        }
        // 记录层级
        update[i] = current;
    }

    // 到最底层
    current = current->forward[0];

    // 存在该节点
    if(current != NULL && current->get_key() == key) {
        // 最底层，删除每级的当前节点
        for(int i = 0; i < _skip_list_level; ++i) {
            //如果该层没有该节点，跳过
            if(update[i]->forward[i] != current)
                break;

            update[i]->forward[i] = current->forward[i];
        }

        // 删除没有元素的层级
        while(_skip_list_level > 0 && _header->forward[_skip_list_level] == 0) {
            _skip_list_level--;
        }

        std::cout << "Successfully deleted key " << key << std::endl;
        _element_count--;
    }
    mtx.unlock();
    return ;
}


// 展示跳表
template<typename K, typename V>
void Skiplist<K, V>::display_list() {

    std::cout << "\n*****Skip List*****" << "\n";
    for(int i = 0; i < _skip_list_level; ++i) {
        // 让 node 指向头节点的第 i 层
        Node<K, V> *node = this->_header->forward[i];
        std::cout << "Level " << i << ": ";

        // 遍历第 i 层的所有节点
        while(node != NULL) {
            std::cout << node->get_key() << ": " << node->get_value() << ";";
            node = node->forward[i];
        }
    }
}

// 将内存中的数据转储到文件
template<typename K, typename V>
void Skiplist<K, V>::dump_file() {

    std::cout << "dump_file--------------" << std::endl;
    _file_writer.open(STORE_FILE);
    Node<K, V> *node = this->_header->forward[0];	// 第 0 层

    while(node != NULL) {
        _file_writer << node->get_key() << ":" << node->get_value() << "\n";
        std::cout << node->get_key() << ":" << node->get_value() << ";\n";
    }

    _file_writer.flush();
    _file_writer.close();

    return ;
}

// 从磁盘加载数据
template<typename K, typename V>
void Skiplist<K, V>::load_file() {

    _file_reader.open(STORE_FILE);
    std::cout << "load_file----------" << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();

    while(getline(_file_reader, line)) {
        get_key_value_from_string(line, key, value);
        if(key->empty() || value->empty()) {
            continue;
        }
        insert_element(*key, *value);
        std::cout << "key:" << *key << "value:" << *value << std::endl;
    }
    _file_reader.close();
}

// 跳表大小
template<typename K, typename V>
int Skiplist<K, V>::size() {
    return _element_count;
}

template<typename K, typename V>
void Skiplist<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value) {
    if(!is_valid_string(str)) return;

    *key = str.substr(0, str.find(delimiter));
    *value = str.substr(str.find(delimiter) + 1, str.length());
}

template<typename K, typename V>
bool Skiplist<K, V>::is_valid_string(const std::string& str) {
    if(str.empty()) {
        return false;
    }
    if(str.find(delimiter) == str::string::npos) {
        return false;
    }
    return true;
}

// 析构
template<typename K, typename V>
Skiplist<K, V>::~Skiplist() {
	if(_file_reader.is_open()) {
		_file_reader.close();
	}
	if(_file_writer.is_open()) {
		_file_writer.close();
	}

	delete _header;
}

template<typename K, typename V>
int SkipList<K, V>::get_random_level(){

    int k = 1;
    while (rand() % 2) {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
};