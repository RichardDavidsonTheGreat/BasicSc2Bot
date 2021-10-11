#include "BasicSc2Bot.h"
using namespace sc2;


size_t BasicSc2Bot::CountUnitType(UnitTypeID unit_type) {
	return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

const Unit* BasicSc2Bot::FindNearestNexus(const Point2D& start) {
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	float distance = std::numeric_limits<float>::max();
	const Unit* target = nullptr;
	for (const auto& u : units) {
		if (u->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) {
			float d = DistanceSquared2D(u->pos, start);
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}
	return target;
}


int BasicSc2Bot::CheckVespeneGatherers(const Unit* asimilator) {
	int i = 0;
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		for (const auto& order : unit->orders) {
			if (order.ability_id == ABILITY_ID::SMART && order.target_pos == asimilator->pos) {
				i = i + 1;
			}
		}
	}
	return i;
}


int BasicSc2Bot::GetNumActiveAssimilators() {
	int i = 0;
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
			if (/*unit->build_progress == 1.0 ||*/ unit->vespene_contents > 0) {
				i = i + 1;

			}

		}
	}
	return i;
}

void BasicSc2Bot::MineVespene() {
	const Unit* miner = nullptr;
	Units workers = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& worker : workers) {
		if (worker->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
			miner = worker;
		}
	}
	const Unit* assimilator = nullptr;
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	double dist = 9999999;
	for (const auto& unit : units) {
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
			if (Distance2D(unit->pos, miner->pos) < dist) {
				if (unit->build_progress == 1.0 && unit->assigned_harvesters < 2 && (unit->vespene_contents > 0)) {
					//if (unit->build_progress == 1.0 && CheckVespeneGatherers(unit) < 3) {
					assimilator = unit;
					dist = Distance2D(assimilator->pos, miner->pos);

				}

			}

		}
	}
	Actions()->UnitCommand(miner, ABILITY_ID::SMART, assimilator);

}


bool BasicSc2Bot::TryBuildGateway() {
	const ObservationInterface* observation = Observation();
	if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1) {
		return false;
	}
	if ((CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) + CountUnitType(UNIT_TYPEID::PROTOSS_WARPGATE)) > 2) {
		return false;
	}
	return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY);
}

bool BasicSc2Bot::TryBuildCyberneticsCore() {
	const ObservationInterface* observation = Observation();
	if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1) {
		return false;
	}
	if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) < 1) {
		return false;
	}
	if (CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) > 0) {
		return false;
	}
	return TryBuildStructure(ABILITY_ID::BUILD_CYBERNETICSCORE);
}

bool BasicSc2Bot::TryBuildTwilightCouncil() {
	const ObservationInterface* observation = Observation();
	if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1) {
		return false;
	}
	if (CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) < 1) {
		return false;
	}
	if (CountUnitType(UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) < 1) {
		return false;
	}
	if (CountUnitType(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL) > 0) {
		return false;
	}
	return TryBuildStructure(ABILITY_ID::BUILD_TWILIGHTCOUNCIL);
}

bool BasicSc2Bot::TryBuildStructureOnLocation(const Unit* builder, ABILITY_ID ability_type_for_structure, const Point2D& position) {
	const ObservationInterface* observation = Observation();
	// If a unit already is building a supply structure of this type, do nothing.
	Actions()->UnitCommand(builder, ability_type_for_structure, position);
	return true;
}

bool BasicSc2Bot::TryBuildStructureOnUnit(const Unit* builder, ABILITY_ID ability_type_for_structure, const Unit* unit) {
	const ObservationInterface* observation = Observation();
	// If a unit already is building a supply structure of this type, do nothing.
	// Also get an scv to build the structure.
	Actions()->UnitCommand(builder, ability_type_for_structure, unit);
	return true;
}


bool BasicSc2Bot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type) {
	const ObservationInterface* observation = Observation();
	// If a unit already is building a supply structure of this type, do nothing.
	// Also get an scv to build the structure.
	const Unit* unit_to_build = nullptr;
	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		for (const auto& order : unit->orders) {
			if (order.ability_id == ability_type_for_structure) {
				return false;
			}
		}
		if (unit->unit_type == unit_type) {
			unit_to_build = unit;
		}
	}
	float rx = GetRandomScalar();
	float ry = GetRandomScalar();
	/*while (Distance2D(Point2D((unit_to_build->pos.x + rx), (unit_to_build->pos.y + ry)), (FindNearestNexus(unit_to_build->pos)->pos)) < 100.0) {
		rx = GetRandomScalar();
		ry = GetRandomScalar();
	}
	*/

	Actions()->UnitCommand(unit_to_build, ability_type_for_structure, Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));
	//Actions()->UnitCommand(unit_to_build, ability_type_for_structure, Point2D(unit_to_build->pos.x + rx, unit_to_build->pos.y + ry));
	return true;
}

