#pragma once
#include <limits>
#include <cstddef>
#include <cstdint>
#include <kernel/subsystems/auth.hpp>

namespace Kernel::FileSubsystem {
    using ResourceID = std::size_t;

    constexpr ResourceID invalidID = std::numeric_limits<ResourceID>::max();

    constexpr uint8_t execFlag = 0x01;
    constexpr uint8_t writeFlag = 0x02;
    constexpr uint8_t readFlag = 0x04;

    class Object {
    public:
        virtual ~Object() {}

    protected:
        Kernel::AuthSubsystem::UserAuthData authData;

        uint8_t userPerm;
        uint8_t groupPerm;
        uint8_t otherPerm;
    };

    class Directory : public Object {
    public:
        ~Directory() override {}
    };

    class IOObject : public Object {
    public:
        virtual ~IOObject() override {}
    };

    class File : public IOObject {
    public:
        ~File() override {}
    };
}