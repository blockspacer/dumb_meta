#include "inspection.h"
#include <rttr/type.h>

static TypeInspectorRegistry global_inspector{};

void meta_system_initialization() {

#define REGISTER_INSPECTOR(T)                                                  \
  static InspectorHelper<T> insp_##T{};                                        \
  global_inspector.register_inspector(rttr::type::get<T>(), insp_##T)

#define REGISTER_INSPECTOREx(T, NAME)                                          \
  static InspectorHelper<T> insp_##NAME{};                                     \
  global_inspector.register_inspector(rttr::type::get<T>(), insp_##NAME)

  REGISTER_INSPECTOR(bool);

  REGISTER_INSPECTOR(char);

  REGISTER_INSPECTOREx(short int, s16);

  REGISTER_INSPECTOREx(unsigned short int, u16);

  REGISTER_INSPECTOR(int);

  REGISTER_INSPECTOREx(unsigned int, ui);

  REGISTER_INSPECTOREx(long long int, ll);

  REGISTER_INSPECTOREx(unsigned long long int, ull);

  REGISTER_INSPECTOR(float);

  REGISTER_INSPECTOR(double);
  using namespace std;
  REGISTER_INSPECTOR(string);

#undef REGISTER_INSPECTOR
}

bool inspect(rttr::variant &var, bool readonly,
             TypeInspector::meta_provider meta) {
  return global_inspector.inspect(var, readonly, meta);
}
