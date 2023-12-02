#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <scoped_allocator>

namespace _1_Idea_of_allocators {
    // чтобы выделить память, Container вызывает allocator, который сам решает, использовать ли new

    template <typename T>
    struct allocator {
        template <typename U>
        allocator(const allocator<U>&) {}
        // для List и т.п. нужно уметь конструироваться, присваиваться и т.д. от других типов

        T* allocate(size_t n) { // выделить n памяти
            return reinterpret_cast<T*>(new char[n*sizeof(T)]);
        }

        void deallocate(T* ptr, size_t) { // освободить её (принимать размер выделенной обязательно)
            delete[] reinterpret_cast<char*>(ptr);
        }

        template <typename... Args>
        void construct(T* ptr, const Args&... args) { // вызвать placement-new
            new (ptr) T(args...);
        }

        void destroy(T* ptr) { // уничтожить объект по указателю явно
            ptr->~T();
        }

        template <typename U>
        struct rebind { // можем получить такой же аллокатор, но от другого типа
            using other = allocator<U>;
        };
    };

    template <typename T, typename Alloc = std::allocator<T>>
    class Vector {
        Alloc alloc_; // в контейнере хранится сам аллокатор, а не указатель/ссылка на него
        // и дальше отдаём тупую работу с памятью под его ответственность
    };

    template <typename T, typename Alloc = allocator<T>>
    class List {
        struct Node {};
        // нам нужно выделять Node-ы, а не T-шки, поэтому делаем так
        using NodeAllocator = typename Alloc::template rebind<Node>::other;
        NodeAllocator node_alloc;

        List(Alloc alloc): node_alloc(alloc) {}
    };

    template <typename T>
    struct My_allocator {
        using value_type = T; // нужно для allocator_traits

        T* buffer;

        My_allocator(size_t n) {} // делаю что-то полезное

        T* allocate(size_t n) {}
        void deallocate(T* ptr, size_t) {}
        template <typename... Args>
        void construct(T* ptr, const Args&... args) {} // вызвать placement-new
        void destroy(T* ptr) {} // уничтожить объект по указателю явно
    };

    void main() {
        My_allocator<int> alloc(179);
        std::vector<int, My_allocator<int>> my_map(alloc); // и теперь он с моим аллокатором
    }
}

namespace _2_Allocator_traits {
    // можно не писать в аллокаторе некоторые методы явно
    template <typename Alloc>
    struct allocator_traits {
        // аналогично с destroy и rebind
        template <typename... Args>
        static void construct(const Alloc& alloc, typename Alloc::value_type* ptr, Args... args) {
            if constexpr (true) { // проверяем, что construct определён нами
                alloc.construct(ptr, args...);
            } else {
                new (ptr) typename Alloc::value_type(args...);
            }
        }
        // и в контейнере надо писать не allocator::construct, а allocator_traits::allocator::construct
    };
}

namespace _3_Allocator_Aware_Containers {
    // при копировании контейнера аллокатор должен сам решать, копироваться ему или делать что-то умнее

    void select_on_container_copy_construction() {} // при копировании мы вызываем это из allocator_traits
    // и если реализовано, вызовется оно, а если нет -> просто конструктор копирования

    bool propagate_on_container_copy_assignment; // и нужно ли заменять аллокатор при присваивании
    // поэтому при копировании и присваивании allocator обязан быть noexcept
}

namespace _4_new_delete_overloading {
    struct Logger {
        Logger() {
            std::cout << "Hi" << std::endl;
        }

        void* operator new(size_t n) {
            std::cout << "get " << n << std::endl;
            void* p = malloc(n);
            if (p == nullptr) {
                auto x = std::get_new_handler(); // пытается выделить память, возвращает указатель на функцию
                if (x == nullptr) {
                    throw std::bad_alloc();
                }
                // её нужно вызвать, если nullptr - кидать исключение
            }
            // и вызывать handler до посинения
            return p;
            // он выделяет память, а строится на ней структура автоматически
        }

        void operator delete(void* p) noexcept {
            // до неё вызывает деструкторы по всем структурам
            std::cout << "usual del" << std::endl;
            free(p);
        }

        void* operator new(size_t n, int x, const char* ha) { // я могу определить свою реализацию
            return new void*[n];
        }

        void operator delete(void* p, int x, const char* ha) {
            std::cout << "unusual del" << std::endl;
        } // тогда нужен такой же delete

        // перед ним НЕ вызываются деструкторы структур
        void operator delete(Logger* p, std::destroying_delete_t) {}

        ~Logger() {
            std::cout << "By" << std::endl;
        }
    };

    void main() {
        Logger* me = new Logger;

        Logger* ptr = new(3, "hohoho") Logger;
        ptr->operator delete(ptr, 5, "hehe"); // тогда придётся вызывать явно
        // если конструктор кинул исключение после выделения памяти - delete всё равно вызовется
    }
}

namespace _5_alignment_and_bit_fields {
    struct alignas(8) Wow { // могу установить размер выравнивания для структуры (степень 2)
        char x:3; // выделит int на 3 бита (!!!)
        long long y:57;
    };

    void main() {
        // new и malloc выделяют байты, начиная с адреса, кратного max_align = 16
        std::cout << alignof(int) << std::endl; // выведет размер структуры (выравнивания)

        // {align, aligned_alloc} -> выравнивающие память функции
    }
}

namespace _6_scoped_allocator {
    template <typename T>
    using MyAlloc = std::allocator<T>;

    void main() {
        using MyString = std::basic_string<char, std::allocator_traits<char>, MyAlloc<char>>;
        MyAlloc<MyString> alloc;

        std::vector<std::string, MyAlloc<std::string>> v1(alloc);
        // строки будут создаваться с их собственным аллокатором
        std::vector<std::string, std::scoped_allocator_adaptor<MyAlloc<std::string>>> v2(alloc);
        // так он передастся внутрь автоматически
    }
}

int main() {
    return 0;
}
