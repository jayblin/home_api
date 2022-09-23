CREATE TABLE IF NOT EXISTS food_creation_log (
	food_id INTEGER,
	user_id INTEGER,
	created_at_stamp TEXT,
	FOREIGN KEY (user_id) REFERENCES user(id),
	FOREIGN KEY (food_id) REFERENCES food(id)
);
