CREATE TABLE IF NOT EXISTS food (
	id INTEGER PRIMARY KEY AUTOINCREMENT,
	title TEXT NOT NULL,
	calories REAL,
	proteins REAL,
	carbohydrates REAL,
	fats REAL
);
