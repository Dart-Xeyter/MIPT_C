#include<iostream>
#include<map>
#include<vector>

using std::cout, std::endl;

namespace _1_Idea_of_templates_and_basic_examples {
    template<typename T> // говорит: создай такую функцию для ВСЕХ типов
    T max(T x, T y) {
        return (x > y ? x : y);
    }

    template<typename T>
    void swap(T& x, T& y) { // происходит "type deduction"
        T t = x;
        x = y;
        y = t;
    }

    template<typename T, typename K>
    void print_first(T t, K& k) {
        cout << t << endl;
    }

    template<typename T>
    class Vector { // шаблонный класс
    public:
        T* array;
        size_t sz, cap;

        template<typename U>
        void func(U);
    };

    template <typename T>
    template <typename U>
    void Vector<T>::func(U) {
        cout << 179179 << endl;
    }

    template<typename T>
    using Mymap = std::map<T, T>;

    template<typename T>
    T f() {
        return 3.14;
    }

    template<typename T>
    const T pi = f<T>();

    void main() {
        int x = 1, y = 2;
        swap(x, y); // выведется swap от int& и int&
        std::string s = "tt";
        print_first(x, s), print_first(s, x); // сработает от разных

        Vector<int> v{nullptr, 1, 179};
        cout << v.cap << endl;
        Mymap<int> m; // можно и так :)

        cout << pi<int> << endl; // шаблонная переменная
    }
}

namespace _2_Template_functions_overloading {
    template<typename T>
    void f(T x) {
        cout << 1 << endl;
    }

    void f(int x) {
        cout << 2 << endl;
    }

    template<typename T, typename U = T> // можно типы по умолчанию
    void func(T x, U y = 3) {}

    void main() {
        f(3); // частное предпочтительнее общего
        f(2U); // общее предпочтительнее каста
        f<int>(3); // без вариантов
    }
}

namespace _3_Specialization_of_templates {
    template <typename T>
    struct Vector {
        T* arr;
        size_t sz;
        size_t cap;
    };

    template <> // специализация - для bool (и только его) вызовется это
    struct Vector<bool> {
        char* arr;
    };

    template <typename T, typename U>
    struct S {
        T x;
        U y;
    };

    template <typename T> // частичная специализация, реально эти 2 структуры независимы
    struct S<T, T> {
        T x;
    };

    template <typename T, typename U> // зайдёт, если условия на типы выполнены
    struct S<const T, U&> {
        //to do
    };

    template <typename T, typename U> // общий случай
    void f(T, U) {
        cout << 1 << endl;
    }

    template <> // а это специализация (потому что <>)
    void f(int, int) {
        cout << 3 << endl;
    }

    template <typename T> // это перегрузка f
    void f(T, T) {
        cout << 2 << endl;
    }

    void main() {
        f(1, 2); // выведет 2 (!)
        // потому что специализация подцепляется к ближайшей сверху
        // сначала в конкурсе выигрывает (T, T), а затем уже смотрят, есть ли у (T, U) специализация
    }
}

namespace _4_Non_type_template_parameters {
    template <int N> // должно быть вычислимой при компиляции константой
    void f(int x) {
        cout << x+N << endl;
    }

    // {template<double K>} -> не корректно, можно от целочисленных типов и всё

    template <typename T, size_t N> // массив фиксированного размера
    struct Array {
        T* arr[N];
    };

    template <template <typename, typename> class Container> // я могу создать шаблон от шаблона...
    class Stack {
        Container<int, std::allocator<int>> c;
    };

    void main() {
        f<2>(1); // выведет 3
        const int b = 2;
        f<b>(2); // корректно
        int b1 = 3;
        const int a = b1;
        // {f<a>(3);} -> уже не сработает

        Stack<std::vector> s; // и это сработает
    }
}

namespace _5_Basic_compile_time_computations {
    template <int N>
    struct Fibonacci { // создадутся все Fibonacci<1..10>
        static const int value = Fibonacci<N-1>::value+Fibonacci<N-2>::value;
    };

