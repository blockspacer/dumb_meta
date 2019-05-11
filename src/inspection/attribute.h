#pragma once

// common attributes

template <typename T> struct Range { T min, max; };
using RangeF = Range<float>;
using RangeI = Range<int>;
using RangeS = Range<short>;
using RangeC = Range<char>;
