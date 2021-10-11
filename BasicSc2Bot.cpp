#include "BasicSc2Bot.h"
#include <iostream>
using namespace sc2;

//get the number of units of unit_type that are on our team
size_t BasicSc2Bot::CountUnitType(UnitTypeID unit_type) {
	return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

//finds the nearest nexus to the position given
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

//not used made by RD when I was attempting to fix the glitch with too many probes going to same assimilator
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

//get the number of assimilators, right now to be considered active an assimilator must have some vespene gass and be built OR building
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

//tasks a probe to mine vespene gass from the nearest assimilator that still has room (assimilators can support 3 probes mining from it)
void BasicSc2Bot::MineVespene() {
	const Unit* miner = nullptr; //get a probe to mine the vespene
	Units workers = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& worker : workers) {
		if (worker->unit_type == UNIT_TYPEID::PROTOSS_PROBE) {
			miner = worker;
		}
	}
	const Unit* assimilator = nullptr; //get an assimilator that is closest to our chosen probe
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	double dist = 9999999;
	for (const auto& unit : units) {
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_ASSIMILATOR) {
			if (Distance2D(unit->pos, miner->pos) < dist) {
				//right now only task 2 probes to an assimilator since we dont need as much vespene gass and in the case where somehow
				//extra probes get tasked to an assimilator the capacity (3) will not be exceeded
				if (unit->build_progress == 1.0 && unit->assigned_harvesters < 2 && (unit->vespene_contents > 0)) {
					assimilator = unit;
					dist = Distance2D(assimilator->pos, miner->pos);

				}

			}

		}
	}
	//task the probe with mining from this assimilator
	Actions()->UnitCommand(miner, ABILITY_ID::SMART, assimilator);

}

//try to build a gateway.
bool BasicSc2Bot::TryBuildGateway() {
	const ObservationInterface* observation = Observation();
	if (CountUnitType(UNIT_TYPEID::PROTOSS_PYLON) < 1) {
		return false;
	}
	if ((CountUnitType(UNIT_TYPEID::PROTOSS_GATEWAY) + CountUnitType(UNIT_TYPEID::PROTOSS_WARPGATE)) > 2) { //We only want 3 gateways (for now)
		return false;
	}
	return TryBuildStructure(ABILITY_ID::BUILD_GATEWAY);
}

//if all of the necessary buildings are built build the cybernetics core
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

//if all of the necessary buildings are built build the twilight council
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

//unused function made by RD may be used later and added upon later but for now ignore
bool BasicSc2Bot::TryBuildStructureOnLocation(const Unit* builder, ABILITY_ID ability_type_for_structure, const Point2D& position) {
	const ObservationInterface* observation = Observation();
	// If a unit already is building a supply structure of this type, do nothing.
	Actions()->UnitCommand(builder, ability_type_for_structure, position);
	return true;
}

//used for making the assimilator since it must be built upon a vesepene gass neutral unit
bool BasicSc2Bot::TryBuildStructureOnUnit(const Unit* builder, ABILITY_ID ability_type_for_structure, const Unit* unit) {
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
	}
	Actions()->UnitCommand(builder, ability_type_for_structure, unit);
	return true;
}

//basic build structure command. Right now it is just randomly trying to build a building. Will need to be updated
//since for one we can only build in areas where the pylon is giving energy (to visualize this run the game and click on a pylon)
//we also want to make it so that we aren't building too near the probes that are trying to mine so as to not get in their way
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
	Actions()->UnitCommand(unit_to_build, ability_type_for_structure, Point2D(unit_to_build->pos.x + rx * 15.0f, unit_to_build->pos.y + ry * 15.0f));

	return true;

	//the following is a rough idea I had in order to try and build no in the path of our mining probes
	/*while (Distance2D(Point2D((unit_to_build->pos.x + rx), (unit_to_build->pos.y + ry)), (FindNearestNexus(unit_to_build->pos)->pos)) < 100.0) {
		rx = GetRandomScalar();
		ry = GetRandomScalar();
	}
	Actions()->UnitCommand(unit_to_build, ability_type_for_structure, Point2D(unit_to_build->pos.x + rx, unit_to_build->pos.y + ry));
	*/
}

