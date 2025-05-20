// db/actions.js
const db = require('./init');

export function saveSubmission(msg) {
  const stmt = db.prepare('INSERT INTO submissions (msg) VALUES (?)');
  const result = stmt.run(msg);
  return result.lastInsertRowid;
}

export function getSubmissionById(id) {
  const stmt = db.prepare('SELECT * FROM submissions WHERE id = ?');
  return stmt.get(id);
}

export function getAllSubmissions() {
    const stmt = db.prepare('SELECT * FROM submissions ORDER BY id DESC');
    return stmt.all();
}  

export function approveSubmission(id) {
  const stmt = db.prepare('UPDATE submissions SET approved = 1 WHERE id = ?');
  stmt.run(id);
}