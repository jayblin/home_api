CREATE TABLE IF NOT EXISTS recipe_step_food (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	recipe_step_id INTEGER NOT NULL,
	food_id INTEGER,
	canonical_mass REAL,
	FOREIGN KEY (recipe_step_id) REFERENCES recipe_step(id),
	FOREIGN KEY (food_id) REFERENCES food(id)
) STRICT
