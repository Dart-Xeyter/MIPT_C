#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <memory>

namespace _1_idea_and_basic_examples {
    void main() {
        std::vector v = {1, 4, 3};
        std::sort(v.begin(), v.end(), [](int x, int y){ return x % 2 < y % 2; }); // лямбда-функция
        auto f = [](int x, int y) -> int { // можно указать, какой тип писать
            if (x < y) {
                return 1;
            } else {
                return 1.0;
            }
        }; // также можно создавать объект лямбда
        f(1, 3); // и вызывать его

        const int c = []() {
            return 1; // здесь вычисляем нетривиальную константу или ссылку
        }(); // и immediate invocation
        std::cout << c << std::endl;
    }
}

namespace _2_capture_lists_in_lambdas {
    struct S {
        int b = 4;

        auto get_F(int a) const {
            return [a](int x) {return x+a;}; // но по ссылке нельзя
        }

        auto get_G() const {
            return [this](int x) {return x+b;}; // а так нужно захватывать поля своего класса (по ссылке)
            // [a](int x) {return x+b;}; не сработает, так как не локальная
        }
    };

    void main() {
        int mid = 5;
        auto f = [mid](int x, int y) {return x-mid < y-mid;}; // можно захватить некоторые внешние переменные
        auto g = [&mid](int x, int y) {return x-mid < y-mid;}; // а так по ссылке
        // в f я не могу менять mid, а в g могу
        auto h = [mid](int x, int y) mutable {return x-mid < y-mid;}; // теперь изменять можно
        auto f2 = [s = std::string()](int x, int y) {return s[x] < s[y];}; // или инициализировать внутри

        auto d1 = [&](){}; // захват всего локального вокруг по ссылке
        auto d2 = [=](){}; // а здесь всего локального по значению
        auto d3 = [&, f](){}; // всё по ссылке, кроме переменной f

        std::cout << S().get_F(3)(4) << std::endl;
        std::cout << S().get_G()(5) << std::endl;

        static int aa = 1;
        auto hh = [](int x) {++aa; return x+aa;}; // статические можно использовать и менять без захвата
        std::cout << hh(5) << ' ' << aa << std::endl;

        auto hhh = [aa](int x) {return x+aa;}; // всё равно не захватывает, а использует объявленную выше
    }
}

namespace _3_closures_as_objects {
    void main() {
        int mid = 5;
        auto f = [mid](int x) {return x+mid;};

        // f преобразуется вот в такую вот фигню
        class __lambda_6_13
        {
        public:
            inline /*constexpr */ int operator()(int x) const
            {
                return x + mid;
            }

        private:
            int mid;

        public:
            __lambda_6_13(int & _mid)
                    : mid{_mid}
            {}

        };
        // если добавить mutable, operator() будет не const
        // если написать [&mid], можно менять, так как станет {int& mid};

        // {decltype(f) f2;} - можно, если нет захвата и с C++20
        decltype([](int x, int y) {return x < y;}) f3; // можно даже так
        decltype(f3) f4 = f3;
        f3 = f4;

        std::unique_ptr<int> u;
        auto g = [u = std::move(u)](int x, int y) {return x-*u < y-*u;};
        auto g1 = std::move(g); // присваивать нельзя, так как нельзя присваивать unique
    }
}

namespace _4_generic_lambdas {
    template <typename... Args>
    void g(Args&&... args) {
        // можно принимать даже пакеты...
        auto t = [...args1 = std::forward<Args>(args)](auto&& f) {std::cout << sizeof...(args1) << std::endl;};
        auto tt = []<typename T>(T& f) {std::cout << 12 << std::endl;}; // можно писать, например, requirements
        t(t);
        tt(t);
    }

    void main() {
        auto f = [](auto x, auto y) {return x < y;}; // шаблонная будет не lambda, а operator()
        std::cout << f(1, 2) << std::endl;

        auto p = [](){std::cout << 179 << ' ';};
        auto twice = [](auto&& f) {return [=](){f(); f();};}; // принимаю и возвращаю функцию
        auto pp = twice(p); // причём f он почему-то не захватывает
        pp();
        std::cout << std::endl;

        g(1, 4.0);
    }
}

