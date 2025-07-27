// item.cpp : Defines the entry point for the application.
//
#include <cassert>
#include <iostream>
#include <string_view>
#include <algorithm>
#include <iterator>
#include <iomanip>
#include <fstream>
#include <exception>
#include <cstdlib>
#include <filesystem>
#include <utils/io_utils.h>
#include <utils/string_utils.h>
#include "item.h"

namespace
{
	const auto nutrition_input_size = 5;
	// Data
	const std::string item_info_fname = std::filesystem::temp_directory_path().append("nc_item_info.txt").string();
	const std::string input_fname = std::filesystem::temp_directory_path().append("nc_input.txt").string();
	std::ifstream fi_item_info;
	std::ofstream fo_item_info;

	// Functions
	bool parse_aliases(const std::string_view& data, item_info::aliases_list_t& to)
	{
		auto aliases = utils::split(data, "=");
		std::transform(
			aliases.begin()
			, aliases.end()
			, std::inserter(to, to.end())
			, [&](auto&& v) {
			return std::string(v);
		});
		return true;
	}

	bool parse_nutrition(const std::string& data, std::vector<float>& nutrition)
	{
		assert(nutrition.empty());
		constexpr std::string_view delim("/");
		auto v = utils::split(data, "/");
		try
		{
			// Iterate over the parts and convert them to float except the last one
			auto size = v.size();
			if (size > nutrition_input_size)
			{
				std::cerr << "Error: Too many nutrition values provided. Expected at most 5 values.\n";
				assert(false);
				return false;
			}
			nutrition.reserve(v.size() - 1);
			// Take the weight and convert it to a factor for aligning the nutrition values to 100 grams
			auto weight = 100.f;
			auto it_size = v.end();
			if (size == nutrition_input_size) // Weight is specified.
			{
				it_size = std::next(v.begin(), size - 1);
				weight = std::stof(std::string(*it_size));
				if (weight <= 0)
				{
					std::cerr << "Error: Invalid weight value provided: " << weight << ". Weight must be greater than 0.\n";
					assert(false);
					return false;
				}
			}
			auto factor = 100.f / weight;
			for (auto it = v.begin(); it != it_size; ++it)
			{
				float value = std::stof(std::string(*it));
				if (value < 0)
				{
					std::cerr << "Error: Negative nutrition value provided: " << value << ". Nutrition values must be non-negative.\n";
					assert(false);
					return false;
				}
				nutrition.push_back(value * factor);
			}
		}
		catch (const std::invalid_argument& e)
		{
			return false;
		}
		return true;
	}

	bool parse_calories(const std::string& data, float& to)
	{
		try
		{
			to = std::stof(data);
		}
		catch (const std::invalid_argument& e)
		{
			return false;
		}
		return true;
	}

	void offer_calories_replacement(float from, float& to)
	{
		if (from != to)
		{
			assert(from >= 0);
			std::cout << "Calories calculated from nutrition: " << from << ". Do you want to replace the stored value of " << to << "? (y/n): ";
			char answer;
			std::cin >> answer;
			if (answer == 'y' || answer == 'Y')
				to = from;
		}
	}
}

// Begin of item
// Operator >>
std::istream& operator >> (std::istream& is, item& obj)
{
	// Title
	if (obj.title.empty())
		if (!item_info::enter_title(obj.title, is))
			return is;

	if (!obj)
		if (auto info = item_info::load(obj.title))
			obj.set_info(info);
		else
		{
			try
			{
				if (utils::input::ask_user("Info of item '" + obj.title + "' not found. Would you like to create it?"))
				{
					obj.set_info(std::make_shared<item_info>(obj.title));
					is >> obj.info();
				}
				else
					return is;
			}
			catch (std::string s)
			{
				std::cout << "Emergency exit (" << s << ")\n";
				return is;
			}
		}

	// Weight
	std::cout << "\t" << "grams: ";
	while (!utils::input::input_t(obj.weight, is, input_fname));
	
	return is;
}

void item::print_nutrition()
{
	info().print_nutrition(weight);
}
// End of item