//if we are close to our unit capacity (represented by FoodCap) build a pylon, this will need to be reworked
bool BasicSc2Bot::TryBuildPylon() {
	const ObservationInterface* observation = Observation();
	// If we are not supply capped, don't build a supply depot.
	if (observation->GetFoodUsed() <= observation->GetFoodCap() - 2)
		return false;
	// Try and build a depot. Find a random SCV and give it the order.
	return TryBuildStructure(ABILITY_ID::BUILD_PYLON);
}

//this is a very complex function, essentially, get a probe, find the closest vespene geyser, build an assimilator on
//top of that vespene geyser. There is a weird issue where after building the assimilator the probe will wait around for it to
//finish (without going into idle). So chain a second command on the probe so once he starts the building he is retasked with mining minerals
bool BasicSc2Bot::TryBuildAssimilator() {
	const Unit* unit_to_build = nullptr;
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		for (const auto& order : unit->orders) {
			if (order.ability_id == ABILITY_ID::BUILD_ASSIMILATOR) {
				return false;
			}
		}
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

//find the nearest minerals to the point start
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

//find the nearest vespene geyser to the point start
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

	//this is a rudamentary idea I had (RD) in order to try and make it so we start mining vespene gass later on in the case
	//where we need more minerals than vespene gass
	/*
	if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) == 15 && GetNumActiveAssimilators() < 1) {
		TryBuildAssimilator();
	}

	if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) == 18 && GetNumActiveAssimilators() < 2) {
		TryBuildAssimilator();
	}
	*/

	//build 2 assmilator
	if (GetNumActiveAssimilators() < 2) {
		TryBuildAssimilator();
	}

	//set the flag to say that its time to research warp gate in the cybernetics core
	if (CountUnitType(UNIT_TYPEID::PROTOSS_STALKER) >= 8) {
		ResearchWarpTime = true;
	}
	//set the flag to say that its time to start the attack because we have enough stalkers
	if (CountUnitType(UNIT_TYPEID::PROTOSS_STALKER) >= 12) {
		AttackTime = true;
	}

	//if flag ResearchWarpTime research Warp Gates in cybernetics core
	if (ResearchWarpTime == true) {
		Units units = Observation()->GetUnits(Unit::Alliance::Self);
		for (const auto& unit : units) {
			if (unit->unit_type == UNIT_TYPEID::PROTOSS_CYBERNETICSCORE) {
				Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_WARPGATE);
			}
		}
	}

	//started by RD to try and get the nexus to use cronoboost on itself whenever it can (this speeds up the production of probes) its not
	//working and it is essential to getting an optimal start
	Units units = Observation()->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		if (unit->unit_type == UNIT_TYPEID::PROTOSS_NEXUS) {
			if (unit->energy >= 50) {
				Actions()->UnitCommand(unit, ABILITY_ID::EFFECT_CHRONOBOOST, unit);
			}

		}
	}

	MineVespene();  //send probes to mine vespene gass from assimilators
	TryBuildPylon(); //build pylons if we are close to supply cap
	TryBuildGateway(); //build 3 gateways
	TryBuildCyberneticsCore(); //build a cybernetics core
	TryBuildTwilightCouncil(); //build twilight council

	return;
}


//whenever a unit has nothing to do they enter idle state. Retask unit depending on what type of unit it is
void BasicSc2Bot::OnUnitIdle(const Unit* unit) {
	switch (unit->unit_type.ToType()) {
	//if our nexux is idle and we still dont have 18 probes train probes
	case UNIT_TYPEID::PROTOSS_NEXUS: {
		if (CountUnitType(UNIT_TYPEID::PROTOSS_PROBE) <= 18) {
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_PROBE);
		}
		break;
	}
	//if our probe is idle retask them to mine the mineral patch closest to them
	//to do: make sure the mineral patch is close to a nexus
	case UNIT_TYPEID::PROTOSS_PROBE: {
		const Unit* mineral_target = FindNearestMineralPatch(unit->pos);
		if (!mineral_target) {
			break;
		}
		Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
		break;
	}
	//if a gateway is idle train stalker if before the attack time, if attack time flag is set turn into a warp gate so we can bring in reinforements
	case UNIT_TYPEID::PROTOSS_GATEWAY: {
		if (AttackTime == true) {
			Actions()->UnitCommand(unit, ABILITY_ID::MORPH_WARPGATE);
		}
		else {
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_STALKER);
		}

		break;
	}
	//if twilight coucil is not doing anything research blink
	case UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL: {
		Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_BLINK);
		break;
	}
	//kept in this peice from the tutorial so we have an idea how to do attacking feel free to erase RD
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

