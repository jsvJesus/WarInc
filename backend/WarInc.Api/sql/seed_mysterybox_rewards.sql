START TRANSACTION;

DELETE FROM datagamerewards
WHERE RewardID IN (
    1, 2, 3, 4, 5, 6, 7,
    14, 15, 16, 17, 18,
    100, 101, 102, 103, 104,
    120, 121, 122, 123, 124,
    140, 141, 142, 143,
    200, 201, 202, 203,
    300, 301, 302, 303, 304, 305, 306, 307
);

INSERT INTO datagamerewards
(
    RewardID,
    RewardName,
    GD_CQ,
    HP_CQ,
    GD_DM,
    HP_DM,
    GD_SB,
    HP_SB
)
VALUES
(1, 'RWD_Kill', 2, 10, 5, 10, 10, 10),
(2, 'RWD_Headshot', 10, 20, 10, 20, 10, 10),
(3, 'RWD_AssistedKill', 1, 10, 1, 5, 10, 10),
(4, 'RWD_ExplosionKill', 100, 50, 10, 10, 10, 10),
(5, 'RWD_FriendlyKill', 0, -20, 0, 0, 0, -20),
(6, 'RWD_AvengeKill', 10, 100, 10, 10, 10, 10),
(7, 'RWD_RevengeKill', 20, 250, 20, 15, 10, 10),

(14, 'RWD_Killstreak2', 100, 50, 10, 10, 0, 0),
(15, 'RWD_Killstreak3', 150, 100, 20, 20, 0, 0),
(16, 'RWD_Killstreak4', 200, 125, 50, 35, 0, 0),
(17, 'RWD_Killstreak5', 500, 150, 100, 50, 0, 0),
(18, 'RWD_Killstreak6', 1000, 200, 250, 75, 0, 0),

(100, 'RWD_CaptureNeutral', 100, 250, 0, 0, 0, 0),
(101, 'RWD_CaptureEnemy', 200, 500, 0, 0, 0, 0),
(102, 'RWD_KillCloseToYourFlag', 50, 50, 0, 0, 0, 0),
(103, 'RWD_KillCloseToTheirFlag', 50, 20, 0, 0, 0, 0),
(104, 'RWD_KillAtSpawn', 0, 0, 0, 0, 0, 0),

(120, 'RWD_Bomb_PlantBomb', 0, 0, 0, 0, 10, 200),
(121, 'RWD_Bomb_DiffuseBomb', 0, 0, 0, 0, 20, 200),
(122, 'RWD_Bomb_WinRoundKills', 0, 0, 0, 0, 10, 10),
(123, 'RWD_Bomb_WinRound', 0, 0, 0, 0, 25, 50),
(124, 'RWD_Bomb_Pickup', 0, 0, 0, 0, 5, 0),

(140, 'RWD_Win', 500, 500, 500, 500, 500, 500),
(141, 'RWD_Loss', 200, 250, 200, 250, 200, 250),
(142, 'RWD_BestPlayerInTeam', 150, 250, 150, 250, 150, 250),
(143, 'RWD_BestPlayerOverall', 250, 500, 250, 500, 250, 500),

(200, 'RWD_BlackopResupply', 10, 10, 10, 10, 10, 10),
(201, 'RWD_UseUAV', 0, 50, 0, 50, 0, 50),
(202, 'RWD_UAVAssist', 50, 50, 50, 50, 50, 50),
(203, 'RWD_DestroyMine', 0, 20, 0, 20, 0, 20),

(300, 'RWD_FirstCaptureNeutral', 150, 100, 0, 0, 0, 0),
(301, 'RWD_FirstCaptureEnemy', 250, 200, 0, 0, 0, 0),
(302, 'RWD_Kill5NotDying', 25, 50, 25, 50, 25, 50),
(303, 'RWD_Kill10NotDying', 65, 100, 65, 100, 65, 100),
(304, 'RWD_Kill25NotDying', 150, 200, 150, 200, 150, 200),
(305, 'RWD_Kill50NotDying', 150, 200, 150, 200, 150, 200),
(306, 'RWD_KillGrenadeAfterDeath', 10, 100, 10, 100, 10, 100),
(307, 'RWD_KillEnemyWithHisGun', 50, 50, 50, 50, 50, 50);

DELETE FROM dataretentionbonuses
WHERE Day BETWEEN 1 AND 15;

INSERT INTO dataretentionbonuses
(
    Day,
    Bonus
)
VALUES
(1, 50),
(2, 75),
(3, 100),
(4, 125),
(5, 150),
(6, 175),
(7, 250),
(8, 300),
(9, 350),
(10, 400),
(11, 450),
(12, 500),
(13, 600),
(14, 750),
(15, 1000);

