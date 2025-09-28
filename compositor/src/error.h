#ifndef REVYV_ERROR_H
#define REVYV_ERROR_H

#include <iostream>

class FailedToSendDataError : public std::runtime_error {
public:
    FailedToSendDataError()
        : std::runtime_error("Failed to send data")
    {
    }
};

class FailedToReceiveDataError : public std::runtime_error {
public:
    FailedToReceiveDataError()
        : std::runtime_error("Failed to receive data")
    {
    }
};

#endif
