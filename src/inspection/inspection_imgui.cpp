#include "inspection.h"
#include "rttr/type.h"
#include "imgui.h"
#include <cassert>

bool TypeInspectorRegistry::inspect_internal(std::string name,
	rttr::variant &var, bool readonly,
	meta_provider meta) {
	rttr::type _t = var.get_type();
	rttr::type target_t = _t.get_raw_type();

	const bool is_pointer = _t.is_pointer();
	
	//TODO: array and maybe container types, do I really need this?

	ImGui::PushID(&var);
	ImGui::AlignTextToFramePadding();

	bool changed = false;

	ImGui::NextColumn();

	ImGui::NextColumn();

	if (has_inspector(target_t)) {
		ImGui::AlignTextToFramePadding();
		ImGui::Text("%s%s", name.c_str(), is_pointer ? "*" : "");
		ImGui::NextColumn();
		changed = registry_[target_t]->inspect(var, readonly, meta);
		ImGui::NextColumn();
	}
	else if (target_t.get_properties().size() > 0) {
		bool node_open = ImGui::TreeNode(name.c_str(), "%s%s", name.c_str(), is_pointer ? "*" : "");
		if (node_open) {
			for (auto &prop : target_t.get_properties()) {
				ImGui::PushID(&prop);

				auto prop_meta_getter =
					[&prop](const rttr::variant &key) -> rttr::variant {
					return prop.get_metadata(key);
				};
				rttr::variant prop_var = prop.get_value(var);
				if (inspect_internal(prop.get_name().to_string(), prop_var,
					prop.is_readonly(), prop_meta_getter)) {

					// handle pointer
					if (!prop.get_type().is_pointer()) {
						prop.set_value(var, prop_var);
					}

					changed = true;
				}

				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}
	else {
		assert(0); // TODO: some standard type inspector is not implemented
	}

	return changed;
}

bool TypeInspectorRegistry::inspect(rttr::variant &var, bool readonly,
	meta_provider meta) {
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
	ImGui::Columns(2);

	bool ret = inspect_internal(var.get_type().get_name().to_string(), var,
		readonly, meta);

	ImGui::Columns(1);
	ImGui::PopStyleVar();

	return ret;
}

void TypeInspectorRegistry::register_inspector(rttr::type type,
	TypeInspector &inspector) {
	registry_.insert({ type, &inspector });
}

void TypeInspectorRegistry::deregister_inspector(rttr::type type) {
	auto &pos = registry_.find(type);
	if (pos != registry_.end()) {
		registry_.erase(pos);
	}
}

bool TypeInspectorRegistry::has_inspector(rttr::type type) const {
	return registry_.find(type) != registry_.end();
}
