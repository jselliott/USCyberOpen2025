const fs = require('fs');
const path = require('path');
const { getAllSubmissions } = require('../../db/actions');

export default function handler(req, res) {
  let flag = 'CTF{missing_flag}';
  try {
    flag = fs.readFileSync('/flag.txt', 'utf8').trim();
  } catch (e) {
    console.error('Failed to load flag:', e);
  }

  const submissions = getAllSubmissions().map((sub) => ({
    ...sub,
    flag: sub.approved ? flag : null,
  }));

  res.status(200).json(submissions);
}
