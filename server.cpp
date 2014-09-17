#include "http.hpp"

void main()
{
	using namespace http;
	http_server hs=http_server();
	hs.Default();
}