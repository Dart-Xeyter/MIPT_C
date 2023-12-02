#include <iostream>
#include <array>
#include <vector>
#include <cassert>
#include <cmath>
#include <utility>

using std::cout, std::endl;

namespace _1_constant_expressions_and_constexpr_functions {
    constexpr int sqr(int x) { // это вычислимая при компиляции функция
        return x*x;
    }

    constexpr bool isPrime(int n) { // for и if можно использовать с C++14
        if (n <= 0) {
            throw 0; // если до этого throw не дойдёт, ОК, иначе CE
        }
        for (int q = 2; q*q <= n; q++) {
            if (n % q == 0) {
                return false;
            }
        }
        return n != 1;
    }

    constexpr int countPrimes() { // можно использовать вот это всё
        int a[5] = {1, 2, 3, 4, 5};
        int res = 0;
        int* ptr = &res;
        for (int* p = a; p < a+5; ++p) {
            *ptr += isPrime(*p);
        }
        // но если где-то внутри UB -> получим CE
        return res;
    }

    struct S {
        int a, b;
        constexpr S(): a(0), b(0) {}
    };

    constexpr int something() {
        S s; // можно
        s.a += 1;
        int* a = new int[100]; // только с C++20
        delete[] a; // и если забыть удалить -> CE (!)
        // {vector<int> v;} -> тоже можно, но пока не поддерживается
        return 3;
    }

    struct Base {
        constexpr virtual int f() {
            return 1;
        }

        constexpr virtual int g() {
            return 1;
        }
    };
    // виртуальные функции тоже отлично работают
    struct Derived: Base {
        constexpr int f() override {
            return 2;
        }

        int g() override { // пока мы не попадём в компайл-тайме сюда, всё нормально
            return 2;
        }
    };

    constexpr int testVirtual(int x) {
        Derived d;
        Base b;
        Base& ref = (x > 0 ? d : b);
        if (ref.f() == 1) {
            throw 0;
        }
        return 1;
    }

    void main() {
        constexpr int x = 5; // нужно (и можно) вычислить на этапе компиляции
        std::array<int, x> a = {1, 2, 3, 4, 5};

        constexpr bool b1 = isPrime(17);
        int b2 = countPrimes();

        constexpr int ss = something();

        constexpr int tt1 = testVirtual(1);
        // {constexpr int tt2 = testVirtual(-1);} -> CE
    }
}

namespace _2_objects_as_non_type_template_parameters {
    template <auto X> // может сам выводить, какой класс передали в template
    struct Test {
        decltype(X) value;
    };

    void main() {
        Test<5> a; // можно делать шаблонным параметров классы
        Test<2.07> b; // только с C++20
    }
}

namespace _3_consteval {
    consteval bool is_prime(int n) { // говорит, что можно вызывать ТОЛЬКО в compile-time
        if (n <= 0) {
            throw 0;
        }
        for (int q = 2; q*q <= n; q++) {
            if (n % q == 0) {
                return false;
            }
        }
        return true;
    }

    constexpr int func(int x) {
        if (std::is_constant_evaluated()) {
            if constexpr (false) {
                // {is_prime(x);} -> нельзя вызывать consteval даже изнутри constexpr
            }
            return 1;
        } else {
            std::cin >> x; // зато можно не constexpr в этой части - до неё не дойдёт при компиляции
            return 2;
        }
    }

    void main() {
        is_prime(1); // гарантированно вычислит в compile-time
        constexpr int a = 1;
        int b;
        std::cin >> b;
        // {is_prime(b);} -> CE
        constexpr int value = func(a);
        cout << value << endl;
        cout << func(b) << endl;
    }
}

namespace _4_static_and_dynamic_initialization {
    constinit int n = 5; // можно заставить инициализировать при компиляции, но не делать константой

    void main() {}
}

namespace _5_basic_reflection {
    // reflection -> узнавание чего-то о программе во время выполнения
    // {А есть ли у этого класса поле с именем "t"?}
}

namespace _6_value_based_metaprogramming {
    template <typename...>
    struct TypeList {};

    template <typename... Ts>
    constexpr size_t size(TypeList<Ts...>) {
        return sizeof...(Ts);
    }

    template <typename... Ts, typename... Us>
    constexpr bool operator==(TypeList<Ts...>, TypeList<Us...>) {
        return false;
    }

    template <typename... Ts>
    constexpr bool operator==(TypeList<Ts...>, TypeList<Ts...>) {
        return true;
    }

    template <typename Head, typename... Tail>
    constexpr auto pop_front(TypeList<Head, Tail...>) {
        return TypeList<Tail...>();
    }

    template <typename T, typename... Ts>
    constexpr auto push_front(TypeList<Ts...>) {
        return TypeList<T, Ts...>();
    }

    template <typename T, typename... Ts>
    constexpr size_t find(TypeList<Ts...>) {
        bool b[] = {std::is_same_v<T, Ts>...};
        return std::find(b, b+sizeof...(Ts), true)-b;
    }

    template <template <typename> typename F, typename... Ts>
    constexpr size_t find(TypeList<Ts...>) {
        bool b[] = {F<Ts>::value...};
        return std::find(b, b+sizeof...(Ts), true)-b;
    }

    template <typename T>
    struct get_impl;

    template <size_t... Ind>
    struct get_impl<std::index_sequence<Ind...>> {
        template <typename T>
        static constexpr T dummy(decltype(Ind, (void*) nullptr)..., T*, ...);
    };

    template <size_t Ind, typename... Ts>
    constexpr auto get(TypeList<Ts...>) {
        return std::type_identity<decltype(get_impl<std::make_index_sequence<Ind>>::dummy( (Ts*)nullptr...))>();
    }

    template <typename U, typename V>
    constexpr bool operator==(U, V) {
        return false;
    }

    template <typename U>
    constexpr bool operator==(U, U) {
        return true;
    }

    void main() {
        static_assert(size(TypeList<int, double, char>()) == 3);
        static_assert(TypeList<int, double>() == TypeList<int, double>());
        static_assert(pop_front(TypeList<int, std::string>()) == TypeList<std::string>());
        static_assert(push_front<double>(TypeList<int, char>()) == TypeList<double, int, char>());
        static_assert(find<double>(TypeList<int, double, char>()) == 1);
        static_assert(find<std::is_pointer>(TypeList<int, double, char*>()) == 2);
        static_assert(get<1>(TypeList<int, double>()) == std::type_identity<double>());
    }
}

int main() {
    return 0;
}
