#pragma once

#include <unordered_set>
#include <UnorderedMap.h>

class Unit {};

template<class T>
using UnorderedSet = UnorderedMap<T, Unit>;