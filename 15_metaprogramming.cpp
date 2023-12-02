#include <iostream>
#include <vector>
#include <functional>

using std::cout, std::endl;

namespace _1_lazy_and_explicit_instantiation {
    //to do
}

namespace _2_basic_primitives_for_template_metaprogramming {
    template <typename T>
    struct type_identity {
        using type = T;
    };

    template <typename T>
    using type_identity_t = typename type_identity<T>::type;

    template <bool B, typename T, typename F>
    struct conditional {
        using type = F;
    };

    template <typename T, typename F>
    struct conditional<true, T, F> {
        using type = T;
    };

    template <typename T, T v>
    struct integral_constant {
        static const T value = v; // константа времени компиляции
    };

    struct always_false: integral_constant<bool, false> {}; // и так используется

    template <typename... Types>
    struct conjunction {
        static const bool value = (Types::value && ...); // правильно, но не лениво (можно conditional)
    };

    using false_type = std::bool_constant<false>;
}

namespace _3_SFINAE_and_enable_if {
    template <typename T>
    auto f(T x) -> typename T::value_type {
        return x[0];
    }

    int f(...) {
        return 0;
    }

    template <typename T>
    auto g(T x) -> decltype(x(), 0) { // comma trick
        return 1; // если x не callable -> g не вызовется
    }

    int g(...) {
        return 0;
    }

    template <bool B, typename T>
    struct enable_if {}; // если B -> false, то нет поля "type"

    template <typename T>
    struct enable_if<true, T>: std::type_identity<T> {}; // иначе оно есть

    template <bool B, typename T = void>
    using enable_if_t = typename enable_if<B, T>::type;

    template <typename T, std::enable_if_t<std::is_invocable_v<T>, bool> = true>
    int h(T x) { // если is_invocable, то он найдёт type, и всё будет норм
        return 1; // если же нет -> type нет, ошибка, и эта версия не рассматривается
    }

    template <typename T, std::enable_if_t<!std::is_invocable_v<T>, bool> = true>
    int h(T x) {
        return 0;
    }

    void main() {
        std::vector<int> v = {2};
        cout << f(v) << endl; // всё нормально, вернёт 2
        cout << f(1) << endl; // понимает, что объявление (!) f не корректно -> нужно идти в f(...)

        std::function<void()> func = [](){};
        cout << g(func) << endl; // это 1
        cout << g(1) << endl; // а это 0
        cout << h(func) << endl;
        cout << h(1) << endl;
        // работает так же
    }
}

namespace _4_compile_time_check_of_method_presence_in_a_class {
    /*template <typename T, typename... Args>
    constexpr decltype(T().construct(Args()...), bool()) has_method_construct_helper(int) {
        return true;
    }*/
    // так плохо, потому что может быть приватный конструктор

    template <typename T>
    T declval(); // так плохо, потому что T может быть не создаваем, и будет CE

    template <typename T>
    T&& declval(); // единственная проблема -> void&& не определено

    template <typename T>
    std::add_rvalue_reference<T> declval(); // if-ает случай void, и всё нормально

    template <typename T, typename... Args>
    constexpr decltype(std::declval<T>().construct(std::declval<Args>()...), bool()) has_method_construct_helper(int) {
        return true;
    }

    template <typename T, typename... Args>
    constexpr bool has_method_construct_helper(...) { // зайдёт, только если зафейлился
        return false;
    }

    template <typename T, typename... Args>
    struct has_method_construct: std::bool_constant<has_method_construct_helper<T, Args...>(0)> {};

    template <typename T, typename... Args>
    constexpr bool has_method_construct_v = has_method_construct<T, Args...>::value;

    struct Test {
        void construct(int, int) {}
    private:
        Test() {}
    };

    void main() {
        cout << has_method_construct_v<Test, int> << endl;
        cout << has_method_construct_v<Test, int, int> << endl;
    }
}

namespace _5_is_nothrow_move_or_move_if_noexcept {
    /*template <typename T>
    constexpr bool is_nothrow_move_constructible = std::is_move_constructible<T>() && noexcept(T(declval<T&&>()));*/
    // плохо, так как если нету T(T&&), у нас CE во второй части выражения

