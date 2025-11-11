#include "vpn/tunnel/Tunnel.h"

namespace vpn::tunnel {

Tunnel::Tunnel(const vpn::core::Session& session)
	: session_(session) {}

bool Tunnel::isOpen() const { return open_; }

} // namespace vpn::tunnel


