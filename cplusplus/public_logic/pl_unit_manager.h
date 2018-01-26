#pragma once

#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

namespace public_logic {

struct UnitBase;
using UnitBasePtr = boost::shared_ptr<UnitBase>;

class UnitManager : private boost::noncopyable {
	std::map<int, UnitBasePtr> units_dict_;

	UnitManager(void) {
	}

	~UnitManager(void) {
	}
public:
	static UnitManager& instance(void) {
		static UnitManager ins;
		return ins;
	}

	void register_unit(const UnitBasePtr& u);
	void tick_all(double dt);
	void clear_all(void);
};

}
