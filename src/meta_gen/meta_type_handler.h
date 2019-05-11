#pragma once
#include "type_info.h"

bool process_type(const std::vector<std::unique_ptr<TypeInterface::Type>> &types, const std::string& storage_path);