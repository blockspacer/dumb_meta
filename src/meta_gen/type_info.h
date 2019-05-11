#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace TypeInterface {
struct Attribute {
  std::string key;
  std::string value;
};
struct Type {
  std::string name;
  std::string qualified_name;
  std::string source_loc; // for debug
  int64_t size;           // in byte
  uint32_t alignment;
  bool is_container;
  std::vector<Attribute> attrs;
};

enum AccessLevel { E_public, E_protected, E_private, E_invalid };
struct Field {
  std::string name;
  std::unique_ptr<Type> type;
  uint64_t offset;
  AccessLevel acc;
  bool is_readonly;
};

struct Method {
  std::string name;
  bool is_static;
  bool is_ctor;
  AccessLevel acc;
  std::vector<Type> params;
  std::vector<Attribute> attrs;
};

struct TypeContainer : public Type {
  TypeContainer() { is_container = true; }
  std::vector<Field> fields;
  std::vector<Method> methods;
};
} // namespace TypeInterface
