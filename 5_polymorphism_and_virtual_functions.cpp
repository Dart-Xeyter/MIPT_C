#include<iostream>

using std::cout, std::endl;

namespace Idea_of_virtual_function {
    struct Base { // эта структура "полиморфная", так как есть virtual-метод
        void f() {
            cout << 1 << endl;
        }

        virtual void g() { // данная функция вызывается только от объектов Base (не от наследников)
            cout << 1 << endl;
        }
    };

    struct Derived: Base { // структура наследуется от полиморфной - тоже полиморфна
        void f() {
            cout << 2 << endl;
        }

        void g() {
            cout << 2 << endl;
        }
    };

    int main() {
        Derived d;
        Base& b = d;
        d.f(); // Выведет 2
        b.f(); // Выведет 1, потому что это объект класса Base
        b.g(); // Выведет 2 так как g в Base виртуальная
        Base* pd = &d;
        pd->f(), pd->g(); // работает так же, как со ссылками
        return 0;
    }
}

namespace More_complicated_examples {
    struct Base {
        virtual void f(int x) {
            cout << 1 << endl;
        }

        virtual void g(int x) const {
            cout << 1 << endl;
        }

        virtual void h() {}
    };

    struct Derived: Base {
        void f(int x) override { // проверяет, что был virtual-метод у родителя
            cout << 2 << endl;
        }

        void f(double x) {
            cout << 3 << endl;
        }

        void g(int x) { // уже без const
            cout << 2 << endl;
        }

        void h() final {} // как override, но проверяет, что у наследников она не переопределяется
    };

    int main() {
        Derived d;
        Base& b = d;
        b.f(3.14); // Выведет 2, так как кандидаты при перегрузке - f из Base, но она virtual
        b.Base::f(3.14); // Выведет 1, так как вызовет версию Base
        b.g(3); // Выведет 1, ведь сигнатура g-шек разная
        return 0;
    }
}

namespace Virtual_destructor {
    struct Base {
        virtual ~Base() = default;
    };

    struct Derived: Base {
        int* p;

        Derived(): p(new int()) {}

        ~Derived() {
            delete p;
        }
    };

    int main() {
        Base* b = new Derived();
        delete b; // если нет виртуального деструктора Base, произойдёт утечка памяти
        return 0;
    }
}

namespace Pure_virtual_functions_and_abstract_classes {
    // это абстрактный класс
    struct Shape { // объект данного класса не создать, так как есть pure-virtual метод
        virtual double area() const = 0; // возникает чисто виртуальная функция
    };

    double Shape::area() const { // могу определить дефолтную инициализацию
        return 3.14;
    }

    struct Figure: Shape {}; // он тоже абстрактный, так как area не определено

    struct Circle: Shape { // класс не абстрактный, ведь area реализовано
        double area() const override {
            return Shape::area();
        }
    };
}

namespace RTTI_and_dynamic_cast {
    struct Base {
        virtual void f() {
            cout << 1 << endl;
        }
    };

    struct Derived: Base {};

    struct Granny {
        virtual ~Granny() = default;
    };

    struct Mother: Granny {};

    struct Father: Granny {};

    struct Son: Mother, Father {};

    int main() {
        Derived d;
        Base& b = d;
        cout << (typeid(b) == typeid(Derived)) << endl; // выведет 1, ведь b на самом деле Derived
        cout << typeid(b).name() << endl; // выведет реальный тип b в каком-то формате
        Derived* c = dynamic_cast<Derived*>(&b); // если классы не виртуальны - CE, если b не Derived - nullptr
        // также работает от наследника к родителю
        Son s;
        Mother& m = s;
        Father* f = dynamic_cast<Father*>(&m); // справится, так как m на самом деле s
        // Father f = static_cast<Father>(m); -> будет CE
        Father* not_f = reinterpret_cast<Father*>(&m); // сработает, но неправильно
        return 0;
    }
}

namespace Vtables_and_virtual_functions_mechanist_implementations {
    struct Grand_base {
        int w;
    };

    struct Base: Grand_base {
        int x;

        virtual void f() {}

        void h() {}
    };

    struct Derived: Base {
        int y;

        void f() override {}

        virtual void g() {}
    };

    /*
    В памяти для Derived хранится:
    [ptr_Derived][w][x][y]
    Где в ptr_Derived указатель на vtable - таблицу с виртуальными функциями Derived
    И если написать b.f() -> он перейдёт по ptr_Derived и вызовет f оттуда
    */
}

namespace Virtual_tables_with_multiple_inheritance {
    struct Mom {
        int m;

        virtual void f() {}
    };

    struct Papa {
        int d;

        virtual void f() {}
    };

    struct Son: Mom, Papa {
        int s;

        void f() override {}
    };
    // хранится [ptr1_son][m][ptr2_son][d][s] -> причём ptr-ы разные (хранят сдвиг от начала: top_offset)
    // чтобы можно было понять, корректно ли кастовать от Granny к Papa (ссылка на Son)
    // если же наследование виртуальное - мы ещё храним virtual_offset -> адрес бабушки
}

namespace Issues_with_virtual_functions {
    struct Base {
        virtual void f() = 0;

        virtual void g() {
            cout << 1 << endl;
        }

        void h() {
            f(); // если вызывать из Derived h -> всё равно найдёт нужную
        }

        Base() {
            g();
            // h() -> будет ошибка, так как при построении виртуальности ещё нету
        }

        virtual void func(int x = 1) {
            cout << "Base " << x << endl;
        }
    };

    struct Derived: Base {
        void f() override {
            cout << "Ok" << endl;
        }

        void g() override {
            cout << 2 << endl;
        }

        void func(int x = 2) override {
            cout << "Derived " << x << endl;
        }
    };

    struct Field {
        long long a, b;
    };

    void main() {
        Derived d; // выведет 1 (потому что для него создаётся честный Base)
        Base& b = d;
        b.func(); // выведет "Derived 1", потому что значения по умолчанию проставляются во время компиляции

        void (Derived::* p)() = &Derived::h;
        (d.*p)(); // выведет Ok
        Field f = reinterpret_cast<Field&>(p);
        cout << f.a << endl; // если g обычная, f.a чётное, иначе нечётное
        cout << f.b << endl; // если Derived виртуальный - хранит top_offset для метода g
    }
}

int main() {
    return 0;
}
