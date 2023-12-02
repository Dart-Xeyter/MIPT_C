#include<iostream>
#include<vector>

using std::cout, std::endl;

namespace _2_Visibility_and_accessibility_inheritance {
    struct Base {
        void f(double) {
            cout << 1 << endl;
        }

        void g(double) {
            cout << 1 << endl;
        }
    };

    struct Derived: Base {
        void f() {
            cout << 2 << endl;
        }

        using Base::g; // теперь g из Base наравне с нашей
        using Base::Base; // теперь можно создать Derived из аргументов Base

        void g(int) {
            cout << 2 << endl;
        }
    };

    void main() {
        Derived d;
        // {d.f(3.14);} -> будет CE! f() перекрывает видимость из Base
        // f оттуда вообще не участвует в конкурсе кандидатов (доступна, но не видна)
        d.Base::f(3.14); // всё нормально
        d.g(3.14); // выведет 1
    }
}

namespace _3_Constructors_and_destructors_inheritance {
    struct Base {
        int x = 0;
        std::vector<int> v;

        Base(int x): x(x) {} // теперь конструктор по умолчанию Derived не работает
    };

    struct Derived: Base {
        std::vector<int> s;
        double d = 3.14;

        using Base::Base; // по умолчанию снова в деле
        Derived(int x, double d): Base(x), d(d) {} // сделает Base, s по умолчанию, затем d
    };

    struct Empty {
        void f() {}
    };

    struct EmptyDerived: Empty {};

    void main() {
        Derived d(1); // вызовет Base::Base
        Derived d1(3, 2.0);

        cout << sizeof(Empty) << endl; // выведет 1 (так как 0 нельзя отличать по адресу)
        cout << sizeof(EmptyDerived) << endl; // выведет 1 (!) Потому что EVO
        // Empty Base Optimization -> не хранить в наследнике пустого родителя
    }
}

namespace _4_Type_conversions_inheritance {
    struct Base {};

    struct Derived: Base {};

    void f1(Base& d) {}

    void f2(Base* d) {}

    void f3(Base d) {} // от Derived будет "срезка"

    void main() {
        Derived d;
        f1(d); // сработает, вызовет часть Base внутри Derived
        f2(&d); // аналогично
        f3(d); // вызовется конструктор копирования Base от Derived-ского Base
        Base& b = d;
        Derived& dd = static_cast<Derived&>(b); // корректно, если b - это ссылка на какой-то Derived
        // если b был реальным Base, то UB
    }
}

namespace _5_Multiple_inheritance {
    struct Granny {
        int g;
    };

    struct Mom: Granny {
        int m;
    };

    struct Dad: Granny {
        int d;
    };

    struct Son: Mom, Dad {
        int s;
    };

    struct Son2: Mom, Granny {}; // inaccessible_Base_class
    // мы не сможем обратиться к Granny, потому что Son2::Granny -> в Mom тоже она есть

    void main() {
        Son s;
        Dad* f = &s; // сдвинет указатель, потому что Son начинается не с Father
        Son* s1 = static_cast<Son*>(f); // и обратно

        // {s.g} -> "diamond problem", из какой бабушки?
        cout << s.Mom::g << endl; // корректно
    }
}

namespace _6_Virtual_inheritance {
    struct Granny {
        int g;
    };

    struct Mom: virtual Granny {
        int m;
    };

    struct Dad: virtual Granny {
        int d;
    };

    struct Son: Mom, Dad { // теперь у Son только 1 Granny
        int s;
    };

    void main() {
        Son s;
        cout << s.g << endl; // работает!
        cout << s.Mom::g << endl; // в начале Mom лежит указатель на Granny
        Mom& m = s;
        Son& me = static_cast<Son&>(m); // всё ещё корректно
        Granny& g = s;
        // {Son& me = static_cast<Son&>(g)} -> нельзя, потому что виртуально, мы не знаем, где что
    }
}

int main() {
    _6_Virtual_inheritance::main();
    return 0;
}
