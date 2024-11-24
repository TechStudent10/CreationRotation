-- Up

ALTER TABLE moderators ADD passw TEXT NOT NULL;

-- Down

ALTER TABLE moderators DROP COLUMN passw;