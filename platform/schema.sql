-- Import once into an existing Loopia MariaDB database. InnoDB is required.
CREATE TABLE platform_users (
 id BIGINT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
 email VARCHAR(254) CHARACTER SET ascii COLLATE ascii_general_ci NOT NULL UNIQUE,
 password_hash VARCHAR(255) NOT NULL,
 session_version INT UNSIGNED NOT NULL DEFAULT 1,
 created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE platform_password_resets (
 user_id BIGINT UNSIGNED PRIMARY KEY,
 token_hash CHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL UNIQUE,
 expires_at BIGINT UNSIGNED NOT NULL,
 FOREIGN KEY(user_id) REFERENCES platform_users(id) ON DELETE CASCADE
) ENGINE=InnoDB;

CREATE TABLE platform_games (
 id VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin PRIMARY KEY,
 title VARCHAR(160) NOT NULL,
 return_path VARCHAR(255) NOT NULL,
 max_save_bytes INT UNSIGNED NOT NULL DEFAULT 1048576,
 active BOOLEAN NOT NULL DEFAULT TRUE
) ENGINE=InnoDB;
INSERT INTO platform_games (id,title,return_path,max_save_bytes)
 VALUES ('paws','Paws','/games/paws/',16777216);

CREATE TABLE platform_saves (
 user_id BIGINT UNSIGNED NOT NULL,
 game_id VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
 revision INT UNSIGNED NOT NULL,
 format_version INT UNSIGNED NOT NULL,
 payload MEDIUMBLOB NOT NULL,
 updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
 PRIMARY KEY(user_id,game_id),
 FOREIGN KEY(user_id) REFERENCES platform_users(id) ON DELETE CASCADE,
 FOREIGN KEY(game_id) REFERENCES platform_games(id)
) ENGINE=InnoDB;

-- Products are durable, non-consumable unlocks. In-game earned coins stay local.
CREATE TABLE platform_products (
 game_id VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
 id VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
 title VARCHAR(160) NOT NULL,
 stripe_price VARCHAR(255) CHARACTER SET ascii NOT NULL,
 amount INT UNSIGNED NOT NULL,
 currency CHAR(3) CHARACTER SET ascii NOT NULL,
 active BOOLEAN NOT NULL DEFAULT FALSE,
 PRIMARY KEY(game_id,id),
 FOREIGN KEY(game_id) REFERENCES platform_games(id)
) ENGINE=InnoDB;

CREATE TABLE platform_purchases (
 id CHAR(32) CHARACTER SET ascii COLLATE ascii_bin PRIMARY KEY,
 user_id BIGINT UNSIGNED NOT NULL,
 game_id VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
 product_id VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
 request_key VARCHAR(64) CHARACTER SET ascii COLLATE ascii_bin NOT NULL,
 stripe_price VARCHAR(255) CHARACTER SET ascii NOT NULL,
 amount INT UNSIGNED NOT NULL,
 currency CHAR(3) CHARACTER SET ascii NOT NULL,
 return_path VARCHAR(255) NOT NULL,
 status VARCHAR(24) NOT NULL DEFAULT 'pending',
 stripe_session VARCHAR(255) CHARACTER SET ascii COLLATE ascii_bin NULL UNIQUE,
 payment_intent VARCHAR(255) CHARACTER SET ascii COLLATE ascii_bin NULL UNIQUE,
 checkout_url TEXT NULL,
 refunded_amount INT UNSIGNED NOT NULL DEFAULT 0,
 disputed BOOLEAN NOT NULL DEFAULT FALSE,
 created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
 updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
 UNIQUE(user_id,request_key),
 FOREIGN KEY(user_id) REFERENCES platform_users(id),
 FOREIGN KEY(game_id,product_id) REFERENCES platform_products(game_id,id)
) ENGINE=InnoDB;

CREATE TABLE platform_events (
 id VARCHAR(255) CHARACTER SET ascii COLLATE ascii_bin PRIMARY KEY,
 event_type VARCHAR(100) NOT NULL,
 received_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;

CREATE TABLE platform_rate_limits (
 bucket CHAR(64) CHARACTER SET ascii PRIMARY KEY,
 hits INT UNSIGNED NOT NULL,
 expires_at BIGINT UNSIGNED NOT NULL,
 INDEX(expires_at)
) ENGINE=InnoDB;
