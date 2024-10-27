--------------------------------------------------------------------------------
-- Up
--------------------------------------------------------------------------------

CREATE TABLE users (
    user_id INT NOT NULL PRIMARY KEY,
    username TEXT NOT NULL
);

CREATE TABLE banned_users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    reason TEXT NOT NULL
);

CREATE TABLE moderators (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL
);

--------------------------------------------------------------------------------
-- Down
--------------------------------------------------------------------------------

DROP TABLE users;
DROP TABLE banned_users;
DROP TABLE moderators;