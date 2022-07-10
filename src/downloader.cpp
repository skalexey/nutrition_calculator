// downloader.cpp

//#ifdef __cpp_lib_format
#include <string>
#include <functional>
#include <format>
//#endif
#include <fstream>
#include <tcp/client.h>
#include <utils/Log.h>
#include <utils/string_utils.h>
#include <utils/file_utils.h>
#include <utils/datetime.h>
#include <tcp/client.h>
#include "downloader.h"

LOG_PREFIX("[downloader]: ");
LOG_POSTFIX("\n");
SET_LOG_DEBUG(true);

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

		return query(host, port, "GET", q, [=](
			const std::vector<char>& data
			, std::size_t sz
			, int code
			)
			{
				LOG_VERBOSE("\nReceived " << sz << " bytes:");
				std::string s(data.begin(), data.begin() + sz);
				LOG_VERBOSE(s);

				auto on_content_length_changed = [=] {
					if (m_content_length <= 0)
					{ // Download is finished
						if (m_content_length < 0)
						{
							notify(erc::receive_size_error);
							return false;
						}

						if (m_f_tp != decltype(m_f_tp)())
						{
							if (m_f_tp < m_tp)
							{
								// Overwrite with the download
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
							{
								// The local file is newer;
								LOG_DEBUG("The local file is newer");
								auto bac_contents = utils::file_contents(this->m_backup);
								auto f_contents = utils::file_contents(m_target);
								if (std::hash<std::string>{}(bac_contents)
									!= std::hash<std::string>{}(f_contents))
									notify(erc::uncommitted_changes);
								else
								{
									LOG_DEBUG("files are the same");
									notify(http_client::erc::no_error);
								}
							}
						}
						else
						{
							notify(erc::file_error);
						}

						m_content_length = -1;
						return false;
					}
					// Download is going to begin
					return true;
				};

				if (m_content_length >= 0)
				{
					std::ofstream f(m_download, std::ios::app | std::ios::binary);
					f.write(data.data(), sz);
					f.close();
					m_content_length -= sz;
					assert(m_content_length >= 0);
					on_content_length_changed();
				}
				else
				{
					// Parse file size
					{
						std::string sc = parse_header(s, "Content-Length");
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
							return;
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
								const std::chrono::zoned_time local("Australia/Sydney", m_tp);
								LOG_VERBOSE("Received modification time: " << local);
								// TODO: use tz lib to support c++11/14/17:
								// https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c
								// https://howardhinnant.github.io/date/tz.html
								// https://github.com/HowardHinnant/date
								// date lib info:
								// https://howardhinnant.github.io/date/date.html
								// , %d %B %4C %H:%M:%S GMT
								LOG_DEBUG(std::format("Parsed date: {0:%a, %d %b %C%y %H:%M:%S GMT}", m_tp));
								m_f_tp = utils::file_modif_time(m_target);
								LOG_DEBUG(std::format("Local file date: {0:%a, %d %b %C%y %H:%M:%S GMT}", m_f_tp));
							}
							catch (...)
							{
								LOG_ERROR("Can't parse file modification date '" << sc << "'");
								notify(erc::parse_date_error);
								return;
							}
						}
						else
						{
							LOG_ERROR("No content length received from the server");
							notify(erc::no_content_length);
						}
					}
				}
			}
		);
	}

	bool downloader::backup_local_file()
	{
		assert(!m_target.empty());
		fs::path bac = m_target.parent_path() / fs::path(m_target.filename().string() + ".bac");
		if (utils::copy_file(m_target, bac) == 0)
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
		if (utils::move_file(m_download, m_target) == 0)
		{
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
		m_f_tp = decltype(m_f_tp)();
	}

	bool downloader::restore_from_backup()
	{
		if (!m_backup.empty())
			if (!m_target.empty())
				return utils::move_file(m_backup, m_target) == 0;
		return false;
	}

	bool downloader::remove_backup()
	{
		if (!m_backup.empty())
			return utils::remove_file(m_backup);
		return false;
	}
}