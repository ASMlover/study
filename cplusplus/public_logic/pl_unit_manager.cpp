#include <iostream>
#include "pl_unit_base.h"
#include "pl_unit_manager.h"

namespace public_logic {

void UnitManager::register_unit(const UnitBasePtr& u) {
	auto it = units_dict_.find(u->unit_id);
	if (it != units_dict_.end())
		std::cout << "UnitBase(" << u->unit_id << ") has been registered" << std::endl;
	else
		units_dict_[u->unit_id] = u;
}

void UnitManager::tick_all(double dt) {
	for (auto& u : units_dict_)
		u.second->tick(dt);
}

void UnitManager::clear_all(void) {
	units_dict_.clear();
}

}
