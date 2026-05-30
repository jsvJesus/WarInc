CREATE DATABASE IF NOT EXISTS warnet
  CHARACTER SET utf8mb4
  COLLATE utf8mb4_unicode_ci;

USE warnet;

CREATE TABLE IF NOT EXISTS accounts (
  customer_id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  username VARCHAR(64) NOT NULL,
  email VARCHAR(190) NULL,
  password_hash VARCHAR(255) NOT NULL,
  account_status INT NOT NULL DEFAULT 100,
  is_developer TINYINT(1) NOT NULL DEFAULT 0,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  last_login_at DATETIME NULL,
  PRIMARY KEY (customer_id),
  UNIQUE KEY uq_accounts_username (username),
  UNIQUE KEY uq_accounts_email (email)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS login_sessions (
  session_id BIGINT UNSIGNED NOT NULL,
  customer_id BIGINT UNSIGNED NOT NULL,
  token_hash CHAR(64) NOT NULL,
  ip_address VARCHAR(64) NOT NULL,
  user_agent VARCHAR(255) NULL,
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  expires_at DATETIME NOT NULL,
  revoked_at DATETIME NULL,
  PRIMARY KEY (session_id),
  KEY ix_login_sessions_customer_id (customer_id),
  KEY ix_login_sessions_token_hash (token_hash),
  CONSTRAINT fk_login_sessions_account
    FOREIGN KEY (customer_id)
    REFERENCES accounts(customer_id)
    ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;