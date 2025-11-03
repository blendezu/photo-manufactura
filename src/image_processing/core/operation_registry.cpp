#include "operation_registry.h"

#include <algorithm>

OperationRegistry& OperationRegistry::getInstance() {
    static OperationRegistry instance;
    return instance;
}

OperationRegistry::OperationRegistry() {
    registerDefaultFilters();
}

void OperationRegistry::registerDefaultFilters() {
    // this is for Default-Filter
    // for example:
    // registerFilter("Vintage", []() { return std::make_shared<VintageFilter>(); },
    //               Category::VINTAGE, "Vintage Look mit weichen Farben", "vintage");
}

void OperationRegistry::registerFiler(const std::string& name, OperationFactory factory,
                                      Category category, const std::string& description,
                                      const std::string& iconName) {
    factories[name] = factory;

    FilterInfo info;
    info.name = name;
    info.category = category;
    info.description = description;
    info.iconName = iconName;
    filterInfos[name] = info;

    filtersByCategory[category].push_back(name);
}

std::vector<std::string> OperationRegistry::getFilterByCategory(Category category) const {
    auto it = filtersByCategory.find(category);
    if (it != filtersByCategory.end()) {
        return it->second;
    }
    return {};
}

OperationRegistry::FilterInfo OperationRegistry::getFilterInfo(const std::string& name) const {
    auto it = filterInfos.find(name);
    if (it != filterInfos.end()) {
        return it->second;
    }

    // Fallback for non registered filter
    return FilterInfo({name, Category::BINARY, "", ""});
}

std::string OperationRegistry::categoryToString(Category category) {
    switch (category) {
        case Category::COLOR_EFFECTS:
            return "Color effects";
        case Category::VINTAGE:
            return "Vintage";
            // ....
        default:
            return "General";
    }
}