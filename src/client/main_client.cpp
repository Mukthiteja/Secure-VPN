#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/SSLManager.h>
#include <Poco/Net/AcceptCertificateHandler.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/SharedPtr.h>
#include <iostream>
#include "vpn/common/Logger.h"

using namespace Poco::Net;

int main() {
	Poco::Logger& log = vpn::common::LoggerFactory::get("client");
	Poco::SharedPtr<InvalidCertificateHandler> pCert = new AcceptCertificateHandler(false);
	Context::Ptr pContext = new Context(Context::CLIENT_USE, "", "", "", Context::VERIFY_NONE);
	SSLManager::instance().initializeClient(nullptr, pCert, pContext);

	HTTPSClientSession session("127.0.0.1", 8443);
	HTTPRequest req(HTTPRequest::HTTP_GET, "/");
	HTTPResponse res;
	std::ostream& os = session.sendRequest(req);
	os << "";
	std::istream& is = session.receiveResponse(res);
	std::string body;
	Poco::StreamCopier::copyToString(is, body);
	log.information("Response status: " + std::to_string(res.getStatus()));
	std::cout << body;
	return 0;
}



