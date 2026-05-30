USE warnet;

CREATE TABLE IF NOT EXISTS game_items (
  item_id INT UNSIGNED NOT NULL,
  item_type TINYINT UNSIGNED NOT NULL DEFAULT 3,
  category TINYINT UNSIGNED NOT NULL DEFAULT 4,
  is_enabled TINYINT(1) NOT NULL DEFAULT 1,

  name VARCHAR(128) NOT NULL DEFAULT '',
  fname VARCHAR(128) NOT NULL DEFAULT '',
  description TEXT NULL,

  level_required INT NOT NULL DEFAULT 0,

  weight DECIMAL(10,3) NOT NULL DEFAULT 0,
  damage_perc DECIMAL(10,3) NOT NULL DEFAULT 0,
  damage_max DECIMAL(10,3) NOT NULL DEFAULT 0,

  damage DECIMAL(10,3) NOT NULL DEFAULT 0,
  damage_decay DECIMAL(10,3) NOT NULL DEFAULT 0,
  num_clips INT NOT NULL DEFAULT 0,
  clip_size INT NOT NULL DEFAULT 0,
  rate_of_fire DECIMAL(10,3) NOT NULL DEFAULT 60,
  spread DECIMAL(10,3) NOT NULL DEFAULT 0,
  recoil DECIMAL(10,3) NOT NULL DEFAULT 0,

  add_gp INT NOT NULL DEFAULT 0,
  add_sp INT NOT NULL DEFAULT 0,

  item1_id INT UNSIGNED NOT NULL DEFAULT 0,
  item1_exp INT NOT NULL DEFAULT 0,
  item2_id INT UNSIGNED NOT NULL DEFAULT 0,
  item2_exp INT NOT NULL DEFAULT 0,
  item3_id INT UNSIGNED NOT NULL DEFAULT 0,
  item3_exp INT NOT NULL DEFAULT 0,
  item4_id INT UNSIGNED NOT NULL DEFAULT 0,
  item4_exp INT NOT NULL DEFAULT 0,
  item5_id INT UNSIGNED NOT NULL DEFAULT 0,
  item5_exp INT NOT NULL DEFAULT 0,
  item6_id INT UNSIGNED NOT NULL DEFAULT 0,
  item6_exp INT NOT NULL DEFAULT 0,

  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

  PRIMARY KEY (item_id),
  KEY ix_game_items_type_category (item_type, category),
  KEY ix_game_items_enabled (is_enabled)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS shop_prices (
  item_id INT UNSIGNED NOT NULL,
  is_enabled TINYINT(1) NOT NULL DEFAULT 1,

  price1 INT UNSIGNED NOT NULL DEFAULT 0,
  price7 INT UNSIGNED NOT NULL DEFAULT 0,
  price30 INT UNSIGNED NOT NULL DEFAULT 0,
  price_perm INT UNSIGNED NOT NULL DEFAULT 0,

  gprice1 INT UNSIGNED NOT NULL DEFAULT 0,
  gprice7 INT UNSIGNED NOT NULL DEFAULT 0,
  gprice30 INT UNSIGNED NOT NULL DEFAULT 0,
  gprice_perm INT UNSIGNED NOT NULL DEFAULT 0,

  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,

  PRIMARY KEY (item_id),
  CONSTRAINT fk_shop_prices_game_items
    FOREIGN KEY (item_id)
    REFERENCES game_items(item_id)
    ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS shop_skill_prices (
  skill_id SMALLINT UNSIGNED NOT NULL,
  is_enabled TINYINT(1) NOT NULL DEFAULT 1,
  lv1 TINYINT UNSIGNED NOT NULL DEFAULT 0,
  lv2 TINYINT UNSIGNED NOT NULL DEFAULT 0,
  lv3 TINYINT UNSIGNED NOT NULL DEFAULT 0,
  lv4 TINYINT UNSIGNED NOT NULL DEFAULT 0,
  lv5 TINYINT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (skill_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO game_items
  (item_id, item_type, category, is_enabled, name, fname, description)
VALUES
  (301142, 3, 1, 1, 'Unlock Loadout Slot', 'unlock_loadout', 'Unlocks one loadout slot.'),
  (301143, 3, 1, 1, 'Create Clan', 'create_clan', 'Allows creating a clan.'),
  (301150, 3, 1, 1, 'Reset Loadout Slot', 'reset_loadout', 'Resets one loadout slot.'),
  (301144, 3, 1, 1, 'Clan Members +5', 'clan_members_5', 'Adds clan member slots.'),
  (301145, 3, 1, 1, 'Clan Members +10', 'clan_members_10', 'Adds clan member slots.'),
  (301146, 3, 1, 1, 'Clan Members +15', 'clan_members_15', 'Adds clan member slots.'),
  (301147, 3, 1, 1, 'Clan Members +20', 'clan_members_20', 'Adds clan member slots.'),
  (301148, 3, 1, 1, 'Clan Members +25', 'clan_members_25', 'Adds clan member slots.'),
  (301149, 3, 1, 1, 'Clan Members +30', 'clan_members_30', 'Adds clan member slots.')
ON DUPLICATE KEY UPDATE
  item_type = VALUES(item_type),
  category = VALUES(category),
  is_enabled = VALUES(is_enabled),
  name = VALUES(name),
  fname = VALUES(fname),
  description = VALUES(description);

INSERT INTO shop_prices
  (item_id, is_enabled, price1, price7, price30, price_perm, gprice1, gprice7, gprice30, gprice_perm)
VALUES
  (301142, 1, 0, 0, 0, 2500, 0, 0, 0, 0),
  (301143, 1, 0, 0, 0, 10000, 0, 0, 0, 0),
  (301150, 1, 0, 0, 0, 1000, 0, 0, 0, 0),
  (301144, 1, 5, 0, 0, 1500, 0, 0, 0, 0),
  (301145, 1, 10, 0, 0, 2500, 0, 0, 0, 0),
  (301146, 1, 15, 0, 0, 3500, 0, 0, 0, 0),
  (301147, 1, 20, 0, 0, 4500, 0, 0, 0, 0),
  (301148, 1, 25, 0, 0, 5500, 0, 0, 0, 0),
  (301149, 1, 30, 0, 0, 6500, 0, 0, 0, 0)
ON DUPLICATE KEY UPDATE
  is_enabled = VALUES(is_enabled),
  price1 = VALUES(price1),
  price7 = VALUES(price7),
  price30 = VALUES(price30),
  price_perm = VALUES(price_perm),
  gprice1 = VALUES(gprice1),
  gprice7 = VALUES(gprice7),
  gprice30 = VALUES(gprice30),
  gprice_perm = VALUES(gprice_perm);