#include <iostream>
#include <iterator>
#include <vector>

namespace _4_Iterators_Kind_of_iterators_Advance_and_Distance {
    /*
    Iterator - можно делать *r, ++r
    InputIterator - можно читать и перемещаться (istream)
    ForwardIterator - читать одно и то же можно сколько угодно раз (unordered_map)
    BidirectionalIterator - ходить в 2 стороны --r (map)
    RandomAccessIterator - сравнивать итераторы, брать разницу и прибавлять число (deque)
    ContiguousIterator - если получить указатель на элемент, ведёт себя, как итератор (vector)
    */
}

namespace _5_Vector_Iterator {
    template <typename T>
    class Vector {
        T* data;
        int sz;
    private:
        template <typename Value>
        struct base_iterator { // общий для iterator и const_iterator
            // он думает, что мы output
            using difference_type = ptrdiff_t;
            using value_type = Value;
            // нужно ещё вот эту дичь добавить
            using reference = value_type&;
            using iterator_category = std::input_iterator_tag;
            // а лучше и это заодно

            value_type* item;

            T& operator*() {
                return *item;
            }

            base_iterator operator++() {}
        };
    public:
        using const_iterator = base_iterator<T>;
        using iterator = base_iterator<T>;

        iterator begin() {
            return base_iterator<T>{*data};
        }

        iterator end() {
            return base_iterator<T>{*(data+sz)};
        }

        const_iterator cbegin() {
            return begin();
        }

        iterator rbegin() { // *r он делает, как *--this
            return std::reverse_iterator(end());
        }
    };
}

namespace Implementation_of_istream_and_ostream {
    template <typename T>
    class IstreamIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using difference_type = ptrdiff_t;
    private:
        std::istream* m_stream;
        T m_value;
    public:
        IstreamIterator(): m_stream(nullptr) {}

        IstreamIterator(std::istream& istr): m_stream(&istr) { // T обязан иметь конструктор по умолчанию
            readValue();
        }

        bool operator==(const IstreamIterator& other) const {
            return m_stream == other.m_stream;
        }

        void readValue() {
            if (m_stream == nullptr) {
                return;
            }
            if (!(*m_stream >> m_value)) {
                m_stream = nullptr;
            }
        }

        const_reference operator*() const { // хотим, чтобы *iter несколько раз возвращало одно и то же
            return m_value;
        }

        IstreamIterator& operator++() { // сдвиг сразу считывает
            readValue();
            return *this;
        }
    };

    template <typename T>
    class OstreamIterator {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using difference_type = ptrdiff_t;
    private:
        std::ostream* m_stream;
        const char* m_sep;
    public:
        OstreamIterator(): m_stream(nullptr) {}

        OstreamIterator(std::ostream& ostr, const char* sep): m_stream(&ostr), m_sep(sep) {}

        bool operator==(const OstreamIterator& other) const {
            return m_stream == other.m_stream;
        }

        OstreamIterator& operator*() { // {*iter = 'a';} -> сработает!
            return *this;
        }

        OstreamIterator& operator++() { // не понятно, что он делает по смыслу - ничего не делает
            return *this;
        }

        OstreamIterator& operator=(const T& value) { // приравнивание равносильно выводу в поток
            *m_stream << value << m_sep;
            return *this;
        }
    };

    template <typename T>
    class BackInserter {
    public:
        using iterator_category = std::output_iterator_tag;
        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using difference_type = ptrdiff_t;
    private:
        T& m_container;
    public:
        BackInserter(T& container): m_container(container) {}

        BackInserter& operator*() { // {*iter = 'a';} -> сработает!
            return *this;
        }

        BackInserter& operator++() { // не понятно, что он делает по смыслу - ничего не делает
            return *this;
        }

        template <typename Elem>
        BackInserter& operator=(const Elem& value) {
            m_container.push_back(value);
            return *this;
        }
    };

    void main() {
        freopen("../input.txt", "r", stdin);
        std::vector<int> vec;
        // считываю vec
        std::copy(IstreamIterator<int>(std::cin), IstreamIterator<int>{}, BackInserter(vec));
        // и вывожу его
        std::copy(vec.begin(), vec.end(), OstreamIterator<int>(std::cout, " "));
        std::cout << std::endl;
    }
}

int main() {
    Implementation_of_istream_and_ostream::main();
    return 0;
}
