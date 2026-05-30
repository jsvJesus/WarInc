/*
Navicat MySQL Data Transfer

Source Server         : Home
Source Server Version : 110801
Source Host           : localhost:3307
Source Database       : warnet

Target Server Type    : MYSQL
Target Server Version : 110801
File Encoding         : 65001

Date: 2026-05-30 17:23:35
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `accounts`
-- ----------------------------
DROP TABLE IF EXISTS `accounts`;
CREATE TABLE `accounts` (
  `customer_id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(64) NOT NULL,
  `email` varchar(190) DEFAULT NULL,
  `password_hash` varchar(255) NOT NULL,
  `account_status` int(11) NOT NULL DEFAULT 100,
  `is_developer` tinyint(1) NOT NULL DEFAULT 0,
  `created_at` datetime NOT NULL DEFAULT current_timestamp(),
  `last_login_at` datetime DEFAULT NULL,
  PRIMARY KEY (`customer_id`),
  UNIQUE KEY `uq_accounts_username` (`username`),
  UNIQUE KEY `uq_accounts_email` (`email`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- ----------------------------
-- Records of accounts
-- ----------------------------
INSERT INTO `accounts` VALUES ('1', 'test', 'test@test.local', '$2a$11$59aOu.N18GYfCtuvhxd1du7RlAoG4.iwgctzuKWnG6Uh6T.J/VFJW', '100', '1', '2026-05-30 17:21:59', '2026-05-30 14:22:52');

-- ----------------------------
-- Table structure for `login_sessions`
-- ----------------------------
DROP TABLE IF EXISTS `login_sessions`;
CREATE TABLE `login_sessions` (
  `session_id` bigint(20) unsigned NOT NULL,
  `customer_id` bigint(20) unsigned NOT NULL,
  `token_hash` char(64) NOT NULL,
  `ip_address` varchar(64) NOT NULL,
  `user_agent` varchar(255) DEFAULT NULL,
  `created_at` datetime NOT NULL DEFAULT current_timestamp(),
  `expires_at` datetime NOT NULL,
  `revoked_at` datetime DEFAULT NULL,
  PRIMARY KEY (`session_id`),
  KEY `ix_login_sessions_customer_id` (`customer_id`),
  KEY `ix_login_sessions_token_hash` (`token_hash`),
  CONSTRAINT `fk_login_sessions_account` FOREIGN KEY (`customer_id`) REFERENCES `accounts` (`customer_id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_uca1400_ai_ci;

-- ----------------------------
-- Records of login_sessions
-- ----------------------------
INSERT INTO `login_sessions` VALUES ('3887033929976369843', '1', 'f1f926ed9d6f91bb366bf5080a6f5ddb91cafdf7a25e7daf64979a2a119c20f3', '127.0.0.1', 'Mozilla/5.0 (Windows NT; Windows NT 10.0; ru-RU) WindowsPowerShell/5.1.26100.8115', '2026-05-30 17:22:52', '2026-05-31 14:22:52', null);
INSERT INTO `login_sessions` VALUES ('12208611835981705217', '1', '91b5167119c8529a04b06741534ede5fcf291acdd075dcfc7569f87ee5348827', '127.0.0.1', 'Mozilla/5.0 (Windows NT; Windows NT 10.0; ru-RU) WindowsPowerShell/5.1.26100.8115', '2026-05-30 17:22:30', '2026-05-31 14:22:30', null);