namespace _5_ranges {
    struct S {
        int begin() {
            return 1;
        }

        double end() {
            return 2;
        }
    };

    namespace rng = std::ranges; // общепринятое сокращение
    namespace views = std::views;

    struct Point {
        float x, y;
    };

    void main() {
        S s; // range может быть что угодно с begin и end (не обязательно одного типа)
        std::vector<int> v;
        std::vector<int> vv;
        std::copy_if(v.begin(), v.end(), std::back_inserter(vv), [](int x) {return x < 5;}); // тупо
        rng::copy_if(v, std::back_inserter(vv), [](int x) {return x < 5;}); // круто

        // но можно не копировать всё в vv, а делать сразу
        auto filtered = v | views::filter([](int x) {return x < 5;}); // создаёт views на нужные элементы
        // он не создаёт объект, а только сохраняет, как надо перебирать объекты (за размер v)
        rng::for_each(filtered, [](int x) {return x;}); // и применяет ко всем функцию
        std::vector<Point> d;
        rng::sort(d, {}, &Point::x); // а ещё можно писать сортировку, как в Питоне
        rng::sort(d, {}, [](Point x) {return x.x+1;}); // так в общем случае

        for (int x : views::iota(0, 10)
                | views::filter([](int x) {return x % 2 == 1;})
                | views::transform([](int x) {return x*x;})) { // можно комбинировать бесконечно
            std::cout << x << std::endl;
        }

        // {rng::sort(filtered);} -> не сработает, не RandomAccessIterator

        auto ff = rng::find(std::vector<int>(1, 2), 3);
        // {*ff} -> CE, так как вернули итератор на уже умерший объект
        std::vector<int> vvv = {1, 2};
        auto fff = rng::find(vvv, 2);
        *fff; // а здесь всё хорошо
        auto g = rng::find(vvv | views::take(1), 4);
        // {auto g = rng::find(std::vector<int>(1, 2) | views::take(1), 4);}
        // CE -> аналогично с ff у нас временный объект не может пайпаться (prvalue)
    }
}

namespace _6_std_function {
    int inc(int x) {
        return ++x;
    }

    struct Dec {
        int operator()(int x) const {
            return --x;
        }
    };

    struct Id {
        int x = 0;

        int id(int y) {
            return y+x;
        }
    };

    struct Test {
        int x = 0;

        int test(int y) const {
            return y+x;
        }
    };

    void main() {
        // можно хранить что угодно с operator() в runtime
        std::function<int(int)> f = [](int x) {return x*2;};
        std::cout << f(2) << std::endl;
        f = inc;
        std::cout << f(2) << std::endl;
        f = Dec();
        std::cout << f(2) << std::endl;

        int (Id::*my_id)(int) = &Id::id; // указатель на член
        std::cout << (Id().*my_id)(2) << std::endl;
        std::function<int(Id&&, int)> ff = my_id; // можно присваивать и его...
        std::cout << ff(Id(), 2) << std::endl;

        int Id::* px = &Id::x; // а также указатель на поле
        std::function<int(Id&&)> fff = px;
        std::cout << fff(Id()) << std::endl;

        int (Test::*is_const)(int) const = &Test::test; // без const может менять Test
        std::function<int(const Test&&, int)> need_const = is_const; // а здесь не должна, поэтому сверху const
    }
}

namespace _7_std_bind {
    void main() {
        auto less = [](int x, int y) {return x < y;};
        auto less_than_5 = std::bind(less, std::placeholders::_3, 5); // placeholder берёт нужный аргумент из входных
        std::cout << less_than_5(6, 6, 1, 4) << std::endl; // здесь, например, он возьмёт 3-ий
        auto greater = std::bind(less, std::placeholders::_2, std::placeholders::_1);
        std::cout << greater(4, 5) << std::endl;
        auto reflexive = std::bind(less, 3, 3);
        std::cout << reflexive() << std::endl;
    }
}

int main() {
    _7_std_bind::main();
}
