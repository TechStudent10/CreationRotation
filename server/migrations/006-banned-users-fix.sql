-- Up

DROP TABLE banned_users;

CREATE TABLE banned_users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    account_id INT NOT NULL,
    issued_by INT NOT NULL,
    reason TEXT NOT NULL
);

-- Down
-- not needed