// Begin of item_info
void item_info::print_nutrition(float weight)
{
	int i = 0;
	for (auto&& n : nutrition)
		std::cout << nutrition_title(i++) << ": "
		 << n * weight / 100.f << "\t";
	std::cout << "Cal: " << cal * weight / 100.f;
	std::cout << "\n";
}

bool item_info::enter_title(std::string& to, std::istream& is)
{
	int trial = 0;
	std::cout << "New item title (or type 'exit' to finish): ";
	do {
		//if (trial > 0)
		//	std::cout << "\tInvalid name '" << to << "'. Enter again (or type 'exit' to finish): ";
		utils::input::input_line(to, is, input_fname);
		if (!utils::input::last_getline_valid())
			return false;
		//auto it = std::remove_if(to.begin(), to.end(), isspace);
		//if (it != to.end())
		//	to.erase(it, to.end());
		trial++;
	} while (to.empty());
	return true;
}

item_info_ptr item_info::load(const std::string& item_title)
{
	item_info_ptr ret(nullptr);
	if (!fi_item_info.is_open())
	{
		fo_item_info.open(item_info_fname, std::ios::app);
		fo_item_info.seekp(0, std::ios::end);
		fi_item_info.open(item_info_fname);
	}
	fi_item_info.clear();
	fi_item_info.seekg(0);
	while (true)
	{
		std::string line;
		getline(fi_item_info, line);
		if (line.empty())
			break;
		auto v = utils::split(line, "\t");
		// TODO: use streams
		int i = 0;
		for (auto&& p : v)
		{
			switch (i)
			{
				case 0: // Title
				{
					aliases_list_t aliases;
					parse_aliases(p, aliases);
					if (aliases.find(item_title) == aliases.end())
						break;
					else
					{
						ret = std::make_shared<item_info>();
						ret->title = *aliases.begin();
					}
					break;
				}
				case 1:	// Nutrition
					if (ret)
						if (!parse_nutrition(std::string(p), ret->nutrition))
						{
							std::cin >> *ret;
							break;
						}
					break;

				case 2: // Callories
					if (ret)
					{
						parse_calories(std::string(p), ret->cal);
						auto kcal = ret->calc_calories();
						offer_calories_replacement(kcal, ret->cal);
					}
					break;
			}
			i++;
		}
		if (ret)
			break;
	}
	return ret;
}

float item_info::calc_calories() const
{
	return nutrition[0] * 4 + nutrition[1] * 9 + nutrition[2] * 4;
}

// Operator >>
std::istream& operator >> (std::istream& is, item_info& obj)
{
	// Title
	if (obj.title.empty())
		if (!item_info::enter_title(obj.title, is))
			return is;
	fo_item_info << obj.title << "\t";
	fo_item_info.flush();
	
	// Nutrition
	obj.enter_nutrition(is);
	fo_item_info << "\t";
	fo_item_info.flush();

	// Cal
	obj.enter_cal(is);
	fo_item_info << "\n";
	fo_item_info.flush();

	return is;
}

bool item_info::enter_nutrition(std::istream& is)
{
	int trial = 0;
	std::string pfcfibw;
	while (nutrition.size() != nutrition_input_size)
	{
		nutrition.clear();
		pfcfibw.clear();
		if (trial == 0)
			std::cout << "\t" << "p/f/c/fib/w: ";
		else
			std::cout << "\t" << "Wrong format\np/f/c/fib/w: ";
		do {
			if (!utils::input::input_line(pfcfibw, is))
				return false;
			auto it = std::remove_if(pfcfibw.begin(), pfcfibw.end(), isspace);
			if (it != pfcfibw.end())
				pfcfibw.erase(it, pfcfibw.end());
		} while (pfcfibw.empty());

		parse_nutrition(pfcfibw, nutrition);
		trial++;
	}
	fo_item_info << pfcfibw;
	fo_item_info.flush();
	return true;
}

bool item_info::enter_cal(std::istream& is)
{
	std::string s;
	std::cout << "\t" << "Calories: ";
	
	while (!utils::input::input_t(cal, is));
	
	fo_item_info << cal;
	fo_item_info.flush();

	return true;
}
// End of item_info