// main.cpp : Defines the entry point for the application.
//

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <string_view>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <exception>
#include <cstdlib>
#include <filesystem>
#include <utils/string_utils.h>
#include <utils/io_utils.h>
#include <utils/file_utils.h>
#include <utils/datetime.h>
#include <utils/string_utils.h>
#include <utils/Log.h>
#include <tcp/client.h>
#include "item.h"

LOG_POSTFIX("\n");

const std::filesystem::path items_fname = std::filesystem::temp_directory_path().append("item_info.txt").string();
const std::filesystem::path input_fname = std::filesystem::temp_directory_path().append("input.txt").string();
std::ofstream items_fo;
std::ifstream items_fi;

using items_list_t = std::vector<item>;

void load_items(items_list_t& to)
{
	while (!items_fi.eof())
		items_fi >> to.emplace_back();
}

void store_item(const item& item)
{
	items_fo << item;
}

bool enter_item(item& to)
{
	// Title
	std::string title;
	if (!item_info::enter_title(title, std::cin))
		return false;

	to.title = title;
	
	if (auto info = item_info::load(title))
	{
		to.set_info(info);
		std::cout << "Item info '" << title << "' found: ";
		to.info().print_nutrition(100.f);
	}
	std::cin >> to;

	return true;
}

int main()
{
	std::cout << "Nutrition Calculator" << std::endl;

	anp::tcp::client c;


	//items_fo.open(items_fname, std::ios::app | std::ios::binary);
	//items_fi.open(items_fname, std::ios::binary);

	items_list_t items;
	//load_items(items);

	auto finish_input = [&] {
		utils::input::close_input();
		auto cur_dt = utils::current_datetime("%02i-%02i-%02i-%03li");
		auto new_fname_input = std::filesystem::path(input_fname.parent_path() / std::filesystem::path(utils::format_str("input-%s.txt", cur_dt.c_str())));
		auto new_fname_info = std::filesystem::path(items_fname.parent_path() / utils::format_str("item_info-%s.txt", cur_dt.c_str()));
		utils::move_file(input_fname.string(), new_fname_input.string());
		utils::copy_file(items_fname.string(), new_fname_info.string());
		// Exit from the input loop
		return false;
	};

	utils::input::register_command("exit");
	utils::input::register_command("end", finish_input);
	utils::input::register_command("new", finish_input);
	utils::input::register_command("total");
	utils::input::register_command("remove_last", [&] {
		items.resize(items.size() - 1);
		utils::file_remove_last_line_f(*utils::input::get_file());
		return true;
	});
	utils::input::register_command("temp_dir", [] {
		LOG(std::filesystem::temp_directory_path());
		return true;
	});

	while (utils::input::last_command != "exit")
	{
		while (
					utils::input::last_command != "end"
				&&	utils::input::last_command != "exit"
				&&	utils::input::last_command != "new"
				&&	utils::input::last_command != "total"	
		)
		{
			item item;

			enter_item(item);

			if (!utils::input::last_getline_valid)
				continue;
			item.print_nutrition();
			items.push_back(item);
			//store_item(item);
		}

		// Sort the items
		std::sort(items.begin(), items.end(), [](auto&& l, auto&& r) {
			float ln = 0.f, rn = 0.f;
			for (auto&& n : l.info().nutrition)
				ln += n;
			ln *= l.weight;
			for (auto&& n : r.info().nutrition)
				rn += n;
			rn *= r.weight;
			return ln > rn;
		});

		std::cout << std::setw(34) << "title |";
		std::cout << std::setw(10) << "w (g) |";
		std::cout << std::setw(10) << "p (g) |";
		std::cout << std::setw(10) << "f (g) |";
		std::cout << std::setw(10) << "c (g) |";
		std::cout << std::setw(10) << "fib (g) |";
		std::cout << std::setw(10) << "cal |";
		std::cout << "\n";

		item_info total_info;
		total_info.nutrition.resize(4);
		float total_weight = 0.f;

		for (auto item : items)
		{
			std::cout << std::setw(32) << item.info().title << " |";
			std::cout << std::setw(8) << item.weight << " |";
			int i = 0;
			for (auto n : item.info().nutrition)
			{
				float val = n * item.weight / 100.f;
				std::cout << std::setw(8) << val << " |";
				total_info.nutrition[i++] += val;
			}
			float val = item.info().cal * item.weight / 100.f;
			std::cout << std::setw(8) << val << " |";
			total_info.cal += val;
			total_weight += item.weight;
			std::cout << "\n";
		}

		std::cout << std::setw(34) << "Total: |";
		std::cout << std::setw(8) << total_weight << " |";
		for (auto n : total_info.nutrition)
			std::cout << std::setw(8) << n << " |";
		std::cout << std::setw(8) << total_info.cal << " |";
		std::cout << "\n";

		if (utils::input::last_command != "total")
			items.clear();

		if (utils::input::last_command != "exit")
			utils::input::reset_last_input();
	}
	return 0;
}

// TODO:
//	* store and load items instead of input
//	* date in file names
//	* end <fname>
//	* from <fname> -load from a file
//	* edit_info <item_title>
//	*'category' command