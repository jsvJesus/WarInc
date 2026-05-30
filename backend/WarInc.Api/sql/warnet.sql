/*
Navicat MySQL Data Transfer

Source Server         : Home
Source Server Version : 110801
Source Host           : localhost:3307
Source Database       : warnet

Target Server Type    : MYSQL
Target Server Version : 110801
File Encoding         : 65001

Date: 2026-05-30 17:54:03
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `accountinfo`
-- ----------------------------
DROP TABLE IF EXISTS `accountinfo`;
CREATE TABLE `accountinfo` (
  `CustomerID` int(10) NOT NULL,
  `email` varchar(100) NOT NULL,
  `firstname` varchar(16) NOT NULL,
  `lastname` varchar(16) NOT NULL,
  `sex` int(10) NOT NULL,
  `dob` datetime NOT NULL,
  `street` char(32) NOT NULL,
  `city` char(16) NOT NULL,
  `state` char(2) NOT NULL,
  `postalcode` varchar(16) DEFAULT NULL,
  `Country` char(2) NOT NULL,
  `phone` char(14) NOT NULL,
  `OptOut1` int(10) NOT NULL,
  `admin_note` varchar(255) DEFAULT NULL,
  `NumReferrals` int(10) DEFAULT NULL,
  `GeoCode` char(2) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of accountinfo
-- ----------------------------

-- ----------------------------
-- Table structure for `accountpwdreset`
-- ----------------------------
DROP TABLE IF EXISTS `accountpwdreset`;
CREATE TABLE `accountpwdreset` (
  `RequestID` int(10) NOT NULL AUTO_INCREMENT,
  `RequestTime` datetime NOT NULL,
  `IP` varchar(64) NOT NULL,
  `token` varchar(32) NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `email` varchar(128) NOT NULL,
  PRIMARY KEY (`RequestID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of accountpwdreset
-- ----------------------------

-- ----------------------------
-- Table structure for `achievements`
-- ----------------------------
DROP TABLE IF EXISTS `achievements`;
CREATE TABLE `achievements` (
  `CustomerID` int(10) NOT NULL,
  `AchID` int(10) NOT NULL,
  `Value` int(10) NOT NULL,
  `Unlocked` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of achievements
-- ----------------------------

-- ----------------------------
-- Table structure for `amazonmessages`
-- ----------------------------
DROP TABLE IF EXISTS `amazonmessages`;
CREATE TABLE `amazonmessages` (
  `MessageIdentity` int(10) NOT NULL AUTO_INCREMENT,
  `MessageId` varchar(128) NOT NULL,
  `MessageBody` varchar(256) DEFAULT NULL,
  `MessageTime` datetime DEFAULT NULL,
  PRIMARY KEY (`MessageIdentity`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of amazonmessages
-- ----------------------------

-- ----------------------------
-- Table structure for `amazonpurchases`
-- ----------------------------
DROP TABLE IF EXISTS `amazonpurchases`;
CREATE TABLE `amazonpurchases` (
  `PurchaseIdentity` int(10) NOT NULL AUTO_INCREMENT,
  `PurchaseID` varchar(128) NOT NULL,
  `CustomerID` int(10) DEFAULT NULL,
  `PurchaseTime` datetime DEFAULT NULL,
  `SKU` varchar(32) DEFAULT NULL,
  `IsRevoked` int(10) DEFAULT NULL,
  PRIMARY KEY (`PurchaseIdentity`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of amazonpurchases
-- ----------------------------

-- ----------------------------
-- Table structure for `amazonshop`
-- ----------------------------
DROP TABLE IF EXISTS `amazonshop`;
CREATE TABLE `amazonshop` (
  `SKU` varchar(255) NOT NULL,
  `IsEnabled` int(10) NOT NULL,
  `PriceUSD_For_Log` float NOT NULL,
  `ItemNote` varchar(255) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of amazonshop
-- ----------------------------

-- ----------------------------
-- Table structure for `cheatlog`
-- ----------------------------
DROP TABLE IF EXISTS `cheatlog`;
CREATE TABLE `cheatlog` (
  `ID` int(10) NOT NULL AUTO_INCREMENT,
  `SessionID` bigint(19) NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `CheatID` int(10) NOT NULL,
  `ReportTime` datetime NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of cheatlog
-- ----------------------------

-- ----------------------------
-- Table structure for `clanapplications`
-- ----------------------------
DROP TABLE IF EXISTS `clanapplications`;
CREATE TABLE `clanapplications` (
  `ClanApplicationID` int(10) NOT NULL AUTO_INCREMENT,
  `ClanID` int(10) NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `ExpireTime` datetime NOT NULL,
  `ApplicationText` varchar(500) NOT NULL,
  `IsProcessed` int(10) NOT NULL,
  PRIMARY KEY (`ClanApplicationID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of clanapplications
-- ----------------------------

-- ----------------------------
-- Table structure for `clandata`
-- ----------------------------
DROP TABLE IF EXISTS `clandata`;
CREATE TABLE `clandata` (
  `ClanID` int(10) NOT NULL AUTO_INCREMENT,
  `ClanName` varchar(64) NOT NULL,
  `ClanNameColor` int(10) NOT NULL,
  `ClanTag` varchar(4) NOT NULL,
  `ClanTagColor` int(10) NOT NULL,
  `ClanEmblemID` int(10) NOT NULL,
  `ClanEmblemColor` int(10) NOT NULL,
  `ClanXP` int(10) NOT NULL,
  `ClanLevel` int(10) NOT NULL,
  `ClanGP` int(10) NOT NULL,
  `OwnerID` int(10) NOT NULL,
  `MaxClanMembers` int(10) NOT NULL,
  `NumClanMembers` int(10) NOT NULL,
  `ClanLore` varchar(512) DEFAULT NULL,
  `Announcement` varchar(512) DEFAULT NULL,
  `ClanCreateDate` datetime DEFAULT NULL,
  PRIMARY KEY (`ClanID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of clandata
-- ----------------------------

-- ----------------------------
-- Table structure for `clanevents`
-- ----------------------------
DROP TABLE IF EXISTS `clanevents`;
CREATE TABLE `clanevents` (
  `ClanEventID` int(10) NOT NULL AUTO_INCREMENT,
  `ClanID` int(10) NOT NULL,
  `EventDate` datetime NOT NULL,
  `EventType` int(10) NOT NULL,
  `EventRank` int(10) NOT NULL,
  `Var1` int(10) NOT NULL,
  `Var2` int(10) NOT NULL,
  `Var3` int(10) NOT NULL,
  `Var4` int(10) NOT NULL,
  `Text1` varchar(64) DEFAULT NULL,
  `Text2` varchar(64) DEFAULT NULL,
  `Text3` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`ClanEventID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of clanevents
-- ----------------------------

-- ----------------------------
-- Table structure for `claninvites`
-- ----------------------------
DROP TABLE IF EXISTS `claninvites`;
CREATE TABLE `claninvites` (
  `ClanInviteID` int(10) NOT NULL AUTO_INCREMENT,
  `ClanID` int(10) NOT NULL,
  `InviterID` int(10) NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `ExpireTime` datetime NOT NULL,
  PRIMARY KEY (`ClanInviteID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of claninvites
-- ----------------------------

-- ----------------------------
-- Table structure for `coupons2`
-- ----------------------------
DROP TABLE IF EXISTS `coupons2`;
CREATE TABLE `coupons2` (
  `CouponCode` varchar(32) NOT NULL,
  `IsUsed` int(10) DEFAULT NULL,
  `CustomerID` int(10) DEFAULT NULL,
  `Team` int(10) DEFAULT NULL,
  `MultiUse` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of coupons2
-- ----------------------------

-- ----------------------------
-- Table structure for `coupons2customermap`
-- ----------------------------
DROP TABLE IF EXISTS `coupons2customermap`;
CREATE TABLE `coupons2customermap` (
  `CouponID` int(10) NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `UseTime` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of coupons2customermap
-- ----------------------------

-- ----------------------------
-- Table structure for `dataabilityprice`
-- ----------------------------
DROP TABLE IF EXISTS `dataabilityprice`;
CREATE TABLE `dataabilityprice` (
  `Lv1` int(10) NOT NULL,
  `Lv2` int(10) NOT NULL,
  `Lv3` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dataabilityprice
-- ----------------------------

-- ----------------------------
-- Table structure for `dataclanxp`
-- ----------------------------
DROP TABLE IF EXISTS `dataclanxp`;
CREATE TABLE `dataclanxp` (
  `ClanLevel` int(10) NOT NULL,
  `ClanXP` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dataclanxp
-- ----------------------------

-- ----------------------------
-- Table structure for `datacountrycodes`
-- ----------------------------
DROP TABLE IF EXISTS `datacountrycodes`;
CREATE TABLE `datacountrycodes` (
  `CountryCode` char(2) NOT NULL,
  `CountryName` varchar(256) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of datacountrycodes
-- ----------------------------

-- ----------------------------
-- Table structure for `datagamerewards`
-- ----------------------------
DROP TABLE IF EXISTS `datagamerewards`;
CREATE TABLE `datagamerewards` (
  `RewardID` int(10) NOT NULL,
  `RewardName` varchar(128) NOT NULL,
  `GD_CQ` int(10) NOT NULL,
  `HP_CQ` int(10) NOT NULL,
  `GD_DM` int(10) NOT NULL,
  `HP_DM` int(10) NOT NULL,
  `GD_SB` int(10) NOT NULL,
  `HP_SB` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of datagamerewards
-- ----------------------------

-- ----------------------------
-- Table structure for `datarankpoints`
-- ----------------------------
DROP TABLE IF EXISTS `datarankpoints`;
CREATE TABLE `datarankpoints` (
  `Rank` int(10) NOT NULL,
  `HonorPoints` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of datarankpoints
-- ----------------------------

-- ----------------------------
-- Table structure for `dataretentionbonuses`
-- ----------------------------
DROP TABLE IF EXISTS `dataretentionbonuses`;
CREATE TABLE `dataretentionbonuses` (
  `Day` int(10) NOT NULL,
  `Bonus` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dataretentionbonuses
-- ----------------------------

-- ----------------------------
-- Table structure for `dataskill2price`
-- ----------------------------
DROP TABLE IF EXISTS `dataskill2price`;
CREATE TABLE `dataskill2price` (
  `SkillID` int(10) NOT NULL,
  `SkillName` varchar(64) DEFAULT NULL,
  `Lv1` int(10) DEFAULT NULL,
  `Lv2` int(10) DEFAULT NULL,
  `Lv3` int(10) DEFAULT NULL,
  `Lv4` int(10) DEFAULT NULL,
  `Lv5` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dataskill2price
-- ----------------------------

-- ----------------------------
-- Table structure for `dataskillprice`
-- ----------------------------
DROP TABLE IF EXISTS `dataskillprice`;
CREATE TABLE `dataskillprice` (
  `Lv1` int(10) NOT NULL,
  `Lv2` int(10) NOT NULL,
  `Lv3` int(10) NOT NULL,
  `Lv4` int(10) NOT NULL,
  `Lv5` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dataskillprice
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_gptransactions`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_gptransactions`;
CREATE TABLE `dbg_gptransactions` (
  `TransactionID` int(10) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) DEFAULT NULL,
  `TransactionTime` datetime DEFAULT NULL,
  `Amount` int(10) DEFAULT NULL,
  `Reason` varchar(64) DEFAULT NULL,
  `Previous` int(10) DEFAULT NULL,
  PRIMARY KEY (`TransactionID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_gptransactions
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_hwinfo`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_hwinfo`;
CREATE TABLE `dbg_hwinfo` (
  `ComputerID` bigint(19) NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `ReportDate` datetime NOT NULL,
  `CPUName` varchar(32) NOT NULL,
  `CPUBrand` varchar(64) NOT NULL,
  `CPUFreq` int(10) NOT NULL,
  `TotalMemory` int(10) NOT NULL,
  `DisplayW` int(10) NOT NULL,
  `DisplayH` int(10) NOT NULL,
  `gfxErrors` int(10) NOT NULL,
  `gfxVendorId` int(10) NOT NULL,
  `gfxDeviceId` int(10) NOT NULL,
  `gfxDescription` varchar(128) NOT NULL,
  `OSVersion` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_hwinfo
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_levelupevents`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_levelupevents`;
CREATE TABLE `dbg_levelupevents` (
  `CustomerID` int(10) DEFAULT NULL,
  `LevelGained` int(10) DEFAULT NULL,
  `ReportTime` datetime DEFAULT NULL,
  `SessionID` bigint(19) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_levelupevents
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_lootrewards`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_lootrewards`;
CREATE TABLE `dbg_lootrewards` (
  `RecordID` int(10) NOT NULL AUTO_INCREMENT,
  `ReportTime` datetime NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `Roll` float NOT NULL,
  `LootID` float NOT NULL,
  `ItemID` int(10) NOT NULL,
  `ExpDays` int(10) NOT NULL,
  `GD` int(10) NOT NULL,
  PRIMARY KEY (`RecordID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_lootrewards
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_referredevents`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_referredevents`;
CREATE TABLE `dbg_referredevents` (
  `CustomerID` int(10) NOT NULL,
  `InvitedID` int(10) NOT NULL,
  `LevelUpTime` datetime NOT NULL,
  `NumReferrers` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_referredevents
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_refpixellog`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_refpixellog`;
CREATE TABLE `dbg_refpixellog` (
  `CustomerID` int(10) NOT NULL,
  `ReferralID` int(10) NOT NULL,
  `PixelUrl` varchar(128) NOT NULL,
  `PixelCallTime` datetime NOT NULL,
  `ErrorMsg` varchar(128) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_refpixellog
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_smscallbacks`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_smscallbacks`;
CREATE TABLE `dbg_smscallbacks` (
  `EventID` int(10) NOT NULL AUTO_INCREMENT,
  `data` varchar(4096) NOT NULL,
  PRIMARY KEY (`EventID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_smscallbacks
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_sqlhacks`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_sqlhacks`;
CREATE TABLE `dbg_sqlhacks` (
  `ID` int(10) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) NOT NULL,
  `IP` varchar(32) NOT NULL,
  `Query` varchar(1024) NOT NULL,
  `StoredProc` varchar(32) NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_sqlhacks
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_srvloginfo`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_srvloginfo`;
CREATE TABLE `dbg_srvloginfo` (
  `RecordID` int(10) NOT NULL AUTO_INCREMENT,
  `ReportTime` datetime DEFAULT NULL,
  `IsProcessed` int(10) DEFAULT NULL,
  `CustomerID` int(10) DEFAULT NULL,
  `CustomerIP` varchar(64) DEFAULT NULL,
  `GameSessionID` bigint(19) DEFAULT NULL,
  `CheatID` int(10) DEFAULT NULL,
  `RepeatCount` int(10) DEFAULT NULL,
  `Msg` varchar(512) DEFAULT NULL,
  `Data` varchar(4096) DEFAULT NULL,
  PRIMARY KEY (`RecordID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_srvloginfo
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_userroundresults`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_userroundresults`;
CREATE TABLE `dbg_userroundresults` (
  `IP` varchar(32) NOT NULL,
  `GameSessionID` bigint(19) NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `GamePoints` int(10) NOT NULL,
  `HonorPoints` int(10) NOT NULL,
  `SkillPoints` int(10) NOT NULL,
  `Kills` int(10) NOT NULL,
  `Deaths` int(10) NOT NULL,
  `ShotsFired` int(10) NOT NULL,
  `ShotsHits` int(10) NOT NULL,
  `Headshots` int(10) NOT NULL,
  `AssistKills` int(10) NOT NULL,
  `Wins` int(10) NOT NULL,
  `Losses` int(10) NOT NULL,
  `CaptureNeutralPoints` int(10) NOT NULL,
  `CaptureEnemyPoints` int(10) NOT NULL,
  `TimePlayed` int(10) NOT NULL,
  `GameReportTime` datetime NOT NULL,
  `GameDollars` int(10) NOT NULL,
  `TeamID` int(10) NOT NULL,
  `MapID` int(10) NOT NULL,
  `MapType` int(10) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_userroundresults
-- ----------------------------

-- ----------------------------
-- Table structure for `dbg_woadminchanges`
-- ----------------------------
DROP TABLE IF EXISTS `dbg_woadminchanges`;
CREATE TABLE `dbg_woadminchanges` (
  `ChangeID` int(10) NOT NULL AUTO_INCREMENT,
  `ChangeTime` datetime DEFAULT NULL,
  `UserName` varchar(64) DEFAULT NULL,
  `Action` int(10) DEFAULT NULL,
  `Field` varchar(512) DEFAULT NULL,
  `RecordID` int(10) DEFAULT NULL,
  `OldValue` varchar(2048) DEFAULT NULL,
  `NewValue` varchar(2048) DEFAULT NULL,
  PRIMARY KEY (`ChangeID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of dbg_woadminchanges
-- ----------------------------

-- ----------------------------
-- Table structure for `financialblocks`
-- ----------------------------
DROP TABLE IF EXISTS `financialblocks`;
CREATE TABLE `financialblocks` (
  `CustomerID` int(10) NOT NULL,
  `LastBlockedTime` datetime NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of financialblocks
-- ----------------------------

-- ----------------------------
-- Table structure for `financialtransactions`
-- ----------------------------
DROP TABLE IF EXISTS `financialtransactions`;
CREATE TABLE `financialtransactions` (
  `CustomerID` int(10) NOT NULL,
  `TransactionID` varchar(64) NOT NULL,
  `TransactionType` int(10) NOT NULL,
  `DateTime` datetime NOT NULL,
  `Amount` float NOT NULL,
  `ResponseCode` varchar(64) NOT NULL,
  `AprovalCode` varchar(16) NOT NULL,
  `ItemID` varchar(32) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of financialtransactions
-- ----------------------------

-- ----------------------------
-- Table structure for `friendsmap`
-- ----------------------------
DROP TABLE IF EXISTS `friendsmap`;
CREATE TABLE `friendsmap` (
  `CustomerID` int(10) DEFAULT NULL,
  `FriendID` int(10) DEFAULT NULL,
  `FriendStatus` int(10) DEFAULT NULL,
  `DateAdded` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of friendsmap
-- ----------------------------

-- ----------------------------
-- Table structure for `gamersfirstuseridmap`
-- ----------------------------
DROP TABLE IF EXISTS `gamersfirstuseridmap`;
CREATE TABLE `gamersfirstuseridmap` (
  `CustomerID` int(10) NOT NULL,
  `GamersfirstID` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of gamersfirstuseridmap
-- ----------------------------

-- ----------------------------
-- Table structure for `inventory`
-- ----------------------------
DROP TABLE IF EXISTS `inventory`;
CREATE TABLE `inventory` (
  `CustomerID` int(10) NOT NULL,
  `ItemID` int(10) NOT NULL,
  `LeasedUntil` datetime NOT NULL,
  `Quantity` int(10) NOT NULL,
  `UpSlot1` int(10) NOT NULL,
  `UpSlot2` int(10) NOT NULL,
  `UpSlot3` int(10) NOT NULL,
  `UpSlot4` int(10) NOT NULL,
  `UpSlot5` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of inventory
-- ----------------------------

-- ----------------------------
-- Table structure for `inventory_fps`
-- ----------------------------
DROP TABLE IF EXISTS `inventory_fps`;
CREATE TABLE `inventory_fps` (
  `CustomerID` int(10) NOT NULL,
  `WeaponID` int(10) NOT NULL,
  `AttachmentID` int(10) NOT NULL,
  `LeasedUntil` datetime NOT NULL,
  `Slot` int(10) NOT NULL,
  `IsEquipped` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of inventory_fps
-- ----------------------------

-- ----------------------------
-- Table structure for `items_attachments`
-- ----------------------------
DROP TABLE IF EXISTS `items_attachments`;
CREATE TABLE `items_attachments` (
  `ItemID` int(10) NOT NULL AUTO_INCREMENT,
  `FNAME` varchar(32) NOT NULL,
  `Type` int(10) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `Description` varchar(256) NOT NULL,
  `MuzzleParticle` varchar(64) NOT NULL,
  `FireSound` varchar(256) NOT NULL,
  `Damage` float NOT NULL,
  `Ranges` float NOT NULL,
  `Firerate` float NOT NULL,
  `Recoil` float NOT NULL,
  `Spread` float NOT NULL,
  `Clipsize` int(10) NOT NULL,
  `ScopeMag` float NOT NULL,
  `ScopeType` varchar(32) NOT NULL,
  `AnimPrefix` varchar(32) NOT NULL,
  `SpecID` int(10) NOT NULL,
  `Category` int(10) NOT NULL,
  `Price1` int(10) NOT NULL,
  `Price7` int(10) NOT NULL,
  `Price30` int(10) NOT NULL,
  `PriceP` int(10) NOT NULL,
  `GPrice1` int(10) NOT NULL,
  `GPrice7` int(10) NOT NULL,
  `GPrice30` int(10) NOT NULL,
  `GPriceP` int(10) NOT NULL,
  `IsNew` int(10) NOT NULL,
  `LevelRequired` int(10) NOT NULL,
  PRIMARY KEY (`ItemID`)
) ENGINE=InnoDB AUTO_INCREMENT=400135 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of items_attachments
-- ----------------------------
INSERT INTO `items_attachments` VALUES ('400000', 'Grip_1', '3', 'Grip 1', 'The bottom rail grip will allow the shooter more control of the weapon by use of the attached grip.', '', '', '0', '0', '0', '0', '-5', '0', '0', '', 'Grip', '1001', '3', '0', '0', '0', '1', '0', '0', '0', '0', '0', '9');
INSERT INTO `items_attachments` VALUES ('400001', 'AK_30_rounds', '4', 'AK 30 rounds', 'This is a standard 30 rounds clip for the russian assault rifle.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4200', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400002', 'Advanced_Break', '0', 'Advanced Break', 'The Advanced Muzzle Break will lower recoil of the weapon and unwanted rising of the barrel during rapid fire.', '', '', '-5', '-5', '0', '-10', '0', '0', '0', '', '', '2001', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '5');
INSERT INTO `items_attachments` VALUES ('400003', 'ACOG', '1', 'ACOG', 'The Acog is a rail mounted telescopic sight used to extend a shooters accuracy on medium to long range shots.', '', '', '0', '0', '0', '0', '0', '0', '50', 'acog_fps', 'ASR_Scar', '5001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '20');
INSERT INTO `items_attachments` VALUES ('400004', 'laser', '2', 'laser', 'The Laser attachment mounts onto the side of a weapon and allows the shooter to mark the target with the laser.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '3001', '2', '0', '0', '0', '1', '0', '0', '0', '0', '0', '18');
INSERT INTO `items_attachments` VALUES ('400005', 'Holographic', '1', 'Holographic', 'The Holosight is a rail mounted holographic sight used to extend a shooters accuracy at close and medium ranges.', '', '', '0', '0', '0', '0', '0', '0', '35', 'eotech_fps', 'ASR_Scar', '5001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '15');
INSERT INTO `items_attachments` VALUES ('400006', 'SCAR_IS', '1', 'SCAR IS', 'This is the improved flip up iron sight system for the Scar assault rifle. It attachess onto the upper rail.', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', 'ASR_Scar', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400007', 'Kobra', '1', 'Kobra', 'The Kobra is a rail mounted red dot sight used to extend a shooters accuracy at close and medium range targets.', '', '', '0', '0', '0', '0', '0', '0', '20', 'kobra_red_dot_fps', 'ASR_Scar', '5001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '5');
INSERT INTO `items_attachments` VALUES ('400008', 'Tactical_Sniper_Scope', '1', 'Tactical Sniper Scope', '8x rail mounted scope designed to be used used atmedium ranges.', '', '', '0', '0', '0', '0', '0', '0', '90', 'psg1', '', '6001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400009', 'SMG_Grip_1', '3', 'SMG Grip 1', 'The M 7 grip provides a shooter with more control and accuracy when firing a weapon at a high rate of fire.', '', '', '0', '0', '0', '0', '-5', '0', '0', '', 'Grip', '1002', '3', '0', '0', '0', '1', '0', '0', '0', '0', '0', '21');
INSERT INTO `items_attachments` VALUES ('400010', 'M_60_rounds', '4', 'M 60 rounds', 'This is a extended magazine for the assault rifle that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '10', '30', '0', '', '', '4001', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '25');
INSERT INTO `items_attachments` VALUES ('400011', 'Compensator_', '0', 'Compensator ', 'The Muzzle Compensator will lower recoil of the weapon and unwanted rising of the barrel during rapid fire.', '', '', '-5', '-5', '0', '0', '-10', '0', '0', '', '', '2001', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '5');
INSERT INTO `items_attachments` VALUES ('400012', 'Flash_Hider', '0', 'Flash Hider', 'The Muzzle FlashHider will lower the visual flash of a weapon when fired at normal or rapid speeds.', 'muzzle_asr_noflash', '', '0', '-5', '0', '0', '0', '0', '0', '', '', '2001', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '15');
INSERT INTO `items_attachments` VALUES ('400013', 'Silencer_', '0', 'Silencer ', 'The Silencer will lower the sound of a weapons discharge when fired at normal or rapid speeds.', 'muzzle_asr_noflash', '', '-20', '-10', '0', '0', '0', '0', '0', '', '', '2001', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '21');
INSERT INTO `items_attachments` VALUES ('400014', 'standard_muzzle', '0', 'standard muzzle', 'The Standard Muzzle break will lower recoil of the weapon and unwanted rising of the barrel during rapid fire.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '2001', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400015', 'M_45_rounds', '4', 'M 45 rounds', 'This is a extended 45 rounds magazine for the assault rifle that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '0', '15', '0', '', '', '4001', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '15');
INSERT INTO `items_attachments` VALUES ('400016', 'M_30_rounds', '4', 'M 30 rounds', 'This is an standard 30 round magazine for the assault rifle that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4001', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400017', 'M_100_CMag', '4', 'M 100 C-Mag', 'This is a 100 round C Mag for assault riflse that allows the shooter to carry far more ammo than a normal clip.', '', '', '0', '0', '0', '0', '20', '70', '0', '', '', '4001', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '35');
INSERT INTO `items_attachments` VALUES ('400018', 'flashlight', '2', 'flashlight', 'The Flashlight can be used to light dark areas and temporarily blind a shooters opponent.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '3001', '2', '0', '0', '0', '1', '0', '0', '0', '0', '0', '14');
INSERT INTO `items_attachments` VALUES ('400019', 'Compact_Scope', '1', 'Compact Scope', 'The Swiss Compact Sight is a rail mounted scope used to extend a shooters accuracy at close and medium ranges.', '', '', '0', '0', '0', '0', '0', '0', '60', 'aw50', '', '5001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '40');
INSERT INTO `items_attachments` VALUES ('400020', 'Red_Dot_SP', '1', 'Red Dot SP', 'The Swiss Red Dot Sight is a rail mounted sight used to extend a shooters accuracy at close and medium range targets', '', '', '0', '0', '0', '0', '0', '0', '35', 'swiss_red_dot_fps', 'ASR_Scar', '5001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30');
INSERT INTO `items_attachments` VALUES ('400021', 'laser', '2', 'laser', 'The Pistol Laser is mounted on the left side of a handgun and used to target the enemy with a laser light.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '3002', '2', '0', '0', '0', '1', '0', '0', '0', '0', '0', '18');
INSERT INTO `items_attachments` VALUES ('400022', 'Flashlight', '2', 'Flashlight', 'The Pistol Flashlight can be used to light dark areas and temporarily blind a shooters opponent.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '3002', '2', '0', '0', '0', '1', '0', '0', '0', '0', '0', '14');
INSERT INTO `items_attachments` VALUES ('400023', 'Reflex_Sight', '1', 'Reflex Sight', 'The Reflex Sight is a rail mounted scope used to extend a shooters accuracy at medium and long ranges.', '', '', '0', '0', '0', '0', '0', '0', '20', 'reflex_fps', 'ASR_Scar', '5001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '5');
INSERT INTO `items_attachments` VALUES ('400024', 'M4_IS', '1', 'M4 IS', 'This is the improved flip up iron sight system for the M4 Blackwater. It attachess onto the upper rail.', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', 'ASR_Scar', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400025', 'SIG_556_IS', '1', 'SIG 556 IS', 'This is the improved flip up iron sight system for the SG 555. It attachess onto the upper rail.', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', 'ASR_Scar', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400026', 'MP7_IS', '1', 'MP7 IS', 'This is the improved flip up iron sight system for the Mp 7. It attachess onto the upper rail.', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', 'ASR_Scar', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400027', 'PSO1', '1', 'PSO-1', 'The PSO1 is a rail mounted telescopic sight used to extend a shooters accuracy on medium to long range shots.', '', '', '0', '0', '0', '0', '0', '0', '70', 'pso1', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400028', 'Famas_30_rounds', '4', 'Famas 30 rounds', 'This is a 30 round magazine for the Famas that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400029', 'SG_30_round', '4', 'SG 30 round', 'This is a 30 round magazine for the SG556 that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4100', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400030', 'VSS_20_rounds', '4', 'VSS 20 rounds', 'This is a 20 round magazine for the Vintorez that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '0', '10', '0', '', '', '4002', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '40');
INSERT INTO `items_attachments` VALUES ('400031', 'VSS_10_rounds', '4', 'VSS 10 rounds', 'This is a 10 round magazine for the SG556 that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4002', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400032', 'MP7_60rnd', '4', 'MP7 60rnd', 'This is a extended 60 round magazine for the MP7 that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '10', '30', '0', '', '', '4003', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '25');
INSERT INTO `items_attachments` VALUES ('400033', 'MP7_30rnd', '4', 'MP7 30rnd', 'This is a 30 round magazine for the MP7 that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4003', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400034', 'SIGP226_Standard', '4', 'SIGP226 Standard', 'This is a special purpose extended mag for the SIGP226 that allows the shooter to carry more ammo.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400035', 'M249_IS', '1', 'M249 IS', 'This is the improved flip up iron sight system for the M249 machine gun. It attachess onto the upper rail.', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400036', 'KT_IS', '1', 'KT IS', 'This is the improved flip up iron sight system for the Keltech shotgun that attaches on it\'s upper rail.', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400038', 'Blackwater_Long_Range', '1', 'Blackwater Long Range', 'The Blackwater Long Range scope is a 10x rail mounted scope designed to be used used  at long ranges.', '', '', '0', '0', '0', '0', '0', '0', '120', 'sv98', '', '6001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '45');
INSERT INTO `items_attachments` VALUES ('400039', 'Swiss_Arms_Scope_8x', '1', 'Swiss Arms Scope 8x', 'The Swiss Arms multirail scope is a rail mounted scope used to extend a shooters accuracy at medium and long ranges.', '', '', '0', '0', '0', '0', '0', '0', '70', 's50hs', '', '6001', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '25');
INSERT INTO `items_attachments` VALUES ('400040', 'Iron_AK74M', '1', 'Iron AK74M', 'This is the improved flip up iron sight system for the AK-74M n that attaches on it\'s upper rail.', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400041', 'Iron_Famas', '1', 'Iron Famas', 'This is the improved flip up iron sight system for the Famas that attaches on it\'s upper rail.', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400042', 'Iron_G36', '1', 'Iron G36', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400043', 'AW_10_rounds', '4', 'AW 10 rounds', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400044', 'PGO', '1', 'PGO', '', '', '', '0', '0', '0', '0', '0', '0', '40', 'rpg7', '', '5002', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '25');
INSERT INTO `items_attachments` VALUES ('400045', 'RPG7_IS', '1', 'RPG-7 IS', '', '', '', '0', '0', '0', '0', '0', '0', '10', 'default', '', '5002', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400046', 'P90_50_rounds', '4', 'P90 50 rounds', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400047', 'Bizon_64_rounds', '4', 'Bizon 64 rounds', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400048', 'SVD_10_rounds', '4', 'SVD 10 rounds', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4004', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400049', 'EVO3_30_rounds', '4', 'EVO-3 30 rounds', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400050', 'USS12_Drum', '4', 'USS-12 Drum', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400051', 'EVO3_IS', '1', 'EVO-3 IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400052', 'Bizon_IS', '1', 'Bizon IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400053', 'USS12_Is', '1', 'USS-12 Is', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400054', 'P90_IS', '1', 'P90 IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400055', 'Pecheneg_IS', '1', 'Pecheneg IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400056', 'PKM_IS', '1', 'PKM IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400057', 'RPK_80_rounds', '4', 'RPK 80 rounds', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4005', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400058', 'Iron_SIG516', '1', 'Iron SIG516', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400059', 'Iron_TAR21', '1', 'Iron TAR21', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400060', 'Iron_RPK', '1', 'Iron RPK', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400061', 'Iron_RPO', '1', 'Iron RPO', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400062', 'AN94_IS', '1', 'AN94 IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400063', 'QLB06_IS', '1', 'QLB06 IS', '', '', '', '0', '0', '0', '0', '0', '0', '10', 'default', '', '4007', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400064', 'L90_Is', '1', 'L90 Is', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400065', 'AT4_IS', '1', 'AT4 IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400066', 'M4_IS2', '1', 'M4 IS2', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400067', 'ATTMOpticAWScope01', '1', 'ATTM_Optic_AW_Scope_01', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400068', 'QLB06_Scope', '1', 'QLB06 Scope', '', '', '', '0', '0', '0', '0', '0', '0', '30', 'rpg7', '', '4007', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '25');
INSERT INTO `items_attachments` VALUES ('400069', 'AK_45_rounds', '4', 'AK 45 rounds', 'This is a extended 45 rounds clip for the russian assault rifle.', '', '', '0', '0', '0', '0', '0', '15', '0', '', '', '4200', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '15');
INSERT INTO `items_attachments` VALUES ('400070', 'AW', '4', 'AW', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400071', 'Jericho', '4', 'Jericho', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400072', 'QLB06_Drum', '4', 'QLB06 Drum', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400073', 'Saiga_Mag', '4', 'Saiga Mag', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400074', 'standard_muzzle', '0', 'standard muzzle', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400075', 'standard_muzzle', '0', 'standard muzzle', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400076', 'standard_muzzle', '0', 'standard muzzle', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400077', 'standard_muzzle', '0', 'standard muzzle', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400078', 'standard_muzzle', '0', 'standard muzzle', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400079', 'MP5_Mag', '4', 'MP5 Mag', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400080', 'SAIGA_IS', '1', 'SAIGA IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400081', 'XA5_IS', '1', 'XA5 IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400082', 'M590_Is', '1', 'M590 Is', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400083', 'Veresk_IS', '1', 'Veresk IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400084', 'Veresk_30_rounds', '4', 'Veresk 30 rounds', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4006', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400085', 'Veresk_45_rounds', '4', 'Veresk 45 rounds', '', '', '', '0', '0', '0', '0', '-10', '15', '0', '', '', '4006', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '20');
INSERT INTO `items_attachments` VALUES ('400086', 'DEagle_Mag', '4', 'DEagle Mag', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400087', 'FN57_Mag', '4', 'FN57 Mag', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400088', 'SMG_Grip_1', '3', 'SMG Grip 1', '', '', '', '0', '0', '0', '0', '-5', '0', '0', '', 'Grip', '1003', '3', '0', '0', '0', '1', '0', '0', '0', '0', '0', '21');
INSERT INTO `items_attachments` VALUES ('400089', 'Barrel_Upgrade_1', '7', 'Barrel Upgrade 1', 'Increases Range and Damage, also increases recoil and spread', '', '', '5', '0', '0', '10', '5', '0', '0', '', '', '10100', '7', '0', '0', '0', '1', '0', '0', '0', '0', '0', '10');
INSERT INTO `items_attachments` VALUES ('400090', 'Barrel_Upgrade_2', '7', 'Barrel Upgrade 2', 'Increases Range and Damage, also increases recoil and spread', '', '', '10', '0', '0', '15', '10', '0', '0', '', '', '10100', '7', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30');
INSERT INTO `items_attachments` VALUES ('400091', 'Barrel_Upgrade_3', '7', 'Barrel Upgrade 3', 'Increases Range and Damage, also increases recoil and spread', '', '', '15', '0', '0', '20', '20', '0', '0', '', '', '10100', '7', '0', '0', '0', '1', '0', '0', '0', '0', '0', '40');
INSERT INTO `items_attachments` VALUES ('400092', 'Barrel_Upgrade_4', '7', 'Barrel Upgrade 4', 'Increases Range and Damage, also increases recoil and spread', '', '', '20', '0', '0', '30', '30', '0', '0', '', '', '10100', '7', '0', '0', '0', '1', '0', '0', '0', '0', '0', '55');
INSERT INTO `items_attachments` VALUES ('400093', 'Premium_Barrel_1', '7', 'Premium Barrel 1', 'Premium Upgrade', '', '', '5', '5', '0', '5', '5', '0', '0', '', '', '10100', '7', '0', '0', '0', '1', '0', '0', '0', '0', '0', '55');
INSERT INTO `items_attachments` VALUES ('400094', 'Premium_Barrel_2', '7', 'Premium Barrel 2', 'Premium Upgrade', '', '', '10', '5', '0', '10', '10', '0', '0', '', '', '10100', '7', '0', '0', '0', '1', '0', '0', '0', '0', '0', '57');
INSERT INTO `items_attachments` VALUES ('400095', 'Premium_Barrel_3', '7', 'Premium Barrel 3', 'Premium Upgrade', '', '', '20', '5', '0', '15', '10', '0', '0', '', '', '10100', '7', '0', '0', '0', '1', '0', '0', '0', '0', '0', '58');
INSERT INTO `items_attachments` VALUES ('400096', 'Stock_Upgrade_1', '6', 'Stock Upgrade 1', '5% recoil reduction', '', '', '0', '0', '0', '-5', '0', '0', '0', '', '', '20100', '6', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400097', 'SG_45_round', '4', 'SG 45 round', '', '', '', '0', '0', '0', '0', '0', '15', '0', '', '', '4100', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '15');
INSERT INTO `items_attachments` VALUES ('400098', 'SG_60_rounds', '4', 'SG 60 rounds', '', '', '', '0', '0', '0', '0', '10', '30', '0', '', '', '4100', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '25');
INSERT INTO `items_attachments` VALUES ('400099', 'SG_100_rounds', '4', 'SG 100 rounds', '', '', '', '0', '0', '0', '0', '20', '70', '0', '', '', '4100', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '35');
INSERT INTO `items_attachments` VALUES ('400100', 'AK_100_rounds', '4', 'AK 100 rounds', '', '', '', '0', '0', '0', '0', '20', '70', '0', '', '', '4200', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '35');
INSERT INTO `items_attachments` VALUES ('400101', 'AK_60_rounds', '4', 'AK 60 rounds', '', '', '', '0', '0', '0', '0', '10', '30', '0', '', '', '4200', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '25');
INSERT INTO `items_attachments` VALUES ('400102', 'Stock_Upgrade_2', '6', 'Stock Upgrade 2', '10% recoil reduction, 5% spread increase', '', '', '0', '0', '0', '-10', '5', '0', '0', '', '', '20100', '6', '0', '0', '0', '1', '0', '0', '0', '0', '0', '20');
INSERT INTO `items_attachments` VALUES ('400103', 'Stock_Upgrade_3', '6', 'Stock Upgrade 3', '15% recoil reduction, 10% spread increase', '', '', '0', '0', '0', '-15', '10', '0', '0', '', '', '20100', '6', '0', '0', '0', '1', '0', '0', '0', '0', '0', '35');
INSERT INTO `items_attachments` VALUES ('400104', 'Stock_Upgrade_4', '6', 'Stock Upgrade 4', '20% recoil reduction,  20% spread increase', '', '', '0', '0', '0', '-20', '20', '0', '0', '', '', '20100', '6', '0', '0', '0', '1', '0', '0', '0', '0', '0', '45');
INSERT INTO `items_attachments` VALUES ('400105', 'Stock_upgrade_5', '6', 'Stock upgrade 5', '30% recoil reduction, 30% spread increase', '', '', '0', '0', '0', '-30', '30', '0', '0', '', '', '20100', '6', '0', '0', '0', '1', '0', '0', '0', '0', '0', '55');
INSERT INTO `items_attachments` VALUES ('400106', 'Premium_Stock_1', '6', 'Premium Stock 1', 'Reduces Recoil by 10%', '', '', '0', '0', '0', '-10', '0', '0', '0', '', '', '20100', '6', '0', '0', '0', '1', '0', '0', '0', '0', '0', '55');
INSERT INTO `items_attachments` VALUES ('400107', 'Premium_Stock_2', '6', 'Premium Stock 2', 'Reduces Recoil by 15%', '', '', '0', '0', '0', '-15', '0', '0', '0', '', '', '20100', '6', '0', '0', '0', '1', '0', '0', '0', '0', '0', '57');
INSERT INTO `items_attachments` VALUES ('400108', 'Premium_Stock_3', '6', 'Premium Stock 3', 'Reduces Recoil by 20%', '', '', '0', '0', '0', '-20', '0', '0', '0', '', '', '20100', '6', '0', '0', '0', '1', '0', '0', '0', '0', '0', '58');
INSERT INTO `items_attachments` VALUES ('400109', 'Trigger_Upgrade_1', '5', 'Trigger Upgrade 1', '', '', '', '0', '0', '3', '10', '10', '0', '0', '', '', '30100', '5', '0', '0', '0', '1', '0', '0', '0', '0', '0', '20');
INSERT INTO `items_attachments` VALUES ('400110', 'Trigger_Upgrade_2', '5', 'Trigger Upgrade 2', '', '', '', '0', '0', '5', '20', '20', '0', '0', '', '', '30100', '5', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30');
INSERT INTO `items_attachments` VALUES ('400111', 'Trigger_Upgrade_3', '5', 'Trigger Upgrade 3', '', '', '', '0', '0', '10', '30', '30', '0', '0', '', '', '30100', '5', '0', '0', '0', '1', '0', '0', '0', '0', '0', '40');
INSERT INTO `items_attachments` VALUES ('400112', 'PG7V_HEAT', '4', 'PG-7V HEAT', 'Standard HEAT rocket.', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '4500', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400114', 'OG7V_HE', '4', 'OG-7V HE', 'High Explosive Fragmentation rocket. +100 damage', '', '', '100', '0', '0', '0', '0', '0', '0', '', '', '4500', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400115', 'TBG7V', '4', 'TBG-7V', 'Single stage thermobaric rocket. +200 damage', '', '', '200', '0', '0', '0', '0', '0', '0', '', '', '4500', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400116', 'Grip_2', '3', 'Grip 2', '', '', '', '0', '0', '0', '5', '-5', '0', '0', '', 'Grip', '1001', '3', '0', '0', '0', '1', '0', '0', '0', '0', '0', '21');
INSERT INTO `items_attachments` VALUES ('400117', 'Grip_3', '3', 'Grip 3', '', '', '', '0', '0', '0', '10', '-10', '0', '0', '', 'Grip', '1001', '3', '0', '0', '0', '1', '0', '0', '0', '0', '0', '29');
INSERT INTO `items_attachments` VALUES ('400118', 'Premium_Grip_1', '3', 'Premium Grip 1', '', '', '', '0', '0', '0', '0', '-10', '0', '0', '', 'Grip', '1001', '3', '0', '0', '0', '1', '0', '0', '0', '0', '0', '55');
INSERT INTO `items_attachments` VALUES ('400119', 'MASADA_IS', '1', 'MASADA IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400120', 'M202_IS', '1', 'M202 IS', '', '', '', '0', '0', '0', '0', '0', '0', '20', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400121', 'USS12_IS', '1', 'USS-12 IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400122', 'SVD_20_rounds', '4', 'SVD 20 rounds', '', '', '', '0', '0', '0', '0', '0', '10', '0', '', '', '4004', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '35');
INSERT INTO `items_attachments` VALUES ('400123', 'RPK_100_rounds', '4', 'RPK 100 rounds', '', '', '', '0', '0', '0', '0', '0', '20', '0', '', '', '4005', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '20');
INSERT INTO `items_attachments` VALUES ('400124', 'RPK_120_rounds', '4', 'RPK 120 rounds', '', '', '', '0', '0', '0', '0', '-10', '40', '0', '', '', '4005', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '35');
INSERT INTO `items_attachments` VALUES ('400125', 'Elite_Silencer', '0', 'Elite Silencer', '', 'muzzle_asr_noflash', '', '-5', '0', '0', '0', '0', '0', '0', '', '', '2001', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '55');
INSERT INTO `items_attachments` VALUES ('400126', 'RX2600_IS', '1', 'RX2600 IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400127', 'M16_IS', '1', 'M16 IS', '', '', '', '0', '0', '0', '0', '0', '0', '10', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400128', 'AKM_IS', '1', 'AKM IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400129', 'AKS_IS', '1', 'AKS IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400130', 'QBZ_IS', '1', 'QBZ IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400131', 'GX11_IS', '1', 'GX11 IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400132', 'GX11_Standard_Magazine', '4', 'GX11 Standard Magazine', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400133', '10_rounds__50_Cal_', '4', '10 rounds .50 Cal ', '', '', '', '0', '0', '0', '0', '0', '0', '0', '', '', '0', '4', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_attachments` VALUES ('400134', 'UZI_IS', '1', 'UZI IS', '', '', '', '0', '0', '0', '0', '0', '0', '0', 'default', '', '0', '1', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0');

-- ----------------------------
-- Table structure for `items_gear`
-- ----------------------------
DROP TABLE IF EXISTS `items_gear`;
CREATE TABLE `items_gear` (
  `ItemID` int(10) NOT NULL AUTO_INCREMENT,
  `FNAME` varchar(64) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `Description` varchar(256) NOT NULL,
  `Category` int(10) NOT NULL,
  `Weight` int(10) NOT NULL,
  `DamagePerc` int(10) NOT NULL,
  `DamageMax` int(10) NOT NULL,
  `Bulkiness` int(10) NOT NULL,
  `Inaccuracy` int(10) NOT NULL,
  `Stealth` int(10) NOT NULL,
  `CustomFunction` int(10) NOT NULL,
  `Protection` char(5) NOT NULL,
  `Price1` int(10) NOT NULL,
  `Price7` int(10) NOT NULL,
  `Price30` int(10) NOT NULL,
  `PriceP` int(10) NOT NULL,
  `IsNew` int(10) NOT NULL,
  `ProtectionLevel` int(10) NOT NULL,
  `LevelRequired` int(10) NOT NULL,
  `GPrice1` int(10) NOT NULL,
  `GPrice7` int(10) NOT NULL,
  `GPrice30` int(10) NOT NULL,
  `GPriceP` int(10) NOT NULL,
  PRIMARY KEY (`ItemID`)
) ENGINE=InnoDB AUTO_INCREMENT=20169 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of items_gear
-- ----------------------------
INSERT INTO `items_gear` VALUES ('20000', 'Grunt', 'Grunt', 'This is your standard combat issue. If the government wanted you Mercs to have anything more, they would issue it to you.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20001', 'Light_Gear', 'Light Gear', 'Standard issue, light weight gear and body armor. It\'s good for those who are constantly on the move. Level 1 protection', '11', '0', '15', '400', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20002', 'Medium_Gear', 'Medium Gear', 'Standard issue, medium weight gear and body armor. Great for sustained field ops. where protection is more important than speed. Level 2 Protection', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20003', 'Heavy_Gear', 'Heavy Gear', 'Standard issue, heavy weight gear and body armor, that is battle tested for the harsh realities of heavy combat. Level 3 Protection', '11', '18', '30', '400', '0', '0', '0', '0', '3', '0', '0', '0', '1', '0', '3', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20004', 'Mr__Clean', 'Mr. Clean', 'For those of you who want to sport that basic operator look.', '12', '0', '0', '1000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20005', 'The_Stealth_Man', 'The Stealth Man', 'The look for those who would rather remain anonymous.', '12', '0', '0', '1000', '0', '-5', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20006', 'K__Style_Helmet', 'K. Style Helmet', 'The good ol\' Turtle Head is guaranteed to protect your brain. (Well, at least a little bit)&#xD;Level 2 Protection', '13', '6', '30', '300', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20007', 'SpecOps', 'SpecOps', 'This is Standard Issue field gear for Spec. Op. snake eaters and other hard core operators.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20011', 'Slickman', 'Slickman', 'Where high speed and low drag meet in a high-tech set up that is ready for all environments.', '10', '5', '5', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20012', 'Guerilla', 'Guerilla', 'Standard surplus gear from the last few decades of warfare. Quite popular in depressed 3rd world nations.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20013', 'Strikeforce', 'Strikeforce', 'High tech gear for those of you who have dreamed of being a ninja in the battle zone!', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20014', 'Ghilli_Suit', 'Ghilli Suit', 'Standard issue sniper suit that let\'s you become one with the earth and the bane of your enemies existence.', '11', '0', '0', '400', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20015', 'Custom_Guerilla', 'Custom Guerilla', 'Custom designed guerilla field gear and body armor that allows for decent protection under fast moving conditions. Level 2 Protection', '11', '10', '25', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20016', 'SWAT_Heavy', 'SWAT Heavy', 'High Tech gear design that brings you stronger armor protection made with lighter composit materials. Level 2 Protection', '11', '7', '25', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '45', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20017', 'Slickman_Gear', 'Slickman Gear', 'High Tech gear that is made with a lightweight ballistic resistant material, that was designed to allow the wearer to operate in various environments. Level 3 Protection', '11', '15', '30', '400', '0', '0', '5', '0', '3', '0', '0', '0', '1', '0', '3', '35', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20018', 'Sifu_Lee', 'Sifu Lee', 'This look brings the cool look of Asia to combat.', '12', '0', '0', '1000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20019', 'Joe_the_Lumberjack', 'Joe the Lumberjack', 'For those of you who want that fresh from the forest look.', '12', '0', '0', '1000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20020', 'Mr__East', 'Mr. East', 'For those who want to share the look of Eastern Europe.', '12', '0', '0', '1000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20021', 'Mr__West', 'Mr. West', 'The look for those of you who hail from the West.', '12', '0', '0', '1000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20022', 'Beret_Cover', 'Beret Cover', 'This cover is a stylish military fashion statement which stands out smashingly in a combat zone!', '13', '0', '0', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20023', 'Boonie_Cover', 'Boonie Cover', 'This Jungle Boonie cover is high speed, great for blending in and blocking out the sun and bugs but it won\'t stop bullets.', '13', '0', '0', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20024', 'M__Style_Helmet', 'M. Style Helmet', 'With its metal face protector the M. is a reliable helmet that provides decent cranial protection.&#xD;Level 3 Protection', '13', '5', '30', '300', '0', '0', '0', '0', '3', '0', '0', '0', '1', '0', '3', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20025', 'Shadow', 'Shadow', 'This head gear was designed to not only make you look scary but also to protect your pretty face.', '13', '0', '10', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20026', 'PMC_Soldier', 'PMC Soldier', 'The fun of being a paramilitary contractor is the cool clothes that you get to wear for your day job.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20028', 'Rebel_Hood', 'Rebel Hood', 'You can\'t sport the true \'Rebel Look\' with out the high speed Rebel Hood. Great for keeping the bugs off but poor for stopping bullets.', '13', '0', '0', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20029', 'Urban_Camo', 'Urban Camo', 'This is general issue for military operations in urban terrain. Said to be all the rage for a street party with RPGs!', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '100', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20030', 'Urban_sniper', 'Urban sniper', 'A high speed, low drag ghilli suite modified for urban combat environments.', '11', '0', '0', '400', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20031', 'IGN_Urban_Camo_', 'IGN Urban Camo ', 'A special edition and highly protective helmet in a Urban camo pattern.', '13', '2', '30', '300', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20032', 'Black_Mask', 'Black Mask', 'The frightening Black Mask of Death, sure to give your enemies the chills.', '13', '0', '10', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '5', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20033', 'Clown_Mask', 'Clown Mask', 'For all of you Jokers out there, we bring you the Clown Mask.', '13', '0', '10', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20034', 'Jason_Mask', 'Jason Mask', 'Hiding a monster, this mask strikes real horror in those who face it.', '13', '0', '10', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20035', 'Skull_Mask', 'Skull Mask', 'The Skull Mask shows you know how to Grin and Bear it!', '13', '0', '10', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '30', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20036', 'Slash_Mask', 'Slash Mask', 'While not just another pretty face, the Slash Mask is a cut above the rest! Get it and look the part of a killer.', '13', '0', '10', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '40', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20037', 'Rebel_modified', 'Rebel modified', 'Rebel groups are known for improvising and this armor shows it with Heavy protection.', '11', '0', '80', '1000', '0', '0', '0', '0', '10', '0', '0', '0', '1', '0', '10', '100', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20038', 'PMC_Desert', 'PMC Desert', 'The ultimate issue of desert camo for PMC operators where its hot and you want to blend in with the sand critters.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20039', 'Prison_Break', 'Prison Break', 'Be truly bold and strong while you look like an easy mark! Wear this when you want to show how tough you really are!', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20040', 'PMC_Forest', 'PMC Forest', 'For a look that blends you in with the forest, this gear is so good that squirrels will be running up your legs!', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20041', 'Boonie_Desert', 'Boonie Desert', 'This boonie hat is great for desert ops, keeping you out of the sun and foiling those pesky bugs.', '13', '0', '0', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20042', 'Boonie_Green', 'Boonie Green', 'While not a bullet stopper this boonie hat will keep you cool and in camo in the wild places.', '13', '0', '0', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20043', 'M9_helmet_black', 'M9 helmet black', 'The M9 helmet gives a good standard of head protection. This one in black.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '5', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20044', 'M9_Helmet_Blue', 'M9 Helmet Blue', 'The M9 helmet gives a good standard of head protection. This one in blue.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20045', 'M9_Helmet_Desert_', 'M9 Helmet Desert ', 'The M9 helmet gives a good standard of head protection. This one in desert style.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20046', 'M9_Helmet_with_Goggles', 'M9 Helmet with Goggles', 'The M9 helmet gives a good standard of head protection. This one with goggles.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '25', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20047', 'M9_Helmet_Green', 'M9 Helmet Green', 'The M9 helmet gives a good standard of head protection. This one in green.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '25', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20048', 'M9_Helmet_Urban', 'M9 Helmet Urban', 'The M9 helmet gives a good standard of head protection. This one in Urban camo.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '30', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20049', 'M9_Helmet_Forest_1', 'M9 Helmet Forest 1', 'The M9 helmet gives a good standard of head protection. This one in a special forest pattern.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '35', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20050', 'M9_Helmet_Goggles', 'M9 Helmet Goggles', 'The M9 helmet gives a good standard of head protection. This one with goggles.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '40', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20051', 'M9_Helmet_Forest', 'M9 Helmet Forest', 'The M9 helmet gives a good standard of head protection. This one in a basic Forest pattern.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '45', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20052', 'Maska_Helmet', 'Maska Helmet', 'Famous Maska style helmet giving the player solid level 2 head protection.', '13', '5', '30', '300', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20053', 'Maska_Helmet_snow', 'Maska Helmet snow', 'Snow patterned Maska style helmet with level 2 head protection.', '13', '5', '30', '300', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20054', 'IBA_Sand', 'IBA Sand', 'Special IBA release gear in Sand toned pattern with Level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20055', 'MTV_Desert', 'MTV Desert', 'Limited edition MTV release Desert patterned gear with Level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20056', 'MTV_Forest', 'MTV Forest', 'Limited edition MTV release Forest patterned gear with Level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20057', 'MTV_Milgreen', 'MTV Milgreen', 'Limited edition MTV release Milgreen patterned gear with Level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20058', 'MTV_Sand', 'MTV Sand', 'Limited edition MTV release Sand patterned gear with Level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20059', 'Light_Gear_Forest', 'Light Gear Forest', 'Light weight Level 1 protection in a Green Forest pattern.', '11', '0', '15', '400', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20060', 'Light_Urban', 'Light Urban', 'Light weight Level 1 protection in a Urban pattern.', '11', '0', '15', '400', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20061', 'Medium_Desert', 'Medium Desert', 'Medium weight level 2 protection in a Desert pattern.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20062', 'Medium_Woodland', 'Medium Woodland', 'Medium weight level 2 protection in a Woodland pattern.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '25', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20063', 'Night_Stalker', 'Night Stalker', 'Night Stalker gear makes you one with the night, the one who is out to kill all others!', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20064', 'Night_Stalker', 'Night Stalker', 'This is a serious face camo job to match the Night Stalker gear.', '12', '0', '0', '1000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20065', 'Night_Stalker_Vest', 'Night Stalker Vest', 'Hi-Tech gear patterned in Night Stalker camo providing level 3 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '3', '0', '0', '0', '1', '0', '3', '100', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20066', 'M9_NVG', 'M9 NVG', 'Standard M9 helmet rigged with NVG set up for night ops.', '13', '2', '25', '300', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20067', 'KStyle_NVG', 'KStyle NVG', 'Standard K style helmet rigged with NVG set up for night ops.', '13', '2', '25', '300', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20069', 'Black_Cobra_Beret', 'Black Cobra Beret', 'Exclusive beret available to Level 25 players with level 2 protection.', '13', '0', '5', '300', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '25', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20070', 'Paradise_Jack', 'Paradise Jack', 'When its time to kick butt in style, this is the gear that has class! Welcome to Paradise!', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20071', 'Sandstorm', 'Sandstorm', 'Designed to keep you light and cool while turning up the heat on your enemies in the toughest environments.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '15', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20072', 'Sandstorm_M6_helmet', 'Sandstorm M6 helmet', 'Exclusive sandstorm patterned helmet with level 2 protection.', '13', '2', '25', '300', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '45', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20073', 'Adaptive_Camouflage', 'Adaptive Camouflage', 'Some of the most technically advanced gear available; this camo makes you a real chameleon in the battlezone.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20074', 'Adaptive_Camouflage', 'Adaptive Camouflage', 'State of the Art adaptive camo in Sniper pattern for supreme stealth ability.', '11', '0', '0', '400', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20075', 'Adaptive_Camouflage', 'Adaptive Camouflage', 'State of the Art adaptive camo in Slickman pattern for ultimate stealth ability and level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20076', 'Testforce_beret', 'Testforce beret', 'Special edition Beret issued to the Mercs who participated in Operation Closed Beta Test.', '13', '0', '5', '300', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '35', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20077', 'MTV_Beta_armor', 'MTV Beta armor', 'Special edition MTV Beta Armor issued to the Mercs who participated in Operation Closed Beta Test.', '11', '10', '25', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '100', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20078', 'SK3_Veteran_Armor', 'SK3 Veteran Armor', 'Special edition SK3 Veteran Armor with Heavy protection.', '11', '15', '30', '400', '0', '0', '5', '0', '3', '0', '0', '0', '1', '0', '3', '100', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20079', 'Testforce_Gear', 'Testforce Gear', 'TF Gear was specifically designed and reserved for those Mercs who participated in Operation Closed Beta Test.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '100', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20080', 'SK3_Gear', 'SK3 Gear', 'This SK3 gear is a special issue that was designed for the Special Veterans Edition of SK3 gear.', '10', '5', '5', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '25', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20081', 'VDV_Vest', 'VDV Vest', 'Unique creation of VDV Vest with Level 1 protection.', '11', '10', '25', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '35', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20082', 'VDV', 'VDV', 'This gear had been specifically designed for Russian Airborne Troops but soon became popular with operators around the globe.', '10', '5', '5', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20083', 'VDV_Beret', 'VDV Beret', 'Exclusive Russian Airborne Troops beret that will complement the VDV gear and utilities.', '13', '0', '5', '10000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20084', 'IGN_Slickman_Gear', 'IGN Slickman Gear', 'Special edition IGN Slickman gear with Level 3 protection.', '11', '15', '35', '500', '0', '0', '5', '0', '3', '0', '0', '0', '1', '0', '3', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20085', 'Halloween_Jack', 'Halloween Jack', 'Special Gear for crazy Mercs who take their Halloween Partying seriously! Available for one week a year, this is the stuff of Nightmares!', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '100', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20086', 'Halloween_Jack', 'Halloween Jack', 'This is the special edition, limited time access, Halloween ghoul face.', '12', '0', '0', '1000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20087', 'Jack_O_Lantern', 'Jack O\' Lantern', 'Limited edition Halloween head cover that is available only one week a year.', '13', '0', '5', '300', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20088', 'Fireman', 'Fireman', 'Standard Firefighting Equipment allows an operator to blend into the urban scene while increasing protection from lethal environments.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20089', 'Cowboy', 'Cowboy', 'Some Mercs just can\'t leave the ranch, and it\'s ok! Wear these duds when you want to stand out amongst the crowd.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20090', 'Construction', 'Construction', 'Being part of the crew is quite apparent when dressing the part of the construction worker. It is ruff and rugged gear, but not real camo.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20091', 'PMC_Hitman', 'PMC Hitman', 'The Hitman always dresses for class and style. This gear is great in the urban scene and stands out in the Battlezone.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20092', 'Spetsnaz', 'Spetsnaz', 'This gear shows that the special forces of Russia had a style that was soon emulated by certain operatives around the world.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20093', 'Slickman_Xfire', 'Slickman Xfire', 'This is a Limited Edition release of Slickman type gear specifically for Xfire.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '100', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20094', 'Blueshirt', 'Blueshirt', 'This is the standard look for those of the Blueshirt crowd. It is definitely a statement of contempt for your enemy in the battlezone.', '10', '0', '0', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '30', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20095', 'Western_Beard', 'Western Beard', 'If you like that western look and the busy man beard that goes with it then this mug is for you!', '12', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20096', 'Boonie_Hat_Leather', 'Boonie Hat Leather', 'This is the boonie hat that comes with a bit of sophistication. It is all leather, which is bound to be a hit, but it will not stop one!', '13', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20097', 'Fireman_Helmet', 'Fireman Helmet', 'The traditional Firefighters helmet has been saving men for years. Give it a try if you want to be different in the battlezone.', '13', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20098', 'Hard_Hat', 'Hard Hat', 'The traditional construction hard hat is just the ticket for knocking around old buildings and showing your enemies that you are tough!', '13', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20099', 'Impulse_Shadow', 'Impulse Shadow', 'Impulse Shadow gear is a Limited Edition special issue designed specifically for Impulse.', '10', '10', '5', '10000', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '70', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20100', 'Impulse_Armor', 'Impulse Armor', 'Special edition Armor for Impulse with Level 3 protection.', '11', '15', '35', '400', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20101', 'M9_Helmet_Impulse_Edition', 'M9 Helmet Impulse Edition', 'The M9 helmet gives a good standard of head protection. Impulse edition', '13', '10', '10', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20102', 'XFire_Maska', 'XFire Maska', 'This is a special edition Xfire highly protective Maska style helmet.', '13', '10', '10', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20103', 'Xfire_Heavy_Armor', 'Xfire Heavy Armor', 'Special edition Xfire Slickman gear with Level 3 protection.', '11', '15', '35', '400', '0', '0', '0', '0', '3', '0', '0', '0', '1', '0', '3', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20104', 'The_War_Goddess', 'The War Goddess', 'Akiko Shimizu Warren trained in the Koga-Ninjitsu Kai & the JMSDF before becoming a Merc. She did hit and run strikes during the Indochina conflict. She specializes in Weapons of all types.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '12', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20105', 'Little_Viper', 'Little Viper', 'Elvira Suarez trained in Cubas Milicias de Tropas Territoriales and the 2nd Army Corps. As a Merc she saw combat in El Paso, Mexico City, and in Panama. Her specialty is in Field Tactics.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '12', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20106', 'Killer_Keller', 'Killer Keller', 'Lawrence Keller had been an Army Ranger who fought in Afghanistan & Pakistan. As a Merc he fought in Mexico and other conflicts. His specialty is in Recon and Defensive Ops.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '12', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20107', 'Bloody_Falcon', 'Bloody Falcon', 'Tomas Sokol was trained in the Arm&aacute;da ?esk&eacute; Republiky & served in Afghanistan & Pakistan. As a Merc, he served in Southern European & Northern Eye operations. His specialty is Explosives.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '12', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20108', 'MTV_KOMP', 'MTV KOMP', 'Special edition MTV KOMP Armor with Level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20109', 'MTV_EDGE', 'MTV EDGE', 'Special edition MTV Edge Armor with Level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20110', 'MTV_WEND', 'MTV WEND', 'Special edition MTV Wend Armor with Level 2 protection.', '11', '5', '20', '400', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '10', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20111', 'KYKY_MTV_Armor', 'KYKY MTV Armor', 'Special edition MTV KYKY Armor with Level 2 protection.', '11', '5', '20', '1000', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20112', 'KFMB_Armor', 'KFMB Armor', 'Special edition MTV KFMB Armor with Level 2 protection.', '11', '5', '20', '1000', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20113', 'MTV_Alienware', 'MTV Alienware', 'Special Edition of MTV armor for Alienware Arena players.', '11', '5', '20', '1000', '0', '0', '0', '0', '2', '0', '0', '0', '1', '0', '2', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20154', 'M9_Australia', 'M9 Australia', 'The M9 Australia helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20155', 'M9_Brazil', 'M9 Brazil', 'The M9 Brazil helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20156', 'M9_Canada', 'M9 Canada', 'The M9 Canada helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20157', 'M9_France', 'M9 France', 'The M9 France helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20158', 'M9_Germany', 'M9 Germany', 'The M9 Germany helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20159', 'M9_Italy', 'M9 Italy', 'The M9  Italy helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20160', 'M9_Netherlands', 'M9 Netherlands', 'The M9 Netherlands helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20161', 'M9_Turkey', 'M9 Turkey', 'The M9 Turkey helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20162', 'M9_UK', 'M9 UK', 'The M9 UK helmet gives a good standard of patriotic head protection.', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20163', 'M9_USA', 'M9 USA', 'The M9 USA helmetgives a good standard of patriotic head protection', '13', '2', '25', '300', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20164', 'The_Brotherhood', 'The Brotherhood', 'A member of &quot;The Brotherhood&quot;, this mercenary is equipped with state of the art equipment for rural and desert combat.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20165', 'Ironstone_Security_Specialists', 'Ironstone Security Specialists', 'A member of &quot;Ironstone Security Sepcialists&quot;, this mercenary is equipped with state of the art equipment for vehicular and arctic combat.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20166', 'The_Roaring_Tigers', 'The Roaring Tigers', 'A member of &quot;The Roaring Tigers&quot;, this mercenary is equipped with state of the art equipment for urban combat and espionage missions.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20167', 'Shadow_Corp__Unlimited', 'Shadow Corp., Unlimited', 'A member of &quot;Shadow Corp., Unlimited&quot;, this mercenary is equipped with state of the art equipment for rural and urban combat.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');
INSERT INTO `items_gear` VALUES ('20168', 'The_United_Warriors', 'The United Warriors', 'A member of &quot;The United Warriors&quot;, this mercenary is equipped with state of the art equipment for urban and guerilla warfare.', '16', '0', '25', '5000', '0', '0', '0', '0', '1', '0', '0', '0', '1', '0', '1', '0', '0', '0', '0', '0');

-- ----------------------------
-- Table structure for `items_generic`
-- ----------------------------
DROP TABLE IF EXISTS `items_generic`;
CREATE TABLE `items_generic` (
  `ItemID` int(10) NOT NULL AUTO_INCREMENT,
  `FNAME` varchar(32) NOT NULL,
  `Category` int(10) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `Description` varchar(512) NOT NULL,
  `Price1` int(10) NOT NULL,
  `Price7` int(10) NOT NULL,
  `Price30` int(10) NOT NULL,
  `PriceP` int(10) NOT NULL,
  `IsNew` int(10) NOT NULL,
  `LevelRequired` int(10) NOT NULL,
  `GPrice1` int(10) NOT NULL,
  `GPrice7` int(10) NOT NULL,
  `GPrice30` int(10) NOT NULL,
  `GPriceP` int(10) NOT NULL,
  `IsStackable` int(10) NOT NULL,
  PRIMARY KEY (`ItemID`)
) ENGINE=InnoDB AUTO_INCREMENT=301151 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of items_generic
-- ----------------------------
INSERT INTO `items_generic` VALUES ('301000', 'BlackOps_Crate_key', '1', 'BlackOps Crate key', 'This Key is used to reload your ammo from BlackOps Crates that are located in the combat area. Stand by the crate if you have the key to reload.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301001', '2X_EXP_Boost', '2', '2X EXP Boost', 'Honor Points Boost will add 100% HP bonus to your total HP at the end of each round.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301003', '2X_WP_Boost', '2', '2X WP Boost', 'War Points Boost will add 100% WP bonus to your total WP at the end of each round.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301004', 'Premium_Account', '1', 'Premium Account', 'Adds a Premium Account Status to your account for 30 days&#xD;- Allows you to host Ranked custom games&#xD;- 50% more XP and WP rewards', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301008', 'Stamina_Boost', '2', 'Stamina Boost', 'Gives you 50% more sprinting time', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301045', 'Change_Gamer_Tag', '1', 'Change Gamer Tag', 'Allows you to change your gamertag', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301046', 'Reset_Accuracy', '1', 'Reset Accuracy', 'Resets shots fired and shots hits to zero.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301047', 'Reset_KillDeath', '1', 'Reset Kill/Death', 'Resets kills and deaths to zero.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301048', 'Reset_skill_tree', '1', 'Reset skill tree', 'Unlearns all skills and returns skill points.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301049', 'Reset_winloss', '1', 'Reset win/loss', 'Resets wins and losses to zero.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301055', 'GenericAbility', '1', 'GenericAbility', 'temporary item used for ability purchase', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301061', 'Double_Up', '5', 'Double Up', 'Double the amount of all ammo that a player can carry (doesn\'t apply to explosives)', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301062', 'Berserker', '5', 'Berserker', 'Random chance of invincibility for 5 sec when a players health falls below 20% but is above 1%', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301063', 'Guardian_Angel', '5', 'Guardian Angel', 'Player can see damaged team mates, (red crosses above their names) and will regenarate them at 50 points per second rate if within 10 meters', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301064', 'Sniffer', '5', 'Sniffer', 'Warns player when a mine or booby trap is near', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301065', 'Eye_Spy', '5', 'Eye Spy', 'Follow an enemy on the map after they have wound the player until one of the players is killed', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301066', 'Silent_Strike', '5', 'Silent Strike', 'The player can use any weapon and not be detected on the map except by players with special detection abilities', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301067', 'Big_Surprise', '5', 'Big Surprise', 'After being killed the player will explode after 3 seconds from a suicide vest', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301068', 'DISABLED_Big_Guns', '5', 'DISABLED: Big Guns', 'DISABLED: Gives a player a 10% chance of receiving a free Artillery Strike when they purchase one', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301069', 'The_Fixer', '5', 'The Fixer', 'Gives the payer a 50% chance to safely switch an enemy mine or booby trap to become a friendly one. If the attempt fails, the mine or booby trap will explode.', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301070', 'Flag_Hog', '5', 'Flag Hog', 'Allows the player to cap a flag 10% faster than normal', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301071', 'Bulk_Up', '5', 'Bulk Up', 'Allows an additional 10% to the players armor rating', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301072', 'Ninja_Killer', '5', 'Ninja Killer', 'Allows a player to see all hidden enemies on the map for 30 seconds after a 3 enemy kill streak', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301073', 'Second_Wind', '5', 'Second Wind', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301074', '', '5', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301075', '', '5', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301076', '', '5', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301077', '', '5', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301078', '', '5', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301079', '', '5', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301080', '', '5', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301083', 'Mortar_Strike', '6', 'Mortar Strike', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301084', 'Mortar_Barrage', '6', 'Mortar Barrage', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301085', 'Artillery_Strike', '6', 'Artillery Strike', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301086', 'Laser_Guided_Bomb', '6', 'Laser Guided Bomb', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301087', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301088', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301089', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301090', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301091', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301092', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301093', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301094', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301095', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301096', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301097', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301098', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301099', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301100', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301101', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301102', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301103', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301104', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301105', 'AIRSTRIKERESERVED', '6', 'AIRSTRIKE_RESERVED', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301106', 'All_Weapons_Rent', '1', 'All Weapons Rent', 'Rent all weapons in store for specified amount of days', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301107', 'Buy_10000WP', '1', 'Buy 10000WP', 'Buy 10000 War Points for Gold Credits', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301108', 'WP_Crate_Permanent', '3', 'WP Crate Permanent', 'Permanent weapons ! By unlocking this crate you will receive random item containing in a crate or WP bonus if you already have this item in your inventory.', '0', '0', '0', '1', '0', '20', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301109', 'WP_Crate_Elite', '3', 'WP Crate Elite', 'Elite and Rare weapons ! By unlocking this crate you will receive random item containing in a crate for a random rental period', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301110', 'Sniper_Crate', '3', 'Sniper Crate', 'PERMANENT ITEMS ONLY ! WP ONLY weapons and RARE weapons !!!&#xD;You will receive random item of the list. If you already have this item you\'ll also receive bonus War Points.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301111', 'CQB_Crate', '3', 'CQB Crate', 'PERMANENT ITEMS ONLY ! WP ONLY weapons and RARE weapons !!!&#xD;You will receive random item of the list. If you already have this item you\'ll also receive bonus War Points.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301112', 'Heavy_Guns_Crate', '3', 'Heavy Guns Crate', 'PERMANENT ITEMS ONLY ! WP ONLY weapons and RARE weapons !!!&#xD;You will receive random item of the list. If you already have this item you\'ll also receive bonus War Points.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301113', 'Assault_Crate', '3', 'Assault Crate', 'PERMANENT ITEMS ONLY ! WP ONLY weapons and RARE weapons !!!&#xD;You will receive random item of the list. If you already have this item you\'ll also receive bonus War Points.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301114', 'Elite_Warrior_', '3', 'Elite Warrior ', 'This crate contains ELITE and RARE weapons only ! You will receive random item of the list for random rental period', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301115', 'Elite_Rentals', '3', 'Elite Rentals', 'ELITE and RARE weapons !!!  By unlocking this crate you will receive random item containing in a crate for a random duration. If you already have this item, rental time will be extended.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301116', 'Mystery_Banker', '3', 'Mystery Banker', 'By unlocking this case you\'ll receive a random amount of WP.', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301117', 'Mega_Loot_', '3', 'Mega Loot !', 'Permanent weapons and gear ! By unlocking this crate you will receive random item containing in a crate with value same or more than piece of the crate', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301118', 'Premium_Weapons_Crate', '7', 'Premium Weapons Crate', 'This crate contains a variation of premium weapons, including rare and Elite guns', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301119', 'Premium_Gear_Crate', '7', 'Premium Gear Crate', 'This crate contains a variation of premium and rare gear and outfits', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301120', 'Premium_Items_Crate', '7', 'Premium Items Crate', 'This crate contains a variation of premium and rare items to give you an advantage on the battlefield', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301121', 'Regular_Weapons_Crate', '7', 'Regular Weapons Crate', 'This crate contains a variation of weapons, including the chance of unlocking an Elite gun', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301122', 'Regular_Gear_Crate', '7', 'Regular Gear Crate', 'This crate contains a variation of gear and outfits', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301123', 'Regular_Items_Crate', '7', 'Regular Items Crate', 'This crate contains a variation of items and boosts to give you an advantage on the battlefield', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301124', 'Paragon_Crate', '7', 'Paragon Crate', 'Most Epic of all loot crates, this crate contains only Elite and Epic guns. Very rare and can be only found when you kill enemy who\'s 10 or more levels higher than you', '0', '0', '0', '1', '0', '10', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301125', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301126', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301127', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301128', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301129', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301130', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301131', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301132', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301133', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301134', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301135', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301136', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301137', '', '7', '', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301138', 'Unlock_Item_Upgrade_Slot_2', '1', 'Unlock Item Upgrade Slot 2', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301139', 'Unlock_Item_Upgrade_Slot_3', '1', 'Unlock Item Upgrade Slot 3', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301140', 'Unlock_Item_Upgrade_Slot_4', '1', 'Unlock Item Upgrade Slot 4', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301141', 'Unlock_Item_Upgrade_Slot_5', '1', 'Unlock Item Upgrade Slot 5', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301142', 'Unlock_New_Loadout_Slot', '1', 'Unlock New Loadout Slot', '', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301143', 'AccountClanCreate', '1', 'Account_ClanCreate', 'Create Clan', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301144', 'AccountClanUpg1', '1', 'Account_ClanUpg1', 'NOTE!!!!&#xD;&#xD;number of added clan members is in 1 day GP price', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301145', 'AccountClanUpg2', '1', 'Account_ClanUpg2', 'NOTE!!!!&#xD;&#xD;number of added clan members is in 1 day GP price', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301146', 'AccountClanUpg3', '1', 'Account_ClanUpg3', 'NOTE!!!!&#xD;&#xD;number of added clan members is in 1 day GP price', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301147', 'AccountClanUpg4', '1', 'Account_ClanUpg4', 'NOTE!!!!&#xD;&#xD;number of added clan members is in 1 day GP price', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301148', 'AccountClanUpg5', '1', 'Account_ClanUpg5', 'NOT USED!!!!', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301149', 'AccountClanUpg6', '1', 'Account_ClanUpg6', 'NOT USED!!!!', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');
INSERT INTO `items_generic` VALUES ('301150', 'ChangeLoadoutClass', '1', 'ChangeLoadoutClass', 'temp item used for change loadout class', '0', '0', '0', '1', '0', '0', '0', '0', '0', '1', '0');

-- ----------------------------
-- Table structure for `items_lootdata`
-- ----------------------------
DROP TABLE IF EXISTS `items_lootdata`;
CREATE TABLE `items_lootdata` (
  `RecordID` int(10) NOT NULL AUTO_INCREMENT,
  `LootID` int(10) NOT NULL,
  `Chance` float DEFAULT NULL,
  `ItemID` int(10) DEFAULT NULL,
  `ExpDaysMin` int(10) DEFAULT NULL,
  `ExpDaysMax` int(10) DEFAULT NULL,
  `GDMin` int(10) DEFAULT NULL,
  `GDMax` int(10) DEFAULT NULL,
  `GDIfHave` int(10) NOT NULL,
  PRIMARY KEY (`RecordID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of items_lootdata
-- ----------------------------

-- ----------------------------
-- Table structure for `items_packages`
-- ----------------------------
DROP TABLE IF EXISTS `items_packages`;
CREATE TABLE `items_packages` (
  `ItemID` int(10) NOT NULL AUTO_INCREMENT,
  `FNAME` varchar(64) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `Description` varchar(512) NOT NULL,
  `Category` int(10) NOT NULL,
  `Price1` int(10) NOT NULL,
  `Price7` int(10) NOT NULL,
  `Price30` int(10) NOT NULL,
  `PriceP` int(10) NOT NULL,
  `IsNew` int(10) NOT NULL,
  `LevelRequired` int(10) NOT NULL,
  `GPrice1` int(10) NOT NULL,
  `GPrice7` int(10) NOT NULL,
  `GPrice30` int(10) NOT NULL,
  `GPriceP` int(10) NOT NULL,
  `IsEnabled` int(10) NOT NULL,
  `AddGP` int(10) NOT NULL,
  `AddSP` int(10) NOT NULL,
  `Item1_ID` int(10) NOT NULL,
  `Item1_Exp` int(10) NOT NULL,
  `Item2_ID` int(10) NOT NULL,
  `Item2_Exp` int(10) NOT NULL,
  `Item3_ID` int(10) NOT NULL,
  `Item3_Exp` int(10) NOT NULL,
  `Item4_ID` int(10) NOT NULL,
  `Item4_Exp` int(10) NOT NULL,
  `Item5_ID` int(10) NOT NULL,
  `Item5_Exp` int(10) NOT NULL,
  `Item6_ID` int(10) NOT NULL,
  `Item6_Exp` int(10) NOT NULL,
  PRIMARY KEY (`ItemID`)
) ENGINE=InnoDB AUTO_INCREMENT=500020 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of items_packages
-- ----------------------------
INSERT INTO `items_packages` VALUES ('500000', 'TEST00', 'TEST00', 'TEST', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500001', 'Night_Stalker', 'Night Stalker', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20063', '2000', '20064', '2000', '20066', '2000', '20065', '2000', '101210', '2000', '0', '0');
INSERT INTO `items_packages` VALUES ('500004', 'Prison_Break', 'Prison Break', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101033', '2000', '20039', '2000', '20016', '2000', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500005', 'Unstoppable', 'Unstoppable', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101083', '2000', '101198', '2000', '101108', '2000', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500006', 'Desert_Swarm', 'Desert Swarm', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20038', '2000', '20055', '2000', '20046', '2000', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500007', 'Reserved_Package', 'Reserved Package', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500008', 'DEV_Kewk_Pack', '[DEV] Kewk Pack', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '20088', '2000', '20053', '2000', '101055', '2000', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500009', 'Bad_Ass', 'Bad Ass', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101045', '2000', '20032', '2000', '20016', '2000', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500010', 'CANADA_PACK', 'CANADA PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101272', '2000', '20156', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500011', 'BRAZIL_PACK', 'BRAZIL PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101277', '2000', '20155', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500012', 'AUSTRALIA_PACK', 'AUSTRALIA PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101271', '2000', '20154', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500013', 'USA_PACK', 'USA PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101274', '2000', '20163', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500014', 'UK_PACK', 'UK PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101273', '2000', '20162', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500015', 'TURKEY_PACK', 'TURKEY PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101268', '2000', '20161', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500016', 'NETHERLANDS_PACK', 'NETHERLANDS PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101269', '2000', '20160', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500017', 'ITALY_PACK', 'ITALY PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101275', '2000', '20159', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500018', 'GERMANY_PACK', 'GERMANY PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101276', '2000', '20158', '2000', '0', '0', '0', '0', '0', '0', '0', '0');
INSERT INTO `items_packages` VALUES ('500019', 'FRANCE_PACK', 'FRANCE PACK!', '', '9', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '1', '0', '0', '101270', '2000', '20157', '2000', '0', '0', '0', '0', '0', '0', '0', '0');

-- ----------------------------
-- Table structure for `items_upgradedata`
-- ----------------------------
DROP TABLE IF EXISTS `items_upgradedata`;
CREATE TABLE `items_upgradedata` (
  `ItemID` int(10) NOT NULL,
  `FNAME` varchar(32) NOT NULL,
  `Category` int(10) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `Description` varchar(512) NOT NULL,
  `Price1` int(10) NOT NULL,
  `Price7` int(10) NOT NULL,
  `Price30` int(10) NOT NULL,
  `PriceP` int(10) NOT NULL,
  `LevelRequired` int(10) NOT NULL,
  `GPrice1` int(10) NOT NULL,
  `GPrice7` int(10) NOT NULL,
  `GPrice30` int(10) NOT NULL,
  `GPriceP` int(10) NOT NULL,
  `GPChance` float NOT NULL,
  `GDChance` float NOT NULL,
  `UpgradeID` int(10) NOT NULL,
  `Value` float NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of items_upgradedata
-- ----------------------------

-- ----------------------------
-- Table structure for `items_weapons`
-- ----------------------------
DROP TABLE IF EXISTS `items_weapons`;
CREATE TABLE `items_weapons` (
  `ItemID` int(10) NOT NULL AUTO_INCREMENT,
  `FNAME` varchar(32) NOT NULL,
  `Category` int(10) NOT NULL,
  `Name` varchar(32) NOT NULL,
  `Description` varchar(256) NOT NULL,
  `MuzzleOffset` varchar(32) NOT NULL,
  `MuzzleParticle` varchar(32) NOT NULL,
  `Animation` varchar(32) NOT NULL,
  `BulletID` varchar(32) NOT NULL,
  `Sound_Shot` varchar(255) NOT NULL,
  `Sound_Reload` varchar(255) NOT NULL,
  `Damage` float NOT NULL,
  `isImmediate` int(10) NOT NULL,
  `Mass` float NOT NULL,
  `Speed` float NOT NULL,
  `DamageDecay` float NOT NULL,
  `Area` float NOT NULL,
  `Delay` float NOT NULL,
  `Timeout` float NOT NULL,
  `NumClips` int(10) NOT NULL,
  `Clipsize` int(10) NOT NULL,
  `ReloadTime` float NOT NULL,
  `ActiveReloadTick` float NOT NULL,
  `RateOfFire` int(10) NOT NULL,
  `Spread` float NOT NULL,
  `Recoil` float NOT NULL,
  `NumGrenades` int(10) NOT NULL,
  `GrenadeName` varchar(32) NOT NULL,
  `Firemode` varchar(3) NOT NULL,
  `DetectionRadius` int(10) NOT NULL,
  `ScopeType` varchar(32) NOT NULL,
  `ScopeZoom` int(10) NOT NULL,
  `Price1` int(10) NOT NULL,
  `Price7` int(10) NOT NULL,
  `Price30` int(10) NOT NULL,
  `PriceP` int(10) NOT NULL,
  `IsNew` int(10) NOT NULL,
  `LevelRequired` int(10) NOT NULL,
  `GPrice1` int(10) NOT NULL,
  `GPrice7` int(10) NOT NULL,
  `GPrice30` int(10) NOT NULL,
  `GPriceP` int(10) NOT NULL,
  `ShotsFired` bigint(19) NOT NULL,
  `ShotsHits` bigint(19) NOT NULL,
  `KillsCQ` int(10) NOT NULL,
  `KillsDM` int(10) NOT NULL,
  `KillsSB` int(10) NOT NULL,
  `IsUpgradeable` int(10) NOT NULL,
  `IsFPS` int(10) NOT NULL,
  `FPSSpec0` int(10) NOT NULL,
  `FPSSpec1` int(10) NOT NULL,
  `FPSSpec2` int(10) NOT NULL,
  `FPSSpec3` int(10) NOT NULL,
  `FPSSpec4` int(10) NOT NULL,
  `FPSSpec5` int(10) NOT NULL,
  `FPSSpec6` int(10) NOT NULL,
  `FPSSpec7` int(10) NOT NULL,
  `FPSSpec8` int(10) NOT NULL,
  `FPSAttach0` int(10) NOT NULL,
  `FPSAttach1` int(10) NOT NULL,
  `FPSAttach2` int(10) NOT NULL,
  `FPSAttach3` int(10) NOT NULL,
  `FPSAttach4` int(10) NOT NULL,
  `FPSAttach5` int(10) NOT NULL,
  `FPSAttach6` int(10) NOT NULL,
  `FPSAttach7` int(10) NOT NULL,
  `FPSAttach8` int(10) NOT NULL,
  `AnimPrefix` varchar(32) NOT NULL,
  PRIMARY KEY (`ItemID`)
) ENGINE=InnoDB AUTO_INCREMENT=101283 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of items_weapons
-- ----------------------------
INSERT INTO `items_weapons` VALUES ('101002', 'ASR_M16', '20', 'M16', 'The M16 rifle is a gas operated standard infantry assault rifle, capable of firing a 5.56 mm round in 3 round bursts that are very effective out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.56', 'Sounds/NewWeapons/Assault/ColtM16', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '27', '1', '10', '300', '75', '0', '0', '0', '4', '30', '2.5', '0.43', '625', '1.5', '7.0035', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400127', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101003', 'SUP_RPG7', '24', 'RPG-7', 'The RPG 7 rocket launcher was designed to fire a standard 40 mm high explosive anti-armor round. It is effective against targets at medium ranges.', '0, 0, 0', 'muzzle_rpg', 'rpg', 'rocket', 'Sounds/NewWeapons/Special/AT4RocketLauncher', 'Sounds/Weapons/SUPPORT/SUP_Generic_Reload', '200', '0', '0.15', '40', '500', '6', '0', '30', '2', '1', '4', '2.43', '30', '2', '10', '0', 'asr_grenade', '100', '0', 'default', '30', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '5002', '0', '0', '0', '0', '0', '0', '0', '0', '400045', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101004', 'HG_FN57', '25', 'FN 57', 'The FN 57 is a standard issue handgun that was designed to fire a 5.7 mm round. It is an effective weapon against close range targets.', '0, 0, 0', 'muzzle_hg', 'pistol', '5.45', 'Sounds/NewWeapons/Handgun/Glock9mm', 'Sounds/Weapons/HG/HG_Generic_Reload', '32', '1', '10', '300', '35', '0', '0', '0', '4', '10', '2.5', '0.43', '800', '2', '3', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '3002', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '400087', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101005', 'ASR_G36', '20', 'G36', 'The MTAC X36 assault rifle fires a 5.56 mm round. Its high rate of fire and medium recoil deliver a reasonably tight spread on impact of targets up to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_G36', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '30', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '650', '1.5', '8.004', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '28', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4100', '30100', '20100', '10100', '0', '400014', '400042', '0', '0', '400029', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101008', 'EXP_M26', '27', 'M26 Grenade', 'The M26 is a High Explosive Fragmentation grenade with a blast area of 10 meters. This little fist full of joy will definitely put some \'BANG\' into your arsenal.', '0, 0, 0', '', 'grenade', 'grenade', '', '', '250', '0', '10', '18', '5', '8', '3', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101022', 'ASR_AK74M', '20', 'AK-74M', 'The AK-74M assault rifle is an upgrade of the AK 74 design with a slightly increased rate of fire. It fires a 5.45 x 39 mm round accurately at medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/Kalashnikov_AK74', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '32', '1', '10', '300', '75', '0', '0', '0', '4', '30', '3', '0.43', '625', '2.1', '8.004', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '21', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400040', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101027', 'ASR_AK74M_Elite', '20', 'AK-74M ELITE', 'This LIMITED EDITION, Gold plated AK-74M is not only shiny, but it has been modified to reduce recoil making it a very effective assault rifle.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/Kalashnikov_AK74', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '33', '1', '10', '300', '90', '0', '0', '0', '4', '30', '3', '0.43', '650', '2', '7', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400040', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101032', 'ASR_AKM', '20', 'AKM', 'The AKM is an assault rifle that was designed as a simplified version of the AK-47 rifle. It fires a 7.62 x 39 mm round and is relatively effective at medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '7.62', 'Sounds/NewWeapons/Assault/Kalashnikov_AKM', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '34', '1', '10', '300', '85', '0', '0', '0', '4', '30', '3', '0.43', '600', '2.2', '10.005', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '10', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400128', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101033', 'asr_akm_elite', '20', 'AKM ELITE', 'The LIMITED EDITION, Gold plated AKM Elite makes a loud statement on the battlefield. It has also been modified as a more effective 7.62 x 39 mm assault rifle.', '0, 0, 0', 'muzzle_asr', 'assault', '7.62', 'Sounds/NewWeapons/Assault/Kalashnikov_AKM', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '35', '1', '15', '300', '90', '0', '0', '0', '4', '30', '3', '0.43', '625', '2.1', '9', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400128', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101035', 'ASR_AKS74U', '20', 'AKS-74U', 'The AKS-74U is a shortened gas operated carbine version of the AKS-74 assault rifle. It fires a 5.45 x 39 mm round and is very effective at medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/AKS-74U', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '27', '1', '10', '300', '55', '0', '0', '0', '4', '30', '3', '0.43', '650', '2', '5.0025', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '39', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400129', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101037', 'ASR_Famas', '20', 'FAMAS F1', 'The Famas F1 assault rifle fires a standard 5.56 mm round using a delayed blowback system. It delivers highly effective fire out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/Famas_F1', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '28', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '900', '1.3', '11.0055', '0', 'asr_grenade', '011', '0', 'default', '0', '0', '0', '0', '1', '0', '31', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400041', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101040', 'ASR_SIG516', '20', 'SIG SAUER 516', 'The 516 Tactical assault rifle fires a 5.56 x 45 mm round using a short stroke piston system. It has a good rate of fire and is reasonably effective on medium range targets.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_416', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '35', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '650', '1.71', '9', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '45', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400058', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101045', 'ASR_L85', '20', 'LS 90', 'LS 90 is a variant assault weapon design that fires a 5.56 x 45 mm round using a gas-operated rotating bolt system. It is extremely accurate at medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/XM8', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '30', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '650', '2.1', '5.0025', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '31', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400064', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101055', 'ASR_M4FFH', '20', 'BLACKWATER M4', 'The Blackwater M4 is a versitile, light weight, commando style assault rifle that is gas operated and fires a 5.56 x 45 mm round effectively in both close and medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/ColtM4', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '34', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '625', '1.5', '8.004', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '41', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400024', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101060', 'MG_PKM', '23', 'PKM', 'The PKM machine gun was designed to fire a 7.62 mm rounds from a linked system and is reasonably accurate out to medium ranged targets.', '0, 0, 0', 'muzzle_asr', 'assault', '7.62', 'Sounds/NewWeapons/LightMachineGun/Kalashnikov_PKM', 'Sounds/Weapons/MG/MG_Generic_Reload', '36', '1', '15', '400', '90', '0', '0', '0', '3', '100', '5', '2.43', '625', '2.08', '10', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '34', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '0', '30100', '20100', '10100', '0', '0', '400056', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101062', 'SMG_Kedr', '26', 'KEDR', 'The Kedr submachine gun fires a 9 mm round from a 26 round clip. It is considered an effective weapon for close range targets.', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/Weapons/SMG/SMG_Generic_Shot', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '24', '1', '10', '330', '35', '0', '0', '0', '3', '26', '2.5', '0.43', '960', '2.73', '7.2', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101063', 'SMG_MP7', '26', 'MP7', 'The RA TR7 submachine gun is a special issue weapon that fires a 4.6 mm standard cartridge from a 30 round magazine. It is effective at close range targets.', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/NewWeapons/SMG/HK_UMP', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '24', '1', '10', '330', '35', '0', '0', '0', '3', '30', '2.5', '0.43', '800', '2.366', '6.4', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '40', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1002', '4003', '30100', '20100', '10100', '0', '400014', '400026', '0', '0', '400033', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101064', 'SMG_UZI', '26', 'UZI', 'The special issue UZI submachine gun is a open bolt, blow back design that fires 9 mm rounds from its magazine housed in the weapons pistol grip.', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/NewWeapons/SMG/Uzi', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '26', '1', '10', '300', '35', '0', '0', '0', '3', '30', '2.5', '0.43', '700', '1.8928', '9.6', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '0', '3001', '0', '4006', '30100', '20100', '10100', '0', '400014', '400134', '0', '0', '400084', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101068', 'SNP_SVD_PSO', '21', 'SVD ', 'The SVD sniper rifle was designed to fire a 7.62 mm round from a 10 round clip. Combined with an Optical Sniper Scope it is extremely accurate.', '0, 0, 0', 'muzzle_asr', 'assault', 'Sniper5.45', 'Sounds/NewWeapons/Sniper/7.6mmCaliber', 'Sounds/Weapons/SNP/SNP_SVD_RELOAD', '70', '0', '1.5', '600', '600', '0', '0', '0', '2', '10', '3.5', '1.43', '80', '3.5', '3', '0', 'asr_grenade', '100', '0', 'pso1', '70', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '6001', '0', '0', '4004', '0', '20100', '10100', '0', '0', '400027', '0', '0', '400048', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101077', 'ASR_An_94', '20', 'AN 94', 'The AN 94 is an advanced assault rifle that functions on a gas operated system, firing a 5.45 x 39 mm round that does a large amount of damage on impact.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/Kalashnikov_AK108', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '33', '1', '12', '320', '80', '0', '0', '0', '4', '30', '3', '0.43', '1000', '1.5', '11.0055', '0', 'asr_grenade', '010', '0', 'default', '0', '0', '0', '0', '1', '0', '41', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400062', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101081', 'ASR_QBZ-95', '20', 'QBZ 95 ', 'The QBZ 95 assault rifle uses a gas operated, rotating bolt system to fire a special 5.8 x 42 mm DBP87 round. It is a highly effective weapon out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/AEK971', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '33', '1', '10', '300', '90', '0', '0', '0', '4', '30', '2.5', '0.43', '660', '1.5', '8', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '10', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400130', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101084', 'SNP_Vintorez', '21', 'VSS VINTOREZ', 'The VSS Vintorez sniper rifle was designed to fire a 9 x 39 mm round using a gas operated, rotating bolt firing system. It is designed to be effective at medium distances.', '0, 0, ', 'muzzle_asr', 'assault', 'Sniper5.45', 'Sounds/NewWeapons/Sniper/VSS_Vintorez', 'Sounds/Weapons/SNP/SNP_Generic_Reload', '125', '0', '1.5', '350', '200', '0', '0', '0', '2', '10', '3.5', '1.43', '200', '2', '5', '0', 'asr_grenade', '101', '0', 'pso1', '60', '0', '0', '0', '1', '0', '51', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '6001', '3001', '0', '4002', '30100', '20100', '10100', '0', '0', '400027', '0', '0', '400031', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101085', 'SNP_AW', '21', 'MAUSER SRG', 'The Mauser SRG sniper rifle was designed to fire a 7.62 mm round from a 5 round clip. It is highly accurate at long ranges when combined with a good scope.', '0, 0, 0', 'muzzle_asr', 'assault', 'Sniper5.45', 'Sounds/NewWeapons/Sniper/0.308caliber', 'Sounds/Weapons/SNP/SNP_Generic_Reload', '105', '0', '1.2', '600', '600', '0', '0', '0', '4', '5', '3.5', '1.43', '60', '3', '3', '0', 'asr_grenade', '100', '0', 'aw50', '80', '0', '0', '0', '1', '0', '29', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '6001', '0', '0', '0', '30100', '20100', '10100', '0', '400076', '400008', '0', '0', '400070', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101087', 'SNP_AW50', '21', 'MAUSER HP50', 'The Mauser HP50 sniper rifle fires a .50 caliber round with devastating accuracy and effect. Each of its rounds deal a very high amount of damage.', '0, 0, 0', 'muzzle_asr', 'assault', 'Sniper5.45', 'Sounds/NewWeapons/Sniper/0.50Caliber', 'Sounds/Weapons/SNP/SNP_Generic_Reload', '180', '0', '1.3', '500', '1000', '0', '0', '0', '4', '5', '5', '1.43', '30', '3', '4', '0', 'asr_grenade', '100', '0', 's50hs', '90', '0', '0', '0', '1', '0', '45', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '6001', '0', '0', '0', '30100', '20100', '10100', '0', '400077', '400038', '0', '0', '400043', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101088', 'SNP_M82', '21', 'M107', 'The M107  sniper rifle fires a .50 caliber round that strikes with deadly effect at extreme ranges. It has a 10 round clip that allows for a high rate of fire.', '0, 0, 0', 'muzzle_asr', 'assault', 'Sniper5.45', 'Sounds/NewWeapons/Sniper/0.50Caliber', 'Sounds/Weapons/SNP/SNP_Generic_Reload', '200', '0', '2', '500', '1000', '0', '0', '0', '2', '10', '5', '1.43', '60', '5', '7', '0', 'asr_grenade', '100', '0', 'sniper', '110', '0', '0', '0', '1', '0', '47', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '6001', '0', '0', '0', '30100', '20100', '10100', '0', '0', '400008', '0', '0', '400133', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101092', 'MG_Pecheneg', '23', 'PECHENEG', 'The Pecheneg machine gun fires 7.62mm rounds on a box and belt system. It delivers highly effective fire at close and medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '7.62', 'Sounds/NewWeapons/LightMachineGun/Pecheneg', 'Sounds/Weapons/MG/MG_Generic_Reload', '37', '1', '15', '400', '90', '0', '0', '0', '3', '100', '5', '2.43', '625', '1.76', '9', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '42', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '0', '30100', '20100', '10100', '0', '0', '400055', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101093', 'MG_RPK', '23', 'RPK-74 ', 'The RPK-74 machine gun fires a 5.45 mm high velocity round that delivers a reasonably accurate rate of fire onto targets out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/LightMachineGun/Kalashnikov_RPK74', 'Sounds/Weapons/MG/MG_Generic_Reload', '34', '1', '15', '400', '90', '0', '0', '0', '4', '80', '4', '2.43', '600', '2.08', '10', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '4005', '30100', '20100', '10100', '0', '0', '400060', '0', '0', '400057', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101095', 'MG_M249_SAW', '23', 'FN M249 ', 'The FN M249 machine gun fires a 5.56 mm round from a belt fed system. It is highly effective against targets at close and medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/LightMachineGun/FN_M249', 'Sounds/Weapons/MG/MG_Generic_Reload', '31', '1', '18', '400', '90', '0', '0', '0', '3', '100', '6', '2.43', '600', '1.6', '5', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '25', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '0', '30100', '20100', '10100', '0', '0', '400035', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101098', 'SHG_Saiga', '22', 'SAIGA', 'The Saiga was developed as a 12 gauge shotgun with a 8 round storage tube. It inflicts a substancial amount of damage on targets at a close range.', '0, 0, 0', 'muzzle_sh1', 'assault', 'buckshot', 'Sounds/NewWeapons/Shotgun/Shotgun', 'Sounds/Weapons/SHOTGUN/SHG_Generic_Reload', '27', '1', '18', '300', '10', '0', '0', '0', '3', '8', '3.5', '1.43', '100', '3.5', '10', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '25', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '0', '0', '20100', '10100', '0', '0', '400080', '0', '0', '400073', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101103', 'SMG_MP5A4', '26', 'MP5', 'The RA X5 submachine gun was designed to fire a 9 mm rounds from a 30 round clip effectively against close ranged targets.', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/NewWeapons/SMG/HK_MP5', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '23', '1', '10', '300', '35', '0', '0', '0', '3', '30', '2.5', '0.43', '700', '1.8928', '7.2', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '0', '0', '30100', '20100', '10100', '0', '400014', '400081', '0', '0', '400079', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101106', 'SMG_AacHoneyBadger_01', '26', 'Honey Badger', 'The Honey Badger submachine gun fires a .300 round at a medium rate of fire causing a good amount of damage on close range targets.', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/NewWeapons/SMG/HK_MP5SD', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '26', '1', '15', '300', '45', '0', '0', '0', '3', '30', '2.5', '0.43', '780', '1.4', '7.2', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '52', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400074', '400066', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101107', 'SMG_P90', '26', 'FN P90 ', 'The FN P90 submachine gun was designed to fire 9 mm rounds from a large 50 round magazine. It is very effective on close range targets.', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/NewWeapons/SMG/FN_P90', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '26', '1', '10', '300', '35', '0', '0', '0', '3', '50', '2.5', '0.43', '900', '2.64992', '8', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '0', '0', '30100', '20100', '10100', '0', '400014', '400054', '0', '0', '400046', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101108', 'SMG_ScorpionEVO3', '26', 'EVO-3', 'The EVO-3 submachine gun was designed to effectively fire 9 mm rounds from a 25 round magazine at close ranged targets at a very high rate of fire..', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/NewWeapons/SMG/Scorpion_EVO3', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '26', '1', '15', '300', '35', '0', '0', '0', '3', '30', '2.5', '0.43', '800', '2.366', '10.4', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '30', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '0', '30100', '20100', '10100', '0', '400014', '400051', '0', '0', '400049', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101109', 'SMG_Bizon', '26', 'BIZON', 'The Bizon submachine gun was designed to fire a 9 mm round from a 64 round magazine delivering accurate fire on close range targets.', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/NewWeapons/SMG/Bizon', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '26', '1', '15', '300', '35', '0', '0', '0', '3', '64', '2.5', '0.43', '900', '2.8392', '8.8', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '0', '0', '30100', '20100', '10100', '0', '400014', '400052', '0', '0', '400047', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101111', 'HG_Beretta92F', '25', 'Taurus PT92', 'The Taurus PT92 handgun was specially designed to fire a 9mm round and is reasonably effective agains close range targets.', '0, 0, 0', 'muzzle_hg', 'pistol', '5.45', 'Sounds/NewWeapons/Handgun/Beretta92', 'Sounds/Weapons/HG/HG_Generic_Reload', '33', '1', '10', '300', '35', '0', '0', '0', '4', '12', '2.5', '0.43', '800', '2', '3', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '3002', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '400071', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101112', 'HG_Beretta93R', '25', 'B93R ', 'The B93R handgun was designed to fire a 9mm parabellum round from a 20 round clip. It is a relatively effective weapon against close ranged targets.', '0, 0, 0', 'muzzle_hg', 'pistol', '5.45', 'Sounds/NewWeapons/Handgun/Beretta93R', 'Sounds/Weapons/HG/HG_Generic_Reload', '30', '1', '6', '300', '35', '0', '0', '0', '4', '15', '2.5', '0.43', '800', '2', '5', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '3002', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '400071', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101115', 'HG_Jericho', '25', 'Jericho 9mm', 'The Jericho was designed as a universal self-loading pistol which fires a 9mm Parabellum round and is highly effective at close range.', '0, 0, 0', 'muzzle_hg', 'pistol', '5.45', 'Sounds/NewWeapons/Handgun/H&K_USP', 'Sounds/Weapons/HG/HG_Generic_Reload', '35', '1', '10', '300', '35', '0', '0', '0', '4', '10', '2.5', '0.43', '800', '2', '3', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '22', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '3002', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '400071', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101120', 'HG_SigP226', '25', 'Sig Sauer P226', 'The SIG P226 is a full-sized, service-type pistol made by SIG Sauer. It been designed to fire a 9 x 19 mm round that makes it an effective weapon against close ranged targets.', '0, 0, 0', 'muzzle_hg', 'pistol', '9mm', 'Sounds/NewWeapons/Handgun/Beretta92', 'Sounds/Weapons/HG/HG_Generic_Reload', '35', '1', '10', '300', '35', '0', '0', '0', '4', '17', '2.5', '0.43', '800', '2', '3', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '39', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '3002', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '400034', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101130', 'SUP_AT4', '24', 'AT4 ', 'The AT4 rocket launcher is a 84 mm recoilless smoothbore weapon designed to deliver high explosive and anti-armour rounds effectively on medium range targets.', '0, 0, 0', 'muzzle_rpg', 'rpg', 'Rocket_AT4', 'Sounds/NewWeapons/Special/AT4RocketLauncher', 'Sounds/Weapons/SUPPORT/SUP_Generic_Reload', '170', '0', '0.15', '50', '500', '5', '0', '30', '2', '1', '4', '2.43', '30', '2', '10', '0', 'asr_grenade', '100', '0', 'rpg8', '30', '0', '0', '0', '1', '0', '31', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '400065', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101131', 'SUP_RPO', '24', 'RPO-2', 'The RPO-2 rocket launcher was designed to fire a 93 mm rocket armed with a thermobaric warhead capable of delivering a huge blast on its target.', '0, 0, 0', 'muzzle_rpg', 'rpg', 'Rocket_RPO', 'Sounds/NewWeapons/Special/RPO2', 'Sounds/Weapons/SUPPORT/SUP_Generic_Reload', '300', '0', '0.15', '60', '500', '8', '0', '30', '2', '1', '4', '2.43', '30', '2', '10', '0', 'asr_grenade', '100', '0', 'rpg7', '20', '0', '0', '0', '1', '0', '41', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '400061', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101136', 'EXP_White_Phosphorus', '27', 'White Phosphorus Grenade', 'The White Phosphorus Grenade is a fragmentation, incendiary grenade with a blast radius of 10 meters, that detonates causing blast and burn damage.', '0, 0, 0', '', 'grenade', 'WhitePhosphorus', '', '', '300', '0', '10', '18', '5', '8', '3', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101137', 'EXP_FlashBang', '27', 'Flash Bang Grenade', 'The Flash Bang Grenade detonates with a loud blast and blinding flash that will stun all enemies within a 20 meter radius of the detonation point.', '0, 0, 0', '', 'grenade', 'FlashBang', '', '', '60', '0', '10', '18', '8', '4', '3', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101138', 'EXP_Satchel', '27', 'Satchel Charge', 'The standard Satchel Charge packs a big bang since it is loaded with high explosives. Use it to blast enemies  out of buildings and bunkers.', '0, 0, 0', '', 'grenade', 'SatchelCharge', '', '', '250', '0', '15', '14', '8', '12', '3', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101139', 'EXP_Claymore', '27', 'Claymore Mine', 'The Claymore Anti-Personnel Mine is a place and forget A.P. mine designed to kill your enemy with a high explosive blast that fires thousands of steel shot.', '0, 0, 0', '', 'mine', 'ClaymoreMine', '', '', '300', '0', '10', '15', '8', '12', '360', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101140', 'EXP_VS50', '27', 'VS50 Mine', 'The VS50 is a small Anti-Personnel high explosive mine designed to maim and kill enemy soldiers. It has a 10 meter blast range.', '0, 0, 0', '', 'mine', 'RegularMine', '', '', '350', '0', '10', '15', '8', '8', '360', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101141', 'EXP_Valmara69', '27', 'V. 69 Mine', 'The V. 69 Mine is a anti-personnel mine that springs up to about a waist height level before it detonates with a high explosive blast.', '0, 0, 0', '', 'mine', 'RegularMine', '', '', '300', '0', '10', '15', '8', '8', '360', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101142', 'EXP_TripWire', '27', 'Trip Wire Booby Trap ', 'The Trip Wire Booby Trap is a high explosive, fragmentation mine similar to the POMZ stake mine. It is set with a trip wire that causes the detonation,', '0, 0, 0', '', 'mine', 'ClaymoreMine', '', '', '160', '0', '10', '15', '8', '6', '360', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101147', 'Exp_SmokeG_Red', '27', 'Red Smoke Grenade', 'A Red Smoke Grenade can be used for marking a location or obscuring you from your enemy\'s sight.', '0, 0, 0', 'muzzle_asr', 'grenade', 'smoke_grenade_red', '', '', '60', '0', '0', '18', '8', '1', '3', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101148', 'Exp_SmokeG_Blue', '27', 'Blue Smoke Grenade', 'A Blue Smoke Grenade can be used for marking a location or obscuring you from your enemy\'s sight.', '0, 0, 0', 'muzzle_asr', 'grenade', 'smoke_grenade_blue', '', '', '60', '0', '0', '18', '8', '1', '3', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101149', 'Exp_SmokeG_Green', '27', 'Green Smoke Grenade', 'A Green Smoke Grenade can be used for marking a location or obscuring you from your enemy\'s sight.', '0, 0, 0', 'muzzle_asr', 'grenade', 'smoke_grenade_green', '', '', '60', '0', '0', '18', '8', '1', '3', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101150', 'Exp_SmokeG_Orange', '27', 'Orange Smoke Grenade', 'A Orange Smoke Grenade can be used for marking a location or obscuring you from your enemy\'s sight.', '0, 0, 0', 'muzzle_asr', 'grenade', 'smoke_grenade_orange', '', '', '60', '0', '0', '18', '8', '1', '3', '0', '1', '2', '1.5', '0.2', '60', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101158', 'SHG_Mossberg590_01', '22', 'MOSSBERG 590', 'The Mossberg 590 is a 12 gauge shotgun which has a 8 round storage tube. It is known to do a extremely high amount of damage at close range.', '0, 0, 0', 'muzzle_asr', 'assault', 'buckshot', 'Sounds/NewWeapons/Sniper/0.308caliber', 'Sounds/Weapons/SHOTGUN/SHG_Generic_Reload', '24', '1', '15', '300', '12', '0', '0', '0', '3', '7', '8', '1.43', '60', '4', '12', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '0', '0', '0', '20100', '10100', '0', '0', '400082', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101169', 'ASR_Mossada', '20', 'MASADA', 'The Masada was designed for close combat operations and is a gas operated assault rifle that fires a 5.45 x 39 mm round. It is highly effective at close ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/AEK971', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '28', '1', '10', '300', '80', '0', '0', '0', '4', '30', '3', '0.43', '700', '1.3', '5.0025', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '51', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400119', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101171', 'ASR_HKG11', '20', 'GX11', 'The GX11 is a gas operarted, rotary breech assault rifle that fires a special 4.73 x 33 mm ammunition. It is highly effective at close ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_G11', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '35', '1', '10', '300', '90', '0', '0', '0', '4', '50', '2.5', '0.43', '1200', '1.35', '11', '0', 'asr_grenade', '010', '0', 'default', '0', '0', '0', '0', '1', '0', '51', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '0', '0', '30100', '20100', '10100', '0', '400014', '400131', '0', '0', '400132', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101172', 'ASR_SIG_SG556', '20', 'SIG SAUER 556', 'The 556 is a gas operated, rotating bolt assault rifle that fires a 5.56 x 45 mm round and was designed for special operations. It is very effective out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/SIG_551', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '27', '1', '10', '300', '75', '0', '0', '0', '4', '30', '2.5', '0.43', '650', '1.53', '6.50325', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '21', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400025', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101173', 'ASR_tar21', '20', 'IMI TAR-21', 'The TAR-21 is an unique gas operated, rotating bolt, bullpup design assault rifle that fires a 5.56 x 45 mm round. It is extremely effective at close and medium range.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_G11', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '33', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '700', '1.7', '7.0035', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '41', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '0', '0', '4001', '30100', '20100', '10100', '0', '400014', '400059', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101180', 'HG_Desert_Eagle', '25', 'Desert Eagle', 'The Desert Eagle handgun was designed to fire a .50 caliber round and deals a substancial amount of damage to targets at close ranges.', '0, 0, 0', 'muzzle_hg', 'pistol', '5.45', 'Sounds/NewWeapons/Handgun/DesertEagle', 'Sounds/Weapons/HG/HG_Generic_Reload', '50', '1', '10', '300', '40', '0', '0', '0', '4', '10', '2.5', '0.43', '800', '3', '22', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '45', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '400086', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101183', 'SHG_Keltech', '22', 'KT DECIDER', 'The KT Decider is a bull-pup shotgun design that has two magazine feed tubes, each side capable of holding 7 shells, for a total capacity of 14+1.', '0, 0, 0', 'muzzle_asr', 'assault', 'buckshot', 'Sounds/NewWeapons/Shotgun/Shotgun', 'Sounds/Weapons/SHOTGUN/SHG_Generic_Reload', '25', '1', '10', '300', '15', '0', '0', '0', '3', '10', '8', '1.43', '50', '4', '8', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '45', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '0', '0', '20100', '10100', '0', '0', '400036', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101189', 'SUP_M202Flash', '24', 'M202', 'The M202 is a multiple launcher that was designed to fire 4 66 mm rockets from a single launcher. The rockets are very effective against targets out to medium ranges.', '0, 0, 0', 'muzzle_rpg', 'assault', 'Rocket_M202', 'Sounds/Weapons/SUPPORT/SUP_Generic_Shot', '', '170', '0', '0.15', '40', '500', '5', '0', '30', '1', '4', '4', '2.43', '60', '2', '10', '0', 'asr_grenade', '100', '0', 'rpg8', '20', '0', '0', '0', '1', '0', '30', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '400120', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101191', 'ASR_Beretta_ARX160', '20', 'AR 2600', 'The RX 2600 was designed with a gas operated, rotating bolt to deliver a solid fire rate of 5.56 x 45 mm rounds effectively on targets at close and medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.56', 'Sounds/NewWeapons/Assault/FN_FAL', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '34', '1', '10', '300', '75', '0', '0', '0', '4', '30', '3', '0.43', '650', '1.44', '8.004', '0', 'asr_grenade', '011', '0', 'default', '0', '0', '0', '0', '1', '0', '51', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400126', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101193', 'ASR_Fn_scar', '20', 'FN SCAR CQC', 'The SCAR 16 CQC is an advanced assault rifle that fires standard 5.56 x 45 mm rounds effectively out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.56', 'Sounds/NewWeapons/Assault/FN_SCAR', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '30', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '1.5', '625', '2.16', '3.0015', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '37', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400006', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101196', 'HG_Jericho', '25', 'Jericho ACP', 'The Jericho ACP  is a handgun designed to fire a .45 ACP round from a 10 round magazine. It is extremely effective at close ranges.', '0, 0, 0', 'muzzle_asr', 'pistol', '5.45', 'Sounds/NewWeapons/Handgun/0.45Generic', 'Sounds/Weapons/HG/HG_Generic_Reload', '40', '1', '10', '300', '35', '0', '0', '0', '4', '10', '2.5', '0.43', '800', '2', '3', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '35', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '3002', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '400071', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101197', 'MG_G36MG', '23', 'RA H23', 'The RA LMG36 was designed to fire a 7.62 mm standard round. This machine gun delivers a reasonable amount of accurate fire onto targets at medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_G36_CMag', 'Sounds/Weapons/MG/MG_Generic_Reload', '31', '1', '10', '300', '90', '0', '0', '0', '3', '100', '4', '2.43', '650', '1.52', '5', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '49', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '0', '30100', '20100', '10100', '0', '0', '400042', '0', '0', '400017', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101200', 'SHG_Usas12', '22', 'USS-12', 'The USS-12 gauge automatic shotgun was designed to fire 12 gauge rounds from a 20 round storage system. It causes a reasonable amount of damage per round.', '0, 0, 0', 'muzzle_asr', 'assault', 'buckshot', 'Sounds/NewWeapons/Shotgun/Shotgun', 'Sounds/Weapons/SHOTGUN/SHG_Generic_Reload', '22', '1', '10', '300', '10', '0', '0', '0', '2', '20', '3.5', '1.43', '150', '6', '15', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '45', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '0', '0', '20100', '10100', '0', '0', '400121', '0', '0', '400050', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101201', 'SMG_SR-1_Veresk', '26', 'VERESK SR-2', 'The MTAC Flasher submachine gun was designed to fire a standard 9 mm round at a high rate of fire with reasonable accuracy at close range targets.', '0, 0, 0', 'muzzle_asr', 'assault', '9mm', 'Sounds/NewWeapons/Assault/Kalashnikov_AK74', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '23', '1', '10', '300', '35', '0', '0', '0', '3', '30', '2.5', '0.43', '800', '2.8392', '5.6', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '0', '4006', '30100', '20100', '10100', '0', '400014', '400083', '0', '0', '400084', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101202', 'SUP_ChinaQLB06', '24', 'QLB 06', 'The QLB06 is a semiautomatic grenade launcher that fires a 25 mm grenade from a rotating cylinder of 6 grenades. It is effective out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', 'Rocket_RG6', 'Sounds/NewWeapons/Special/40mmGrenadeLauncher', 'Sounds/Weapons/SUPPORT/QLB_Reload', '150', '0', '1.15', '50', '500', '4', '0', '30', '2', '6', '4', '2.43', '60', '2', '10', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '36', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '4007', '0', '0', '0', '0', '0', '0', '0', '400078', '400063', '0', '0', '400072', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101210', 'ASR_Fn_scar_NightCamo', '20', 'FN SCAR NIGHT STALKER', 'The Scar 17 CQC Nightstalker is a assault rifle that fires standard 5.56 x 45 mm rounds effectively out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/FN_SCAR', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '32', '1', '10', '300', '90', '0', '0', '0', '4', '30', '2.5', '0.43', '625', '2', '3.0015', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400006', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101214', 'MG_M249_SAW_Elite', '23', 'M249 ELITE', 'The M249 Elite special edition machine gun fires a 5.56 mm round from a belt fed system. It is modified to carry more rounds and deal higher damage.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/LightMachineGun/FN_M249', 'Sounds/Weapons/MG/MG_Generic_Reload', '32', '1', '18', '400', '90', '0', '0', '0', '3', '120', '6', '2.43', '625', '1.52', '5', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '1001', '0', '30100', '20100', '10100', '0', '0', '400035', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101215', 'ASR_SIG516_Elite', '20', 'SIG SAUER 516 ELITE', 'The 516 Elite is a more powerful version of the standard 516 with a telescopic sight. It has a special camo pattern as well as an extended clip.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_416', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '36', '1', '10', '300', '90', '0', '0', '0', '4', '30', '2.5', '0.43', '660', '1.6', '6', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400058', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101216', 'SUP_RPG7_Elite', '24', 'RPG-7 elite', 'The RPG 7 Elite is an updated rocket launcher that fires an upgraded 40 mm high explosive round. It is effective against targets at medium ranges.', '0, 0, 0', 'muzzle_rpg', 'rpg', 'rocket', 'Sounds/NewWeapons/Special/AT4RocketLauncher', 'Sounds/Weapons/SUPPORT/SUP_Generic_Reload', '220', '0', '0.15', '50', '500', '7', '0', '0', '2', '1', '4', '2.43', '40', '2', '10', '0', 'asr_grenade', '100', '0', 'rpg8', '40', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '5002', '0', '0', '0', '0', '0', '0', '0', '0', '400045', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101217', 'SNP_AW_Des', '21', 'MAUSER SRG SPECOPS', 'The Mauser SRG Desert sniper rifle fires a .300 magnum round. It\'s enhanced features include a larger clip size, lower recoil, and better scope magnification.', '0, 0, 0', 'muzzle_asr', 'assault', 'Sniper5.45', 'Sounds/NewWeapons/Sniper/0.308caliber', 'Sounds/Weapons/SNP/SNP_Generic_Reload', '110', '0', '1', '600', '600', '0', '0', '0', '4', '5', '3.5', '1.43', '60', '3', '3', '0', 'asr_grenade', '100', '0', 'sv98', '90', '0', '0', '0', '1', '0', '34', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '6001', '0', '0', '0', '30100', '20100', '10100', '0', '400076', '400008', '0', '0', '400070', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101218', 'ASR_G36_Elite', '20', 'G36 ELITE', 'The X36 Elite is a special issue version of the standard X36 assault rifle which fires a 5.56 x 45 mm round and is effectively out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_G36', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '31', '1', '10', '300', '90', '0', '0', '0', '4', '30', '2.5', '0.43', '660', '1.5', '7', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4100', '30100', '20100', '10100', '0', '400014', '400042', '0', '0', '400029', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101219', 'ASR_Fn_scar_Elite', '20', 'FN SCAR CQC ELITE', 'The Scar 16 Elite is a Veteran Edition assault rifle that fires 5.56 x 45 mm rounds very effectively out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/FN_SCAR', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '32', '1', '10', '300', '90', '0', '0', '0', '4', '30', '2.5', '0.43', '625', '2', '3', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400006', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101221', 'HG_Desert_Eagle_Elite', '25', 'Desert Eagle Elite', 'The Desert Eagle Elite handgun is a .50 caliber handgun that packs a 10 round clip. This limited edition weapon delivers devastating damage on targets at close range.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Handgun/DesertEagle', 'Sounds/Weapons/HG/HG_Generic_Reload', '50', '1', '10', '300', '50', '0', '0', '0', '4', '10', '2.5', '0.43', '800', '3', '15', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '30100', '20100', '10100', '0', '0', '0', '0', '0', '400086', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101224', 'HG_Jericho', '25', 'Jericho Magnum', 'The STI Eagle Elite handgun is an upgraded version of the STI Eagle and is designed to fire a .45 ACP round effectively at close ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Handgun/DesertEagle', 'Sounds/Weapons/HG/HG_Generic_Reload', '36', '1', '10', '300', '45', '0', '0', '0', '4', '10', '2.5', '0.43', '800', '2', '9', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101227', 'SMG_Bizon_Elite', '26', 'BIZON ELITE', 'The Bizon Elite submachine gun was upgraded in its design to fire a 9 mm round from a 64 round magazine delivering accurate fire on close range targets.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/SMG/Bizon', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '27', '1', '10', '300', '35', '0', '0', '0', '3', '64', '2.5', '0.43', '900', '2.5', '8.6', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '0', '0', '30100', '20100', '10100', '0', '400014', '400052', '0', '0', '400047', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101228', 'SMG_MP5A4_Vet', '26', 'MP5 ELITE', 'The RA X5 Elite submachine gun was designed to fire a 9 mm rounds from a 40 round clip effectively against close ranged targets.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/SMG/HK_MP5', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '27', '1', '10', '300', '35', '0', '0', '0', '3', '30', '2.5', '0.43', '840', '2.8392', '5.6', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101229', 'SMG_P90_Vet', '26', 'FN P90 ELITE', 'The FN P90 Elite submachine gun is a highly effective weapon that carries a unique 60 round magazine. It fires a standard 5.56 mm round.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/SMG/FN_P90', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '27', '1', '10', '300', '35', '0', '0', '0', '3', '50', '2.5', '0.43', '960', '2.8392', '5.6', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101230', 'SMG_ScorpionEVO3_Elite', '26', 'EVO-3 ELITE', 'The EVO-3 Elite submachine gun was designed to effectively fire 9 mm rounds from a 40 round magazine at close ranged targets at a very high rate of fire..', '0, 0, 0', 'muzzle_asr', 'smg', '5.45', 'Sounds/NewWeapons/SMG/Scorpion_EVO3', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '27', '1', '10', '300', '35', '0', '0', '0', '3', '30', '2.5', '0.43', '960', '2.8392', '5.6', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101232', 'SNP_AW_Elite', '21', 'MAUSER SRG ELITE', 'The Mauser SRG Elitet sniper rifle is a Veterans special edition of the standard SRG. It fires a .300 magnum round very effectively at long ranges.', '0, 0, 0', 'muzzle_asr', 'assault', 'Sniper5.45', 'Sounds/NewWeapons/Sniper/0.308caliber', 'Sounds/Weapons/SNP/SNP_Generic_Reload', '120', '0', '2', '600', '800', '0', '0', '0', '4', '5', '3.5', '1.43', '60', '3', '3', '0', 'asr_grenade', '100', '0', 'aw50', '50', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '6001', '0', '0', '0', '30100', '20100', '10100', '0', '400076', '400008', '0', '0', '400070', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101233', 'SUP_M202Flash_Vet', '24', 'M202 Elite', 'The M202 Elite is an upgraded multiple launcher that was designed to fire 4 66 mm rockets from a single launcher. The upgraded rockets are highly effective.', '0, 0, 0', 'muzzle_rpg', 'assault', 'Rocket_M202', 'Sounds/Weapons/SUPPORT/SUP_Generic_Shot', 'Sounds/Weapons/SUPPORT/SUP_Generic_Reload', '200', '0', '0.15', '45', '500', '7', '0', '0', '1', '4', '4', '2.43', '60', '2', '10', '0', 'asr_grenade', '100', '0', 'rpg8', '0', '0', '0', '0', '1', '0', '100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101234', 'SUP_RPO_Elite', '24', 'RPO-2 Elite', 'The RPO-2 Elite rocket launcher was redesigned to fire an upgraded 93 mm rocket armed with a thermobaric warhead capable on medium ranged targets.', '0, 0, 0', 'muzzle_rpg', 'rpg', 'Rocket_RPO', 'Sounds/NewWeapons/Special/RPO2', 'Sounds/Weapons/SUPPORT/SUP_Generic_Reload', '300', '0', '0.15', '45', '500', '8', '0', '0', '2', '1', '4', '2.43', '30', '2', '10', '0', 'asr_grenade', '100', '0', 'rpg8', '0', '0', '0', '0', '1', '0', '100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101238', 'SHG_Saiga_Vet', '22', 'SAIGA ELITE', 'The Saiga Elite shotgun is a limited veteran edition shotgun that fires a 12 gauge round. It delivers devastating fire on targets at close ranges.', '0, 0, 0', 'muzzle_sh1', 'assault', 'buckshot', 'Sounds/NewWeapons/Shotgun/Shotgun', 'Sounds/Weapons/SHOTGUN/SHG_Generic_Reload', '28', '1', '10', '300', '10', '0', '0', '0', '3', '10', '3.5', '1.43', '120', '3', '8', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '100', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101242', 'ASR_tar21_Impulse', '20', 'IMI TAR-21 ELITE', 'The Elite IMI TAR-21 is a bullpup design assault rifle that fires a 5.56 x 45 mm round. It is an extremely effective weapon that had limited release.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_G11', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '34', '1', '10', '300', '90', '0', '0', '0', '4', '30', '2.5', '0.43', '700', '1.5', '8.004', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '0', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101244', 'HG_Colt1911', '25', '1911 ', 'The 1911 handgun was designed to fire a .45 caliber round from a 7 round magazine. It is an effective weapon for engauging close range targets.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Handgun/Glock9mm', 'Sounds/Weapons/HG/HG_Generic_Reload', '38', '1', '10', '300', '20', '0', '0', '0', '4', '7', '2.5', '2', '800', '1.6', '3', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '15', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101245', 'SHG_Mossberg590_01_Elite', '22', 'MOSSBERG 590 ELITE', 'The Mossberg 590 Elite shotgun is a.12 gauge shotgun like the standard 590 but it causes more damage at an extended range.', '0, 0, 0', 'muzzle_asr', 'assault', 'buckshot', 'Sounds/NewWeapons/Sniper/0.308caliber', 'Sounds/Weapons/SHOTGUN/SHG_Generic_Reload', '24', '1', '10', '300', '15', '0', '0', '0', '3', '8', '8', '2', '60', '4', '10', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '5001', '3001', '0', '0', '0', '20100', '10100', '0', '0', '400082', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101246', 'SMG_PS90', '26', 'FN P90 S', 'The FN P90S submachine gun is a highly effective weapon that carries a unique 50 round magazine. It fires a standard 5.56 mm round.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/SMG/FN_P90', 'Sounds/Weapons/SMG/SMG_Generic_Reload', '26', '1', '10', '300', '40', '0', '0', '0', '3', '50', '2.5', '2', '900', '2.47', '8', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '58', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '0', '0', '30100', '20100', '10100', '0', '400014', '400054', '0', '0', '400046', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101247', 'SNP_Blaser_LRS2_01', '21', 'BLASER TACTICAL', 'The Blaser is a bolt action sniper rifle, designed to fire a 7.62 mm round from a 5 round clip. It is a highly accurate weapon at longer ranges.', '0, 0, 0', 'muzzle_asr', 'assault', 'Sniper5.45', 'Sounds/NewWeapons/Sniper/SV98', 'Sounds/Weapons/SNP/SNP_Generic_Reload', '125', '0', '1.6', '600', '1000', '0', '0', '0', '4', '5', '3.5', '2', '45', '3', '2', '0', 'asr_grenade', '100', '0', 'psg1', '70', '0', '0', '0', '1', '0', '42', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '6001', '0', '0', '0', '30100', '20100', '10100', '0', '0', '400008', '0', '0', '400070', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101253', 'Item_LLDR', '28', 'SOFLAM', 'A Laser Designator device that can be used to call in air and artillery support.', '0, 0, 0', '', 'assault', '5.45', '', '', '0', '1', '10', '300', '0', '0', '0', '0', '1', '0', '4', '2', '0', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '10', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101254', 'Item_Cypher2', '28', 'MAV', 'A portable UAV drone that can be remotely piloted and used to detect and mark enemy units.', '0, 0, 0', '', 'assault', '5.45', '', '', '0', '1', '10', '300', '0', '0', '0', '0', '1', '0', '4', '2', '0', '0', '0', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '28', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101255', 'Item_Cypher2', '28', 'Temp Usable Item', '', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '20', '1', '10', '300', '50', '0', '0', '0', '2', '10', '4', '2', '10', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101256', 'Item_Medkit_01', '28', 'Medkit', 'A standard medical field kit that allows you to heal wounded team mates.  Drop the medkit in front of you to heal wounded friendly players that stand near it.', '0, 0, 0', 'muzzle_asr', 'mine', '5.45', '', '', '20', '1', '10', '300', '50', '0', '0', '0', '1', '0', '4', '2', '10', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101257', 'Item_MotionSensor_01', '28', 'Motion Sensor', 'A deployable motion sensor that reveals the location of enemies that enter within its discovery radius onto your mini-map.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '30', '1', '10', '300', '50', '0', '0', '0', '1', '0', '4', '2', '10', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101258', 'Item_RespawnBeacon_01', '28', 'Respawn Beacon', 'A small hand-held device that when placed, allows you and other friendly players to spawn on its location.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '20', '1', '10', '300', '50', '0', '0', '0', '1', '0', '4', '2', '10', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101259', 'Item_GunTurret_Crate_01', '28', 'Auto-Turret', 'An automated turret with excellent stopping power that can be deployed to attack nearby enemy players.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/ColtM16', '', '15', '1', '10', '300', '50', '0', '0', '250', '1', '0', '45', '45', '1', '5', '550', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101260', 'Item_Riot_Shield_Crate_01', '28', 'Tac-Shield', 'A deployable tactical shield that is placed at your feet and can be used for cover.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '2000', '1', '10', '300', '50', '0', '0', '0', '1', '0', '4', '2', '2', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101261', 'Item_bandage_01', '28', 'Bandages ', 'Healing bandages that restore 25% of your health when used.&#xD;Sells in packs of 10.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '25', '1', '10', '300', '50', '0', '0', '0', '10', '10', '4', '2', '20', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101262', 'Item_bandage_02', '28', 'Bandages DX', 'Bandages Deluxe. These are premium bandages that restore 45% of your health instantly!&#xD;Sells in packs of 10.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '45', '1', '10', '300', '50', '0', '0', '0', '10', '10', '4', '2', '30', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101263', 'Item_GunTurret_Crate_01', '28', 'Turret DX', 'An automated turret with moderate stopping power that can be deployed to attack nearby enemy players.&#xD;Sells in packs of 3.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/ColtM16', '', '15', '1', '10', '300', '50', '0', '0', '250', '10', '3', '45', '45', '10', '1', '550', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101264', 'Item_MotionSensor_01', '28', 'MSensor DX', 'A deployable motion sensor that reveals the location of enemies that enter within its discovery radius onto your mini-map. Sells in packs of 5.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '15', '1', '10', '150', '50', '0', '0', '0', '10', '5', '4', '2', '10', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101265', 'Item_RespawnBeacon_01', '28', 'Respawn DX', 'A small hand-held device that when placed, allows you spawn on its location and is destroyed after use.&#xD;Sells in packs of 5.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '20', '1', '10', '300', '50', '0', '0', '0', '10', '5', '4', '2', '10', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101266', 'Item_Riot_Shield_Crate_02', '28', 'Shield DX', 'A deployable shield that is placed at your feet and can be used for cover. &#xD;Sells in packs of 5.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '600', '1', '10', '400', '50', '0', '0', '0', '10', '5', '4', '2', '14', '20', '20', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101267', 'MEL_Knife_01', '29', 'V-27 Combat Knife', 'A standard issue knife with an 8&quot; fixed blade, this knife was first designed in 1985 for the United States Army and was later adapted by several other countries as well as mercenary groups throughout the world due to its sturdy design and stopping pow', '0, 0, 0', '', 'melee', 'melee', '', '', '80', '1', '10', '300', '1', '0', '0', '0', '1', '1', '0', '0', '120', '0', '0', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101268', 'ASR_AKM_Turkey', '20', 'AKM Turkey', 'The AKM is an assault rifle that was designed as a simplified version of the AK-47 rifle. It fires a 7.62 x 39 mm round and is relatively effective at medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '7.62', 'Sounds/NewWeapons/Assault/Kalashnikov_AKM', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '34', '1', '10', '300', '85', '0', '0', '0', '4', '30', '3', '0.43', '600', '2.2', '10.005', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400128', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101269', 'ASR_An_94_Netherlands', '20', 'AN 94 Netherlands', 'The AN 94 is an advanced assault rifle that functions on a gas operated system, firing a 5.45 x 39mm round that does a large amount of damage on impact.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/Kalashnikov_AK108', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '33', '1', '12', '320', '80', '0', '0', '0', '4', '30', '3', '0.43', '1000', '1.5', '11.0055', '0', 'asr_grenade', '010', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400062', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101270', 'ASR_Famas_french', '20', 'FAMAS F1 French', 'The Famas F1 assault rifle fires a standard 5.56mm round using a delayed blowback system. It delivers highly effective fire out of medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/Famas_F1', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '28', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '900', '1.3', '11.0055', '0', 'asr_grenade', '011', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400041', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101271', 'ASR_Fn_scar_Aus', '20', 'FN SCAR CQC Australia', 'The FN SCAR 16 CQC is an advanced assault rifle that fires standard 5.56 x 45 mm rounds effectively out to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.56', 'Sounds/NewWeapons/Assault/FN_SCAR', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '30', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '1.5', '625', '2.16', '3.0015', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400006', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101272', 'ASR_G36_Canada', '20', 'G36 Canada', 'The MTAC X36 assault rifle fires a 5.56mm round.  Its high rate of fire and medium recoil deliver a reasonably tight spread on impact of targets up to medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_G36', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '30', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '650', '1.5', '8.004', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4100', '30100', '20100', '10100', '0', '400014', '400042', '0', '0', '400029', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101273', 'ASR_L85_UK', '20', 'LS 90 UK', 'The LS 90 is a variant assault weapon design that fires a 5.56 x 45 mm round using a gas-operated rotating bolt system.  It is extremely accurate at medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/XM8', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '30', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '650', '2.1', '5.0025', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400064', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101274', 'ASR_M4FFH_USA', '20', 'BLACKWATER M4 USA', 'The Blackwater M4 is a versitile, light weight, commando style assault rifle that is gas operated and fires a 5.56 x 45 mm round effectively in both close and medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/ColtM4', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '34', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '625', '1.5', '8.004', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400024', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101275', 'ASR_Mossada_Italy', '20', 'MASADA ITALY', 'The Masada was designed for close combat operations and is a gas operated assault rifle that fires a 5.45 x 39 mm round.  It is highly effective at close ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/AEK971', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '28', '1', '10', '300', '80', '0', '0', '0', '4', '30', '3', '0.43', '700', '1.3', '5.0025', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400119', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101276', 'ASR_Sig516_German', '20', 'SIG SAUER 516 Germany', 'The 516 Tactical assault rifle fires a 5.56 x 45 mm round using a short stroke piston system.  It has a good rate of fire and is reasonably effective on medium range targets.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/HK_416', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '35', '1', '10', '300', '80', '0', '0', '0', '4', '30', '2.5', '0.43', '650', '1.71', '6.003', '0', 'asr_grenade', '111', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4001', '30100', '20100', '10100', '0', '400014', '400058', '0', '0', '400016', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101277', 'ASR_AK74_Brazil', '20', 'AK-74M Brazil', 'The AK-74M assault rifle is an upgrade of the AK-74 design with a slightly increased rate of fire. It fires a 5.45 x 39 mm round accurately at medium ranges.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', 'Sounds/NewWeapons/Assault/Kalashnikov_AK74', 'Sounds/Weapons/ASR/ASR_Generic_Reload', '32', '1', '10', '300', '75', '0', '0', '0', '4', '30', '3', '0.43', '625', '2.1', '8.004', '0', 'asr_grenade', '101', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '2001', '5001', '3001', '1001', '4200', '30100', '20100', '10100', '0', '400014', '400040', '0', '0', '400001', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101278', 'MEL_BaseballBat_01', '29', 'Ol\' Slugger', 'Handy for those times when dead mercenaries suddenly rise again...', '0, 0, 0', '', 'melee', 'melee', '', '', '90', '1', '10', '300', '1', '0', '0', '0', '1', '1', '0', '0', '90', '0', '0', '0', 'asr_grenade', '001', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101279', 'Item_Syringe_01_Adrenaline', '28', 'Adrenaline Shot', 'A shot of Adrenaline that once introduced to your blood stream increases your run and sprint speed by 10% for 15 seconds.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '10', '1', '10', '300', '50', '0', '0', '0', '1', '0', '4', '2', '1', '20', '20', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101280', 'Item_Syringe_01_Adrenaline', '28', 'Speed DX', 'A shot of Epinephrine that once introduced to your blood stream increases your run and sprint speed by 30% for 5 seconds. Sells in packs of 10.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '30', '1', '10', '300', '50', '0', '0', '0', '10', '10', '4', '2', '3', '20', '20', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101281', 'Item_Syringe_01_Morphene', '28', 'Morphine Shot', 'A shot of morphine that once introduced to your blood stream reduces damage taken by 25% for 15 seconds.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '25', '1', '10', '300', '50', '0', '0', '0', '1', '0', '4', '2', '1', '20', '20', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');
INSERT INTO `items_weapons` VALUES ('101282', 'Item_Syringe_01_Morphene', '28', 'Armor DX', 'A shot of Methylmorphine that once introduced to your blood stream reduces damage taken by 50% for 5 seconds. Sells in packs of 10.', '0, 0, 0', 'muzzle_asr', 'assault', '5.45', '', '', '50', '1', '10', '300', '50', '0', '0', '0', '10', '10', '4', '2', '3', '20', '20', '0', 'asr_grenade', '100', '0', 'default', '0', '0', '0', '0', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '1', '1', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '');

-- ----------------------------
-- Table structure for `leaderboard`
-- ----------------------------
DROP TABLE IF EXISTS `leaderboard`;
CREATE TABLE `leaderboard` (
  `Pos` int(10) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) NOT NULL,
  `gamertag` varchar(32) NOT NULL,
  `Rank` int(10) NOT NULL,
  `HonorPoints` int(10) NOT NULL,
  `Wins` int(10) NOT NULL,
  `Losses` int(10) NOT NULL,
  `Kills` int(10) NOT NULL,
  `Deaths` int(10) NOT NULL,
  `ShotsFired` int(10) NOT NULL,
  `ShotsHit` int(10) NOT NULL,
  `TimePlayed` int(10) NOT NULL,
  `HavePremium` int(10) NOT NULL,
  PRIMARY KEY (`Pos`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of leaderboard
-- ----------------------------

-- ----------------------------
-- Table structure for `leaderboard1`
-- ----------------------------
DROP TABLE IF EXISTS `leaderboard1`;
CREATE TABLE `leaderboard1` (
  `Pos` int(10) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) NOT NULL,
  `gamertag` varchar(32) NOT NULL,
  `Rank` int(10) NOT NULL,
  `HonorPoints` int(10) NOT NULL,
  `Wins` int(10) NOT NULL,
  `Losses` int(10) NOT NULL,
  `Kills` int(10) NOT NULL,
  `Deaths` int(10) NOT NULL,
  `ShotsFired` int(10) NOT NULL,
  `ShotsHit` int(10) NOT NULL,
  `TimePlayed` int(10) NOT NULL,
  `HavePremium` int(10) NOT NULL,
  PRIMARY KEY (`Pos`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of leaderboard1
-- ----------------------------

-- ----------------------------
-- Table structure for `leaderboard30`
-- ----------------------------
DROP TABLE IF EXISTS `leaderboard30`;
CREATE TABLE `leaderboard30` (
  `Pos` int(10) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) NOT NULL,
  `gamertag` varchar(32) NOT NULL,
  `Rank` int(10) NOT NULL,
  `HonorPoints` int(10) NOT NULL,
  `Wins` int(10) NOT NULL,
  `Losses` int(10) NOT NULL,
  `Kills` int(10) NOT NULL,
  `Deaths` int(10) NOT NULL,
  `ShotsFired` int(10) NOT NULL,
  `ShotsHit` int(10) NOT NULL,
  `TimePlayed` int(10) NOT NULL,
  `HavePremium` int(10) NOT NULL,
  PRIMARY KEY (`Pos`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of leaderboard30
-- ----------------------------

-- ----------------------------
-- Table structure for `leaderboard7`
-- ----------------------------
DROP TABLE IF EXISTS `leaderboard7`;
CREATE TABLE `leaderboard7` (
  `Pos` int(10) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) NOT NULL,
  `gamertag` varchar(32) NOT NULL,
  `Rank` int(10) NOT NULL,
  `HonorPoints` int(10) NOT NULL,
  `Wins` int(10) NOT NULL,
  `Losses` int(10) NOT NULL,
  `Kills` int(10) NOT NULL,
  `Deaths` int(10) NOT NULL,
  `ShotsFired` int(10) NOT NULL,
  `ShotsHit` int(10) NOT NULL,
  `TimePlayed` int(10) NOT NULL,
  `HavePremium` int(10) NOT NULL,
  PRIMARY KEY (`Pos`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of leaderboard7
-- ----------------------------

-- ----------------------------
-- Table structure for `loginid`
-- ----------------------------
DROP TABLE IF EXISTS `loginid`;
CREATE TABLE `loginid` (
  `CustomerID` int(10) NOT NULL AUTO_INCREMENT,
  `AccountName` varchar(32) NOT NULL,
  `Password` varchar(16) NOT NULL,
  `AccountStatus` int(10) NOT NULL,
  `GamePoints` int(10) NOT NULL,
  `HonorPoints` int(10) NOT NULL,
  `SkillPoints` int(10) NOT NULL,
  `Gamertag` varchar(32) NOT NULL,
  `dateregistered` datetime NOT NULL,
  `lastlogindate` datetime NOT NULL,
  `lastloginIP` varchar(16) NOT NULL,
  `lastgamedate` datetime NOT NULL,
  `ReferralID` int(10) NOT NULL,
  `lastjoineddate` datetime NOT NULL,
  `MD5Password` varchar(32) NOT NULL,
  `ClanID` int(10) NOT NULL,
  `GameDollars` int(10) NOT NULL,
  `Faction1Score` int(10) NOT NULL,
  `Faction2Score` int(10) NOT NULL,
  `Faction3Score` int(10) NOT NULL,
  `Faction4Score` int(10) NOT NULL,
  `Faction5Score` int(10) NOT NULL,
  `ClanRank` int(10) NOT NULL,
  `lastRetBonusDate` datetime DEFAULT NULL,
  `IsFPSEnabled` int(10) NOT NULL,
  `reg_sid` varchar(128) DEFAULT NULL,
  `ClanContributedXP` int(10) NOT NULL,
  `ClanContributedGP` int(10) NOT NULL,
  `IsDeveloper` int(10) NOT NULL,
  PRIMARY KEY (`CustomerID`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of loginid
-- ----------------------------

-- ----------------------------
-- Table structure for `logins`
-- ----------------------------
DROP TABLE IF EXISTS `logins`;
CREATE TABLE `logins` (
  `LoginID` int(10) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) NOT NULL,
  `LoginTime` datetime NOT NULL,
  `IP` varchar(16) NOT NULL,
  `LoginSource` int(10) NOT NULL,
  PRIMARY KEY (`LoginID`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of logins
-- ----------------------------

-- ----------------------------
-- Table structure for `loginsessions`
-- ----------------------------
DROP TABLE IF EXISTS `loginsessions`;
CREATE TABLE `loginsessions` (
  `CustomerID` int(10) NOT NULL,
  `SessionKey` varchar(50) NOT NULL,
  `SessionID` int(10) NOT NULL,
  `LoginIP` varchar(16) NOT NULL,
  `TimeLogged` datetime NOT NULL,
  `TimeUpdated` datetime NOT NULL,
  `GameSessionID` bigint(19) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of loginsessions
-- ----------------------------

-- ----------------------------
-- Table structure for `masterserverinfo`
-- ----------------------------
DROP TABLE IF EXISTS `masterserverinfo`;
CREATE TABLE `masterserverinfo` (
  `ServerID` int(10) NOT NULL,
  `LastUpdated` datetime NOT NULL,
  `CreateGameKey` int(10) NOT NULL,
  `IP` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of masterserverinfo
-- ----------------------------

-- ----------------------------
-- Table structure for `matomyusermap`
-- ----------------------------
DROP TABLE IF EXISTS `matomyusermap`;
CREATE TABLE `matomyusermap` (
  `CustomerID` int(10) NOT NULL,
  `ce_pub` varchar(32) DEFAULT NULL,
  `ce_cid` varchar(64) DEFAULT NULL,
  `DateRegistered` datetime DEFAULT NULL,
  `IsConverted` int(10) DEFAULT NULL,
  `DateConverted` datetime DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of matomyusermap
-- ----------------------------

-- ----------------------------
-- Table structure for `profiledata`
-- ----------------------------
DROP TABLE IF EXISTS `profiledata`;
CREATE TABLE `profiledata` (
  `CustomerID` int(10) NOT NULL,
  `Skills` char(10) NOT NULL,
  `Achievements` varchar(256) NOT NULL,
  `Abilities` varchar(256) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of profiledata
-- ----------------------------

-- ----------------------------
-- Table structure for `profile_chars`
-- ----------------------------
DROP TABLE IF EXISTS `profile_chars`;
CREATE TABLE `profile_chars` (
  `LoadoutID` int(10) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) NOT NULL,
  `Class` int(10) NOT NULL,
  `HonorPoints` int(10) NOT NULL,
  `TimePlayed` int(10) NOT NULL,
  `Loadout` varchar(256) NOT NULL,
  `Skills` char(31) NOT NULL,
  `SpendSP1` int(10) NOT NULL,
  `SpendSP2` int(10) NOT NULL,
  `SpendSP3` int(10) NOT NULL,
  PRIMARY KEY (`LoadoutID`)
) ENGINE=InnoDB AUTO_INCREMENT=24 DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of profile_chars
-- ----------------------------

-- ----------------------------
-- Table structure for `securitylog`
-- ----------------------------
DROP TABLE IF EXISTS `securitylog`;
CREATE TABLE `securitylog` (
  `ID` int(10) NOT NULL AUTO_INCREMENT,
  `EventID` int(10) NOT NULL,
  `Date` datetime NOT NULL,
  `IP` varchar(64) NOT NULL,
  `CustomerID` int(10) NOT NULL,
  `EventData` varchar(256) NOT NULL,
  PRIMARY KEY (`ID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of securitylog
-- ----------------------------

-- ----------------------------
-- Table structure for `stats`
-- ----------------------------
DROP TABLE IF EXISTS `stats`;
CREATE TABLE `stats` (
  `CustomerID` int(10) NOT NULL,
  `Kills` int(10) NOT NULL,
  `Deaths` int(10) NOT NULL,
  `ShotsFired` int(10) NOT NULL,
  `ShotsHits` int(10) NOT NULL,
  `Headshots` int(10) NOT NULL,
  `AssistKills` int(10) NOT NULL,
  `Wins` int(10) NOT NULL,
  `Losses` int(10) NOT NULL,
  `CaptureNeutralPoints` int(10) NOT NULL,
  `CaptureEnemyPoints` int(10) NOT NULL,
  `TimePlayed` int(10) NOT NULL,
  `CheatAttempts` int(10) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of stats
-- ----------------------------

-- ----------------------------
-- Table structure for `steamgporders`
-- ----------------------------
DROP TABLE IF EXISTS `steamgporders`;
CREATE TABLE `steamgporders` (
  `OrderID` bigint(19) NOT NULL AUTO_INCREMENT,
  `CustomerID` int(10) DEFAULT NULL,
  `SteamID` bigint(19) DEFAULT NULL,
  `InitTxnTime` datetime DEFAULT NULL,
  `Price` int(10) DEFAULT NULL,
  `GP` int(10) DEFAULT NULL,
  `ItemCode` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`OrderID`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of steamgporders
-- ----------------------------

-- ----------------------------
-- Table structure for `steamgpshop`
-- ----------------------------
DROP TABLE IF EXISTS `steamgpshop`;
CREATE TABLE `steamgpshop` (
  `SteamGPItemId` int(10) NOT NULL AUTO_INCREMENT,
  `IsEnabled` int(10) DEFAULT NULL,
  `GP` int(10) DEFAULT NULL,
  `BonusGP` int(10) DEFAULT NULL,
  `PriceCentsUSD` int(10) DEFAULT NULL,
  PRIMARY KEY (`SteamGPItemId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of steamgpshop
-- ----------------------------

-- ----------------------------
-- Table structure for `steamuseridmap`
-- ----------------------------
DROP TABLE IF EXISTS `steamuseridmap`;
CREATE TABLE `steamuseridmap` (
  `CustomerID` int(10) DEFAULT NULL,
  `SteamID` bigint(19) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of steamuseridmap
-- ----------------------------

-- ----------------------------
-- Table structure for `vitalstatsv2`
-- ----------------------------
DROP TABLE IF EXISTS `vitalstatsv2`;
CREATE TABLE `vitalstatsv2` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `UpdateTime` datetime NOT NULL,
  `TotalUsers` int(10) DEFAULT NULL,
  `NewUsers` int(10) DEFAULT NULL,
  `LoggedIn` int(10) DEFAULT NULL,
  `NonActive` int(10) DEFAULT NULL,
  `PlayedGame` int(10) DEFAULT NULL,
  `CCU` int(10) NOT NULL,
  `DAU` int(10) DEFAULT NULL,
  `MAU` int(10) DEFAULT NULL,
  `ActiveSessions` int(10) DEFAULT NULL,
  `CashNum` int(10) DEFAULT NULL,
  `CashTotal` float DEFAULT NULL,
  `GPNum` int(10) DEFAULT NULL,
  `GPTotal` int(10) DEFAULT NULL,
  `DAU1` int(10) DEFAULT NULL,
  `MAU1` int(10) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of vitalstatsv2
-- ----------------------------

-- ----------------------------
-- Table structure for `vitalstatsv3`
-- ----------------------------
DROP TABLE IF EXISTS `vitalstatsv3`;
CREATE TABLE `vitalstatsv3` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `UpdateTime` datetime NOT NULL,
  `TotalVisitors` int(10) DEFAULT NULL,
  `TotalUsersLogin` int(10) DEFAULT NULL,
  `TotalUsersActivated` int(10) DEFAULT NULL,
  `TotalUsersJoined` int(10) DEFAULT NULL,
  `TotalUsersPlayed` int(10) DEFAULT NULL,
  `NewRegistered` int(10) DEFAULT NULL,
  `NewActivated` int(10) DEFAULT NULL,
  `MAU_Login` int(10) DEFAULT NULL,
  `MAU_Joined` int(10) DEFAULT NULL,
  `MAU_Played` int(10) DEFAULT NULL,
  `DAU_Login` int(10) DEFAULT NULL,
  `DAU_Joined` int(10) DEFAULT NULL,
  `DAU_Played` int(10) DEFAULT NULL,
  `Cash24Num` int(10) DEFAULT NULL,
  `Cash24Total` float DEFAULT NULL,
  `Cash24Paying` int(10) DEFAULT NULL,
  `CashTotal` float DEFAULT NULL,
  `CashPaying` int(10) DEFAULT NULL,
  `ARPU` float DEFAULT NULL,
  `ARPPU` float DEFAULT NULL,
  `PlayedToday` int(10) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of vitalstatsv3
-- ----------------------------

-- ----------------------------
-- Table structure for `vitalstats_ccu`
-- ----------------------------
DROP TABLE IF EXISTS `vitalstats_ccu`;
CREATE TABLE `vitalstats_ccu` (
  `id` int(10) NOT NULL AUTO_INCREMENT,
  `UpdateTime` datetime NOT NULL,
  `CCU` int(10) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

-- ----------------------------
-- Records of vitalstats_ccu
-- ----------------------------
