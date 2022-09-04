// http_client.cpp

#include <exception>
#include <string_view>
#include <map>
#include <string>
#include <tcp/client.h>
#include <utils/Log.h>
#include <utils/string_utils.h>
#include <utils/profiler.h>
#include <tcp/client.h>
#include "http_client.h"

LOG_PREFIX("[http_client]: ");
LOG_POSTFIX("\n");
SET_LOG_DEBUG(true);

namespace anp
{
	http_client::http_client()
		: m_cv_ul(m_cv_mtx)
		, m_client(std::make_unique<tcp::client>())
	{
	}

	// Define destructor in cpp because of the incompleteness of tcp/client type in the header
	http_client::~http_client()
	{
		LOG_DEBUG("http_client::~http_client");
	}

	std::string http_client::parse_header(const std::string& response, const std::string& header)
	{
		std::string what = utils::format_str("%s: ", header.c_str());
		auto res = response.find(what);
		if (res != std::string::npos)
		{
			auto res2 = response.find("\r", res);
			if (res2 != std::string::npos)
			{
				auto s = response.substr(res + what.size(), res2 - res - what.size());
				LOG_DEBUG(header << ": " << s);
				return s;
			}
		}
		return "";
	}

	int http_client::request(
		const std::string& host
		, int port
		, const std::string& request
		, const http_response_cb& on_receive
	)
	{
		request_async(host, port, request, on_receive);
		wait();
		return m_error_code;
	}

	void http_client::request_async(
		const std::string& host
		, int port
		, const std::string& request
		, const http_response_cb& on_receive
	)
	{
		reset();

		m_client->set_on_close([&] {
			notify(m_error_code);
		});

		m_client->set_on_connect([=, this](const std::error_code& ec) {
			if (!ec)
			{
				LOG_DEBUG("Send...");
				m_client->send(request);
			}
			else
			{
				LOG_ERROR("Error during connection.");
				notify(connection_process_error);
			}
		});


		if (m_client->connect(host, port))
		{
			LOG_DEBUG("Set on_receive task");
			m_client->set_on_receive([=](
				const std::vector<char>& data
				, std::size_t sz
				, int id
			) {
					LOG_DEBUG("\nReceived " << sz << " bytes:");
					std::string s(data.begin(), data.begin() + sz);
					LOG_DEBUG(s);

					auto on_content_length_changed = [=] {
						if (m_content_length <= 0)
						{ // Response receiving is finished
							if (m_content_length < 0)
								return false;

							m_content_length = -1;
							return false;
						}
						// Download is going to begin
						return true;
					};

					auto on_headers_received = [&] {
						// TODO: implement get() for headers.
						std::string sc = m_headers.get("Content-Length");
						if (!sc.empty())
						{
							try
							{
								m_content_length = std::atoi(sc.c_str());
							}
							catch (...)
							{
								LOG_ERROR("Can't parse content length '" << sc << "'");
								notify(erc::parse_size_error);
							}

							if (!on_content_length_changed())
							{
								return notify(erc::receive_size_error);
							}
						}
						else
						{
							LOG_ERROR("No content length received from the server");
							notify(erc::no_content_length);
							return true;
						}
						return 0;
					};

					std::size_t c(0); // Cursor

					if (m_is_header)
					{
						do
						{
							// Parse headers
							auto p = s.find("\r\n", c); // Header end position
							if (p != std::string::npos)
							{
								if (p == 0)
								{
									// Headers block finished
									m_is_header = false;
									if (auto r = on_headers_received())
									{
										LOG_DEBUG("Headrs parsing error: " << r);
										return false;
									}
									break;
								}
								else
								{
									// Header
									std::string_view h(s.begin() + c, s.begin() + p);
									auto colon_p = h.find_first_of(":", c);
									if (colon_p != std::string::npos)
									{
										auto n = h.substr(0, colon_p); // Header name
										auto v = h.substr(colon_p);
										m_headers.add({ n, v });
									}
									else
									{
										// Parse HTTP status
										auto hp = h.find("HTTP/1");
										if (hp != std::string::npos)
										{
											auto sp = h.find_first_of(' '); // Begin of status
											if (sp != std::string::npos)
											{
												auto spe = h.find_first_of(' ', sp + 1);// End of status
												if (spe != std::string::npos)
												{
													try
													{
														m_status = std::atoi(h.substr(sp + 1, spe - sp).data());
													}
													catch (const std::exception& ex)
													{
														LOG_ERROR("Can't parse HTTP status");
														return !!notify(erc::status_parse_error);
													}
												}
											}
										}
									}
								}
								c = p + 2;
							}
							else
							{
								// Store remaining data to parse with the next packet
								m_last_part = s.substr(c);
								break;
							}
						} while (true);
					}

					if (m_content_length >= 0)
						if (on_receive) // Pass control to the user callback
							PROFILE_TIME(return on_receive(m_headers, data, sz));
					
				return true;
			});
		}
		else
		{
			m_error_code = erc::connection_error;
		}
	}

	void http_client::wait() {
		LOG_DEBUG("Wait end of response...");
		m_cv.wait(m_cv_ul);
		LOG_DEBUG("Response received.");
		if (m_client->is_connected())
			m_client->disconnect();
	}

	int http_client::query(
		const std::string& host,
		int port,
		const std::string& method,
		const std::string& query,
		const http_response_cb& on_receive,
		const headers_t& headers,
		const std::string& body
	)
	{
		query_async(host, port, method, query, on_receive, headers, body);
		wait();
		return m_error_code;
	}

	void http_client::query_async(
		const std::string& host,
		int port,
		const std::string& method,
		const std::string& query,
		const http_response_cb& on_receive,
		const headers_t& headers,
		const std::string& body
	)
	{
		std::string req = utils::format_str(
			"%s %s HTTP/1.1\r\n"
			, utils::str_toupper(method).c_str()
			, query.c_str()
		);
		std::map<std::string, std::string> def_headers{
			{ "Connection", "keep-alive" },
			{ "Host", host }
		};

		auto add_header = [&](const std::string& name, const std::string& value) {
			req += header({ name, value }).to_string();
		};

		for (auto&& h : headers.data)
		{
			add_header(h.name, h.value);
			auto it = def_headers.find(h.name);
			if (it != def_headers.end())
				def_headers.erase(it);
		}

		for (auto&& [n, v] : def_headers)
			add_header(n, v);
		
		req += "\r\n";
		req += body;

		request_async(
			host
			, port
			, req
			, on_receive
		);
	}

	int http_client::notify(int ec)
	{
		on_before_notify(ec);
		m_error_code = ec;
		m_cv.notify_one();
		on_notify(ec);
		return m_error_code;
	}

	void http_client::reset()
	{
		LOG_DEBUG("http_client::reset()");
		m_error_code = erc::unknown;
		m_client = std::make_unique<anp::tcp::client>();
		m_is_header = true;
		on_reset();
	}
	
	// Begin of headers_t
	std::string http_client::headers_t::to_string() const
	{
		std::string r;
		for (auto&& h : data)
			r += h.to_string();
		return r;
	}

	http_client::headers_t::operator std::string() const
	{
		return to_string();
	}

	void http_client::headers_t::add(const http_client::header& h)
	{
		data.push_back(h);
	}
	// End of headers_t

	// Begin of header
	std::string http_client::header::to_string() const
	{
		return utils::format_str("%s: %s\r\n", name.c_str(), value.c_str());
	}

	http_client::header::operator std::string() const
	{
		return to_string();
	}
	// End of header
	
}