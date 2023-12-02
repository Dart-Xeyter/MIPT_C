#include <bits/stdc++.h>

namespace _1_unique_ptr {
    template<typename T, typename Deleter = std::default_delete<T>>
    class unique_ptr {
        T *ptr;
        [[no_unique_address]] Deleter deleter; // говорит не выделять память под deleter, если получится
    public:
        unique_ptr(T *ptr) : ptr(ptr) {}

        unique_ptr(const unique_ptr&) = delete; // копировать нельзя

        unique_ptr(unique_ptr&& other) noexcept: ptr(other.ptr) {
            other.ptr = nullptr; // можно переносить, но прошлый должен отвязаться
        }

        ~unique_ptr() {
            deleter(ptr); // мы можем делать что-то интеллектуальнее {delete ptr;}
        }
    };
}

namespace _2_shared_ptr {
    template <typename T>
    class shared_ptr {
        T* ptr;
        size_t* count; // храним их кол-во, общее для всех shared_ptr на этот ptr, но разное для разных ptr

        shared_ptr(T* ptr): ptr(ptr), count(new size_t(1)) {}

        shared_ptr(const shared_ptr& other): ptr(ptr), count(other.count) {
            ++*count;
        }

        shared_ptr(shared_ptr&& other): ptr(ptr), count(other.count) {
            other.ptr = other.count = nullptr;
        }

        ~shared_ptr() {
            if (ptr == nullptr) {
                return;
            }
            if (--*count == 0) {
                delete ptr;
                delete count;
            }
        }
    };
}

namespace _3_make_unique_and_make_shared {
    template <typename T>
    class shared_ptr {
        template <typename... Args>
        struct ControlBlock { // храним 2 вариант создания - на месте
            T object;
            size_t count = 1;
        };
        // и храним, как создали, чтобы понимать, что удалять в деструкторе
    };

    template <typename T, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) { // мы хотим уметь создавать указатель сразу внутри shared_ptr
        auto cptr = new shared_ptr<T>::ControlBlock(std::forward<T>(args)...);
        return shared_ptr<T>(cptr);
    }

    void main() {
        // {f(shared_ptr<int>(new size_t(1)), g())};
        // мы можем выделить память, потом пойти выполнять g и словить исключение -> memory leak
    }
}

namespace _4_weak_ptr {
    // у нас может быть какой-то цикл из указателей, которые не удалятся, когда нужно
    // поэтому введём указатель, не влияющий на удаляемость объекта (и не удаляющий его с собой)
    template <typename T>
    class weak_ptr {
        bool expired() {} // проверяет, жив ли ещё объект (shared_count != 0)
        // чтобы проверить нельзя вызывать *count -> он мог быть удалён
        // поэтому храним ещё weak_count, в котором число weak-ссылок соответственно

        ~weak_ptr() {
            // если weak_count = 0, то удаляем ControlBlock и count-ы, но не T
        }
    };
}

namespace _5_enable_shared_from_this {
    template <typename T>
    class enable_shared_from_this;

    template <typename T>
    class shared_ptr {
        T* ptr;
    public:
        shared_ptr(T* ptr): ptr(ptr) { // в конструкторе if-аем этот случай
            if constexpr (std::is_base_of_v<enable_shared_from_this<T>, T>) {
                ptr->wptr = *this;
            }
        }
    };

    // Curiously Recursive Template Pattern
    template <typename T>
    class enable_shared_from_this {
        std::weak_ptr<T> ptr; // храним указатель на себя, не влияющий на удаление объекта
    public:
        std::shared_ptr<T> shared_from_this() const {
            return ptr.lock(); // вернёт shared_ptr от этого weak_ptr
        }
    };

    struct S: public std::enable_shared_from_this<S> { // придётся унаследоваться от вспомогательного класса
        std::shared_ptr<S> get_ptr() { // нельзя сделать shared_ptr<S>(*this) -> удалит лишний раз
            return shared_from_this();
        }
    };
}

int main() {
    return 0;
}
