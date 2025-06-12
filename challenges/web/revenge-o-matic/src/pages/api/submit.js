// pages/api/submit.js
import { saveSubmission } from '../../db/actions';
import { launchBot } from '../../utils/bot'; // optional

export default async function handler(req, res) {
  if (req.method !== 'POST') return res.status(405).end();

  const { beg } = req.body;
  if (!beg) return res.status(400).json({ error: 'Missing beg message' });

  const id = saveSubmission(beg);

  launchBot(id);

  res.status(200).json({ id });
}
