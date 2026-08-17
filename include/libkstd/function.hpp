#pragma once

namespace KernelSTD {
    class CallableBase {
        virtual invoke();
    };

    template<typename Functor>
    class Callable : CallableBase {

    };

    template<typename Signature>
    class Function;

    template<typename Ret, typename... Args>
    class Function<Ret(Args...)> {

    };
}