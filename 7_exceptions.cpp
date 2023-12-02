#include<iostream>
#include<memory>

using std::cout, std::endl;

namespace _1_Idea_and_basic_examples {
    void f3() {
        throw std::string("179"); // выбрасывается из функции, уничтожая все переменные
    }

    void g() {
        f3(); // продолжает полёт рекурсивно
    }

    void f2() {
        try {
            g(); // пока не будет внутри try
        } catch (std::string& s) { // check, что подходит
            cout << "string " << s << endl;
            throw 179;
        }
    }

    void f1() {
        try {
            f2();
        } catch (char c) {
            cout << "char " << c << endl;
        } catch (...) { // ловит всё!
            cout << "all" << endl;
        }
    }

    void main() {
        f1();
    }
}

namespace _2_Difference_between_exceptions_and_RE {
    // Not every RE is an exception!!!

    void main() {
        int* x = new int[1]; // может кинуть исключение
        int* y = new(std::nothrow) int[2]; // если плохо, вернёт nullptr
        cout << /* 1/0 */' ' << endl; // это не С++ исключение, а исключение операционки
    }
}

namespace _3_Going_deeper_into_exception_throwing_mechanism {
    struct S {
        int field = 0;
    };

    void f() {
        int x;
        std::cin >> x;
        if (x % 2 == 0) {
            throw S{1};
        }
    }

    struct Base {};
    struct Derived: Base {};

    void f_base() {
        throw Base();
    }

    void f_int() {
        const int x = 1;
        throw x;
    }

    struct Granny {};
    struct Father: Granny {};
    struct Mother: Granny {};
    struct Son: Father, Mother {};

    void main() {
        try {
            try {
                f();
            } catch (S s) { // скопируется
                s.field++;
                throw s;
            }
        } catch (const S& s) { // не скопируется
            cout << s.field << endl;
        }

        try {
            f_base();
        } catch (Base&) {
            cout << "caught" << endl; // умеет ловить, если исключение - наследник
        }

        try {
            f_int();
        } catch (unsigned) { // не поймает, ведь есть каст
            cout << "unsigned" << endl;
        } catch (int& x) { // но константность снимается!
            cout << "int" << endl;
        }

        try {
            try {
                Son s;
                throw s;
            } catch (Granny&) { // не поймает! Потому что не понятно, какую бабушку
                cout << 1 << endl;
            } catch (Son &s) {
                throw; // продолжи полёт текущего исключения без копирования
            }
        } catch (...) {}

        try {
            Son s;
            throw s;
        } catch (Mother&) { // зайдёт сюда
            cout << "mother" << endl;
            throw;
        } catch (Son&) { // дальше проверять не будет
            cout << "son" << endl;
        }
    }
}

namespace _4_RAII_idiom_and_constructors_exceptions {
    // Resource Acquisition is Initialization
    int g(const int* p) {
        if (*p == -1) {
            throw std::runtime_error("Boom!");
        }
        return *p;
    }

    int f(int x) {
        int* p = new int(x);
        int ans = g(p);
        delete p;
        return ans;
    }

    template <typename T>
    struct SmartPtr {
        T* p;
        SmartPtr(T* p): p(p) {}
        ~SmartPtr() {
            delete p;
        }
    };

    int save_f(int x) {
        SmartPtr<int> p(new int(x)); // копирование не понятно, так как будет 2 delete
        std::unique_ptr<int> p1(new int(x)); // нельзя копировать
        std::shared_ptr<int> p2(new int(x)); // можно, хранит счётчик кол-ва
        int ans = g(p.p);
        return ans;
    }

    struct Except {
        std::shared_ptr<int> p;

        Except(int x): p(new int(x)) {
            if (x == 0) {
                throw std::runtime_error("Boom!"); // вызовутся деструкторы полей
            }
        }
    };

    struct Exceptional {
        int* p;

        Exceptional(int x): p(new int(x)) {
            if (x == 0) {
                throw std::runtime_error("Boom!"); // но деструктор Exceptional не вызовется
            }
        }

        ~Exceptional() {
            delete p;
        }
    };

    struct Ex_dest {
        int* p;

        Ex_dest(int x): p(new int(x)) {}

        ~Ex_dest() {
            /*if (*p == 0) {
                throw 2;
            }*/
            // всё умрёт, если уже летит исключение
            if (*p == 0 && !std::uncaught_exceptions()) { // проверяет, есть ли актуальное исключение
                throw std::runtime_error("Oh!"); // всё равно бан за исключение в деструкторе
            }
            delete p;
        }
    };

    void func(int x) {
        Ex_dest ex(0);
        if (x == 0) {
            throw 1; // начнётся уничтожение ex
        }
    }

    int g1(int x) try { // оборачивает функцию в try-catch
        if (x == 0) {
            throw 1;
        }
        return x+1;
    } catch (...) { // в итоге всё равно бросает exception после выхода из g1
        cout << "No :(" << endl;
        return x+1;
    }

    struct EX {
        int* p;
        int x;

        EX() try: p(new int(1)), x(0) {
            cout << "Yes!" << endl;
            throw 1;
        } catch (...) {
            cout << "NO :(" << endl;
            delete p;
        }
    };

    void main() {
        try {
            f(0); // всё нормально
            f(-1); // oops, утечка памяти...
            save_f(-1); // а вот здесь всё нормально
        } catch(...) {}
        try {
            Exceptional ex(0); // будет утечка памяти, так как p не удалится
        } catch (...) {}
        try {
            EX ex;
        } catch (...) {
            cout << "Hey!" << endl;
        }
    }
}

namespace _5_Exceptions_specifications_and_noexcept_operator {
    struct Exceptional {
        int* p;

        Exceptional(int x): p(new int(x)) {
            if (x == 0) {
                throw std::runtime_error("Boom!"); // но деструктор Exceptional не вызовется
            }
        }

        ~Exceptional() noexcept(false) { // разрешает выбрасывать исключение
            delete p;
            throw 1;
        }
    };

    int f() noexcept { // гарантирует, что всё пойдёт так (без UB и throw)
        return 1;
    }

    int g() noexcept(noexcept(f())) { // второй noexcept - operator; возвращает (f -> noexcept: bool)
        return 2;
    }
};

int main() {
    _4_RAII_idiom_and_constructors_exceptions::main();
    return 0;
}
