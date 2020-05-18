#include "Helper.h"

std::shared_ptr<std::vector<size_t>> Helper::destroy_order_ = std::make_shared<std::vector<size_t>>();
size_t Helper::move_counter_ = 0;