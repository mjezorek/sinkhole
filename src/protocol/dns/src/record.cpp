#include "sinkhole.hpp"
#include "network.hpp"
#include "io.hpp"
#include "string.hpp"
#include "timers.hpp"
#include "include/dns.hpp"
#include "include/query.hpp"
#include "include/datasocket.hpp"

using namespace Sinkhole::Protocol::DNS;

SOARecord::SOARecord() : ResourceRecord()
{
}

SOARecord::SOARecord(ResourceRecord &rr) : ResourceRecord(rr)
{
}

