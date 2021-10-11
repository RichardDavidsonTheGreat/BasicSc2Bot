#ifndef BASIC_SC2_BOT_H_
#define BASIC_SC2_BOT_H_

#include "sc2api/sc2_api.h"
#include "sc2api/sc2_args.h"
#include "sc2lib/sc2_lib.h"
#include "sc2utils/sc2_manage_process.h"
#include "sc2utils/sc2_arg_parser.h"
#include "sc2api/sc2_api.h"
#include "sc2api/sc2_unit_filters.h"
#include "sc2lib/sc2_lib.h"
#include "sc2api/sc2_interfaces.h"
#include "sc2api/sc2_agent.h"
#include "sc2api/sc2_map_info.h"

using namespace sc2;

class BasicSc2Bot : public sc2::Agent {
public:
	//make sure to add all method declarations here!!!
	virtual void OnGameStart();
	virtual void OnStep();
	size_t CountUnitType(UnitTypeID unit_type);
	bool TryBuildGateway();
	void OnUnitIdle(const Unit* unit);
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::PROTOSS_PROBE);
	bool TryBuildPylon();
	const Unit* FindNearestMineralPatch(const Point2D& start);
	const Unit* FindNearestVespeneGeyser(const Point2D& start);
	bool TryBuildAssimilator();
	bool TryBuildStructureOnUnit(const Unit* builder, ABILITY_ID ability_type_for_structure, const Unit* unit);
	bool TryBuildStructureOnLocation(const Unit* builder, ABILITY_ID ability_type_for_structure, const Point2D& position);
	void MineVespene();
	int GetNumActiveAssimilators();
	bool TryBuildCyberneticsCore();
	const Unit* FindNearestNexus(const Point2D& start);
	int CheckVespeneGatherers(const Unit* asimilator);
	bool TryBuildTwilightCouncil();

private:
	//these are flags to be used to determine state of the game
	bool ResearchWarpTime = false; //time to research warp gate
	bool AttackTime = false; //time to attack!!
};

#endif