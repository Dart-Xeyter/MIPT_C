#include<iostream>
#include<vector>

using namespace std;

namespace Introduction {
    using vi = std::vector<int>;

    int f() {
        return 0;
    }

    int g() {
        return 0;
    }

    int h() {
        return 0;
    }

    void Kinds_of_errors_and_UB() {
        /*
        CE -> во время компиляции (синтаксические, семантические, лексические)
        {$} -> lexical (не получается разбить на токены)
        {a+;} -> syntax (получилось разбить, но не получается состыковать)
        {съешь этим столом себя} -> semantic (понятно, что просят, но сделать невозможно)

        И ещё есть "ошибка прав доступа"
        */
        /*
        RE -> во время выполнения
        */
        //Floating point exception (деление на 0)
        int v1 = 0, t1 = 1/v1;
        //Segmentation fault (ошибка при работе с памятью)
        vi a(3);
        int t2 = a[-179];
        /*
        UB -> undefine behaviour
        */
        // Раз q не переполнилось при *179179179, оно всегда <57 -> оптимизируем проверку на true
        for (int q = 0; q < 57; q++) {
            std::cout << q*179179179 << std::endl;
        }
        // При обращении к некорректному индексу возвращаем что угодно
        vi s(3);
        std::cout << s[57] << std::endl;
        /*
        Unspecified behaviour
        */
        // 6 вариантов порядка вычислений f(), g(), h()
        int res = f()+g()*h();
    }

    void types_and_operations() {
        // Integral types:
        int x1; // ~4 байта
        long x2; // никому не нужен :)
        long long x3; // ~8 байт
        short x4; // ~2 байта
        char x5; // ~1 байт

        int8_t y1; // =1 байт
        int16_t y2; // =2 байта
        int32_t y3; // =4 байта
        int64_t y4; // =8 байт
        //int128_t y5; // =16 байт
        // Floating point types: -> [sign, mantissa, exponent]
        float z1; // ~4 байта
        double z2; // ~8 байт
        long double z3; // ~16 байт

        /*
        Выражения - lvalue (которым можно присваивать) и rvalue (которым нельзя)
            Виды операций:
        {+, -, *, /, %, |, ^, <<, >>, ~} -> rvalue
        {++a, --a} -> lvalue, {a++, a--} -> rvalue
        {=, +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>=} -> lvalue
        {&&, ||, !} -> rvalue, вычисляются лениво
        {?:} (тернарный оператор) -> lvalue, если (bool ? lvalue : lvalue), иначе rvalue
        {,} -> возвращает правую часть
        {sizeof(x), sizeof(int)} -> возвращает число байт, занимаемой переменной/типом
        */
        int x, y, z;
        x = y = z = 1; // x = (y = (z = 1))), правоассоциативно
        x+y+z; // (x+y)+z, левоассоциативно

        x = y, z; // x = y
        x = (y, z); // x = z

        Kinds_of_errors_and_UB(); // 1.7
    }
}

void func(int); // Объявление
void func(int) {} // Определение
// One Definition Rule - определять функцию в программе можно только 1 раз, объявлять - сколько угодно.

int x; // определение
extern int x; // объявление

/*
Scope - область видимости.
Global scope     -> вся программа.
Local scope      -> до конца текущего {}
Potential scope  ->
Actual scope     ->
*/

int main() {
    return 0;
}
