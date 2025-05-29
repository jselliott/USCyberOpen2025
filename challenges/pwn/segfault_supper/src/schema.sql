DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS menu;
DROP TABLE IF EXISTS orders;

CREATE TABLE users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password_hash TEXT NOT NULL,
    last_login INTEGER DEFAULT 0,  -- Unix epoch timestamp
    is_admin INTEGER DEFAULT 0
);

CREATE TABLE menu (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    price REAL NOT NULL
);

CREATE TABLE orders (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER,
    menu_id INTEGER,
    special_instructions TEXT NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (menu_id) REFERENCES menu(id)
);

INSERT INTO menu (name, price) VALUES ('Binary Burrito', 5.99);
INSERT INTO menu (name, price) VALUES ('Null Pointer Nachos', 6.49);
INSERT INTO menu (name, price) VALUES ('Rootkit Ramen', 10.49);
INSERT INTO menu (name, price) VALUES ('DDoS Donuts', 4.99);
INSERT INTO menu (name, price) VALUES ('Packet Pizza', 7.49);
INSERT INTO menu (name, price) VALUES ('Trojan Tacos', 4.75);
INSERT INTO menu (name, price) VALUES ('Firewall Fries', 3.50);
INSERT INTO menu (name, price) VALUES ('Exploit Espresso', 2.99);
INSERT INTO menu (name, price) VALUES ('Fabulous Flag Falafel', 999999999.99);
