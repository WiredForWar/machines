#pragma once

#include <cstdint>

namespace HttpStatus
{

enum Status : std::uint16_t
{
    Ok = 200,
    Created = 201,
    NoContent = 204,
    BadRequest = 400,
    NotFound = 404,
    ServiceUnavailable = 503,
};

} // namespace HttpStatus