    template <>
    struct Fibonacci<0> {
        static const int value = 0;
    };

    template <>
    struct Fibonacci<1> {
        static const int value = 1;
    };

    template <int N>
    const int Fib = Fibonacci<N>::value;

    void main() {
        cout << Fib<10> << endl; // выведет 55, за O(n)
    }
}

namespace _6_Dependent_names {
    template <typename T>
    struct S {
        using A = int;
    };

    template <typename T>
    struct S<T*> {
        static const int A = 0;

        template<int N>
        using G = std::array<int, N>;
    };

    template <typename T>
    void f() {
        /*
        S<T>::A * x1; // A может быть и int, и type
        // по умолчанию считается, что число

        std::vector<T>::iterator x2;
        // более того, даже это value, потому что он не знает, кто iterator внутри vector<T>

        S<T>::A<5> x3; // всё равно считает, что value!
        // Потому что ((A < 5) > x3)

        typename S<T>::A<5> x4; // считает, что тип S<T>::A

        typename S<T>::template A<5> x5; // наконец-то работает!
        */
    }

    template <typename T>
    struct Base {
        int x = 0;
    };

    template <typename T>
    struct Derived: Base<T> {
        void f() {
            // {++x;} -> CE!
            // возможно, существует версия Base, где x не объявлён
            ++Base<T>::x; // я прошу его залезть в родителя и взять x оттуда
            ++this->x; // я гарантирую ему, что x есть (на 1 этапе компиляции)
        }
    };

    void main() {
        f<int>(); // будет CE, потому что считает, что A -> value
    }
}

namespace _7_Basic_type_traits {
    template <typename T>
    struct is_pointer { // эта структура - метафункция, поэтому с маленькой
        static const bool value = false;
    };

    template <typename T> // специализация под указатель
    struct is_pointer<T*> {
        static const bool value = true;
    };

    template <typename T>
    const bool is_pointer_v = is_pointer<T>::value;

    template <typename T>
    void make_nullptr(T x) {
        if constexpr (is_pointer_v<T>) {
            x = nullptr; // этот код не будет сгенерирован, если T -> не указатель
        }
    }


    template <typename T>
    struct remove_reference {
        using type = T;
    };

    template <typename T>
    struct remove_reference<T&> {
        using type = T;
    };

    template <typename T> // теперь
    using remove_reference_t = typename remove_reference<T>::type;

    void main() {
        make_nullptr<std::string>("1"); // CE не будет!
        int* x = new int[1];
        make_nullptr<int*>(x); // указатель обнулится :)
    }
}

namespace _8_Variadic_templates {
    void print() {} // когда аргументы кончатся, вызовет это

    template <typename Head, typename... Ts>
    void print(Head head, Ts... args) { // распаковал пакет типов в пакет аргументов
        size_t len = sizeof...(args); // число аргументов
        cout << head << endl;
        print(args...); // передаём всё остальное назад
    }

    void main() {
        print(1, "123", 3.14, 1222); // всё выведет
        const int x = 1;
        static_assert(x == 1); // выдаст CE, если не выполнено
    }
}

namespace _9_Fold_expressions {
    template <typename... Tail>
    auto sum(const Tail&... tail) {
        static_assert((std::is_same_v<Tail, int> && ...)); // проверяет, что все int
        return (tail+...); // он распаковывает это в (tail_1 + (tail_2 + (... + tail_n)))
    }

    template <typename... Types>
    auto print(const Types&... types) {
        ((cout << types << ' '), ...); // бинарный оператор ","; перечисляет такие конструкции
    }

    template <int... Numbers>
    struct Check {
        static_assert((Numbers+...) == 10); // проверяет, что их сумма -> 10
    };

    void main() {
        cout << sum(1, 2, 3, 4) << endl;
        print("ab", 3.0, 't');
        // {Check<1, 1>} -> будет CE
    }
}

int main() {
    _3_Specialization_of_templates::main();
    return 0;
}
