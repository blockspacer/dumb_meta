#include "meta_type_handler.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <rttr/registration.h>
#pragma optimize("", off)

namespace {

// sample usage dumb_template_engine("the id is {{id}} and name is {{name}}.",
// {{"id", "123"}, {"name", "hello"}});
std::string
dumb_template_engine(const std::string &input,
                     std::unordered_map<std::string, std::string> props) {
  const int place_holder_size = 2;

  std::string output = input;

  auto pos = output.find("{{");

  if (pos == std::string::npos)
    return output; // nothing to do

  do {
    auto next_pos = output.find("}}", pos);

    if (next_pos == std::string::npos)
      return {}; // unbalanced bracket

    std::string keyword(output, pos + place_holder_size,
                        next_pos - pos - place_holder_size);
    std::string val = props[keyword];
    output.replace(begin(output) + pos,
                   begin(output) + next_pos + place_holder_size, val);

    pos = output.find("{{", pos + place_holder_size);
  } while (pos != std::string::npos);

  return output;
}

std::string
rttr_template_gen(const std::unique_ptr<TypeInterface::Type> &type) {

  auto access_map = [](std::string name, TypeInterface::AccessLevel level)
      -> std::pair<std::string, std::string> {
    const char *level_mapping[] = {"registration::public_access",
                                   "registration::protected_access",
                                   "registration::private_access"};

    return {name, level_mapping[level]};
  };
  // only for cxxrecord for now
  if (type->is_container) {
    TypeInterface::TypeContainer *cls =
        static_cast<TypeInterface::TypeContainer *>(type.get());

    const std::string global_tmpl = R"(
{
    registration::class_<{{type_name}}>("{{type_name}}")
        {{props}}
        {{methods}}
        ;
}
)";
    const std::string meta_templ =
        R"-( metadata("{{meta_key}}", {{meta_key}} { {{meta_value}} }) )-";

    const std::string prop_templ =
        R"(
.property("{{name}}", &{{type_name}}::{{prop}}, {{access}})
{{meta}})";
    const std::string prop_readonly_templ =
        R"(
.property_readonly("{{name}}", &{{type_name}}::{{prop}}, {{access}})
{{meta}})";
    const std::string method_templ =
        R"(
.method("{{name}}", &{{type_name}}::{{method}}, {{access}})
{{meta}})";
    const std::string ctor_templ =
        R"(
.constructor<{{args}}>({{access}})
{{meta}})";

    std::string prop{};
    std::string method{};

    for (auto &p : cls->fields) {
      std::string meta{"("};
      for (auto &attr : p.type->attrs) {
        meta += dumb_template_engine(meta_templ, {{"meta_key", attr.key},
                                                  {"meta_value", attr.value}}) +
                ",";
      }
      if (meta.size() > 1)
        meta[meta.size() - 1] = ')';
      else
        meta.erase(0);

      if (p.is_readonly) {
        prop += dumb_template_engine(prop_readonly_templ,
                                     {{"type_name", cls->qualified_name},
                                      {"name", p.name},
                                      {"prop", p.name},
                                      {"meta", meta},
                                      access_map("access", p.acc)});
      } else {
        prop += dumb_template_engine(prop_templ,
                                     {{"type_name", cls->qualified_name},
                                      {"name", p.name},
                                      {"prop", p.name},
                                      {"meta", meta},
                                      access_map("access", p.acc)});
      }
    }

    for (auto &m : cls->methods) {

      std::string meta{};
      for (auto &attr : m.attrs) {
        meta += dumb_template_engine(meta_templ, {{"meta_key", attr.key},
                                                  {"meta_value", attr.value}}) +
                ",";
      }
      if (meta.size() > 1)
        meta[meta.size() - 1] = ')';
      else
        meta.erase(0);

      if (m.is_ctor) {
        std::string params{};

        for (auto &p : m.params) {
          params += p.qualified_name + ",";
        }

        if (!params.empty()) {
          params.erase(params.size() - 1);
        }

        method += dumb_template_engine(
            ctor_templ,
            {{"args", params}, {"meta", meta}, access_map("access", m.acc)});
      } else {
        method += dumb_template_engine(method_templ,
                                       {{"type_name", cls->qualified_name},
                                        {"name", m.name},
                                        {"method", m.name},
                                        {"meta", meta},
                                        access_map("access", m.acc)});
      }
    }

    return dumb_template_engine(global_tmpl,
                                {{"type_name", cls->qualified_name},
                                 {"props", prop},
                                 {"methods", method}});
  } else {
    return {};
  }
}
} // namespace

bool process_type(
    const std::vector<std::unique_ptr<TypeInterface::Type>> &types,
    const std::string &storage_path) {
  using namespace rttr;

  std::string header = R"(
#include <rttr/registration>

)";

  std::string content{};

  if (!storage_path.empty()) {
    for (auto &type : types) {
      auto str = rttr_template_gen(type);

      header += "#include \"" + type->source_loc + "\"\n";

      content += "// " + type->source_loc;
      content += str;
    }

    header += R"(RTTR_REGISTRATION
{
 using namespace rttr;
)";
    content += "}";

    content = header + content;

    std::ofstream writer{storage_path};
    if (writer.good()) {
      writer.write(content.c_str(), content.size());
      writer.flush();

      return true;
    } else {
      return false;
    }

  } else {
    return false;
  }
}
