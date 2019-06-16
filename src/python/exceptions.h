#pragma once

#include <exception>

namespace py
{

class ErrorAlreadySet : public std::exception
{};

class CastError : public std::exception
{};

class LoadError : public std::exception
{};

} // namespace py
