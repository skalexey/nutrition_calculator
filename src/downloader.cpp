// downloader.cpp

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
#include "downloader.h"

LOG_PREFIX("[downloader]: ");
LOG_POSTFIX("\n");
SET_LOCAL_LOG_DEBUG(true);
namespace fs = std::filesystem;
namespace ch = std::chrono;

namespace anp
{
	int downloader::download_file(
		const std::string& host,
		int port,
		const std::string& q,
		const fs::path& target_path
	)
	{
		m_target = target_path;
		m_f_tp = utils::file::modif_time(m_target);
		LOG_DEBUG("	Init local file modification time: " << utils::time_to_string(m_f_tp));

		return query(host, port, "GET", q, [=, this](
			const std::vector<char>& data
			, std::size_t sz
			, int code
			) -> bool
			{
				LOG_DEBUG("\nReceived " << sz << " bytes:");
				std::string s(data.begin(), data.begin() + sz);
				LOG_DEBUG(s);

				//if (errcode() != http_client::erc::no_error 
				//	&& errcode() != http_client::erc::unknown)
				//	return false;

				if (s.find("Auth error") != std::string::npos)
				{
					notify(erc::auth_error);
					return false;
				}

				auto on_content_length_changed = [=, this] {
					if (m_content_length <= 0)
					{ // Download is finished
						if (m_content_length < 0)
						{
							notify(erc::receive_size_error);
							return false;
						}
						
						// If file exists, but modification time is null
						if (utils::file::exists(m_target) && m_f_tp == decltype(m_f_tp)())
						{
							notify(erc::file_error);
						}
						else
						{
							if (!utils::file::exists(m_target))
							{	// Create the file
								auto f = std::ofstream(m_target);
								f.close();
							}
							// Check if files contents differ
							bool content_differs = false;
							if (!utils::file::same(m_download, m_target))
								content_differs = true;

							// Check the modification time
							LOG_DEBUG("Check the modification time");
							if (m_f_tp < m_tp)
							{
								LOG_DEBUG("m_f_tp < m_tp (" << utils::time_to_string(m_f_tp) << ", " << utils::time_to_string(m_tp) << ")");
								// Overwrite with the download
								if (content_differs)
								{
									if (!backup_local_file())
										notify(erc::backup_error);
									else
									{
										if (replace_with_download())
											notify(http_client::erc::no_error);
										else
											notify(erc::store_download_error);
									}
								}
								else
									notify(http_client::erc::no_error);
							}
							else
							{
								// The local file is newer;
								LOG_DEBUG("The local file is newer");
								if (content_differs)
									notify(erc::uncommitted_changes);
								else
								{
									LOG_DEBUG("Files contents are the same");
									notify(http_client::erc::no_error);
								}
							}
						}

						m_content_length = -1;
						return false;
					}
					else
					{
						// Download is going to begin
						return true;
					}
				};
				
				bool is_header = false;
				
				if (m_content_length < 0)
				{
					// Parse file size
					{
						std::string sc = parse_header(s, "Content-Length");
						if (!sc.empty())
						{
							is_header = true;
							try
							{
								m_content_length = std::atoi(sc.c_str());
							}
							catch (...)
							{
								LOG_ERROR("Can't parse content length '" << sc << "'");
								notify(erc::parse_size_error);
							}

							if (on_content_length_changed())
							{
								// Start writing to a file
								create_download_file();
							}
						}
						else
						{
							LOG_ERROR("No content length received from the server");
							notify(erc::no_content_length);
							return true;
						}
					}
					// Parse date
					{
						std::string sc = parse_header(s, "Modification-Time");
						if (!sc.empty())
						{
							try
							{
								m_tp = utils::parse_datetime_http(sc);
#if !defined(__APPLE__) && !defined(__GNUC__)
								const std::chrono::zoned_time local("Australia/Sydney", m_tp);
								LOG_VERBOSE("Received modification time: " << local);
#endif
								// tz lib can be used to support C++11/14/17,
								// but it is already integrated into C++20.
								// Info:
								// https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
								// https://howardhinnant.github.io/date/tz.html
								// https://github.com/HowardHinnant/date
								// date lib info:
								// https://howardhinnant.github.io/date/date.html
								// , %d %B %4C %H:%M:%S GMT
								LOG_DEBUG("Parsed date: " << utils::time_to_string(m_tp));
								LOG_DEBUG("Local file date: " << utils::time_to_string(m_f_tp));
							}
							catch (...)
							{
								LOG_ERROR("Can't parse file modification date '" << sc << "'");
								notify(erc::parse_date_error);
								return true;
							}
						}
						else
						{
							LOG_ERROR("No modification date received from the server");
							notify(erc::no_file);
							return true;
						}
					}
				}
			
				if (m_content_length >= 0)
				{
					LOG_DEBUG("Downloaded data processing. m_content_length: " << m_content_length);
					const std::remove_reference<decltype(data)>::type::value_type* p = nullptr;
					std::size_t wsz = 0;
					if (is_header)
					{
						LOG_DEBUG("Parse header");
						decltype(data) term = {'\r', '\n', '\r', '\n'};
						auto it = std::search(data.begin(), data.end(), term.begin(), term.end());
						if (it != data.end())
						{
							auto offset = std::distance(data.begin(), it) + term.size();
							p = data.data() + offset;
							wsz = sz - offset;
							LOG_DEBUG("Header size: " << offset);
							LOG_DEBUG("Payload size: " << wsz);
							LOG_DEBUG("Whole data block size: " << sz);
							assert(wsz >= 0);
						}
					}
					else
					{
						LOG_DEBUG("Process the data payload");
						p = data.data();
						wsz = sz;
					}
					
					if (p != nullptr && wsz > 0)
					{
						LOG_DEBUG("Write the download to a file");
						std::ofstream f(m_download, std::ios::app | std::ios::binary);
						f.write(p, wsz);
						f.close();
						m_content_length -= wsz;
						assert(m_content_length >= 0);
						on_content_length_changed();
					}
				}
				return true;
			}
		);
	}

