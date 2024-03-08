// authenticator.cpp

#include <string>
#include <algorithm>
#include <type_traits>
#ifdef __cpp_lib_format
#include <format>
#endif
#include <fstream>
#include <tcp/client.h>
#include <utils/log.h>
#include <utils/string_utils.h>
#include <utils/file_utils.h>
#include <utils/datetime.h>
#include <tcp/client.h>
#include "authenticator.h"

LOG_PREFIX("[authenticator]: ");
LOG_POSTFIX("\n");
SET_LOCAL_LOG_DEBUG(true);
namespace fs = std::filesystem;
namespace ch = std::chrono;

namespace anp
{
	int authenticator::auth(
		const std::string& host,
		int port,
		const std::string& user,
		const std::string& token
	)
	{
		return 0;
	}

	void authenticator::on_notify(int ec)
	{
	}

	void authenticator::on_reset()
	{
	}
}