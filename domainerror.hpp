#pragma once
#include <stdexcept>

namespace warehouse {

struct DomainError : std::runtime_error {
    using runtime_error::runtime_error;
};

}