	bool downloader::is_file_updated()
	{
		return m_is_file_updated;
	}

	bool downloader::backup_local_file()
	{
		assert(!m_target.empty());
		fs::path bac = m_target.parent_path() / fs::path(m_target.filename().string() + ".bac");
		if (utils::file::copy_file(m_target, bac) == 0)
		{
			LOG_VERBOSE("Backup of the local file created in '" << bac << "'");
			m_backup = bac;
			return true;
		}
		LOG_ERROR("Error while backuping the local file '" << m_target << "' into '" << bac << "'");
		return false;
	}

	bool downloader::replace_with_download()
	{
		if (utils::file::move_file(m_download, m_target) == 0)
		{
			m_is_file_updated = true;
			LOG_VERBOSE("Download file successfully stored into the local file's path '" << m_target << "'");
			return true;
		}
		LOG_ERROR("Error while placing the downloaded file '" << m_download << "' into the target path '" << m_target << "'");
		return false;
	}

	void downloader::create_download_file()
	{
		m_download = m_target.string() + ".dwl";
		std::ofstream f(m_download, std::ios::trunc | std::ios::binary);
		f.close();
	}

	void downloader::on_notify(int ec)
	{
		if (errcode() == http_client::erc::no_error)
			remove_backup();
		else
			restore_from_backup();
	}

	void downloader::on_reset()
	{
		m_content_length = -1;
		m_backup.clear();
		m_download.clear();
		m_tp = decltype(m_tp)();
		LOG_DEBUG("Reset remote file modification time");
		m_is_file_updated = false;
	}

	bool downloader::restore_from_backup()
	{
		if (!m_backup.empty())
			if (!m_target.empty())
				return utils::file::move_file(m_backup, m_target) == 0;
		return false;
	}

	bool downloader::remove_backup()
	{
		if (!m_backup.empty())
			return utils::file::remove_file(m_backup);
		return false;
	}
}