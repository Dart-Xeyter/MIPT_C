#include<bits/stdc++.h>
#define endl '\n'
const int INF = 1000000001;

using namespace std;

namespace Classes_and_structures {
    struct S {
        int x;
        double y = 3.14;
        char c = 'a';

        struct SS {
            float f = 1.05;
        } ss; // создал переменную ss в поле S

        void print() {
            cout << x << ' ' << y << ' ' << c << endl;
        }

        S* f() {
            return this; // указатель на себя
        }
    };

    void S_example() {
        S s{2, 3}; // проинициализирует первые 2 поля
        s.print(); // вызовет внутреннюю функцию
        S* s1 = new S{1};
        cout << (*s1).x << ' ' << s1->y << endl; // синонимы
    }
}

namespace Encapsulation_and_access_modifiers {
    struct S { // по умолчанию поля публичны
    private: // нельзя обращаться вне класса
        int x = 1;

        void g(int y) {}
    public: // видно откуда угодно
        int& f() {
            return x;
        }

        void print() {
            cout << x << endl;
        }

        void g(double y) {}
    };

    class SS { // по умолчанию поля приватны
        friend S;
        friend void main();
        // внутри друзей поля SS видны, несмотря на private

        int y = 1;
    };

    void main() {
        S s;
        // {s.x} -> не доступна вне класса
        int& x = s.f();
        x++; // можно изменять private, если уж мы получили к нему доступ
        s.print();

        s.g(0.0); // корректно
        // {s.g(0)} -> CE, так как хотя s.g(int) недоступна, она видна
    }
}

namespace Constructors_and_destructors {
    int r_is = 0;

    class Compl {
        double re = 0.0;
        double im = 0.0;

        int &r;
        const int R;
        // Должны быть инициализированы при создании экземпляра
    public:
        Compl(double re, double im) : re(re), im(im), r(r_is), R(r_is) {}
        Compl() : r(r_is), R(r_is) {}

        ~Compl() {} // деструктор
    };

    struct A {
        A() {
            cout << "A created!" << endl;
        }
        ~A() {
            cout << "A destroyed!" << endl;
        }
    };

    struct String {
        char* arr;
        size_t sz;
        size_t cap;

        String(size_t n, char c): arr(new char[n]), sz(n), cap(n) {
            memset(arr, c, n);
        }

        String(initializer_list<char> lst) // если строчка слишком длинная, переносим
                : arr(new char[lst.size()])
                , sz(lst.size())
                , cap(lst.size())
        {
            copy(lst.begin(), lst.end(), arr);
        }

        String(const String& s) : String(s.sz, '\0') { // копирование через вызов конструктора
            memcpy(arr, s.arr, sz);
        }

        ~String() { // деструктор
            delete[] arr;
        }
    };

    void main() {
        Compl s1; // если бы не было конструкторов, проинициализировалось бы по умолчанию
        Compl s2{1, 2}, s3 = Compl(2, 3); // почти синонимы

        vector<int> v = {1, 2, 3}; // initializer list
    }
}

namespace Assignment_operator_and_The_Rule_Of_Three {
    // Если есть хотя бы 1 из {конструктор, деструктор, копирование}, должны быть все 3
    struct String {
        char* arr;
        size_t sz;
        size_t cap;

        String(size_t n, char c): arr(new char[n]), sz(n), cap(n) {
            memset(arr, c, n);
        }

        String(const String& s) : String(s.sz, '\0') {
            memcpy(arr, s.arr, sz);
        }

        String& stupid_operator(const String &s) {
            if (this == &s) { // при {s = s} удалять arr нельзя
                return *this;
            }
            delete[] arr;
            arr = new char[s.cap], sz = s.sz, cap = s.cap;
            memcpy(arr, s.arr, sz);
            return *this;
        }

        // copy and swap
        String& operator=(String s) {
            swap(s);
            return *this;
        }

        ~String() {
            delete[] arr;
        }

        void swap(String &s) {
            ::swap(arr, s.arr), ::swap(sz, s.sz), ::swap(cap, s.cap);
        }
    };
}

namespace Operators_overloading {
    class BigInteger {
        vector<int> digits;
    public:
        BigInteger(int x) {
            //to do
        }

        BigInteger& operator+=(const BigInteger& another) { // должен работать быстрее, чем +
            //code
            return *this;
        }

        BigInteger& operator++() { // префиксный инкремент
            return *this += 1;
        }

        BigInteger operator++(int) { // постфиксный инкремент (фиктивный аргумент для отличия)
            BigInteger copy = *this;
            ++*this;
            return copy;
        }
    };

    BigInteger operator+(BigInteger a, const BigInteger& b) { // создаётся новый объект
        a += b;
        return a;
    }

