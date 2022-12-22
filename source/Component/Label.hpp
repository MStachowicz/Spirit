#pragma once

#include <string>

namespace Component
{
    class Label
    {
    public:
        Label(const std::string& pName)
            : mName{pName}
        {}

        std::string mName;
    };
}