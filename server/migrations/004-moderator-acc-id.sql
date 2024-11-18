-- Up

ALTER TABLE moderators ADD account_id INT;
ALTER TABLE moderators DROP COLUMN user_id;
ALTER TABLE banned_users DROP COLUMN user_id;

-- Down

-- ALTER TABLE moderators ADD user_id INT NOT NULL;
-- ALTER TABLE moderators DROP COLUMN account_id;
