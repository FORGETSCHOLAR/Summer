#pragma once
#include <system_error>


class AcceptException : public std::runtime_error
{
public:
    AcceptException(int err, const std::string& msg)
        : std::runtime_error(msg), error_code(err) {}

    int getErrorCode() const { return error_code; }
    std::string getErrorMsg() const{
        return what();
    }

private:
    int error_code;
};