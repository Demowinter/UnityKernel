#pragma once
#include <memory>
#include <type_traits>

namespace STDLib {
    template<typename Ret, typename... Args>
    struct CallableBase {
        virtual Ret invoke(Args... args) = 0;
        virtual ~CallableBase() = default;
    };

    template<typename Functor, typename Ret, typename... Args>
    struct Callable : CallableBase<Ret, Args...> {
        template<typename F>
        Callable(F func) : func(func) {}

        Ret invoke(Args... args) override {
            return func(args...);
        }

    private:
        Functor func;
    };

    template<typename Signature>
    class Function;

    template<typename Ret, typename... Args>
    class Function<Ret(Args...)> {
        using CB = CallableBase<Ret, Args...>;

    public:
        Function() = default;
        Function(const Function& obj) = delete;
        Function(Function&& obj) { swap(obj); }

        template<typename Functor>
        Function(Functor obj) {
            using CallableObject = Callable<Functor, Ret, Args...>;

            functor = std::unique_ptr<CallableObject>{new CallableObject{obj}};
        }

        void swap(Function& obj) {
            std::swap(functor, obj.functor);
        }

        Ret operator()(Args... args) {
            return functor->invoke(args...);
        }

        explicit operator bool() const {
            return static_cast<bool>(functor);
        }

        void clear() {
            functor = nullptr;
        }

    private:
        std::unique_ptr<CB> functor = nullptr;
    };
}