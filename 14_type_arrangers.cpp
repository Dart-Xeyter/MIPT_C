#include <iostream>
#include <memory_resource>
#include <variant>
#include <vector>
#include <cstddef>
#include <any>

using std::cin, std::cout, std::endl;

namespace _1_type_erasure {
    class Any {
        struct Base { // храним объект этого типа, но на самом деле того, что нам надо
            virtual ~Base() = default;
            virtual Base* copy() = 0;
        };

        template <typename T>
        struct Derived: Base {
            T obj;

            Derived(const T& obj): obj(obj) {}

            Base* copy() override {
                return new Derived(obj);
            }
        };

        Base* obj = nullptr; // type erasure
    public:
        template <typename T>
        Any(const T& obj): obj(new Derived<T>(obj)) {}

        Any(const Any& other): obj(other.obj->copy()) {}

        Any(const Any&& other): obj(other.obj) {
            obj = nullptr;
        }

        ~Any() {
            delete obj;
        }
    };

    void main() {
        std::any a = 5; // можно хранить что угодно
        a = 3.14;
        std::cout << std::any_cast<double>(a) << std::endl; // взаимодействовать, указывая тип
        a = "34";
        try {
            std::any_cast<int>(a); // если не угадал - exception
        } catch (...) {
            std::cout << "Hey!" << endl;
        }
        a = 1; // также он должен уметь уничтожать предыдущее значение
    }
}

namespace _2_shared_ptr_with_custom_deleter_and_allocator {
    struct ControlBlock { // храним общую структуру со счётчиками
        size_t shared_cnt;
        size_t weak_cnt;
    };

    template <typename T>
    struct SharedPtr {
        template <typename Alloc>
        struct BlockByMake: ControlBlock {
            Alloc alloc;
            T obj; // можно хранить массив char-ов или уничтожать Alloc руками
            // так как в деструкторе нужно избежать уничтожения obj
            // ведь weak_cnt могло кончиться позже shared_cnt, и он тогда уже за-destroy-ен
        };

        template <typename Deleter, typename Alloc>
        struct BlockByPtr: ControlBlock {
            Deleter deleter;
            Alloc alloc;
            T* ptr; // здесь мы храним пойнтер, потому что он мне дан, я не могу конструировать новый T
        };
    };
}

namespace _3_unions_and_small_strings_optimization {
    union U { // хранит в каждый момент времени только 1 из переменных на единой памяти
        int x = 5;
        double y;
    }; //

    union U2 {
        int x;
        double y;
        std::string s; // хранится объект с нетривиальным конструктором
        U2(): x(5) {} // поэтому нужно реализовать U2() и ~U2(); и всякие =
        ~U2() {}
    };

    void main() {
        U u;
        cout << sizeof(u) << endl; // выведет 8 -> размер максимум из размеров полей
        cout << u.x << endl; // просто 5, если обращаться {u.y} -> UB

        U2 u2;
        // {u2.s = "a";} не работает, так как будет думать, что в u2.s уже строка, а там пока число
        new (&u2.s) std::string("hoho"); // приходится так
        cout << u2.s << endl;
        u2.s.~basic_string<char>(); // и если теперь не вызвать деструктор, будет утечка памяти

        union { // анонимный юнион - кладём в локальные переменные int1 и p на единой памяти
            int int1;
            const char* p;
        };
        int1 = 3;
        p = "a"; // здесь int1 перезатёрся

        // small string optimization - если длина строки меньше, чем размер size_t
        // тогда мы можем хранить саму строку на памяти, выделенной под capacity
        // для этого храним union из size_t capacity и char[] local_buffer
    }
}

namespace _4_std_function_implementation {
    template <typename...>
    class function;

    template <typename Result, typename... Args>
    class function<Result(Args...)> {
        struct BaseFunctor {
            virtual Result operator()(Args...) = 0;
            virtual ~BaseFunctor() = default;
        };

        template <typename F>
        struct DerivedFunctor: BaseFunctor {
            F f;

            DerivedFunctor(F f): f(std::move(f)) {}

            Result operator()(Args... args) override {
                return f(std::forward<Args>(args)...);
            }
        };

        BaseFunctor* fptr;
    public:
        template <typename Functor>
        function(Functor&& functor) {
            fptr = new DerivedFunctor<Functor>(std::forward<Functor>(functor));
        }

        Result operator()(Args... args) const {
            return fptr->operator()(std::forward<Args>(args)...);
        }

        ~function() {
            delete fptr;
        }
    };

    //move_only_function
}

namespace _5_polymorphic_allocator {
    struct BaseResource { // занимается только выделением чистой памяти
        virtual void* allocate(size_t size, size_t align) = 0;
    }; // по сути, дублирование в некотором смысле allocator_traits

    void main() {
        std::pmr::monotonic_buffer_resource resource(100); // ресурс, аналогичный StackStorage
        auto resource2 = std::pmr::new_delete_resource(); // здесь работаем через new-delete
        auto resource3 = std::pmr::null_memory_resource(); // не даёт выделять память

        std::pmr::set_default_resource(&resource); // можно установить по умолчанию какой-то
        std::pmr::polymorphic_allocator<int> alloc(&resource); // создаёт аллокатор с переданным ресурсом

        auto resource4 = std::pmr::synchronized_pool_resource(); // здесь хитрее
        // вы храним chunk-и с блоками по 1, 2, 4, 8... байт и выделяем из нужного - поэтому можно освобождать
    }
}

