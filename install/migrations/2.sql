CREATE TABLE IF NOT EXISTS food_composition (
	composite_id INTEGER,
	particular_id INTEGER,
	FOREIGN KEY (composite_id) REFERENCES food(id),
	FOREIGN KEY (particular_id) REFERENCES food(id)
);