    BigInteger operator+(int a, const BigInteger& b) { // чтобы левый операнд мог быть не BigInteger
        BigInteger sum = a;
        sum += b;
        return sum;
    }

    bool operator<(const BigInteger& s1, const BigInteger& s2) {
        //to do;
        return true;
    }

    bool operator>(const BigInteger& s1, const BigInteger& s2) { // выражая через <
        return s2 < s1;
    }

    ostream& operator<<(ostream& out, const BigInteger& a) { // вывод типа
        //to do;
        return out;
    }

    class S {
        int x;
        double d;

        // начиная с С++20 можно реализовать автоматическое определение сравнений
        // friend auto operator<=>(const S&, const S&) = default;
    };

    bool operator<(const S& a, const S& b) { // и реализовать свой оператор <
        return true; // только надо заморочиться, чтобы в <=> использовался он
    }

    // partial_ordering -> могут быть сравнимы не все
    // weak_ordering -> все сравнимы
    // strong_ordering -> если {a = b}, то для любой f выполнено {f(a) = f(b)}
}

namespace Const_methods_and_get_index_overloading {
    struct S {
        char* s;
        int& x;
        int y;
        mutable int z;

        char& operator[](size_t index) { // обращение к индексу
            return s[index];
        }

        // принимает *this по константной ссылке
        const char& operator[](size_t index) const { // обращение в const S
            ++x; // корректно, так как {int& const x;}
            // {++y;} -> некорректно, так как {int const y;}
            ++z; // корректно, так как mutable
            return s[index];
        }
    };

    void main() {
        string str1 = "abc";
        const string& str2 = str1;
        const char& c = str2[0]; // если бы возвращался char, вывелось бы снизу неправильно
        str1[0] = 'z';
        cout << c << endl;
    }
}

namespace Static_fields_and_methods {
    struct S {
        static int count; // поле общее для всех объектов данной структуры
        static const int c1 = 0; // если не const, нельзя инициализировать по умолчанию
        // причём для, например, double, нельзя так делать даже при const

        S() {
            ++count;
        }

        ~S() {
            --count;
        }
    };

    int S::count = 0; // она не определена, пока не проинициализирована

    class Singleton { // хочу, чтобы у меня был <= 1 объект данного типа
        int x;

        Singleton() = default;
        Singleton(const Singleton&) = delete; // запрещаю копировать объект

        static Singleton* ptr;
    public:
        static Singleton& get() { // метод, общий для всех объектов класса
            if (ptr == nullptr) {
                ptr = new Singleton();
            }
            return *ptr;
        }
    };

    Singleton* Singleton::ptr = nullptr; // я могу объявлять private-поля вне класса

    int main() {
        // {int S::count = 0;} -> нельзя объявлять локально
        S s1;
        {
            S s2;
            cout << S::count << endl; // можно вызывать, как атрибут структуры
        }
        cout << s1.count << endl;

        // они будут одинаковым объектом
        Singleton& x1 = Singleton::get();
        Singleton& x2 = Singleton::get();
        return 0;
    }
}

namespace Functional_objects_and_call_function_overloading {
    struct Compare { // это класс-функтор
        bool operator()(const string& a, const string& b) const {
            return a < b;
        }
    };

    int main() {
        set<string, Compare> s; // s будет сравнивать объекты, вызывая () от экземпляра Compare

        Compare comp; // это функциональный объект
        cout << comp("abc", "abcd") << endl; // вызываю () от объекта данного класса, как функцию
        return 0;
    }
}

namespace User_defined_type_conversions {
    struct BigInteger {
        int x;

        operator int() const { // преобразование объекта класса к int
            return x;
        }

        explicit operator string() const { // запрещает неявные преобразования
            return to_string(x);
        }

        explicit operator bool() const { // в if и for будет применяться
            return x != 0;
        }

        explicit BigInteger(int x): x(x) {} // конструктор от типа к нашему (только явный)
    };

    BigInteger operator""_bi(const char* digits, size_t len) {
        return BigInteger(len);
    }
    // создавать объект, как 1LL, от одного из этих 2 типов
    BigInteger operator""_bi(unsigned long long x) {
        return BigInteger(x);
    }

    int main() {
        BigInteger a{1};
        cout << a << endl; // он преобразуется к int
        string b = "ab";
        cout << static_cast<string>(a)+b << endl; // не сконвертируется без прямого указания
        string c(a); // каст явный

        BigInteger t = (BigInteger)(234_bi+"abc"_bi); // воспринимает литералы, как BigInteger
        cout << t << endl;
        return 0;
    }
}

signed main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
    User_defined_type_conversions::main();
    return 0;
}
