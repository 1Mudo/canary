/**
 * Canary - A free and open-source MMORPG server emulator
 * Copyright (©) 2019-2022 OpenTibiaBR <opentibiabr@outlook.com>
 * Repository: https://github.com/opentibiabr/canary
 * License: https://github.com/opentibiabr/canary/blob/main/LICENSE
 * Contributors: https://github.com/opentibiabr/canary/graphs/contributors
 * Website: https://docs.opentibiabr.com/
 */

#include "pch.hpp"

#include "creatures/combat/spells.hpp"
#include "creatures/creature.hpp"
#include "creatures/interactions/chat.hpp"
#include "creatures/players/player.hpp"
#include "creatures/players/wheel/player_wheel.hpp"
#include "game/game.hpp"
#include "io/iologindata.hpp"
#include "io/ioprey.hpp"
#include "items/item.hpp"
#include "lua/functions/creatures/player/player_functions.hpp"

int PlayerFunctions::luaPlayerSendInventory(lua_State* L) {
	// player:sendInventory()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	player->sendInventoryIds();
	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerSendLootStats(lua_State* L) {
	// player:sendLootStats(item, count)
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Item* item = getUserdata<Item>(L, 2);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	uint8_t count = getNumber<uint8_t>(L, 3, 0);
	if (count == 0) {
		lua_pushnil(L);
		return 1;
	}

	player->sendLootStats(item, count);
	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerUpdateSupplyTracker(lua_State* L) {
	// player:updateSupplyTracker(item)
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Item* item = getUserdata<Item>(L, 2);
	if (!item) {
		lua_pushnil(L);
		return 1;
	}

	player->updateSupplyTracker(item);
	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerUpdateKillTracker(lua_State* L) {
	// player:updateKillTracker(creature, corpse)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Creature* monster = getUserdata<Creature>(L, 2);
	if (!monster) {
		lua_pushnil(L);
		return 1;
	}

	Container* corpse = getUserdata<Container>(L, 3);
	if (!corpse) {
		lua_pushnil(L);
		return 1;
	}

	player->updateKillTracker(corpse, monster->getName(), monster->getCurrentOutfit());
	pushBoolean(L, true);

	return 1;
}

// Player
int PlayerFunctions::luaPlayerCreate(lua_State* L) {
	// Player(id or guid or name or userdata)
	Player* player;
	if (isNumber(L, 2)) {
		uint32_t id = getNumber<uint32_t>(L, 2);
		if (id >= Player::getFirstID() && id <= Player::getLastID()) {
			player = g_game().getPlayerByID(id);
		} else {
			player = g_game().getPlayerByGUID(id);
		}
	} else if (isString(L, 2)) {
		ReturnValue ret = g_game().getPlayerByNameWildcard(getString(L, 2), player);
		if (ret != RETURNVALUE_NOERROR) {
			lua_pushnil(L);
			lua_pushnumber(L, ret);
			return 2;
		}
	} else if (isUserdata(L, 2)) {
		if (getUserdataType(L, 2) != LuaData_t::Player) {
			lua_pushnil(L);
			return 1;
		}
		player = getUserdata<Player>(L, 2);
	} else {
		player = nullptr;
	}

	if (player) {
		pushUserdata<Player>(L, player);
		setMetatable(L, -1, "Player");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerResetCharmsMonsters(lua_State* L) {
	// player:resetCharmsBestiary()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setCharmPoints(0);
		player->setCharmExpansion(false);
		player->setUsedRunesBit(0);
		player->setUnlockedRunesBit(0);
		for (int8_t i = CHARM_WOUND; i <= CHARM_LAST; i++) {
			player->parseRacebyCharm(static_cast<charmRune_t>(i), true, 0);
		}
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerUnlockAllCharmRunes(lua_State* L) {
	// player:unlockAllCharmRunes()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		for (int8_t i = CHARM_WOUND; i <= CHARM_LAST; i++) {
			const auto charm = g_iobestiary().getBestiaryCharm(static_cast<charmRune_t>(i));
			if (charm) {
				int32_t value = g_iobestiary().bitToggle(player->getUnlockedRunesBit(), charm, true);
				player->setUnlockedRunesBit(value);
			}
		}
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayeraddCharmPoints(lua_State* L) {
	// player:addCharmPoints()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		int16_t charms = getNumber<int16_t>(L, 2);
		if (charms >= 0) {
			g_iobestiary().addCharmPoints(player, static_cast<uint16_t>(charms));
		} else {
			charms = -charms;
			g_iobestiary().addCharmPoints(player, static_cast<uint16_t>(charms), true);
		}
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerIsPlayer(lua_State* L) {
	// player:isPlayer()
	pushBoolean(L, getUserdata<const Player>(L, 1) != nullptr);
	return 1;
}

int PlayerFunctions::luaPlayerGetGuid(lua_State* L) {
	// player:getGuid()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getGUID());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetIp(lua_State* L) {
	// player:getIp()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getIP());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetAccountId(lua_State* L) {
	// player:getAccountId()
	Player* player = getUserdata<Player>(L, 1);
	if (!player || player->getAccountId() == 0) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushnumber(L, player->getAccountId());

	return 1;
}

int PlayerFunctions::luaPlayerGetLastLoginSaved(lua_State* L) {
	// player:getLastLoginSaved()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getLastLoginSaved());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetLastLogout(lua_State* L) {
	// player:getLastLogout()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getLastLogout());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetAccountType(lua_State* L) {
	// player:getAccountType()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getAccountType());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetAccountType(lua_State* L) {
	// player:setAccountType(accountType)
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		lua_pushnil(L);
		return 1;
	}

	if (player->getAccount()->setAccountType(getNumber<account::AccountType>(L, 2)) != account::ERROR_NO) {
		lua_pushnil(L);
		return 1;
	}

	if (player->getAccount()->save() != account::ERROR_NO) {
		lua_pushnil(L);
		return 1;
	}

	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerAddBestiaryKill(lua_State* L) {
	// player:addBestiaryKill(name[, amount = 1])
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		const auto mtype = g_monsters().getMonsterType(getString(L, 2));
		if (mtype) {
			g_iobestiary().addBestiaryKill(player, mtype, getNumber<uint32_t>(L, 3, 1));
			pushBoolean(L, true);
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerIsMonsterBestiaryUnlocked(lua_State* L) {
	// player:isMonsterBestiaryUnlocked(raceId)
	Player* player = getUserdata<Player>(L, 1);
	if (player == nullptr) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	auto raceId = getNumber<uint16_t>(L, 2, 0);
	if (!g_monsters().getMonsterTypeByRaceId(raceId)) {
		reportErrorFunc("Monster race id not exists");
		pushBoolean(L, false);
		return 0;
	}

	for (auto finishedMonsters = g_iobestiary().getBestiaryFinished(player);
		 uint16_t finishedRaceId : finishedMonsters) {
		if (raceId == finishedRaceId) {
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 0;
}

int PlayerFunctions::luaPlayergetCharmMonsterType(lua_State* L) {
	// player:getCharmMonsterType(charmRune_t)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		charmRune_t charmid = getNumber<charmRune_t>(L, 2);
		uint16_t raceid = player->parseRacebyCharm(charmid, false, 0);
		if (raceid > 0) {
			const auto mtype = g_monsters().getMonsterTypeByRaceId(raceid);
			if (mtype) {
				pushUserdata<MonsterType>(L, mtype);
				setMetatable(L, -1, "MonsterType");
			} else {
				lua_pushnil(L);
			}
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemovePreyStamina(lua_State* L) {
	// player:removePreyStamina(amount)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		g_ioprey().CheckPlayerPreys(player, getNumber<uint8_t>(L, 2, 1));
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddPreyCards(lua_State* L) {
	// player:addPreyCards(amount)
	if (Player* player = getUserdata<Player>(L, 1)) {
		player->addPreyCards(getNumber<uint64_t>(L, 2, 0));
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetPreyCards(lua_State* L) {
	// player:getPreyCards()
	if (const Player* player = getUserdata<Player>(L, 1)) {
		lua_pushnumber(L, static_cast<lua_Number>(player->getPreyCards()));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetPreyExperiencePercentage(lua_State* L) {
	// player:getPreyExperiencePercentage(raceId)
	if (const Player* player = getUserdata<Player>(L, 1)) {
		if (const PreySlot* slot = player->getPreyWithMonster(getNumber<uint16_t>(L, 2, 0));
			slot && slot->isOccupied() && slot->bonus == PreyBonus_Experience && slot->bonusTimeLeft > 0) {
			lua_pushnumber(L, static_cast<lua_Number>(100 + slot->bonusPercentage));
		} else {
			lua_pushnumber(L, 100);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemoveTaskHuntingPoints(lua_State* L) {
	// player:removeTaskHuntingPoints(amount)
	if (Player* player = getUserdata<Player>(L, 1)) {
		pushBoolean(L, player->useTaskHuntingPoints(getNumber<uint64_t>(L, 2, 0)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetTaskHuntingPoints(lua_State* L) {
	// player:getTaskHuntingPoints()
	const Player* player = getUserdata<Player>(L, 1);
	if (player == nullptr) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}

	lua_pushnumber(L, static_cast<double>(player->getTaskHuntingPoints()));
	return 1;
}

int PlayerFunctions::luaPlayerAddTaskHuntingPoints(lua_State* L) {
	// player:addTaskHuntingPoints(amount)
	if (Player* player = getUserdata<Player>(L, 1)) {
		auto points = getNumber<uint64_t>(L, 2);
		player->addTaskHuntingPoints(getNumber<uint64_t>(L, 2));
		lua_pushnumber(L, static_cast<lua_Number>(points));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetPreyLootPercentage(lua_State* L) {
	// player:getPreyLootPercentage(raceid)
	if (const Player* player = getUserdata<Player>(L, 1)) {
		if (const PreySlot* slot = player->getPreyWithMonster(getNumber<uint16_t>(L, 2, 0));
			slot && slot->isOccupied() && slot->bonus == PreyBonus_Loot) {
			lua_pushnumber(L, slot->bonusPercentage);
		} else {
			lua_pushnumber(L, 0);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerPreyThirdSlot(lua_State* L) {
	// get: player:preyThirdSlot() set: player:preyThirdSlot(bool)
	if (Player* player = getUserdata<Player>(L, 1);
		PreySlot* slot = player->getPreySlotById(PreySlot_Three)) {
		if (lua_gettop(L) == 1) {
			pushBoolean(L, slot->state != PreyDataState_Locked);
		} else {
			if (getBoolean(L, 2, false)) {
				slot->eraseBonus();
				slot->state = PreyDataState_Selection;
				slot->reloadMonsterGrid(player->getPreyBlackList(), player->getLevel());
				player->reloadPreySlot(PreySlot_Three);
			} else {
				slot->state = PreyDataState_Locked;
			}

			pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerTaskThirdSlot(lua_State* L) {
	// get: player:taskHuntingThirdSlot() set: player:taskHuntingThirdSlot(bool)
	if (Player* player = getUserdata<Player>(L, 1);
		TaskHuntingSlot* slot = player->getTaskHuntingSlotById(PreySlot_Three)) {
		if (lua_gettop(L) == 1) {
			pushBoolean(L, slot->state != PreyTaskDataState_Locked);
		} else {
			if (getBoolean(L, 2, false)) {
				slot->eraseTask();
				slot->reloadReward();
				slot->state = PreyTaskDataState_Selection;
				slot->reloadMonsterGrid(player->getTaskHuntingBlackList(), player->getLevel());
				player->reloadTaskSlot(PreySlot_Three);
			} else {
				slot->state = PreyTaskDataState_Locked;
			}

			pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayercharmExpansion(lua_State* L) {
	// get: player:charmExpansion() set: player:charmExpansion(bool)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		if (lua_gettop(L) == 1) {
			pushBoolean(L, player->hasCharmExpansion());
		} else {
			player->setCharmExpansion(getBoolean(L, 2, false));
			pushBoolean(L, true);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetCapacity(lua_State* L) {
	// player:getCapacity()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getCapacity());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetCapacity(lua_State* L) {
	// player:setCapacity(capacity)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->capacity = getNumber<uint32_t>(L, 2);
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetTraining(lua_State* L) {
	// player:setTraining(value)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		bool value = getBoolean(L, 2, false);
		player->setTraining(value);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetIsTraining(lua_State* L) {
	// player:isTraining()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		pushBoolean(L, false);
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		return 1;
	}

	lua_pushnumber(L, player->isExerciseTraining());
	return 1;
}

int PlayerFunctions::luaPlayerGetFreeCapacity(lua_State* L) {
	// player:getFreeCapacity()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getFreeCapacity());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetKills(lua_State* L) {
	// player:getKills()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	lua_createtable(L, player->unjustifiedKills.size(), 0);
	int idx = 0;
	for (const auto &kill : player->unjustifiedKills) {
		lua_createtable(L, 3, 0);
		lua_pushnumber(L, kill.target);
		lua_rawseti(L, -2, 1);
		lua_pushnumber(L, kill.time);
		lua_rawseti(L, -2, 2);
		pushBoolean(L, kill.unavenged);
		lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, ++idx);
	}

	return 1;
}

int PlayerFunctions::luaPlayerSetKills(lua_State* L) {
	// player:setKills(kills)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	luaL_checktype(L, 2, LUA_TTABLE);
	std::vector<Kill> newKills;

	lua_pushnil(L);
	while (lua_next(L, 2) != 0) {
		// -2 is index, -1 is value
		luaL_checktype(L, -1, LUA_TTABLE);
		lua_rawgeti(L, -1, 1); // push target
		lua_rawgeti(L, -2, 2); // push time
		lua_rawgeti(L, -3, 3); // push unavenged
		newKills.emplace_back(luaL_checknumber(L, -3), luaL_checknumber(L, -2), getBoolean(L, -1));
		lua_pop(L, 4);
	}

	player->unjustifiedKills = std::move(newKills);
	player->sendUnjustifiedPoints();
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetReward(lua_State* L) {
	// player:getReward(rewardId[, autoCreate = false])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint64_t rewardId = getNumber<uint64_t>(L, 2);
	bool autoCreate = getBoolean(L, 3, false);
	if (auto reward = player->getReward(rewardId, autoCreate)) {
		pushUserdata<Item>(L, reward);
		setItemMetatable(L, -1, reward);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemoveReward(lua_State* L) {
	// player:removeReward(rewardId)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint32_t rewardId = getNumber<uint32_t>(L, 2);
	player->removeReward(rewardId);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetRewardList(lua_State* L) {
	// player:getRewardList()
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	std::vector<uint64_t> rewardVec;
	player->getRewardList(rewardVec);
	lua_createtable(L, rewardVec.size(), 0);

	int index = 0;
	for (const auto &rewardId : rewardVec) {
		lua_pushnumber(L, static_cast<lua_Number>(rewardId));
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetDailyReward(lua_State* L) {
	// player:setDailyReward(value)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setDailyReward(getNumber<uint8_t>(L, 2));
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetDepotLocker(lua_State* L) {
	// player:getDepotLocker(depotId)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint32_t depotId = getNumber<uint32_t>(L, 2);
	DepotLocker* depotLocker = player->getDepotLocker(depotId);
	if (depotLocker) {
		depotLocker->setParent(player);
		pushUserdata<Item>(L, depotLocker);
		setItemMetatable(L, -1, depotLocker);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetStashCounter(lua_State* L) {
	// player:getStashCount()
	const Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t sizeStash = getStashSize(player->getStashItems());
		lua_pushnumber(L, sizeStash);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetDepotChest(lua_State* L) {
	// player:getDepotChest(depotId[, autoCreate = false])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint32_t depotId = getNumber<uint32_t>(L, 2);
	bool autoCreate = getBoolean(L, 3, false);
	DepotChest* depotChest = player->getDepotChest(depotId, autoCreate);
	if (depotChest) {
		player->setLastDepotId(depotId);
		pushUserdata<Item>(L, depotChest);
		setItemMetatable(L, -1, depotChest);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetInbox(lua_State* L) {
	// player:getInbox()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Inbox* inbox = player->getInbox();
	if (inbox) {
		pushUserdata<Item>(L, inbox);
		setItemMetatable(L, -1, inbox);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetSkullTime(lua_State* L) {
	// player:getSkullTime()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getSkullTicks());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetSkullTime(lua_State* L) {
	// player:setSkullTime(skullTime)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setSkullTicks(getNumber<int64_t>(L, 2));
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetDeathPenalty(lua_State* L) {
	// player:getDeathPenalty()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, static_cast<uint32_t>(player->getLostPercent() * 100));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetExperience(lua_State* L) {
	// player:getExperience()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getExperience());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddExperience(lua_State* L) {
	// player:addExperience(experience[, sendText = false])
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		int64_t experience = getNumber<int64_t>(L, 2);
		bool sendText = getBoolean(L, 3, false);
		player->addExperience(nullptr, experience, sendText);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemoveExperience(lua_State* L) {
	// player:removeExperience(experience[, sendText = false])
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		int64_t experience = getNumber<int64_t>(L, 2);
		bool sendText = getBoolean(L, 3, false);
		player->removeExperience(experience, sendText);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetLevel(lua_State* L) {
	// player:getLevel()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getLevel());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetMagicShieldCapacityFlat(lua_State* L) {
	// player:getMagicShieldCapacityFlat(useCharges)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getMagicShieldCapacityFlat(getBoolean(L, 2, false)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetMagicShieldCapacityPercent(lua_State* L) {
	// player:getMagicShieldCapacityPercent(useCharges)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getMagicShieldCapacityPercent(getBoolean(L, 2, false)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSendSpellCooldown(lua_State* L) {
	// player:sendSpellCooldown(spellId, time)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}
	uint8_t spellId = getNumber<uint32_t>(L, 2, 1);
	uint32_t time = getNumber<uint32_t>(L, 3, 0);

	player->sendSpellCooldown(spellId, time);
	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerSendSpellGroupCooldown(lua_State* L) {
	// player:sendSpellGroupCooldown(groupId, time)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}
	SpellGroup_t groupId = getNumber<SpellGroup_t>(L, 2, SPELLGROUP_ATTACK);
	uint32_t time = getNumber<uint32_t>(L, 3, 0);

	player->sendSpellGroupCooldown(groupId, time);
	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerGetMagicLevel(lua_State* L) {
	// player:getMagicLevel()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getMagicLevel());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetBaseMagicLevel(lua_State* L) {
	// player:getBaseMagicLevel()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getBaseMagicLevel());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetMana(lua_State* L) {
	// player:getMana()
	const Player* player = getUserdata<const Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getMana());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddMana(lua_State* L) {
	// player:addMana(manaChange[, animationOnLoss = false])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	int32_t manaChange = getNumber<int32_t>(L, 2);
	bool animationOnLoss = getBoolean(L, 3, false);
	if (!animationOnLoss && manaChange < 0) {
		player->changeMana(manaChange);
	} else {
		CombatDamage damage;
		damage.primary.value = manaChange;
		damage.origin = ORIGIN_NONE;
		g_game().combatChangeMana(nullptr, player, damage);
	}
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetMaxMana(lua_State* L) {
	// player:getMaxMana()
	const Player* player = getUserdata<const Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getMaxMana());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetMaxMana(lua_State* L) {
	// player:setMaxMana(maxMana)
	Player* player = getPlayer(L, 1);
	if (player) {
		player->manaMax = getNumber<int32_t>(L, 2);
		player->mana = std::min<int32_t>(player->mana, player->manaMax);
		g_game().addPlayerMana(player);
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetManaSpent(lua_State* L) {
	// player:getManaSpent()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getSpentMana());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddManaSpent(lua_State* L) {
	// player:addManaSpent(amount)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->addManaSpent(getNumber<uint64_t>(L, 2));
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetBaseMaxHealth(lua_State* L) {
	// player:getBaseMaxHealth()
	const Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->healthMax);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetBaseMaxMana(lua_State* L) {
	// player:getBaseMaxMana()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->manaMax);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetSkillLevel(lua_State* L) {
	// player:getSkillLevel(skillType)
	skills_t skillType = getNumber<skills_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player && skillType <= SKILL_LAST) {
		lua_pushnumber(L, player->skills[skillType].level);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetEffectiveSkillLevel(lua_State* L) {
	// player:getEffectiveSkillLevel(skillType)
	skills_t skillType = getNumber<skills_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player && skillType <= SKILL_LAST) {
		lua_pushnumber(L, player->getSkillLevel(skillType));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetSkillPercent(lua_State* L) {
	// player:getSkillPercent(skillType)
	skills_t skillType = getNumber<skills_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player && skillType <= SKILL_LAST) {
		lua_pushnumber(L, player->skills[skillType].percent);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetSkillTries(lua_State* L) {
	// player:getSkillTries(skillType)
	skills_t skillType = getNumber<skills_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player && skillType <= SKILL_LAST) {
		lua_pushnumber(L, player->skills[skillType].tries);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddSkillTries(lua_State* L) {
	// player:addSkillTries(skillType, tries)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		skills_t skillType = getNumber<skills_t>(L, 2);
		uint64_t tries = getNumber<uint64_t>(L, 3);
		player->addSkillAdvance(skillType, tries);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetMagicLevel(lua_State* L) {
	// player:setMagicLevel(level[, manaSpent])
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t level = getNumber<uint16_t>(L, 2);
		player->magLevel = level;
		if (getNumber<uint64_t>(L, 3, 0) > 0) {
			uint64_t manaSpent = getNumber<uint64_t>(L, 3);
			uint64_t nextReqMana = player->vocation->getReqMana(level + 1);
			player->manaSpent = manaSpent;
			player->magLevelPercent = Player::getPercentLevel(manaSpent, nextReqMana);
		} else {
			player->manaSpent = 0;
			player->magLevelPercent = 0;
		}
		player->sendStats();
		player->sendSkills();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetSkillLevel(lua_State* L) {
	// player:setSkillLevel(skillType, level[, tries])
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		skills_t skillType = getNumber<skills_t>(L, 2);
		uint16_t level = getNumber<uint16_t>(L, 3);
		player->skills[skillType].level = level;
		if (getNumber<uint64_t>(L, 4, 0) > 0) {
			uint64_t tries = getNumber<uint64_t>(L, 4);
			uint64_t nextReqTries = player->vocation->getReqSkillTries(skillType, level + 1);
			player->skills[skillType].tries = tries;
			player->skills[skillType].percent = Player::getPercentLevel(tries, nextReqTries);
		} else {
			player->skills[skillType].tries = 0;
			player->skills[skillType].percent = 0;
		}
		player->sendStats();
		player->sendSkills();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddOfflineTrainingTime(lua_State* L) {
	// player:addOfflineTrainingTime(time)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		int32_t time = getNumber<int32_t>(L, 2);
		player->addOfflineTrainingTime(time);
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetOfflineTrainingTime(lua_State* L) {
	// player:getOfflineTrainingTime()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getOfflineTrainingTime());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemoveOfflineTrainingTime(lua_State* L) {
	// player:removeOfflineTrainingTime(time)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		int32_t time = getNumber<int32_t>(L, 2);
		player->removeOfflineTrainingTime(time);
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddOfflineTrainingTries(lua_State* L) {
	// player:addOfflineTrainingTries(skillType, tries)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		skills_t skillType = getNumber<skills_t>(L, 2);
		uint64_t tries = getNumber<uint64_t>(L, 3);
		pushBoolean(L, player->addOfflineTrainingTries(skillType, tries));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetOfflineTrainingSkill(lua_State* L) {
	// player:getOfflineTrainingSkill()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getOfflineTrainingSkill());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetOfflineTrainingSkill(lua_State* L) {
	// player:setOfflineTrainingSkill(skillId)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		int8_t skillId = getNumber<int8_t>(L, 2);
		player->setOfflineTrainingSkill(skillId);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerOpenStash(lua_State* L) {
	// player:openStash(isNpc)
	Player* player = getUserdata<Player>(L, 1);
	bool isNpc = getBoolean(L, 2, false);
	if (player) {
		player->sendOpenStash(isNpc);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}

	return 1;
}

int PlayerFunctions::luaPlayerGetItemCount(lua_State* L) {
	// player:getItemCount(itemId[, subType = -1])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint16_t itemId;
	if (isNumber(L, 2)) {
		itemId = getNumber<uint16_t>(L, 2);
	} else {
		itemId = Item::items.getItemIdByName(getString(L, 2));
		if (itemId == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	int32_t subType = getNumber<int32_t>(L, 3, -1);
	lua_pushnumber(L, player->getItemTypeCount(itemId, subType));
	return 1;
}

int PlayerFunctions::luaPlayerGetStashItemCount(lua_State* L) {
	// player:getStashItemCount(itemId)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint16_t itemId;
	if (isNumber(L, 2)) {
		itemId = getNumber<uint16_t>(L, 2);
	} else {
		itemId = Item::items.getItemIdByName(getString(L, 2));
		if (itemId == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	const ItemType &itemType = Item::items[itemId];
	if (itemType.id == 0) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushnumber(L, player->getStashItemCount(itemType.id));
	return 1;
}

int PlayerFunctions::luaPlayerGetItemById(lua_State* L) {
	// player:getItemById(itemId, deepSearch[, subType = -1])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint16_t itemId;
	if (isNumber(L, 2)) {
		itemId = getNumber<uint16_t>(L, 2);
	} else {
		itemId = Item::items.getItemIdByName(getString(L, 2));
		if (itemId == 0) {
			lua_pushnil(L);
			return 1;
		}
	}
	bool deepSearch = getBoolean(L, 3);
	int32_t subType = getNumber<int32_t>(L, 4, -1);

	Item* item = g_game().findItemOfType(player, itemId, deepSearch, subType);
	if (item) {
		pushUserdata<Item>(L, item);
		setItemMetatable(L, -1, item);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetVocation(lua_State* L) {
	// player:getVocation()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushUserdata<Vocation>(L, player->getVocation());
		setMetatable(L, -1, "Vocation");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetVocation(lua_State* L) {
	// player:setVocation(id or name or userdata)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Vocation* vocation;
	if (isNumber(L, 2)) {
		vocation = g_vocations().getVocation(getNumber<uint16_t>(L, 2));
	} else if (isString(L, 2)) {
		vocation = g_vocations().getVocation(g_vocations().getVocationId(getString(L, 2)));
	} else if (isUserdata(L, 2)) {
		vocation = getUserdata<Vocation>(L, 2);
	} else {
		vocation = nullptr;
	}

	if (!vocation) {
		pushBoolean(L, false);
		return 1;
	}

	player->setVocation(vocation->getId());
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetSex(lua_State* L) {
	// player:getSex()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getSex());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetSex(lua_State* L) {
	// player:setSex(newSex)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		PlayerSex_t newSex = getNumber<PlayerSex_t>(L, 2);
		player->setSex(newSex);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetPronoun(lua_State* L) {
	// player:getPronoun()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getPronoun());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetPronoun(lua_State* L) {
	// player:setPronoun(newPronoun)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		PlayerPronoun_t newPronoun = getNumber<PlayerPronoun_t>(L, 2);
		player->setPronoun(newPronoun);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetTown(lua_State* L) {
	// player:getTown()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushUserdata<Town>(L, player->getTown());
		setMetatable(L, -1, "Town");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetTown(lua_State* L) {
	// player:setTown(town)
	Town* town = getUserdata<Town>(L, 2);
	if (!town) {
		pushBoolean(L, false);
		return 1;
	}

	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setTown(town);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetGuild(lua_State* L) {
	// player:getGuild()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	const auto guild = player->getGuild();
	if (!guild) {
		lua_pushnil(L);
		return 1;
	}

	pushUserdata<Guild>(L, guild);
	setMetatable(L, -1, "Guild");
	return 1;
}

int PlayerFunctions::luaPlayerSetGuild(lua_State* L) {
	// player:setGuild(guild)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	const auto guild = getUserdataShared<Guild>(L, 2);
	player->setGuild(guild);

	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetGuildLevel(lua_State* L) {
	// player:getGuildLevel()
	Player* player = getUserdata<Player>(L, 1);
	if (player && player->getGuild()) {
		lua_pushnumber(L, player->getGuildRank()->level);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetGuildLevel(lua_State* L) {
	// player:setGuildLevel(level)
	uint8_t level = getNumber<uint8_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getGuild()) {
		lua_pushnil(L);
		return 1;
	}

	GuildRank_ptr rank = player->getGuild()->getRankByLevel(level);
	if (!rank) {
		pushBoolean(L, false);
	} else {
		player->setGuildRank(rank);
		pushBoolean(L, true);
	}

	return 1;
}

int PlayerFunctions::luaPlayerGetGuildNick(lua_State* L) {
	// player:getGuildNick()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushString(L, player->getGuildNick());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetGuildNick(lua_State* L) {
	// player:setGuildNick(nick)
	const std::string &nick = getString(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setGuildNick(nick);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetGroup(lua_State* L) {
	// player:getGroup()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushUserdata<Group>(L, player->getGroup());
		setMetatable(L, -1, "Group");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetGroup(lua_State* L) {
	// player:setGroup(group)
	Group* group = getUserdata<Group>(L, 2);
	if (!group) {
		pushBoolean(L, false);
		return 1;
	}

	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setGroup(group);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetSpecialContainersAvailable(lua_State* L) {
	// player:setSpecialContainersAvailable(stashMenu, marketMenu, depotSearchMenu)
	bool supplyStashMenu = getBoolean(L, 2, false);
	bool marketMenu = getBoolean(L, 3, false);
	bool depotSearchMenu = getBoolean(L, 4, false);
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setSpecialMenuAvailable(supplyStashMenu, marketMenu, depotSearchMenu);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetStamina(lua_State* L) {
	// player:getStamina()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getStaminaMinutes());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetStamina(lua_State* L) {
	// player:setStamina(stamina)
	uint16_t stamina = getNumber<uint16_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->staminaMinutes = std::min<uint16_t>(2520, stamina);
		player->sendStats();
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetSoul(lua_State* L) {
	// player:getSoul()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getSoul());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddSoul(lua_State* L) {
	// player:addSoul(soulChange)
	int32_t soulChange = getNumber<int32_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->changeSoul(soulChange);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetMaxSoul(lua_State* L) {
	// player:getMaxSoul()
	Player* player = getUserdata<Player>(L, 1);
	if (player && player->vocation) {
		lua_pushnumber(L, player->vocation->getSoulMax());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetBankBalance(lua_State* L) {
	// player:getBankBalance()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getBankBalance());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetBankBalance(lua_State* L) {
	// player:setBankBalance(bankBalance)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	player->setBankBalance(getNumber<uint64_t>(L, 2));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetStorageValue(lua_State* L) {
	// player:getStorageValue(key)
	const auto &player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint32_t key = getNumber<uint32_t>(L, 2);
	lua_pushnumber(L, player->getStorageValue(key));
	return 1;
}

int PlayerFunctions::luaPlayerSetStorageValue(lua_State* L) {
	// player:setStorageValue(key, value)
	int32_t value = getNumber<int32_t>(L, 3);
	uint32_t key = getNumber<uint32_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (IS_IN_KEYRANGE(key, RESERVED_RANGE)) {
		std::ostringstream ss;
		ss << "Accessing reserved range: " << key;
		reportErrorFunc(ss.str());
		pushBoolean(L, false);
		return 1;
	}

	if (player) {
		player->addStorageValue(key, value);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetStorageValueByName(lua_State* L) {
	// player:getStorageValueByName(name)
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	auto name = getString(L, 2);
	lua_pushnumber(L, player->getStorageValueByName(name));
	return 1;
}

int PlayerFunctions::luaPlayerSetStorageValueByName(lua_State* L) {
	// player:setStorageValueByName(storageName, value)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	auto storageName = getString(L, 2);
	int32_t value = getNumber<int32_t>(L, 3);

	player->addStorageValueByName(storageName, value);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerAddItem(lua_State* L) {
	// player:addItem(itemId, count = 1, canDropOnMap = true, subType = 1, slot = CONST_SLOT_WHEREEVER, tier = 0)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		pushBoolean(L, false);
		return 1;
	}

	uint16_t itemId;
	if (isNumber(L, 2)) {
		itemId = getNumber<uint16_t>(L, 2);
	} else {
		itemId = Item::items.getItemIdByName(getString(L, 2));
		if (itemId == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	int32_t count = getNumber<int32_t>(L, 3, 1);
	int32_t subType = getNumber<int32_t>(L, 5, 1);

	const ItemType &it = Item::items[itemId];

	int32_t itemCount = 1;
	int parameters = lua_gettop(L);
	if (parameters >= 4) {
		itemCount = std::max<int32_t>(1, count);
	} else if (it.hasSubType()) {
		if (it.stackable) {
			itemCount = std::ceil(count / (float_t)it.stackSize);
		}

		subType = count;
	} else {
		itemCount = std::max<int32_t>(1, count);
	}

	bool hasTable = itemCount > 1;
	if (hasTable) {
		lua_newtable(L);
	} else if (itemCount == 0) {
		lua_pushnil(L);
		return 1;
	}

	bool canDropOnMap = getBoolean(L, 4, true);
	Slots_t slot = getNumber<Slots_t>(L, 6, CONST_SLOT_WHEREEVER);
	auto tier = getNumber<uint8_t>(L, 7, 0);
	for (int32_t i = 1; i <= itemCount; ++i) {
		int32_t stackCount = subType;
		if (it.stackable) {
			stackCount = std::min<int32_t>(stackCount, it.stackSize);
			subType -= stackCount;
		}

		Item* item = Item::CreateItem(itemId, stackCount);
		if (!item) {
			if (!hasTable) {
				lua_pushnil(L);
			}
			return 1;
		}

		if (tier > 0) {
			item->setTier(tier);
		}

		ReturnValue ret = g_game().internalPlayerAddItem(player, item, canDropOnMap, slot);
		if (ret != RETURNVALUE_NOERROR) {
			delete item;
			if (!hasTable) {
				lua_pushnil(L);
			}
			return 1;
		}

		if (hasTable) {
			lua_pushnumber(L, i);
			pushUserdata<Item>(L, item);
			setItemMetatable(L, -1, item);
			lua_settable(L, -3);
		} else {
			pushUserdata<Item>(L, item);
			setItemMetatable(L, -1, item);
		}
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddItemEx(lua_State* L) {
	// player:addItemEx(item[, canDropOnMap = false[, index = INDEX_WHEREEVER[, flags = 0]]])
	// player:addItemEx(item[, canDropOnMap = true[, slot = CONST_SLOT_WHEREEVER]])
	Item* item = getUserdata<Item>(L, 2);
	if (!item) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_ITEM_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}

	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	if (item->getParent() != VirtualCylinder::virtualCylinder) {
		reportErrorFunc("Item already has a parent");
		pushBoolean(L, false);
		return 1;
	}

	bool canDropOnMap = getBoolean(L, 3, false);
	ReturnValue returnValue;
	if (canDropOnMap) {
		Slots_t slot = getNumber<Slots_t>(L, 4, CONST_SLOT_WHEREEVER);
		returnValue = g_game().internalPlayerAddItem(player, item, true, slot);
	} else {
		int32_t index = getNumber<int32_t>(L, 4, INDEX_WHEREEVER);
		uint32_t flags = getNumber<uint32_t>(L, 5, 0);
		returnValue = g_game().internalAddItem(player, item, index, flags);
	}

	if (returnValue == RETURNVALUE_NOERROR) {
		ScriptEnvironment::removeTempItem(item);
	}
	lua_pushnumber(L, returnValue);
	return 1;
}

int PlayerFunctions::luaPlayerRemoveStashItem(lua_State* L) {
	// player:removeStashItem(itemId, count)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint16_t itemId;
	if (isNumber(L, 2)) {
		itemId = getNumber<uint16_t>(L, 2);
	} else {
		itemId = Item::items.getItemIdByName(getString(L, 2));
		if (itemId == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	const ItemType &itemType = Item::items[itemId];
	if (itemType.id == 0) {
		lua_pushnil(L);
		return 1;
	}

	uint32_t count = getNumber<uint32_t>(L, 3);
	pushBoolean(L, player->withdrawItem(itemType.id, count));
	return 1;
}

int PlayerFunctions::luaPlayerRemoveItem(lua_State* L) {
	// player:removeItem(itemId, count[, subType = -1[, ignoreEquipped = false]])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint16_t itemId;
	if (isNumber(L, 2)) {
		itemId = getNumber<uint16_t>(L, 2);
	} else {
		itemId = Item::items.getItemIdByName(getString(L, 2));
		if (itemId == 0) {
			lua_pushnil(L);
			return 1;
		}
	}

	uint32_t count = getNumber<uint32_t>(L, 3);
	int32_t subType = getNumber<int32_t>(L, 4, -1);
	bool ignoreEquipped = getBoolean(L, 5, false);
	pushBoolean(L, player->removeItemOfType(itemId, count, subType, ignoreEquipped));
	return 1;
}

int PlayerFunctions::luaPlayerSendContainer(lua_State* L) {
	// player:sendContainer(container)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Container* container = getUserdata<Container>(L, 2);
	if (!container) {
		lua_pushnil(L);
		return 1;
	}

	player->sendContainer(static_cast<uint8_t>(container->getID()), container, container->hasParent(), static_cast<uint8_t>(container->getFirstIndex()));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetMoney(lua_State* L) {
	// player:getMoney()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getMoney());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddMoney(lua_State* L) {
	// player:addMoney(money)
	uint64_t money = getNumber<uint64_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		g_game().addMoney(player, money);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemoveMoney(lua_State* L) {
	// player:removeMoney(money[, flags = 0[, useBank = true]])
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint64_t money = getNumber<uint64_t>(L, 2);
		int32_t flags = getNumber<int32_t>(L, 3, 0);
		bool useBank = getBoolean(L, 4, true);
		pushBoolean(L, g_game().removeMoney(player, money, flags, useBank));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerShowTextDialog(lua_State* L) {
	// player:showTextDialog(id or name or userdata[, text[, canWrite[, length]]])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	int32_t length = getNumber<int32_t>(L, 5, -1);
	bool canWrite = getBoolean(L, 4, false);
	std::string text;

	int parameters = lua_gettop(L);
	if (parameters >= 3) {
		text = getString(L, 3);
	}

	Item* item;
	if (isNumber(L, 2)) {
		item = Item::CreateItem(getNumber<uint16_t>(L, 2));
	} else if (isString(L, 2)) {
		item = Item::CreateItem(Item::items.getItemIdByName(getString(L, 2)));
	} else if (isUserdata(L, 2)) {
		if (getUserdataType(L, 2) != LuaData_t::Item) {
			pushBoolean(L, false);
			return 1;
		}

		item = getUserdata<Item>(L, 2);
	} else {
		item = nullptr;
	}

	if (!item) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_ITEM_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}

	if (length < 0) {
		length = Item::items[item->getID()].maxTextLen;
	}

	if (!text.empty()) {
		item->setAttribute(ItemAttribute_t::TEXT, text);
		length = std::max<int32_t>(text.size(), length);
	}

	item->setParent(player);
	player->setWriteItem(item, length);
	player->sendTextWindow(item, length, canWrite);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerSendTextMessage(lua_State* L) {
	// player:sendTextMessage(type, text[, position, primaryValue = 0, primaryColor = TEXTCOLOR_NONE[, secondaryValue = 0, secondaryColor = TEXTCOLOR_NONE]])
	// player:sendTextMessage(type, text, channelId)

	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	int parameters = lua_gettop(L);

	TextMessage message(getNumber<MessageClasses>(L, 2), getString(L, 3));
	if (parameters == 4) {
		uint16_t channelId = getNumber<uint16_t>(L, 4);
		ChatChannel* channel = g_chat().getChannel(*player, channelId);
		if (!channel || !channel->hasUser(*player)) {
			pushBoolean(L, false);
			return 1;
		}
		message.channelId = channelId;
	} else {
		if (parameters >= 6) {
			message.position = getPosition(L, 4);
			message.primary.value = getNumber<int32_t>(L, 5);
			message.primary.color = getNumber<TextColor_t>(L, 6);
		}

		if (parameters >= 8) {
			message.secondary.value = getNumber<int32_t>(L, 7);
			message.secondary.color = getNumber<TextColor_t>(L, 8);
		}
	}

	player->sendTextMessage(message);
	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerSendChannelMessage(lua_State* L) {
	// player:sendChannelMessage(author, text, type, channelId)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint16_t channelId = getNumber<uint16_t>(L, 5);
	SpeakClasses type = getNumber<SpeakClasses>(L, 4);
	const std::string &text = getString(L, 3);
	const std::string &author = getString(L, 2);
	player->sendChannelMessage(author, text, type, channelId);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerSendPrivateMessage(lua_State* L) {
	// player:sendPrivateMessage(speaker, text[, type])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	const Player* speaker = getUserdata<const Player>(L, 2);
	const std::string &text = getString(L, 3);
	SpeakClasses type = getNumber<SpeakClasses>(L, 4, TALKTYPE_PRIVATE_FROM);
	player->sendPrivateMessage(speaker, type, text);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerChannelSay(lua_State* L) {
	// player:channelSay(speaker, type, text, channelId)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Creature* speaker = getCreature(L, 2);
	SpeakClasses type = getNumber<SpeakClasses>(L, 3);
	const std::string &text = getString(L, 4);
	uint16_t channelId = getNumber<uint16_t>(L, 5);
	player->sendToChannel(speaker, type, text, channelId);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerOpenChannel(lua_State* L) {
	// player:openChannel(channelId)
	uint16_t channelId = getNumber<uint16_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		g_game().playerOpenChannel(player->getID(), channelId);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetSlotItem(lua_State* L) {
	// player:getSlotItem(slot)
	const Player* player = getUserdata<const Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint32_t slot = getNumber<uint32_t>(L, 2);
	Thing* thing = player->getThing(slot);
	if (!thing) {
		lua_pushnil(L);
		return 1;
	}

	Item* item = thing->getItem();
	if (item) {
		pushUserdata<Item>(L, item);
		setItemMetatable(L, -1, item);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetParty(lua_State* L) {
	// player:getParty()
	const Player* player = getUserdata<const Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Party* party = player->getParty();
	if (party) {
		pushUserdata<Party>(L, party);
		setMetatable(L, -1, "Party");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddOutfit(lua_State* L) {
	// player:addOutfit(lookType)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->addOutfit(getNumber<uint16_t>(L, 2), 0);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddOutfitAddon(lua_State* L) {
	// player:addOutfitAddon(lookType, addon)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t lookType = getNumber<uint16_t>(L, 2);
		uint8_t addon = getNumber<uint8_t>(L, 3);
		player->addOutfit(lookType, addon);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemoveOutfit(lua_State* L) {
	// player:removeOutfit(lookType)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t lookType = getNumber<uint16_t>(L, 2);
		pushBoolean(L, player->removeOutfit(lookType));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemoveOutfitAddon(lua_State* L) {
	// player:removeOutfitAddon(lookType, addon)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t lookType = getNumber<uint16_t>(L, 2);
		uint8_t addon = getNumber<uint8_t>(L, 3);
		pushBoolean(L, player->removeOutfitAddon(lookType, addon));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerHasOutfit(lua_State* L) {
	// player:hasOutfit(lookType[, addon = 0])
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t lookType = getNumber<uint16_t>(L, 2);
		uint8_t addon = getNumber<uint8_t>(L, 3, 0);
		pushBoolean(L, player->canWear(lookType, addon));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSendOutfitWindow(lua_State* L) {
	// player:sendOutfitWindow()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->sendOutfitWindow();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddMount(lua_State* L) {
	// player:addMount(mountId or mountName)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint8_t mountId;
	if (isNumber(L, 2)) {
		mountId = getNumber<uint8_t>(L, 2);
	} else {
		const std::shared_ptr<Mount> mount = g_game().mounts.getMountByName(getString(L, 2));
		if (!mount) {
			lua_pushnil(L);
			return 1;
		}
		mountId = mount->id;
	}
	pushBoolean(L, player->tameMount(mountId));
	return 1;
}

int PlayerFunctions::luaPlayerRemoveMount(lua_State* L) {
	// player:removeMount(mountId or mountName)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint8_t mountId;
	if (isNumber(L, 2)) {
		mountId = getNumber<uint8_t>(L, 2);
	} else {
		const std::shared_ptr<Mount> mount = g_game().mounts.getMountByName(getString(L, 2));
		if (!mount) {
			lua_pushnil(L);
			return 1;
		}
		mountId = mount->id;
	}
	pushBoolean(L, player->untameMount(mountId));
	return 1;
}

int PlayerFunctions::luaPlayerHasMount(lua_State* L) {
	// player:hasMount(mountId or mountName)
	const Player* player = getUserdata<const Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	std::shared_ptr<Mount> mount = nullptr;
	if (isNumber(L, 2)) {
		mount = g_game().mounts.getMountByID(getNumber<uint8_t>(L, 2));
	} else {
		mount = g_game().mounts.getMountByName(getString(L, 2));
	}

	if (mount) {
		pushBoolean(L, player->hasMount(mount));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddFamiliar(lua_State* L) {
	// player:addFamiliar(lookType)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->addFamiliar(getNumber<uint16_t>(L, 2));
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerRemoveFamiliar(lua_State* L) {
	// player:removeFamiliar(lookType)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t lookType = getNumber<uint16_t>(L, 2);
		pushBoolean(L, player->removeFamiliar(lookType));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerHasFamiliar(lua_State* L) {
	// player:hasFamiliar(lookType)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t lookType = getNumber<uint16_t>(L, 2);
		pushBoolean(L, player->canFamiliar(lookType));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetFamiliarLooktype(lua_State* L) {
	// player:setFamiliarLooktype(lookType)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setFamiliarLooktype(getNumber<uint16_t>(L, 2));
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetFamiliarLooktype(lua_State* L) {
	// player:getFamiliarLooktype()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->defaultOutfit.lookFamiliarsType);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetPremiumDays(lua_State* L) {
	// player:getPremiumDays()
	Player* player = getUserdata<Player>(L, 1);
	if (player && player->getAccount()) {
		lua_pushnumber(L, player->getAccount()->getPremiumRemainingDays());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddPremiumDays(lua_State* L) {
	// player:addPremiumDays(days)
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		lua_pushnil(L);
		return 1;
	}

	auto premiumDays = player->getAccount()->getPremiumRemainingDays();

	if (premiumDays == std::numeric_limits<uint16_t>::max()) {
		return 1;
	}

	int32_t addDays = std::min<int32_t>(0xFFFE - premiumDays, getNumber<uint16_t>(L, 2));
	if (addDays <= 0) {
		return 1;
	}

	player->getAccount()->addPremiumDays(addDays);

	if (player->getAccount()->save() != account::ERROR_NO) {
		return 1;
	}

	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerRemovePremiumDays(lua_State* L) {
	// player:removePremiumDays(days)
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		lua_pushnil(L);
		return 1;
	}

	auto premiumDays = player->getAccount()->getPremiumRemainingDays();

	if (premiumDays == std::numeric_limits<uint16_t>::max()) {
		return 1;
	}

	int32_t removeDays = std::min<int32_t>(0xFFFE - premiumDays, getNumber<uint16_t>(L, 2));
	if (removeDays <= 0) {
		return 1;
	}

	player->getAccount()->addPremiumDays(-removeDays);

	if (player->getAccount()->save() != account::ERROR_NO) {
		return 1;
	}

	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetTibiaCoins(lua_State* L) {
	// player:getTibiaCoins()
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	auto [coins, result] = player->getAccount()->getCoins(account::CoinType::COIN);

	if (result == account::ERROR_NO) {
		lua_pushnumber(L, coins);
	}

	return 1;
}

int PlayerFunctions::luaPlayerAddTibiaCoins(lua_State* L) {
	// player:addTibiaCoins(coins)
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	if (player->account->addCoins(account::CoinType::COIN, getNumber<uint32_t>(L, 2)) != account::ERROR_NO) {
		reportErrorFunc("Failed to add coins");
		lua_pushnil(L);
		return 1;
	}

	if (player->getAccount()->save() != account::ERROR_NO) {
		reportErrorFunc("Failed to save account");
		lua_pushnil(L);
		return 1;
	}

	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerRemoveTibiaCoins(lua_State* L) {
	// player:removeTibiaCoins(coins)
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	if (player->account->removeCoins(account::CoinType::COIN, getNumber<uint32_t>(L, 2)) != account::ERROR_NO) {
		reportErrorFunc("Failed to remove coins");
		return 1;
	}

	if (player->getAccount()->save() != account::ERROR_NO) {
		reportErrorFunc("Failed to save account");
		lua_pushnil(L);
		return 1;
	}

	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerGetTransferableCoins(lua_State* L) {
	// player:getTransferableCoins()
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	auto [coins, result] = player->getAccount()->getCoins(account::CoinType::TRANSFERABLE);

	if (result == account::ERROR_NO) {
		lua_pushnumber(L, coins);
	}

	return 1;
}

int PlayerFunctions::luaPlayerAddTransferableCoins(lua_State* L) {
	// player:addTransferableCoins(coins)
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	if (player->account->addCoins(account::CoinType::TRANSFERABLE, getNumber<uint32_t>(L, 2)) != account::ERROR_NO) {
		reportErrorFunc("failed to add transferable coins");
		lua_pushnil(L);
		return 1;
	}

	if (player->getAccount()->save() != account::ERROR_NO) {
		reportErrorFunc("failed to save account");
		lua_pushnil(L);
		return 1;
	}

	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerRemoveTransferableCoins(lua_State* L) {
	// player:removeTransferableCoins(coins)
	Player* player = getUserdata<Player>(L, 1);
	if (!player || !player->getAccount()) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		lua_pushnil(L);
		return 1;
	}

	if (player->account->removeCoins(account::CoinType::TRANSFERABLE, getNumber<uint32_t>(L, 2)) != account::ERROR_NO) {
		reportErrorFunc("failed to remove transferable coins");
		lua_pushnil(L);
		return 1;
	}

	if (player->getAccount()->save() != account::ERROR_NO) {
		reportErrorFunc("failed to save account");
		lua_pushnil(L);
		return 1;
	}

	pushBoolean(L, true);

	return 1;
}

int PlayerFunctions::luaPlayerHasBlessing(lua_State* L) {
	// player:hasBlessing(blessing)
	uint8_t blessing = getNumber<uint8_t>(L, 2);
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushBoolean(L, player->hasBlessing(blessing));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerAddBlessing(lua_State* L) {
	// player:addBlessing(blessing)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint8_t blessing = getNumber<uint8_t>(L, 2);
	uint8_t count = getNumber<uint8_t>(L, 3);

	player->addBlessing(blessing, count);
	player->sendBlessStatus();
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerRemoveBlessing(lua_State* L) {
	// player:removeBlessing(blessing)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	uint8_t blessing = getNumber<uint8_t>(L, 2);
	uint8_t count = getNumber<uint8_t>(L, 3);

	if (!player->hasBlessing(blessing)) {
		pushBoolean(L, false);
		return 1;
	}

	player->removeBlessing(blessing, count);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetBlessingCount(lua_State* L) {
	// player:getBlessingCount(index)
	Player* player = getUserdata<Player>(L, 1);
	uint8_t index = getNumber<uint8_t>(L, 2);
	if (index == 0) {
		index = 1;
	}

	if (player) {
		lua_pushnumber(L, player->getBlessingCount(index));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerCanLearnSpell(lua_State* L) {
	// player:canLearnSpell(spellName)
	const Player* player = getUserdata<const Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	const std::string &spellName = getString(L, 2);
	const auto spell = g_spells().getInstantSpellByName(spellName);
	if (!spell) {
		reportErrorFunc("Spell \"" + spellName + "\" not found");
		pushBoolean(L, false);
		return 1;
	}

	if (player->hasFlag(PlayerFlags_t::IgnoreSpellCheck)) {
		pushBoolean(L, true);
		return 1;
	}

	const auto &vocMap = spell->getVocMap();
	if (vocMap.count(player->getVocationId()) == 0) {
		pushBoolean(L, false);
	} else if (player->getLevel() < spell->getLevel()) {
		pushBoolean(L, false);
	} else if (player->getMagicLevel() < spell->getMagicLevel()) {
		pushBoolean(L, false);
	} else {
		pushBoolean(L, true);
	}
	return 1;
}

int PlayerFunctions::luaPlayerLearnSpell(lua_State* L) {
	// player:learnSpell(spellName)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		const std::string &spellName = getString(L, 2);
		player->learnInstantSpell(spellName);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerForgetSpell(lua_State* L) {
	// player:forgetSpell(spellName)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		const std::string &spellName = getString(L, 2);
		player->forgetInstantSpell(spellName);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerHasLearnedSpell(lua_State* L) {
	// player:hasLearnedSpell(spellName)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		const std::string &spellName = getString(L, 2);
		pushBoolean(L, player->hasLearnedInstantSpell(spellName));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSendTutorial(lua_State* L) {
	// player:sendTutorial(tutorialId)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint8_t tutorialId = getNumber<uint8_t>(L, 2);
		player->sendTutorial(tutorialId);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerOpenImbuementWindow(lua_State* L) {
	// player:openImbuementWindow(item)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}

	Item* item = getUserdata<Item>(L, 2);
	if (!item) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_ITEM_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}

	player->openImbuementWindow(item);
	return 1;
}

int PlayerFunctions::luaPlayerCloseImbuementWindow(lua_State* L) {
	// player:closeImbuementWindow()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}

	player->closeImbuementWindow();
	return 1;
}

int PlayerFunctions::luaPlayerAddMapMark(lua_State* L) {
	// player:addMapMark(position, type, description)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		const Position &position = getPosition(L, 2);
		uint8_t type = getNumber<uint8_t>(L, 3);
		const std::string &description = getString(L, 4);
		player->sendAddMarker(position, type, description);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSave(lua_State* L) {
	// player:save()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		if (!player->isOffline()) {
			player->loginPosition = player->getPosition();
		}
		pushBoolean(L, IOLoginData::savePlayer(player));
		if (player->isOffline()) {
			delete player; // avoiding memory leak
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerPopupFYI(lua_State* L) {
	// player:popupFYI(message)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		const std::string &message = getString(L, 2);
		player->sendFYIBox(message);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerIsPzLocked(lua_State* L) {
	// player:isPzLocked()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushBoolean(L, player->isPzLocked());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetClient(lua_State* L) {
	// player:getClient()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_createtable(L, 0, 2);
		setField(L, "version", player->getProtocolVersion());
		setField(L, "os", player->getOperatingSystem());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetHouse(lua_State* L) {
	// player:getHouse()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	House* house = g_game().map.houses.getHouseByPlayerId(player->getGUID());
	if (house) {
		pushUserdata<House>(L, house);
		setMetatable(L, -1, "House");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSendHouseWindow(lua_State* L) {
	// player:sendHouseWindow(house, listId)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	House* house = getUserdata<House>(L, 2);
	if (!house) {
		lua_pushnil(L);
		return 1;
	}

	uint32_t listId = getNumber<uint32_t>(L, 3);
	player->sendHouseWindow(house, listId);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerSetEditHouse(lua_State* L) {
	// player:setEditHouse(house, listId)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	House* house = getUserdata<House>(L, 2);
	if (!house) {
		lua_pushnil(L);
		return 1;
	}

	uint32_t listId = getNumber<uint32_t>(L, 3);
	player->setEditHouse(house, listId);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerSetGhostMode(lua_State* L) {
	// player:setGhostMode(enabled)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	bool enabled = getBoolean(L, 2);
	if (player->isInGhostMode() == enabled) {
		pushBoolean(L, true);
		return 1;
	}

	player->switchGhostMode();

	Tile* tile = player->getTile();
	const Position &position = player->getPosition();

	SpectatorHashSet spectators;
	g_game().map.getSpectators(spectators, position, true, true);
	for (Creature* spectator : spectators) {
		Player* tmpPlayer = spectator->getPlayer();
		if (tmpPlayer != player && !tmpPlayer->isAccessPlayer()) {
			if (enabled) {
				tmpPlayer->sendRemoveTileThing(position, tile->getStackposOfCreature(tmpPlayer, player));
			} else {
				tmpPlayer->sendCreatureAppear(player, position, true);
			}
		} else {
			tmpPlayer->sendCreatureChangeVisible(player, !enabled);
		}
	}

	if (player->isInGhostMode()) {
		for (const auto &it : g_game().getPlayers()) {
			if (!it.second->isAccessPlayer()) {
				it.second->notifyStatusChange(player, VIPSTATUS_OFFLINE);
			}
		}
		IOLoginData::updateOnlineStatus(player->getGUID(), false);
	} else {
		for (const auto &it : g_game().getPlayers()) {
			if (!it.second->isAccessPlayer()) {
				it.second->notifyStatusChange(player, player->statusVipList);
			}
		}
		IOLoginData::updateOnlineStatus(player->getGUID(), true);
	}
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetContainerId(lua_State* L) {
	// player:getContainerId(container)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Container* container = getUserdata<Container>(L, 2);
	if (container) {
		lua_pushnumber(L, player->getContainerID(container));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetContainerById(lua_State* L) {
	// player:getContainerById(id)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	Container* container = player->getContainerByID(getNumber<uint8_t>(L, 2));
	if (container) {
		pushUserdata<Container>(L, container);
		setMetatable(L, -1, "Container");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetContainerIndex(lua_State* L) {
	// player:getContainerIndex(id)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getContainerIndex(getNumber<uint8_t>(L, 2)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetInstantSpells(lua_State* L) {
	// player:getInstantSpells()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	std::vector<std::shared_ptr<InstantSpell>> spells;
	for (auto &[key, spell] : g_spells().getInstantSpells()) {
		if (spell->canCast(player)) {
			spells.push_back(spell);
		}
	}

	lua_createtable(L, spells.size(), 0);

	int index = 0;
	for (auto spell : spells) {
		pushInstantSpell(L, *spell);
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int PlayerFunctions::luaPlayerCanCast(lua_State* L) {
	// player:canCast(spell)
	Player* player = getUserdata<Player>(L, 1);
	const auto spell = getUserdataShared<InstantSpell>(L, 2);
	if (player && spell) {
		pushBoolean(L, spell->canCast(player));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerHasChaseMode(lua_State* L) {
	// player:hasChaseMode()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushBoolean(L, player->chaseMode);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerHasSecureMode(lua_State* L) {
	// player:hasSecureMode()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushBoolean(L, player->secureMode);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetFightMode(lua_State* L) {
	// player:getFightMode()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->fightMode);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetBaseXpGain(lua_State* L) {
	// player:getBaseXpGain()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getBaseXpGain());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetBaseXpGain(lua_State* L) {
	// player:setBaseXpGain(value)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setBaseXpGain(getNumber<uint16_t>(L, 2));
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetVoucherXpBoost(lua_State* L) {
	// player:getVoucherXpBoost()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getVoucherXpBoost());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetVoucherXpBoost(lua_State* L) {
	// player:setVoucherXpBoost(value)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setVoucherXpBoost(getNumber<uint16_t>(L, 2));
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetGrindingXpBoost(lua_State* L) {
	// player:getGrindingXpBoost()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getGrindingXpBoost());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetGrindingXpBoost(lua_State* L) {
	// player:setGrindingXpBoost(value)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setGrindingXpBoost(getNumber<uint16_t>(L, 2));
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetStoreXpBoost(lua_State* L) {
	// player:getStoreXpBoost()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getStoreXpBoost());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetStoreXpBoost(lua_State* L) {
	// player:setStoreXpBoost(value)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t experience = getNumber<uint16_t>(L, 2);
		player->setStoreXpBoost(experience);
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetStaminaXpBoost(lua_State* L) {
	// player:getStaminaXpBoost()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getStaminaXpBoost());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetStaminaXpBoost(lua_State* L) {
	// player:setStaminaXpBoost(value)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		player->setStaminaXpBoost(getNumber<uint16_t>(L, 2));
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetExpBoostStamina(lua_State* L) {
	// player:setExpBoostStamina(percent)
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		uint16_t stamina = getNumber<uint16_t>(L, 2);
		player->setExpBoostStamina(stamina);
		player->sendStats();
		pushBoolean(L, true);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetExpBoostStamina(lua_State* L) {
	// player:getExpBoostStamina()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getExpBoostStamina());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetIdleTime(lua_State* L) {
	// player:getIdleTime()
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		lua_pushnumber(L, player->getIdleTime());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetFreeBackpackSlots(lua_State* L) {
	// player:getFreeBackpackSlots()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
	}

	lua_pushnumber(L, std::max<uint16_t>(0, player->getFreeBackpackSlots()));
	return 1;
}

int PlayerFunctions::luaPlayerIsOffline(lua_State* L) {
	Player* player = getUserdata<Player>(L, 1);
	if (player) {
		pushBoolean(L, player->isOffline());
	} else {
		pushBoolean(L, true);
	}

	return 1;
}

int PlayerFunctions::luaPlayerOpenMarket(lua_State* L) {
	// player:openMarket()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	player->sendMarketEnter(player->getLastDepotId());
	pushBoolean(L, true);
	return 1;
}

// Forge
int PlayerFunctions::luaPlayerOpenForge(lua_State* L) {
	// player:openForge()
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->sendOpenForge();
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerCloseForge(lua_State* L) {
	// player:closeForge()
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->closeForgeWindow();
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerAddForgeDusts(lua_State* L) {
	// player:addForgeDusts(amount)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->addForgeDusts(getNumber<uint64_t>(L, 2, 0));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerRemoveForgeDusts(lua_State* L) {
	// player:removeForgeDusts(amount)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->removeForgeDusts(getNumber<uint64_t>(L, 2, 0));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetForgeDusts(lua_State* L) {
	// player:getForgeDusts()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	lua_pushnumber(L, static_cast<lua_Number>(player->getForgeDusts()));
	return 1;
}

int PlayerFunctions::luaPlayerSetForgeDusts(lua_State* L) {
	// player:setForgeDusts()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->setForgeDusts(getNumber<uint64_t>(L, 2, 0));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerAddForgeDustLevel(lua_State* L) {
	// player:addForgeDustLevel(amount)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->addForgeDustLevel(getNumber<uint64_t>(L, 2, 1));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerRemoveForgeDustLevel(lua_State* L) {
	// player:removeForgeDustLevel(amount)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->removeForgeDustLevel(getNumber<uint64_t>(L, 2, 1));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetForgeDustLevel(lua_State* L) {
	// player:getForgeDustLevel()
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	lua_pushnumber(L, static_cast<lua_Number>(player->getForgeDustLevel()));
	return 1;
}

int PlayerFunctions::luaPlayerGetForgeSlivers(lua_State* L) {
	// player:getForgeSlivers()
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	auto [sliver, core] = player->getForgeSliversAndCores();
	lua_pushnumber(L, static_cast<lua_Number>(sliver));
	return 1;
}

int PlayerFunctions::luaPlayerGetForgeCores(lua_State* L) {
	// player:getForgeCores()
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	auto [sliver, core] = player->getForgeSliversAndCores();
	lua_pushnumber(L, static_cast<lua_Number>(core));
	return 1;
}

int PlayerFunctions::luaPlayerSetFaction(lua_State* L) {
	// player:setFaction(factionId)
	Player* player = getUserdata<Player>(L, 1);
	if (player == nullptr) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	Faction_t factionId = getNumber<Faction_t>(L, 2);
	player->setFaction(factionId);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetFaction(lua_State* L) {
	// player:getFaction()
	const Player* player = getUserdata<Player>(L, 1);
	if (player == nullptr) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	lua_pushnumber(L, player->getFaction());
	return 1;
}

int PlayerFunctions::luaPlayerIsUIExhausted(lua_State* L) {
	// player:isUIExhausted()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	uint16_t time = getNumber<uint16_t>(L, 2);
	pushBoolean(L, player->isUIExhausted(time));
	return 1;
}

int PlayerFunctions::luaPlayerUpdateUIExhausted(lua_State* L) {
	// player:updateUIExhausted(exhaustionTime = 250)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->updateUIExhausted();
	pushBoolean(L, true);
	return 1;
}

// Bosstiary Cooldown Timer
int PlayerFunctions::luaPlayerBosstiaryCooldownTimer(lua_State* L) {
	// player:sendBosstiaryCooldownTimer()
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->sendBosstiaryCooldownTimer();
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerAddBosstiaryKill(lua_State* L) {
	// player:addBosstiaryKill(name[, amount = 1])
	if (Player* player = getUserdata<Player>(L, 1);
		player) {
		const auto mtype = g_monsters().getMonsterType(getString(L, 2));
		if (mtype) {
			g_ioBosstiary().addBosstiaryKill(player, mtype, getNumber<uint32_t>(L, 3, 1));
			pushBoolean(L, true);
		} else {
			lua_pushnil(L);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int PlayerFunctions::luaPlayerSetBossPoints(lua_State* L) {
	// player:setBossPoints()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->setBossPoints(getNumber<uint32_t>(L, 2, 0));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerSetRemoveBossTime(lua_State* L) {
	// player:setRemoveBossTime()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->setRemoveBossTime(getNumber<uint8_t>(L, 2, 0));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetSlotBossId(lua_State* L) {
	// player:getSlotBossId(slotId)
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	uint8_t slotId = getNumber<uint8_t>(L, 2);
	auto bossId = player->getSlotBossId(slotId);
	lua_pushnumber(L, static_cast<lua_Number>(bossId));
	return 1;
}

int PlayerFunctions::luaPlayerGetBossBonus(lua_State* L) {
	// player:getBossBonus(slotId)
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	uint8_t slotId = getNumber<uint8_t>(L, 2);
	auto bossId = player->getSlotBossId(slotId);

	uint32_t playerBossPoints = player->getBossPoints();
	uint16_t currentBonus = g_ioBosstiary().calculateLootBonus(playerBossPoints);

	auto bossLevel = g_ioBosstiary().getBossCurrentLevel(player, bossId);
	uint16_t bonusBoss = currentBonus + (bossLevel == 3 ? 25 : 0);

	lua_pushnumber(L, static_cast<lua_Number>(bonusBoss));
	return 1;
}

int PlayerFunctions::luaPlayerSendSingleSoundEffect(lua_State* L) {
	// player:sendSingleSoundEffect(soundId[, actor = true])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	SoundEffect_t soundEffect = getNumber<SoundEffect_t>(L, 2);
	bool actor = getBoolean(L, 3, true);

	player->sendSingleSoundEffect(player->getPosition(), soundEffect, actor ? SourceEffect_t::OWN : SourceEffect_t::GLOBAL);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerSendDoubleSoundEffect(lua_State* L) {
	// player:sendDoubleSoundEffect(mainSoundId, secondarySoundId[, actor = true])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	SoundEffect_t mainSoundEffect = getNumber<SoundEffect_t>(L, 2);
	SoundEffect_t secondarySoundEffect = getNumber<SoundEffect_t>(L, 3);
	bool actor = getBoolean(L, 4, true);

	player->sendDoubleSoundEffect(player->getPosition(), mainSoundEffect, actor ? SourceEffect_t::OWN : SourceEffect_t::GLOBAL, secondarySoundEffect, actor ? SourceEffect_t::OWN : SourceEffect_t::GLOBAL);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetName(lua_State* L) {
	// player:getName()
	const auto player = getUserdata<const Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	pushString(L, player->getName());
	return 1;
}

int PlayerFunctions::luaPlayerHasGroupFlag(lua_State* L) {
	// player:hasGroupFlag(flag)
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	pushBoolean(L, player->hasFlag(getNumber<PlayerFlags_t>(L, 2)));
	return 1;
}

int PlayerFunctions::luaPlayerSetGroupFlag(lua_State* L) {
	// player:setGroupFlag(flag)
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->setFlag(getNumber<PlayerFlags_t>(L, 2));
	return 1;
}

int PlayerFunctions::luaPlayerRemoveGroupFlag(lua_State* L) {
	// player:removeGroupFlag(flag)
	const Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	player->removeFlag(getNumber<PlayerFlags_t>(L, 2));
	return 1;
}

// Hazard system
int PlayerFunctions::luaPlayerAddHazardSystemPoints(lua_State* L) {
	// player:setHazardSystemPoints(amount)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		pushBoolean(L, false);
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		return 1;
	}

	player->setHazardSystemPoints(getNumber<int32_t>(L, 2, 0));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetHazardSystemPoints(lua_State* L) {
	// player:getHazardSystemPoints()
	const auto player = getUserdata<Player>(L, 1);
	if (!player) {
		pushBoolean(L, false);
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		return 1;
	}

	lua_pushnumber(L, player->getHazardSystemPoints());
	return 1;
}

int PlayerFunctions::luaPlayerSetLoyaltyBonus(lua_State* L) {
	// player:setLoyaltyBonus(amount)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	player->setLoyaltyBonus(getNumber<uint16_t>(L, 2));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerGetLoyaltyBonus(lua_State* L) {
	// player:getLoyaltyBonus()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushnumber(L, player->getLoyaltyBonus());
	return 1;
}

int PlayerFunctions::luaPlayerGetLoyaltyPoints(lua_State* L) {
	// player:getLoyaltyPoints()
	const auto &player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushnumber(L, player->getLoyaltyPoints());
	return 1;
}

int PlayerFunctions::luaPlayerGetLoyaltyTitle(lua_State* L) {
	// player:getLoyaltyTitle()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	pushString(L, player->getLoyaltyTitle());
	return 1;
}

int PlayerFunctions::luaPlayerSetLoyaltyTitle(lua_State* L) {
	// player:setLoyaltyTitle(name)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	player->setLoyaltyTitle(getString(L, 2));
	pushBoolean(L, true);
	return 1;
}

// Wheel of destiny system
int PlayerFunctions::luaPlayerInstantSkillWOD(lua_State* L) {
	// player:instantSkillWOD(name[, value])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	std::string name = getString(L, 2);
	if (lua_gettop(L) == 2) {
		pushBoolean(L, player->wheel()->getInstant(name));
	} else {
		player->wheel()->setSpellInstant(name, getBoolean(L, 3));
		pushBoolean(L, true);
	}
	return 1;
}

int PlayerFunctions::luaPlayerUpgradeSpellWOD(lua_State* L) {
	// player:upgradeSpellsWOD([name[, add]])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	if (lua_gettop(L) == 1) {
		player->wheel()->resetUpgradedSpells();
		return 1;
	}

	std::string name = getString(L, 2);
	if (lua_gettop(L) == 2) {
		lua_pushnumber(L, static_cast<lua_Number>(player->wheel()->getSpellUpgrade(name)));
		return 1;
	}

	bool add = getBoolean(L, 3);
	if (add) {
		player->wheel()->upgradeSpell(name);
	} else {
		player->wheel()->downgradeSpell(name);
	}

	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerRevelationStageWOD(lua_State* L) {
	// player:revelationStagesWOD([name[, set]])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	if (lua_gettop(L) == 1) {
		player->wheel()->resetUpgradedSpells();
		return 1;
	}

	std::string name = getString(L, 2);
	if (lua_gettop(L) == 2) {
		lua_pushnumber(L, static_cast<lua_Number>(player->wheel()->getStage(name)));
		return 1;
	}

	bool value = getNumber<uint8_t>(L, 3);
	player->wheel()->setSpellInstant(name, value);

	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerReloadData(lua_State* L) {
	// player:reloadData()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	player->sendSkills();
	player->sendStats();
	player->sendBasicData();
	player->wheel()->sendGiftOfLifeCooldown();
	g_game().reloadCreature(player);
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerOnThinkWheelOfDestiny(lua_State* L) {
	// player:onThinkWheelOfDestiny([force = false])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	player->wheel()->onThink(getBoolean(L, 2, false));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerAvatarTimer(lua_State* L) {
	// player:avatarTimer([value])
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}

	if (lua_gettop(L) == 1) {
		lua_pushnumber(L, (lua_Number)player->wheel()->getOnThinkTimer(WheelOnThink_t::AVATAR));
	} else {
		player->wheel()->setOnThinkTimer(WheelOnThink_t::AVATAR, getNumber<int64_t>(L, 2));
		pushBoolean(L, true);
	}
	return 1;
}

int PlayerFunctions::luaPlayerGetWheelSpellAdditionalArea(lua_State* L) {
	// player:getWheelSpellAdditionalArea(spellname)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	auto spellName = getString(L, 2);
	if (spellName.empty()) {
		reportErrorFunc("Spell name is empty");
		pushBoolean(L, false);
		return 0;
	}

	auto spell = g_spells().getInstantSpellByName(spellName);
	if (!spell) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_SPELL_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	pushBoolean(L, player->wheel()->getSpellAdditionalArea(spellName));
	return 1;
}

int PlayerFunctions::luaPlayerGetWheelSpellAdditionalTarget(lua_State* L) {
	// player:getWheelSpellAdditionalTarget(spellname)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	auto spellName = getString(L, 2);
	if (spellName.empty()) {
		reportErrorFunc("Spell name is empty");
		pushBoolean(L, false);
		return 0;
	}

	auto spell = g_spells().getInstantSpellByName(spellName);
	if (!spell) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_SPELL_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	lua_pushnumber(L, player->wheel()->getSpellAdditionalTarget(spellName));
	return 1;
}

int PlayerFunctions::luaPlayerGetWheelSpellAdditionalDuration(lua_State* L) {
	// player:getWheelSpellAdditionalDuration(spellname)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	auto spellName = getString(L, 2);
	if (spellName.empty()) {
		reportErrorFunc("Spell name is empty");
		pushBoolean(L, false);
		return 0;
	}

	auto spell = g_spells().getInstantSpellByName(spellName);
	if (!spell) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_SPELL_NOT_FOUND));
		pushBoolean(L, false);
		return 0;
	}

	lua_pushnumber(L, player->wheel()->getSpellAdditionalDuration(spellName));
	return 1;
}

int PlayerFunctions::luaPlayerUpdateConcoction(lua_State* L) {
	// player:updateConcoction(itemid, timeLeft)
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}
	player->updateConcoction(getNumber<uint16_t>(L, 2), getNumber<uint16_t>(L, 3));
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerClearSpellCooldowns(lua_State* L) {
	// player:clearSpellCooldowns()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		lua_pushnil(L);
		return 1;
	}
	player->clearCooldowns();
	pushBoolean(L, true);
	return 1;
}

int PlayerFunctions::luaPlayerIsVip(lua_State* L) {
	// player:isVip()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}
	pushBoolean(L, player->isVip());
	return 1;
}

int PlayerFunctions::luaPlayerGetVipDays(lua_State* L) {
	// player:getVipDays()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}

	lua_pushnumber(L, player->getPremiumDays());
	return 1;
}

int PlayerFunctions::luaPlayerGetVipTime(lua_State* L) {
	// player:getVipTime()
	Player* player = getUserdata<Player>(L, 1);
	if (!player) {
		reportErrorFunc(getErrorDesc(LUA_ERROR_PLAYER_NOT_FOUND));
		pushBoolean(L, false);
		return 1;
	}

	lua_pushinteger(L, player->getPremiumLastDay());
	return 1;
}
