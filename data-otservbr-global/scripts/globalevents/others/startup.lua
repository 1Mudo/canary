local serverstartup = GlobalEvent("serverstartup")
function serverstartup.onStartup()
	logger.debug("Loading map attributes")
	logger.debug("Loaded {} npcs and spawned {} monsters", Game.getNpcCount(), Game.getMonsterCount())
	logger.debug("Loaded {} towns with {} houses in total", #Game.getTowns(), #Game.getHouses())
	-- Sign table
	loadLuaMapSign(SignTable)
	logger.debug("Loaded {} signs in the map", #SignTable)
	-- Book/Document table
	loadLuaMapBookDocument(BookDocumentTable)

	-- Action and unique tables
	-- Chest table
	loadLuaMapAction(ChestAction)
	loadLuaMapUnique(ChestUnique)
	-- Corpse table
	loadLuaMapAction(CorpseAction)
	loadLuaMapUnique(CorpseUnique)
	-- Doors key table
	loadLuaMapAction(KeyDoorAction)
	-- Doors level table
	loadLuaMapAction(LevelDoorAction)
	-- Doors quest table
	loadLuaMapAction(QuestDoorAction)
	loadLuaMapUnique(QuestDoorUnique)
	-- Item table
	loadLuaMapAction(ItemAction)
	loadLuaMapUnique(ItemUnique)
	-- Item daily reward table
	-- This is temporary disabled > loadLuaMapAction(DailyRewardAction)
	-- Item unmoveable table
	loadLuaMapAction(ItemUnmoveableAction)
	-- Lever table
	loadLuaMapAction(LeverAction)
	loadLuaMapUnique(LeverUnique)
	-- Teleport (magic forcefields) table
	loadLuaMapAction(TeleportAction)
	loadLuaMapUnique(TeleportUnique)
	-- Teleport item table
	loadLuaMapAction(TeleportItemAction)
	loadLuaMapUnique(TeleportItemUnique)
	-- Tile table
	loadLuaMapAction(TileAction)
	loadLuaMapUnique(TileUnique)
	-- Tile pick table
	loadLuaMapAction(TilePickAction)
	-- Create new item on map
	CreateMapItem(CreateItemOnMap)
	-- Update old quest storage keys
	updateKeysStorage(QuestKeysUpdate)

	logger.debug("Loaded all actions in the map")
	logger.debug("Loaded all uniques in the map")

	for i = 1, #startupGlobalStorages do
		Game.setStorageValue(startupGlobalStorages[i], 0)
	end

	local time = os.time()
	db.asyncQuery('TRUNCATE TABLE `players_online`')

	local resetSessionsOnStartup = configManager.getBoolean(configKeys.RESET_SESSIONS_ON_STARTUP)
	if AUTH_TYPE == "session" then
		if resetSessionsOnStartup then
			db.query('TRUNCATE TABLE `account_sessions`')
		else
			db.query('DELETE FROM `account_sessions` WHERE `expires` <= ' .. time)
		end
	end

	-- reset Daily Reward status
	db.query('UPDATE `players` SET `isreward` = ' .. DAILY_REWARD_NOTCOLLECTED)

	-- reset storages and allow purchase of boost in the store
	db.query('UPDATE `player_storage` SET `value` = 0 WHERE `player_storage`.`key` = 51052')

	-- reset familiars message storage
	db.query('DELETE FROM `player_storage` WHERE `key` = ' .. Global.Storage.FamiliarSummonEvent10)
	db.query('DELETE FROM `player_storage` WHERE `key` = ' .. Global.Storage.FamiliarSummonEvent60)

	-- delete canceled and rejected guilds
	db.asyncQuery('DELETE FROM `guild_wars` WHERE `status` = 2')
	db.asyncQuery('DELETE FROM `guild_wars` WHERE `status` = 3')

	-- Delete guilds that are pending for 3 days
	db.asyncQuery('DELETE FROM `guild_wars` WHERE `status` = 0 AND (`started` + 72 * 60 * 60) <= ' .. os.time())

	db.asyncQuery('DELETE FROM `players` WHERE `deletion` != 0 AND `deletion` < ' .. time)
	db.asyncQuery('DELETE FROM `ip_bans` WHERE `expires_at` != 0 AND `expires_at` <= ' .. time)
	db.asyncQuery('DELETE FROM `market_history` WHERE `inserted` <= \z
	' .. (time - configManager.getNumber(configKeys.MARKET_OFFER_DURATION)))

	-- Move expired bans to ban history
	local banResultId = db.storeQuery('SELECT * FROM `account_bans` WHERE `expires_at` != 0 AND `expires_at` <= ' .. time)
	if banResultId ~= false then
		repeat
			local accountId = Result.getNumber(banResultId, 'account_id')
			db.asyncQuery('INSERT INTO `account_ban_history` (`account_id`, `reason`, `banned_at`, \z
			`expired_at`, `banned_by`) VALUES (' .. accountId .. ', \z
			' .. db.escapeString(Result.getString(banResultId, 'reason')) .. ', \z
			' .. Result.getNumber(banResultId, 'banned_at') .. ', ' .. Result.getNumber(banResultId, 'expires_at') .. ', \z
			' .. Result.getNumber(banResultId, 'banned_by') .. ')')
			db.asyncQuery('DELETE FROM `account_bans` WHERE `account_id` = ' .. accountId)
		until not Result.next(banResultId)
		Result.free(banResultId)
	end

	-- Ferumbras Ascendant quest
	for i = 1, #GlobalStorage.FerumbrasAscendant.Habitats do
		local storage = GlobalStorage.FerumbrasAscendant.Habitats[i]
		Game.setStorageValue(storage, 0)
	end

	-- Check house auctions
	local resultId = db.storeQuery('SELECT `id`, `highest_bidder`, `last_bid`, (SELECT `balance` FROM \z
	`players` WHERE `players`.`id` = `highest_bidder`) AS `balance` FROM `houses` WHERE `owner` = 0 AND \z
	`bid_end` != 0 AND `bid_end` < ' .. time)
	if resultId ~= false then
		repeat
			local house = House(Result.getNumber(resultId, 'id'))
			if house then
				local highestBidder = Result.getNumber(resultId, 'highest_bidder')
				local balance = Result.getNumber(resultId, 'balance')
				local lastBid = Result.getNumber(resultId, 'last_bid')
				if balance >= lastBid then
					db.query('UPDATE `players` SET `balance` = ' .. (balance - lastBid) .. ' WHERE `id` = ' .. highestBidder)
					house:setOwnerGuid(highestBidder)
				end
				db.asyncQuery('UPDATE `houses` SET `last_bid` = 0, `bid_end` = 0, `highest_bidder` = 0, \z
				`bid` = 0 WHERE `id` = ' .. house:getId())
			end
		until not Result.next(resultId)
		Result.free(resultId)
	end

	-- store towns in database
	db.query("TRUNCATE TABLE `towns`")
	for i, town in ipairs(Game.getTowns()) do
		local position = town:getTemplePosition()
		db.query("INSERT INTO `towns` (`id`, `name`, `posx`, `posy`, `posz`) VALUES (" .. town:getId() .. ", " .. db.escapeString(town:getName()) .. ", " .. position.x .. ", " .. position.y .. ", " .. position.z .. ")")
	end

	do
		-- Event Schedule rates
		local lootRate = EventsScheduler.getEventSLoot()
		if lootRate ~= 100 then
			SCHEDULE_LOOT_RATE = lootRate
		end

		local expRate = EventsScheduler.getEventSExp()
		if expRate ~= 100 then
			SCHEDULE_EXP_RATE = expRate
		end

		local skillRate = EventsScheduler.getEventSSkill()
		if skillRate ~= 100 then
			SCHEDULE_SKILL_RATE = skillRate
		end

		local spawnRate = EventsScheduler.getSpawnMonsterSchedule()
		if spawnRate ~= 100 then
			SCHEDULE_SPAWN_RATE = spawnRate
		end

		if expRate ~= 100 or lootRate ~= 100 or spawnRate ~= 100 or skillRate ~= 100 then
			logger.info("[Events] Exp: {}%, loot: {}%, Spawn: {}%, Skill: {}%", expRate, lootRate, spawnRate, skillRate)
		end
	end

	-- Hireling System
	HirelingsInit()
end

serverstartup:register()
