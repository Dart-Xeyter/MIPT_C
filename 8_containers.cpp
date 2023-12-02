#include <iostream>
#include <iterator>
#include <vector>
#include <map>
#include <forward_list>

namespace _2_Implementation_of_vector {
    template <typename T>
    class Vector {
        T* begin_;
        T* end_;
        T* buffer_end;
    public:
        size_t size() const noexcept {
            return end_-begin_;
        }

        size_t capacity() const noexcept {
            return buffer_end-begin_;
        }

        Vector(size_t count, const T& value = T()):
                begin_(reinterpret_cast<T*>(new char[count*sizeof(T)])),
                end_(begin_+count), buffer_end(begin_+count) {
            // только ещё память нужно класть по адресу, кратному размеру Vector
            T* it = begin_;
            try {
                for (; it != end_; ++it) {
                    // {*it = value;} -> неверно!
                    // На сырой памяти вызовет конструктор копирования
                    new (it) T(value); // вызывает конструктор данного типа по переданному адресу
                }
            } catch (...) {
                for (T* delit; delit != it; ++delit) {
                    delit->~T();
                }
                delete[] reinterpret_cast<char*>(begin_);
                throw;
            }
        }
    };
}

namespace _3_Deque {}

namespace _4_5_Iterators {}

namespace _6_Implementation_of_list {
    template <typename T>
    class List {
        struct BaseNode { // чтобы не искать в
            BaseNode* prev;
            BaseNode* next;
        };

        struct Node: BaseNode {
            T value;
        };

        BaseNode fake = BaseNode{nullptr, nullptr}; // нужно, как end()
        size_t sz = 0;

        struct base_iterator {
            BaseNode* ptr;
        };
    };
}

namespace _7_Implementation_of_map {
    const std::map<int, int> a = {{1, 1}};
    // a[key] = value; -> CE
    // потому что a[] возвращает value&
    int s = a.at(1); // так можно

    template <typename Key, typename Value, typename Cmp> // нам нужна exception-safety относительно Cmp
    // но это не проблема, так как Cmp выполняется до поворотов
    class map {
        struct BaseNode {
            BaseNode* l;
            BaseNode* r;
            BaseNode* p;
            bool red;
        };

        struct Node: BaseNode {
            std::pair<const Key, Value> kv; // чтобы нельзя было разыменовать и изменить, сломав map
            // из-за "const" во время {for (p &q : map)} всё равно происходит копирование...
        };

        BaseNode fake; // левый сын - наше дерево, а end() -> фейковый корень
    };
}

namespace _8_Implementation_of_unordered_map {
    // вычисляем для каждого объекта hash - число-индекс в массиве
    // открытая адресация -> ищем свободное место многократным применением hash-функции
    // метод цепочек -> храним в ячейке связный список ключей с таким хешем

    template <typename Key, typename Value, typename Hash, typename Equal>
    class unordered_map {
        struct Node {
            std::pair<const Key, Value> kv;
            int hash; // храним текущий хеш, чтобы понимать, когда кончился локальный forward_list
        };

        struct iterator {};

        size_t sz, num_buckets; // одна ячейка-список в методе цепочек называется "bucket"
        double max_load_factor = 1.0; // заполненность, при достижении которой выполняется rebuild

        double load_factor() { // текущая заполненность unordered_map-ы
            return (double)sz/num_buckets;
        }

        // проблема - обход контейнера должен выполняться за O(sz), а не за O(num_buckets)
        std::forward_list<Node> elements; // поэтому храним всё в глобальном списке
        std::vector<iterator> buckets; // а здесь поддерживаем, где начинается соответствующий forward_list
        // нужно уметь делать erase (и, соответственно, находить предыдущую вершину)
        // для этого храним в buckets указатель на последнюю вершину предыдущего bucket, а не на первую нашего

        void rehash() {} // увеличивает num_buckets, лучше не просто *2, чтобы коллизии не сохранять
    };
}

int main() {
    return 0;
}
