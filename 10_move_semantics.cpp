#include <iostream>
#include <vector>

namespace _1_idea_of_move_semantics_and_std_move {
    template <typename T>
    void swap(T& x, T& y) {
        T t = std::move(x); // возвращает объект, который можно перемещать бесплатно
        x = std::move(y);
        y = std::move(t);
    }
}

namespace _2_move_semantics_support {
    struct String {
        char* str;
        size_t sz, cap;

        String(String&& s): str(s.str), sz(s.sz), cap(s.cap) { // rvalue-reference
            s.str = nullptr, s.sz = s.cap = 0; // нужно занулить move-нутый объект
        }

        String& operator=(String&& s) noexcept {
            String copy = std::move(s);
            swap(copy);
            return *this;
        }

        void swap(String& s) {
            std::swap(str, s.str);
            std::swap(sz, s.sz);
            std::swap(cap, s.cap);
        }

        // The Rule of Five -> копирование, присваивание, move-копирование, move-присваивание, деструктор
    };
}

namespace _3_move_implementation {
    template <typename T>
    std::remove_reference_t<T>&& move(T&& x) noexcept {
        return static_cast<std::remove_reference_t<T>&&>(x); // в принципе, логично
    }
}

namespace _4_formal_definitions_of_lvalue_and_rvalue {
    // на (l/r)value делятся не типы, а выражения
    /*
             lvalue          |          rvalue
    --------------------------------------------------------
          переменные x       |   литералы 5 (кроме "abc")
      a = b, a *= b, a &= b  |  a+b, a % b, a || b, a <= b
        разыменовывание *a   |      взятие адреса &a
            ++a, --a         |          a++, a--
       ( ? lvalue : lvalue)  |    ( ? хотя бы 1 rvalue)
          ( , lvalue)        |         ( , rvalue)
         ???_cast<T&>()      |      ???_cast<T/T&&>()
            T& f() {}        |         T/T&& f() {}
    */
}

namespace _5_rvalue_references_and_their_properties {
    void f(const int&) { // годится всегда
        std::cout << 1 << std::endl;
    }

    void f(int&&) { // при вызове от rvalue попадём сюда
        std::cout << 2 << std::endl;
    }

    void f(int&) { // а от lvalue сюда
        std::cout << 3 << std::endl;
    }

    void main() {
        int x = 0;
        int& y_ = x; // сюда можно присваивать только lvalue-expressions
        // {int&& y = x} -> CE
        int&& y__ = x+1; // можно присваивать rvalue-expressions

        f(y__); // выведет 3, потому что y -> lvalue (!) и не нужен cast
        f(y_); // и здесь тоже 3
        f((const int&)x); // тут будет 1, так как lvalue и константный
        f(y__+1), f(0); // а здесь, соответственно, выдаст 2
    }
}

// move говорит компилятору - воспринимай этот объект, как временный, rvalue, можешь портить его
// а потом компилятор вызывает соответствующую версию функции, которая может быть оптимальнее

namespace _5__reference_qualifiers {
    struct S {
        void f() const & { // нужно вернуть копию данных из объекта
            std::cout << "lvalue" << std::endl;
        }

        void f() && { // можно тупо move-нуть их
            std::cout << "rvalue" << std::endl;
        }
    };

    void main() {
        S s;
        s.f(); // работает, если вызывается от lvalue выражения
        S().f(); // а здесь выражение уже rvalue
    }
}

namespace _6_perfect_forwarding_and_universal_references {
    template <typename T>
    void bad_move(T& x) {} // не работал бы при bad_move(1);

    template <typename... Args>
    void construct(Args&&... args); // здесь это НЕ rvalue
    // если Args -> шаблонный параметр именно текущей функции, по Args&& можно принимать любую ссылку
    // причём const Args&& или vector<Args>&& под это уже не попадают

    template <typename T>
    class Allocator {
        template <typename... Args>
        void construct(T* ptr, Args&&... args) { // тип всех Args-ов выведется, как & либо &&, из того, какой был
            new (ptr) T(std::forward<Args>(args)...);
            // forward вернёт по объекту rvalue, если он передался, как rvalue, и lvalue иначе
        }
    };

    template <typename T>
    T&& forward_bad1(T&& x) {} // тогда при принятии бывшего rvalue мы принимаем lvalue x, как rvalue, беда

    template <typename T>
    T&& forward_bad2(T& x) {} // а тут в функцию нельзя будет принять rvalue

    template <typename T>
    T&& forward(std::remove_reference_t<T>& x) { // и здесь тип будет правильный, примется lvalue
        return static_cast<T&&>(x);
    }

    template <typename T>
    T&& forward(std::remove_reference_t<T>&& x) { // для аргументов rvalue, (если тип тоже rvalue)
        static_assert(std::is_lvalue_reference_v<T>);
        return static_cast<T&&>(x);
    }
}

namespace _7_xvalues_RVO_copy_elision {
    // rvalue делится на xvalue и prvalue, а lvalue и xvalue образуют вместе glvalue

    // xvalue -> это {static_cast<T&&>(...)} и {T&& f(...)}, всё остальное prvalue
    // то есть, то, что было lvalue, но после испортилось (например, было move-нуто)
    // соответственно, prvalue - это штука, не хранимая в памяти, а промежуточный результат операции

    std::string f(std::string) { // попробует сделать copy-elision, иначе обязан сделать move-конструктор
        std::string s;
        return s; // Named RVO
    }

    std::string g(const std::string& a, const std::string& b) { // в чистом виде copy-elision
        return a+b; // просто RVO
    }

    void main() {
        std::string s = std::string(); // не вызовется move-конструктор!
        f(std::string(std::string())); // аналогично, конструируется только 1 раз
        // происходит copy elision -> компилятор гарантированно оптимизирует цепочки prvalue
        // то есть, если мы вызываем prvalue-expression от prvalue, он создаёт только внешний объект
        f(std::string(std::vector<std::string>(1)[0])); // но здесь не соптимизируется
        // temporary materialization -> каст prvalue к xvalue (и создание реального объекта)
        // происходит при передаче prvalue в функцию, инициализация им переменной, вызова у него метода...
        f("1"), g("1", "2");
    }
}

namespace _8_vector_move_semantics {
    // теперь в векторе мы можем не копировать значения, а делать move
    // но если кинулось exception, у нас беда, поэтому вызываем std::move_if_noexcept(value);

    // если у всех типов поддержана move-семантика, можно тупо написать {void push_back(T value)}
    // по значению - lvalue скопируется и move-нется, а rvalue 2 раза move-нется

    void main() {
        std::vector<std::string> v(4, "abc");
        v.push_back(v[3]); // надо при реаллокации сначала переложить последний элемент, а потом уже остальные
        // иначе мы положим в конец ссылку на уже пустой move-нутый объект
    }
}

int main() {
    return 0;
}
