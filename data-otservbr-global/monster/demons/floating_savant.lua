local mType = Game.createMonsterType("Floating Savant")
local monster = {}

monster.description = "a floating savant"
monster.experience = 8000
monster.outfit = {
	lookType = 1063,
	lookHead = 113,
	lookBody = 94,
	lookLegs = 78,
	lookFeet = 78,
	lookAddons = 0,
	lookMount = 0
}

monster.raceId = 1637
monster.Bestiary = {
	class = "Demon",
	race = BESTY_RACE_DEMON,
	toKill = 2500,
	FirstUnlock = 100,
	SecondUnlock = 1000,
	CharmsPoints = 50,
	Stars = 4,
	Occurrence = 0,
	Locations = "The Extension Site"
}

monster.health = 8000
monster.maxHealth = 8000
monster.race = "undead"
monster.corpse = 28598
monster.speed = 165
monster.manaCost = 0

monster.changeTarget = {
	interval = 5000,
	chance = 8
}

monster.strategiesTarget = {
	nearest = 100,
}

monster.flags = {
	summonable = false,
	attackable = true,
	hostile = true,
	convinceable = false,
	pushable = false,
	rewardBoss = false,
	illusionable = false,
	canPushItems = true,
	canPushCreatures = true,
	staticAttackChance = 90,
	targetDistance = 1,
	runHealth = 0,
	healthHidden = false,
	isBlockable = false,
	canWalkOnEnergy = false,
	canWalkOnFire = true,
	canWalkOnPoison = false,
}

monster.light = {
	level = 0,
	color = 0,
}

monster.summon = {
	maxSummons = 1,
	summons = {
		{ name = "Lava Lurker Attendant", chance = 70, interval = 2000, count = 1 }
	}
}

monster.voices = {
	interval = 5000,
	chance = 10,
	{ text = "tssooosh tsoooosh tssoooosh!", yell = false },
	{ text = "We didn't stop the fire!", yell = false },
}

monster.loot = {
	{ name = "small enchanted ruby", chance = 10000, maxCount = 5 },
	{ name = "red crystal fragment", chance = 10000, maxCount = 2 },
	{ id = 3039, chance = 10000, maxCount = 3 }, -- red gem
	{ name = "red piece of cloth", chance = 10000, maxCount = 3 },
	{ name = "flask of demonic blood", chance = 12000, maxCount = 5 },
	{ name = "demonic essence", chance = 10000, maxCount = 5 },
	{ id = 3456, chance = 10000 }, -- pick
	{ name = "sample of monster blood", chance = 10000, maxCount = 5 }
}

monster.attacks = {
	{ name = "melee", interval = 2000, chance = 100, minDamage = -100, maxDamage = -200 },
	{ name = "combat", interval = 2000, chance = 15, type = COMBAT_FIREDAMAGE, minDamage = -390, maxDamage = -480, range = 7, shootEffect = CONST_ANI_FIRE, target = false },
	{ name = "combat", interval = 2000, chance = 14, type = COMBAT_ENERGYDAMAGE, minDamage = -390, maxDamage = -480, range = 7, shootEffect = CONST_ANI_ENERGY, target = false },
	{ name = "combat", interval = 2000, chance = 12, type = COMBAT_FIREDAMAGE, minDamage = -300, maxDamage = -400, radius = 3, effect = CONST_ME_EXPLOSIONAREA, target = false },
	{ name = "combat", interval = 2000, chance = 10, type = COMBAT_FIREDAMAGE, minDamage = -490, maxDamage = -630, length = 4, spread = 3, effect = CONST_ME_HITBYFIRE, target = false }
}

monster.defenses = {
	defense = 40,
	armor = 74,
	mitigation = 1.96,
}

monster.elements = {
	{ type = COMBAT_PHYSICALDAMAGE, percent = 0 },
	{ type = COMBAT_ENERGYDAMAGE, percent = 0 },
	{ type = COMBAT_EARTHDAMAGE, percent = 0 },
	{ type = COMBAT_FIREDAMAGE, percent = 100 },
	{ type = COMBAT_LIFEDRAIN, percent = 0 },
	{ type = COMBAT_MANADRAIN, percent = 0 },
	{ type = COMBAT_DROWNDAMAGE, percent = 0 },
	{ type = COMBAT_ICEDAMAGE, percent = -10 },
	{ type = COMBAT_HOLYDAMAGE, percent = 0 },
	{ type = COMBAT_DEATHDAMAGE, percent = 50 },
}

monster.immunities = {
	{ type = "paralyze", condition = true },
	{ type = "outfit", condition = false },
	{ type = "invisible", condition = true },
	{ type = "bleed", condition = false }
}

mType:register(monster)
