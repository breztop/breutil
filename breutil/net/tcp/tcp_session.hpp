#pragma once

#ifdef BRE_CORO
#include "tcp_session_impl_v2.hpp"
namespace bre {
using bre::impl::v2::TCPSession;
}
#else
#include "tcp_session_impl_v1.hpp"
namespace bre {
using bre::impl::v1::TCPSession;
}
#endif
