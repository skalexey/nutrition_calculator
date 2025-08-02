#pragma once

#include "item.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>

class item_info_manager
{
public:
    // Constructor
    item_info_manager(const std::filesystem::path& file_path);
    
    // Load all item infos from file
    bool load_all();
    
    // Save operations
    bool save_all();
    bool save_item_at_index(size_t index);
    
    // Access operations
    item_info_ptr find_item(const std::string& title);
    item_info_ptr get_item_at_index(size_t index);
    size_t get_item_count() const { return m_items.size(); }
    
    // Add new item
    void add_item(const item_info_ptr& info);
    
    // Edit existing item
    bool edit_item(const std::string& title);
    
    // Validation and maintenance
    void validate_and_fix_items();
    
    // Get items that need calorie review
    std::vector<size_t> get_items_needing_calorie_review() const;
    
    // Review and fix calories
    void review_calories();
    
    // Initialization function to be called from main
    static void initialize();
    static item_info_manager& get_instance();
    
    // Cleanup
    static void shutdown();

private:
    std::filesystem::path m_file_path;
    std::vector<item_info_ptr> m_items;
    std::unordered_map<std::string, size_t> m_title_to_index;
    
    // Helper functions
    void build_title_index();
    bool parse_line(const std::string& line, item_info_ptr& info);
    std::string serialize_item(const item_info& info);
    void update_file_line(size_t line_number, const std::string& content);
    
    // Validation helpers
    bool is_item_complete(const item_info& info) const;
    bool needs_calorie_review(const item_info& info) const;
};
