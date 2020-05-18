#pragma once

#include <UnorderedMap.h>
#include <unordered_set>

class Unit {};

template <class T>
using UnorderedSet = UnorderedMap<T, Unit>;