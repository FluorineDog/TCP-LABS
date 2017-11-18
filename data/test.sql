CREATE TABLE accounts (
	account text primary key, 
	password text,
	nickname text, 
	salt text 
);

CREATE TABLE message (
	rowid integer primary key autoincrement, 
	sender text,
	receiver text,
	timestamp integer,
	message text 
);
