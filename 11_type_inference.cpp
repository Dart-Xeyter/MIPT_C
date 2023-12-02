#include <iostream>
#include <utility>
#include <array>
#include <tuple>
#include <functional>

namespace _1_auto {
    template <typename T>
    void f1(T x) {}
    void f2(auto x) {} // данная конструкция эквивалентна f_T

    void print(auto type) {} // допустим, выводит тип аргумента
    void print_c(const auto type) {}
    void print_t(auto&& type) {} // особая конструкция, сама определяет, lvalue или rvalue
    // если передать туда lvalue, будет "T&", если rvalue -> "T&&"

    inline auto rec(int i) { // пишем inline, чтобы можно было в хедере определять, и не было CE
        if (false) {
            return 0; // мы вывели тип здесь
        }
        if (i > 0) {
            return rec(i-1)+1; // если бы не return сверху, тип понять мы бы не смогли
        }
        return 0;
    }

    template <bool Flag>
    auto function() { // хотя типы разные, он разнесёт их в разные функции раньше
        if constexpr (Flag) {
            return "0";
        } else {
            return 0;
        }
    }

    void main() {
        print(5); // выведет "int"
        const int* const ptr = nullptr;
        print(ptr); // будет "const int*" -> снимает верхний const
        int a[5]; // тип имеет int[5]
        print(a); // но скажет "int*", потому что массив в функцию передаётся так

        print_c(ptr); // "const int* const" -> снова сожрёт const, но сам навесит
        // аналогично убираются в auto и ссылки

        print_t(1); // скажет "int&&"
        const int y = 1;
        print_t(y); // выведет "const int&" -> lvalue и тип остался
    }
}

namespace _2_decltype {
    // decltype() возвращает ровно тот тип, который имеет переменная или выражение
    decltype(auto) f(const int x) { // теперь const-ы и ссылки не отбрасываются
        return x;
    }

    auto g(auto x) -> decltype(auto); // может вывести тип возврата в зависимости от типа x

    void main() {
        int a[5];
        decltype(a) b; // сделает b типа a -> в точности "int[5]"

        int x = 5;
        decltype((x)) y = x; // если было lvalue-expression, получим "int&"
        decltype((1)) z; // а если rvalue -> либо "int", либо "int&&"
    }
}

namespace _3_class_argument_deduction {
    template <typename First, typename Second>
    struct Pair {
        First first;
        Second second;
    };

    template <typename U, typename V>
    Pair(U, V) -> Pair<U, V>; // помогает делать deduction type

    template <typename T>
    struct Vector {
        template <typename Iter>
        Vector(Iter begin, Iter end) {} // хотим уметь конструироваться по любым итераторам
    };

    template <typename Iter> // теперь нам подойдёт хоть List::iterator, хоть Map, хоть...
    Vector(Iter, Iter) -> Vector<typename std::iterator_traits<Iter>::value_type>;


    void main() {
        std::pair<int, double> pr;
        pr = std::make_pair(1, 2.0); // помогает тебе создавать данный объект без указания типов
        std::array arr = {3, 4}; // выведет шаблоны автоматически
        // {std::array<int> arr2 = {3};} -> но частичный вывод шаблонов невозможен

        Pair my = {3, 1.0}; // благодаря выводу типов это работает автоматически

        std::tuple<int, int, double> tpl = {1, 1, 3.17}; // можно сложить что угодно
        std::cout << std::get<1>(tpl) << std::endl; // доступ к 1 аргументу

        int x = 2;
        int& y = x;
        // {std::tuple<int&> t = std::make_tuple(y);} -> так нельзя с ссылками
        std::reference_wrapper<int> z = x;
        std::tuple<int&> t = std::make_tuple(z); // а так можно...
        std::tuple<int&, int&&> tt = std::forward_as_tuple(y, 1); // и ещё так

        auto [dd1, dd2, dd3] = tpl; // можно распаковать tuple, pair
        auto [arr1, arr2] = arr; // а также array, type[]
        auto [first, second] = my; // и некоторые user-defined types
    }
}

struct Wow {
    int a[3];

    Wow(int x, int y, int z) {
        a[0] = x, a[1] = y, a[2] = z;
    }
};

template <>
struct std::tuple_size<Wow> {
    const static size_t value = 3; // пишем длину распаковываемой штуки
};

template <size_t index>
struct std::tuple_element<index, Wow> {
    using type = int; // здесь её тип
};

template <size_t index>
typename std::tuple_element<index, Wow>::type& get(Wow &p) {
    return p.a[index]; // а тут, как получать index-овый элемент
}

int main() {
    Wow w(3, 1, 4);
    auto& [w1, w2, w3] = w; // и так можно распаковывать свою структуру
    w1 = 5;
    std::cout << w.a[0] << ' ' << w2 << ' ' << w3 << std::endl;
    return 0;
}
