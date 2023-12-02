#include<iostream>
#include<vector>

using namespace std;

namespace Pointers_references_constants_and_type_conversions {
    void Pointers() {
        int x = 1;
        int* w = &x; // выдаёт номер первого байта в памяти, где хранится переменная
        int y = *w; // по индексу в памяти возвращает значение в нём
        int** w1 = &w; // рекурсивно сколько угодно раз
        **w1 = 2; // x изменится
        // &x = 1 -> нельзя (rvalue)

        std::vector<int> a = {1, 2};
        auto p = &a[0];
        std::cout << *(p+1) << std::endl; // выведет 2

        void* t = nullptr; // можно кастовать к любому указателю и любой указатель кастовать к нему
        int xx = 0;
        int* x1 = &xx;
        void* yy = x1;
        std::string* z1 = (std::string*)yy;
        std::string zz = *z1;
    }

    int f() {
        static int c = 0; // теперь хранится в data
        c++;
        return c;
    }

    void Kinds_of_memory() {
        /*
        ОС изначально выделяет программе оперативную память 3 типов.

        data (static memory) -> глобальные переменные, строки и другие перманентные данные
        text -> сама программа (для выполнения)
        stack (automatic memory) -> локальные переменные и вызовы функций
        */
        /*
        dynamic memory -> память, выделяемая под всякие векторы по запросу программы
        */
        int* t = new int(1); // забирает динамическую память у ОС, чтобы записать туда переменную
        delete t; // возвращает память обратно ОС

        std::vector<int> a; // Хранит указатель на начало, кол-во элементов и общий выделенный размер (*T, *sz, *cap)

        std::cout << f() << std::endl; // инициализируется 1 раз, так как static
        std::cout << f() << std::endl; // выведет 2, работает, как глобальная переменная
    }

    void Arrays () {
        double a[5] = {1, 2}; // будет {1, 2, 0, 0, 0}
        double s[10000000]; // выделяется на стеке, получится StackOverflow
        double* a1 = a; // неявная конвертация, даёт адрес 1 элемента

        a1[1] = 1[a1] = *(a1+1); // первые 2 заменяются на 3-ю для массивов и указателей
        a1 += 2;
        std::cout << a1[-1] << std::endl; // эквивалентно *(a1-1), выдаст 2

        // ++a, a = b -> запрещено, хотя в указателях можно, но оно всё равно lvalue
    }

    void f1(int *p) {}

    void f2(int p[]) {}

    void Arrays_2_3() {
        int *a = new int[100]; //положить в a указатель на 1-ый из 100 последовательных элементов памяти
        delete[] a; //удалить все x

        // Variable length array (VLA)
        int n;
        cin >> n;
        int s[n]; // официально UB

        int b[100];
        f1(b), f2(b); // просто синонимичные конструкции

        const char* S = "abc\0def"; // C-шная строка
        cout << S << endl; // По договорённости её конец в \0, поэтому выведет "abc"
    }

    void f(int x) {
        cout << "Hola, " << x << endl;
    }

    bool cmp(int x, int y) {
        return x > y;
    }

    void sort1(vector<int> &a, bool(*cmp)(int, int)) {
        sort(a.begin(), a.end(), cmp);
    }

    void minus1(int x) {}
    void minus1(double x) {}

    void Functions_and_pointers_to_functions_2_4() {
        void (*pf)(int) = &f; // указатель на функцию
        void (**ppf)(int) = &pf;
        f(3);
        pf(4); // работает, как f
        // ppf(5) -> не работает

        vector<int> a = {57, 179, 2022};
        sort1(a, cmp);

        // Выберет, какую запускать, в зависимости от параметра
        void minus(int);
        void minus(...); // сколько угодно каких угодно аргументов
        void minus(double);
        // Если происходит неявный каст - компилятор сам выбирает куда. По приоритетам:
        /*
        1) Без каста
        2) promotion
        3) standard
        4) user-defined
        5) ellipsis
        */

        void (*pminus)(int) = &minus1; // подставит версию minus1, которая подходит
    }

    int& example() {
        int last_x = 0;
        return last_x;
    }

    void f1() {}

    void References_2_5() {
        int x, y;
        void swap(int x, int y); // поменяет местами копии внутри функции, а не сами переменные
        void swap(int &x, int &y); // передаёт указатели, и меняет значения под ними

        int MIPT = 0, Misha = 0;
        {
            int &PHYSTECH = MIPT; // я говорю, что MIPT и PHYSTECH - это в точности одно и то же
            PHYSTECH += Misha; // так как PHYSTECH и MIPT - это одно и тоже, то я написал MIPT += Misha
            // В конце PHYSTECH не уничтожится, так как это MIPT, а у него область видимости не кончилась
        }

        int* t1 = nullptr;
        int*& t2 = t1; // ссылка на указатель
        int a1[100];
        int (&a2)[100] = a1; // ссылка на массив
        void (&f2)() = f1; // ссылка на функцию

        int new_x = example(); // UB, так как мы возвратили ссылку на уже удалённую переменную
    }

    void check(const int& x) {}

    void Constants() {
        const int x = 5; // отсутствуют константные операции
        // {=, +=, --, &&=, ...} -> модифицирующие операции те, которых нет у {const type}
        const int* p1 = &x; // указатель на const int
        p1++; // можно
        int* const p2 = nullptr; // константный указатель
        // {p2++} -> нельзя

        int a = 1;
        const int* a_it = &a; // указатель на a, через который его нельзя менять
        const int& a_ref = a; // ссылка на a, с помощью которой его нельзя менять
        // {int& b = a_ref} -> нельзя

        check(5); // создаст временный объект с 5 и передаст ссылку на него

        int first = 5;
        const double& second = first; // инициализируется ссылкой на временную переменную (double)x
        first++;
        cout << first << ' ' << second << endl; // выведет 6, 5
    }

    void Type_conversions() {
        int x = 5;
        auto y = static_cast<double>(x); // основной способ явного приведения типа
        // dynamic_cast -> узнаем потом
        auto& y1 = reinterpret_cast<float&>(x); // говорит читать биты x и работать с ним, как с float
        const int& x1 = x;
        auto& y2 = const_cast<int&>(x1);

        auto y3 = (double)x; // перебирает вообще все варианты кастов, пока не подойдёт
    }
}

int main() {
    return 0;
}
