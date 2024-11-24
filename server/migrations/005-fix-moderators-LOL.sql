-- Up

DROP TABLE moderators;

CREATE TABLE moderators (
    id INT AUTO_INCREMENT PRIMARY KEY,
    account_id INT NOT NULL,
    passw TEXT NOT NULL
);

-- Down
-- not needed!