INSERT INTO items_packages
(
    ItemID,
    FNAME,
    Name,
    Description,
    Category,
    Price1,
    Price7,
    Price30,
    PriceP,
    IsNew,
    LevelRequired,
    GPrice1,
    GPrice7,
    GPrice30,
    GPriceP,
    IsEnabled,
    AddGP,
    AddSP,
    Item1_ID,
    Item1_Exp,
    Item2_ID,
    Item2_Exp,
    Item3_ID,
    Item3_Exp,
    Item4_ID,
    Item4_Exp,
    Item5_ID,
    Item5_Exp,
    Item6_ID,
    Item6_Exp
)
VALUES
(
    500900,
    'Mystery_Box_Rookie',
    'Rookie Mystery Box',
    'Contains starter combat gear, weapons, boosts or Game Dollars.',
    3,
    10,
    50,
    150,
    300,
    1,
    0,
    100,
    250,
    500,
    1000,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
),
(
    500901,
    'Mystery_Box_Combat',
    'Combat Mystery Box',
    'Contains combat weapons, armor, boosts or Game Dollars.',
    3,
    25,
    100,
    300,
    600,
    1,
    0,
    250,
    500,
    1000,
    2000,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
),
(
    500902,
    'Mystery_Box_Elite',
    'Elite Mystery Box',
    'Contains rare weapons, armor, premium items or Game Dollars.',
    3,
    50,
    200,
    600,
    1200,
    1,
    0,
    500,
    1000,
    2500,
    5000,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
)
ON DUPLICATE KEY UPDATE
    FNAME = VALUES(FNAME),
    Name = VALUES(Name),
    Description = VALUES(Description),
    Category = VALUES(Category),
    Price1 = VALUES(Price1),
    Price7 = VALUES(Price7),
    Price30 = VALUES(Price30),
    PriceP = VALUES(PriceP),
    IsNew = VALUES(IsNew),
    LevelRequired = VALUES(LevelRequired),
    GPrice1 = VALUES(GPrice1),
    GPrice7 = VALUES(GPrice7),
    GPrice30 = VALUES(GPrice30),
    GPriceP = VALUES(GPriceP),
    IsEnabled = VALUES(IsEnabled),
    AddGP = VALUES(AddGP),
    AddSP = VALUES(AddSP),
    Item1_ID = VALUES(Item1_ID),
    Item1_Exp = VALUES(Item1_Exp),
    Item2_ID = VALUES(Item2_ID),
    Item2_Exp = VALUES(Item2_Exp),
    Item3_ID = VALUES(Item3_ID),
    Item3_Exp = VALUES(Item3_Exp),
    Item4_ID = VALUES(Item4_ID),
    Item4_Exp = VALUES(Item4_Exp),
    Item5_ID = VALUES(Item5_ID),
    Item5_Exp = VALUES(Item5_Exp),
    Item6_ID = VALUES(Item6_ID),
    Item6_Exp = VALUES(Item6_Exp);

DELETE FROM items_lootdata
WHERE LootID IN (500900, 500901, 500902);

INSERT INTO items_lootdata
(
    LootID,
    Chance,
    ItemID,
    ExpDaysMin,
    ExpDaysMax,
    GDMin,
    GDMax,
    GDIfHave
)
VALUES
(500900, 30, 20000, 1, 3, 0, 0, 25),
(500900, 20, 20001, 1, 3, 0, 0, 25),
(500900, 15, 101033, 1, 3, 0, 0, 50),
(500900, 15, 301000, 1, 3, 0, 0, 25),
(500900, 20, 0, 0, 0, 100, 250, 0),

(500901, 20, 20016, 3, 7, 0, 0, 75),
(500901, 20, 20039, 3, 7, 0, 0, 75),
(500901, 20, 101033, 3, 7, 0, 0, 100),
(500901, 15, 101210, 3, 7, 0, 0, 100),
(500901, 10, 301004, 3, 7, 0, 0, 75),
(500901, 15, 0, 0, 0, 250, 500, 0),

(500902, 15, 20063, 7, 30, 0, 0, 150),
(500902, 15, 20064, 7, 30, 0, 0, 150),
(500902, 15, 20065, 7, 30, 0, 0, 150),
(500902, 15, 20066, 7, 30, 0, 0, 150),
(500902, 15, 101210, 7, 30, 0, 0, 250),
(500902, 10, 301004, 7, 30, 0, 0, 150),
(500902, 15, 0, 0, 0, 500, 1500, 0);

COMMIT;