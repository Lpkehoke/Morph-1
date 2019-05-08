#pragma once

#include <exception>

namespace py
{

class error_already_set : public std::exception
{};

class cast_error : public std::exception
{};

class load_error : public std::exception
{};

} // namespace py
