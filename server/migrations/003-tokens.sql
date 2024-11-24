-- Up

DROP TABLE users;

CREATE TABLE users (
    account_id INT PRIMARY KEY NOT NULL,
    user_id INT NOT NULL,
    username TEXT NOT NULL,
    token TEXT NOT NULL
);

ALTER TABLE banned_users ADD account_id INT NOT NULL;

-- Down

DROP TABLE users;
ALTER TABLE banned_users DROP COLUMN account_id;
