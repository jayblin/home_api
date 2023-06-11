CREATE TABLE IF NOT EXISTS user_food_log (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	user_id INTEGER,
	food_id INTEGER,
	created_at TEXT,
	FOREIGN KEY (user_id) REFERENCES user(id),
	FOREIGN KEY (food_id) REFERENCES food(id)
) STRICT