    template <typename T>
    constexpr decltype(T(std::declval<T>()), bool()) is_nothrow_move_constructible_helper(int) {
        return noexcept(T(std::declval<T>())); // зато можно сделать так
    }

    template <typename T> // если не noexcept-move, но нет copy, то всё равно move
    std::conditional_t<!std::is_nothrow_constructible_v<T> && std::is_copy_constructible_v<T>, const T&, T&&>
            move_if_noexcept(T& x) noexcept {
        return std::move(x);
    }
}

namespace _6_is_base_of {
    template <typename B, typename D> // плохо не указатели, так как иначе может сработать operator к D от B
    constexpr auto is_base_of_helper(B*) -> std::true_type;

    template <typename B, typename D>
    constexpr auto is_base_of_helper(...) -> std::false_type;

    template <typename B, typename D> // declval<D*>() плохо, ведь это не компайл-тайм
    auto test_is_base_of(int) -> decltype(is_base_of_helper<B, D>(static_cast<D*>(nullptr)));
    // если мы не наследник, выберется версия с ..., иначе (если публичный единственный) -> с B*

    template <typename B, typename D> // если мы приватный или множественный наследник
    // он пойдёт в верхнюю функцию, вызовет helper(B*) и схватит CE после выбора версии, и сработает SFINAE в test
    auto test_is_base_of(...) -> std::true_type;

    template <typename B, typename D>
    struct is_base_of: std::conjunction<std::is_class<B>, std::is_class<D>,
            decltype(test_is_base_of<B, D>(0))> {};

    template <typename B, typename D>
    constexpr bool is_base_of_v = is_base_of<B, D>::value;

    struct Base {};

    struct Derived1: public Base {};
    struct Derived2: private Base {};

    struct Son: Derived1, Derived2 {};

    void main() {
        cout << is_base_of_v<Derived1, Base> << endl;
        cout << is_base_of_v<Base, Derived1> << endl;
        cout << is_base_of_v<Base, Derived2> << endl;
        cout << is_base_of_v<Base, Son> << endl;
    }
}

namespace _7_common_type {
    template <typename... Types>
    struct common_type;

    template <typename First, typename Second, typename... Tail>
    struct common_type<First, Second, Tail...>: std::type_identity<common_type<First, typename common_type<Second, Tail...>::type>> {};

    template <typename First, typename Second> // ? от выражений && вернёт тоже ссылку, которая нам не нужна
    struct common_type<First, Second>: std::type_identity<std::remove_reference_t<decltype(true ? std::declval<First>() : std::declval<Second>())>> {};

    template <typename T>
    struct common_type<T>: std::type_identity<T> {};

    struct Granny {};

    struct Mother: Granny {};

    void main() {
        static_assert(std::is_same_v<common_type<Mother, Granny>::type, Granny>);
    }
}

namespace _8_concepts {
    struct S {};

    template <typename T>
    bool f() {
        bool b = requires(T x) {++x;}; // проверяет, что в скобках компилируемое выражение
        return b;
    }

    template <typename T>
    concept Polymorphic = std::is_polymorphic_v<T>; // вид типов

    template <typename T>
    concept InputIterator = requires(T x) {*x; ++x;}; // задаются условиями на них или тем, что они умеют

    template <typename T>
    concept RandomAccessIterator_ = requires(T x) {*x; ++x; x += 179;}; // не поймёт, что частный случай

    template <typename T>
    concept RandomAccessIterator = InputIterator<T> && requires(T x) {x += 179;}; // а вот так норм

    template <InputIterator Iter>
    void f(Iter iter, int n) { // можно принимать только некоторый определённый вид типов
        for (int q = 0; q < n; q++) {
            ++iter;
        }
    }

    template <RandomAccessIterator Iter>
    void f(Iter iter, int n) { // можно перегружать
        iter += n;
    }

    void f(Polymorphic auto iter, int n) {} // как {auto iter;} -> но с проверкой, что Polymorphic

    int main() {
        std::cout << f<int>() << endl;
        std::cout << f<S>() << endl;
        return 0;
    }
}

int main() {
    _6_is_base_of::main();
    return 0;
}
