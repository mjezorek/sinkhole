#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "include/http.hpp"

using namespace Sinkhole::Protocol::HTTP;

HTTPClass::HTTPClass(HTTPAction *a) : action(a)
{
}

