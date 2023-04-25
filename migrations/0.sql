CREATE TABLE IF NOT EXISTS migration (
	filename TEXT,
	executed_at_stamp TEXT,
	UNIQUE (filename)
);
