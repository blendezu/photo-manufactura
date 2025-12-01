#include "operation_registry.h"

#include <memory>

#include "../operations/effects/gray_image.h"
#include "../operations/effects/vintage1.h"

OperationRegistry& OperationRegistry::getInstance() {
    static OperationRegistry instance;
    return instance;
}

OperationRegistry::OperationRegistry() {
    registerDefaultFilters();
}

void OperationRegistry::registerDefaultFilters() {
    registerFilter(
        "Vintage 1", []() { return std::make_shared<Vintage1>(); }, Category::VINTAGE,
        "Vintage Look mit weichen Farben", "vintage");

    registerFilter(
        "Gray Image", []() { return std::make_shared<GrayImage>(); }, Category::MONOCHROME,
        "Monochrome image with only one channel", "monochrome");
}

void OperationRegistry::registerFilter(const std::string& name, OperationFactory factory,
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

std::vector<std::string> OperationRegistry::getFiltersByCategory(Category category) const {
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
    return FilterInfo({name, Category::GENERAL, "", ""});
}

std::string OperationRegistry::categoryToString(Category category) {
    switch (category) {
        case Category::MONOCHROME:
            return "Monochrome";
        case Category::VINTAGE:
            return "Vintage";
        default:
            return "General";
    }
}