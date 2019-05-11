#include "attribute.h"
#include "imgui.h"
#include "inspection.h"
#include <limits>
#include <rttr/type.h>
#include <string>
#include <type_traits>

namespace {
// imgui type mapping
template <typename T> ImGuiDataType type_map();
template <> ImGuiDataType type_map<int>() {
  return ImGuiDataType_::ImGuiDataType_S32;
}
template <> ImGuiDataType type_map<unsigned int>() {
  return ImGuiDataType_::ImGuiDataType_U32;
}
template <> ImGuiDataType type_map<long long int>() {
  return ImGuiDataType_::ImGuiDataType_S64;
}
template <> ImGuiDataType type_map<unsigned long long int>() {
  return ImGuiDataType_::ImGuiDataType_U64;
}
} // namespace

template <> std::string to_string<std::string>(std::string str) { return str; }

MAKE_INSPECTOR(std::string) {
  MAKE_INSPECTOR_PREAMBLE

  constexpr int size = 255;
  std::unique_ptr<char[]> mem(new char[size]);
  std::memset(mem.get(), 0, size);
  value.copy(mem.get(), value.size());

  bool changed = ImGui::InputText("", mem.get(), size);
  if (changed && !readonly) {
    value = mem.get();
  }

  return changed;
}

MAKE_INSPECTOR(bool) {
  MAKE_INSPECTOR_PREAMBLE
  return ImGui::Checkbox("", &value);
}
MAKE_INSPECTOR(char) {
  MAKE_INSPECTOR_PREAMBLE

  Range<value_type> range{std::numeric_limits<value_type>::min(),
                          std::numeric_limits<value_type>::max()};

  if (auto &meta = meta_getter(std::string{"RangeC"})) {
    range = meta.get_value<Range<value_type>>();
  }

  int upper = value;
  bool changed = ImGui::SliderInt("", &upper, range.min, range.max);
  if (changed && !readonly) {
    value = static_cast<value_type>(upper);
  }

  return changed;
}
MAKE_INSPECTOR(short) {
  MAKE_INSPECTOR_PREAMBLE

  Range<value_type> range{std::numeric_limits<value_type>::min(),
                          std::numeric_limits<value_type>::max()};

  if (auto &meta = meta_getter(std::string{"RangeS"})) {
    range = meta.get_value<Range<value_type>>();
  }

  int upper = value;
  bool changed = ImGui::SliderInt("", &upper, range.min, range.max);
  if (changed && !readonly) {
    value = static_cast<value_type>(upper);
  }

  return changed;
}
MAKE_INSPECTOR(unsigned short) {
  MAKE_INSPECTOR_PREAMBLE

  Range<value_type> range{std::numeric_limits<value_type>::min(),
                          std::numeric_limits<value_type>::max()};

  if (auto &meta = meta_getter(std::string{"RangeS"})) {
    range = meta.get_value<Range<value_type>>();
  }

  int upper = value;
  bool changed = ImGui::SliderScalar("", ImGuiDataType_U32, &upper, &range.min,
                                     &range.max);
  if (changed && !readonly) {
    value = static_cast<value_type>(upper);
  }

  return changed;
}

#define MAKE_INTEGRAL_INSPECTOR                                                \
  MAKE_INSPECTOR_PREAMBLE                                                      \
  Range<value_type> range{std::numeric_limits<value_type>::min(),              \
                          std::numeric_limits<value_type>::max()};             \
  if (auto &meta = meta_getter(std::string{"RangeI"})) {                       \
    range = meta.get_value<Range<value_type>>();                               \
  }                                                                            \
  return ImGui::SliderScalar("", type_map<value_type>(), &value, &range.min,   \
                             &range.max);

MAKE_INSPECTOR(int) { MAKE_INTEGRAL_INSPECTOR; }
MAKE_INSPECTOR(unsigned int) { MAKE_INTEGRAL_INSPECTOR; }
MAKE_INSPECTOR(long long int) { MAKE_INTEGRAL_INSPECTOR; }
MAKE_INSPECTOR(unsigned long long int) { MAKE_INTEGRAL_INSPECTOR; }
MAKE_INSPECTOR(float) {
  MAKE_INSPECTOR_PREAMBLE

  Range<value_type> range{std::numeric_limits<value_type>::min(),
                          std::numeric_limits<value_type>::max()};

  if (auto &meta = meta_getter(std::string{"RangeF"})) {
    range = meta.get_value<Range<value_type>>();
  }

  return ImGui::SliderFloat("", &value, range.min, range.max);
}
MAKE_INSPECTOR(double) {
  MAKE_INSPECTOR_PREAMBLE

  Range<value_type> range{std::numeric_limits<value_type>::min(),
                          std::numeric_limits<value_type>::max()};

  if (auto &meta = meta_getter(std::string{"RangeF"})) {
    range = meta.get_value<Range<value_type>>();
  }

  float v = static_cast<float>(value);

  bool changed = ImGui::SliderFloat("", &v, range.min, range.max);

  if (changed && !readonly) {
    value = v;
  }

  return changed;
}

#undef MAKE_INTEGRAL_INSPECTOR