bool BasicSc2Bot::TryBuildPylon() {
	const ObservationInterface* observation = Observation();
	// If we are not supply capped, don't build a supply depot.
	if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
		return false;
	// Try and build a depot. Find a random SCV and give it the order.
	return TryBuildStructure(ABILITY_ID::BUILD_PYLON);
}

bool BasicSc2Bot::TryBuildVespeneGeyser() {
	const Unit* unit_to_build = nullptr;
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		//for (const auto& order : unit->orders) {
		//	if (order.ability_id == UNIT_TYPEID::PROTOSS_PROBE) {
		//		return false;
		//	}
		//}
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
			unit_to_build = unit;
		}
	}
	const Unit* vespene_target = FindNearestVespeneGeyser(unit_to_build->pos);
	TryBuildStructureOnUnit(unit_to_build, ABILITY_ID::BUILD_ASSIMILATOR, vespene_target);
	const Unit* mineral_target = FindNearestMineralPatch(unit_to_build->pos);
	Actions()->UnitCommand(unit_to_build, ABILITY_ID::SMART, mineral_target, true);
	return true;
}

const Unit* BasicSc2Bot::FindNearestMineralPatch(const Point2D& start) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const Unit* target = nullptr;
	for (const auto& u : units) {
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
			float d = DistanceSquared2D(u->pos, start);
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}
	return target;
}

const Unit* BasicSc2Bot::FindNearestVespeneGeyser(const Point2D& start) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	const Unit* target = nullptr;
	for (const auto& u : units) {
		if (u->unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) {
			float d = DistanceSquared2D(u->pos, start);
			if (d < distance) {
				distance = d;
				target = u;
			}
		}
	}
	return target;
}


void BasicSc2Bot::OnGameStart() {
	std::cout << "Hello, World!" << std::endl;
	return;
}

void BasicSc2Bot::OnStep() {
	//std::cout << "Loop: " << Observation()->GetGameLoop() << std::endl;
	//std::cout << "Minerals: " << Observation()->GetMinerals() << std::endl;
	//std::cout << "Units: " << Observation()->GetVespene() << std::endl;
	/*
	if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) == 15 && GetNumActiveAssimilators() < 1) {
		TryBuildVespeneGeyser();
	}

	if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) == 18 && GetNumActiveAssimilators() < 2) {
		TryBuildVespeneGeyser();
	}
	*/
	if (GetNumActiveAssimilators() < 2) {
		TryBuildVespeneGeyser();
	}

	if (CountUnitType(UNIT_TYPEID::PROTOSS_STALKER) >= 8) {
		ResearchWarpTime = true;
	}
	if (CountUnitType(UNIT_TYPEID::PROTOSS_STALKER) >= 12) {
		AttackTime = true;
	}
	if (ResearchWarpTime == true) {
		Units units = Observation()->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			if (unit->unit_type == UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) {
				Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_WARPGATE);
			}
		}
	}
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) {
			if (unit->energy >= 50) {
				Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_CHRONOBOOST, unit);
			}

		}
	}

	MineVespene();
	TryBuildPylon();
	TryBuildGateway();
	TryBuildCyberneticsCore();
	TryBuildTwilightCouncil();

	return;
}

void BasicSc2Bot::OnUnitIdle(const Unit* unit) {
	switch (unit->unit_type.ToType()) {
	case UNIT_TYPEID::PROTOSS_NEXUS: {
		if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) <= 18) {
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
		}
		break;
	}
	case UNIT_TYPEID::PROTOSS_PROBE: {
		const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
		if (!mineral_target) {
			break;
		}
		Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
		break;
	}

	case UNIT_TYPEID::PROTOSS_GATEWAY: {
		if (AttackTime == true) {
			Actions()->UnitCommand(unit, ABILITY_ID::MORPH_WARPGATE);
		}
		else {
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_STALKER);
		}

		break;
	}

	case UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL: {
		Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_BLINK);
		break;
	}
											 /*
			case UNIT_TYPEID::TERRAN_MARINE: {
				const GameInfo & game_info = Observation()->GetGameInfo();
				Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front());
				break;
			}
			*/
	default: {
		break;
	}
	}

}