namespace _6_std_variant {
    template <typename... Types>
    struct Overload: Types... { // наследуемся от всех Types
        using Types::operator()...; // и подгружаем себе их operator()
    };
    template <typename... Types> Overload(Types...) -> Overload<Types...>; // делает type deduction

    void main() {
        std::variant<std::string, std::vector<char>> v; // как union
        v = std::vector('a', 'b'); // делаем активным соответствующее поле
        cout << std::get<1>(v).size() << endl; // можно обращаться по индексу
        cout << std::get<std::vector<char>>(v).back() << endl; // или по типу
        try {
            std::get<0>(v).size(); // если поле не активно -> бросит исключение
        } catch (...) {
            cout << "RE" << endl;
        }
        // если функция применима ко всем членам variant, её можно писать унифицировано
        std::visit([](auto &item){cout << item.back() << endl;}, v);
        auto lam1 = [](std::string item) {cout << "string" << item.back() << "\n";};
        auto lam2 = [](std::vector<char> item) {cout << "vector" << item.back() << "\n";};
        std::visit(Overload<decltype(lam1), decltype(lam2)>{lam1, lam2}, v); // по сути, делается это
        std::visit(Overload(lam1, lam2), v); // за счёт deduction и так работает

        cout << v.index() << endl; /// номер активного элемента
        // variant умеет хешироваться и сравниваться (но с равными активными типами)
    }
}

namespace _7_variant_implementation {
    template <typename... Types> // тут нужно вычислить размер buffer-а
    struct GetMaxSizeOf {
        static const size_t value = 64;
    };

    template <size_t N, typename... Types> // тут нужно вычислить размер buffer-а
    struct GetTypeByIndex {
        using type = int;
    };

    template <typename T, typename... Types>
    struct GetIndexByType {
        static const size_t value = 0;
    };

    template <typename... Types>
    class Variant;

    template <typename T, typename... Types>
    struct VariantChoice {
        using Derived = Variant<Types...>; // это наш Variant

        VariantChoice() {} // требует конструктор по умолчанию

        VariantChoice(const T& value) { // мы конструируем из VariantChoice его
            new (static_cast<Derived&>(*this).buffer) T(value);
            // также нужно присвоить нужным образом "current_index"
        }

        void destroy() { // нельзя запускать в VariantChoice, так как поля Variant уже будут уничтожены
            Derived& thisVariant = static_cast<Derived&>(*this);
            if (thisVariant.current_index == GetIndexByType<T, Types...>::value) {
                reinterpret_cast<T*>(thisVariant.buffer)->~T(); // уничтожаем только из нужного типа
            }
        }
    };

    template <typename... Types>
    struct VariantStorage { // проблема, как в destroy (не создался buffer)
        // поэтому вынесем его в отдельный класс, который уничтожится после всех VariantChoice
        alignas(std::max_align_t) char buffer[GetMaxSizeOf<Types...>::value]; // сырая память
        size_t current_index = 0; // какой сейчас активный тип
    };

    template <typename... Types>
    class Variant:
            private VariantStorage<Types...>,
            private VariantChoice<Types, Types...>... {
        template <typename T, typename... Ts>
        friend struct VariantChoice; // friend, чтобы можно было static_cast делать

        template <size_t N, typename... Ts>
        friend auto& get(Variant<Ts...>&);
    public:
        // мне нужно вызвать конструктор от типа по правилам перегрузки функций
        using VariantChoice<Types, Types...>::VariantChoice...;
        // подгрузим конструкторы от всех типов из пакетов, и сработает нужный
        ~Variant() {
            (VariantChoice<Types, Types...>::destroy(), ...); // вызываем от всех, сработает от 1
        }
    };

    template <size_t N, typename... Types>
    auto& get(Variant<Types...>& v) { // он не метод класса, так как иначе {v.template get<1>();}
        static_assert(N < sizeof...(Types));
        using T = typename GetTypeByIndex<N, Types...>::type; // понимаем его тип
        return reinterpret_cast<T&>(v.buffer);
    }

    void main() {
        Variant<int, double, std::string> v = 5;
        // visit -> посмотреть на cpp-reference (на записи?)
    }
}

namespace _8_laundry_problem {
    struct A {
        virtual int f();
    };

    struct B: A {
        virtual int f() {
            new (this) A;
            return 1;
        }
    };

    int A::f() {
        new (this) B; // мы изменяем тип, лежащий на нашей памяти
        return 2;
    }

    int h() {
        A a;
        int n = a.f(); // компилятор имеет право оптимизировать
        int m = a.f(); // всё равно у "a" тип "A" -> зачем идти в vtable
        int m_good = std::launder(&a)->f(); // говорит забыть компилятору информацию про pointer (и тип)
        return n+m;
    }

    void main() {
        cout << h() << endl;
        //std::vector<const int> s; {CE} -> снова проблема с оптимизациями компилятора
        // потому что при push_back/pop_back будет placement-new -> и проблема с изменением const-а
    }
}

int main() {
    return 0;
}
