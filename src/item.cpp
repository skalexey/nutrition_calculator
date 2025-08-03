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
#include <limits>
#include <utils/io_utils.h>
#include <utils/string_utils.h>
#include "item.h"
#include "item_info_manager.h"

namespace
{
	const auto nutrition_input_size = 5;
	// Data
	const std::string item_info_fname = std::filesystem::temp_directory_path().append("nc_item_info.txt").string();
	const std::string input_fname = std::filesystem::temp_directory_path().append("nc_input.txt").string();

	// Helper function to get the global manager
	item_info_manager& get_manager()
	{
		return item_info_manager::get_instance();
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
					// Clear any remaining input after creating item_info
					is.clear();
					std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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

	// Weight - read from input stream (file) or interactively
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
		 << std::fixed << std::setprecision(1) << n * weight / 100.f << "\t";
	std::cout << "Cal: " << std::fixed << std::setprecision(1) << cal * weight / 100.f;
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
	return get_manager().find_item(item_title);
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
	
	// Nutrition
	obj.enter_nutrition(is);

	// Cal
	obj.enter_cal(is);
	
	// Check if calories need updating
	// Calculate calories from the NON-normalized nutrition values (for the reference weight)
	float non_normalized_nutrition[4];
	for (int i = 0; i < 4; i++) {
		non_normalized_nutrition[i] = obj.nutrition[i] * (obj.reference_weight / 100.0f);
	}
	float calculated_cal_for_reference = non_normalized_nutrition[0] * 4 + non_normalized_nutrition[1] * 9 + non_normalized_nutrition[2] * 4;
	
	if (std::abs(calculated_cal_for_reference - obj.cal) > 5.0f) // Compare non-normalized values
	{
		std::cout << "Calories calculated from nutrition: " << std::fixed << std::setprecision(1) << calculated_cal_for_reference 
				  << ". Do you want to replace the entered value of " << obj.cal << "? (y/n): ";
		char answer;
		std::cin >> answer;
		if (answer == 'y' || answer == 'Y') {
			// Store the calculated value, but normalize it to 100g
			obj.cal = calculated_cal_for_reference * (100.0f / obj.reference_weight);
		} else {
			// Store the entered value, but normalize it to 100g  
			obj.cal = obj.cal * (100.0f / obj.reference_weight);
		}
	}
	else {
		// If no significant difference, normalize the entered calories to 100g
		obj.cal = obj.cal * (100.0f / obj.reference_weight);
	}

	// Add to manager
	get_manager().add_item(std::make_shared<item_info>(obj));
	get_manager().save_all();

	return is;
}

bool item_info::enter_nutrition(std::istream& is)
{
	int trial = 0;
	std::string pfcf;
	float reference_weight = 0;
	
	while (nutrition.size() != 4)  // Changed from nutrition_input_size to 4
	{
		nutrition.clear();
		pfcf.clear();
		if (trial == 0)
			std::cout << "\t" << "p/f/c/fib: ";
		else
			std::cout << "\t" << "Wrong format\np/f/c/fib: ";
		do {
			if (!utils::input::input_line(pfcf, is))
				return false;
			auto it = std::remove_if(pfcf.begin(), pfcf.end(), isspace);
			if (it != pfcf.end())
				pfcf.erase(it, pfcf.end());
		} while (pfcf.empty());

		// Parse nutrition inline (simplified version)
		try {
			auto v = utils::split(pfcf, "/");
			if (v.size() >= 4) {
				nutrition.clear();
				for (size_t i = 0; i < 4 && i < v.size(); ++i) {
					nutrition.push_back(std::stof(std::string(v[i])));
				}
			}
		} catch (const std::exception&) {
			// Parsing failed, will retry
		}
		
		trial++;
	}
	
	// Ask for reference weight to normalize values to 100g
	std::cout << "\t" << "Reference weight (grams) for these nutrition values: ";
	while (!utils::input::input_t(reference_weight, is) || reference_weight <= 0) {
		std::cout << "\t" << "Please enter a positive weight: ";
	}
	
	// Store reference weight for calorie normalization
	this->reference_weight = reference_weight;
	
	// Normalize nutrition values to 100g
	if (reference_weight != 100.0f) {
		float factor = 100.0f / reference_weight;
		for (auto& value : nutrition) {
			value *= factor;
		}
		std::cout << "\t" << "Normalized values to 100g: " 
				  << std::fixed << std::setprecision(1)
				  << nutrition[0] << "/" << nutrition[1] << "/" 
				  << nutrition[2] << "/" << nutrition[3] << std::endl;
	}
	
	return true;
}

bool item_info::enter_cal(std::istream& is)
{
	std::cout << "\t" << "Calories: ";
	while (!utils::input::input_t(cal, is));
	return true;
}
// End of item_info