const path = require('path');
const Database = require('better-sqlite3');

const dbPath = path.join(process.cwd(), 'data.sqlite');
const db = new Database(dbPath);

// Initialize table
db.exec(`
  CREATE TABLE IF NOT EXISTS submissions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    msg TEXT NOT NULL,
    approved INTEGER DEFAULT 0
  );
`);

module.exports = db;