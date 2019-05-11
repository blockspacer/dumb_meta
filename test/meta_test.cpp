#include "imgui.h"
#include "prop.h"
#include <functional>
#include <limits>
#include <rttr/type.h>
//#include <rttr/variant.h>
#include <string>
#include <unordered_map>

#include "inspection/inspection.h"
#pragma optimize( "", off )  

rttr::variant inst;
void user_init() {
  meta_system_initialization();
  inst = test::Human{};

  auto &h = inst.get_value<test::Human>();
  h.remote_id = &h.age;
}

void user_update() {
  ImGui::Begin("orz");

  inspect(inst, false, {});

  ImGui::End();
}

void user_destroy() {}
