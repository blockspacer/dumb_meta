#pragma once

// dont process this file
#ifndef REFLECT

#include <rttr/type.h>
void meta_system_initialization();

// type inspection support interface
class TypeInspector {
public:
	using meta_provider = std::function<rttr::variant(const rttr::variant &)>;
	virtual ~TypeInspector() = default;
	virtual bool inspect(rttr::variant &var, bool readonly,
		meta_provider meta) = 0;	
};

bool inspect(rttr::variant &var, bool readonly,
	TypeInspector::meta_provider meta);

class TypeInspectorRegistry : public TypeInspector {
public:
	bool inspect(rttr::variant &var, bool readonly, meta_provider meta) override;

	void register_inspector(rttr::type type, TypeInspector &inspector);
	void deregister_inspector(rttr::type type);
	bool has_inspector(rttr::type type) const;

private:
	std::unordered_map<rttr::type, TypeInspector *> registry_;

	bool inspect_internal(std::string name, rttr::variant &var, bool readonly,
		meta_provider meta);
};

// for custom type inspection, just specialize this template class
template <typename T> class InspectorHelper : public TypeInspector {
public:
	using value_type = T;
	bool inspect(rttr::variant &var, bool readonly, meta_provider meta) override {
		auto type = var.get_type().get_raw_type(); // to handle pointer
		if (type == rttr::type::get<T>()) {
			return inspect_internal(var, readonly, meta);
		}
		else {
			return false;
		}
	}

private:
	bool inspect_internal(rttr::variant &var, bool readonly, meta_provider meta);
};

// type description
template <class T> std::string to_string(T t) { return std::to_string(t); }


// misc helpers

#define MAKE_INSPECTOR(T)                                                      \
  template <>                                                                  \
  bool InspectorHelper<T>::inspect_internal(rttr::variant &var, bool readonly, \
                                            meta_provider meta_getter)

#define MAKE_INSPECTOR_PREAMBLE                                                \
  rttr::type type = var.get_type();                                            \
  value_type* value_ptr = nullptr;                                             \
  if(type.is_pointer()){                                                       \
    value_ptr = var.get_value<value_type*>();                                  \
    if(value_ptr == nullptr){                                                  \
      ImGui::Text("nullptr");                                                  \
      return false;                                                            \
    }                                                                          \
  }                                                                            \
  value_type &value = value_ptr ? *value_ptr : var.get_value<value_type>();    \
  if (readonly) {                                                              \
    ImGui::Text("%s", to_string(value));                                       \
    return false;                                                              \
  }

#endif