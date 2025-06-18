
import { getSubmissionById } from '../../../db/actions';
import { approveBegging } from '@/app/actions';
import { headers } from 'next/headers';

import dotenv from 'dotenv';
dotenv.config();

const botSecret = process.env.BOT_SECRET;

console.log(`[BOT] Bot secret is: ${botSecret}`)

export default async function AdminPage({ params }) {
  const { id } = await params;
  const page_headers = await headers();
  const authHeader = await page_headers.get('x-bot-auth');
  const isDev = process.env.NODE_ENV === 'development';

  if (!isDev && authHeader !== botSecret) {
    return <div>404 Not Found</div>; // or redirect to home
  }

  const submission = getSubmissionById(id);
  if (!submission) return <div>Submission not found</div>;

  return (
    <div className="p-8">
      <h1 className="text-xl font-bold">Admin Review</h1>
      <form action={ approveBegging }>
        <input type="hidden" name="id" value={ submission.id } />
        <button type="submit" className="mt-4 bg-green-600 text-white px-4 py-2 rounded">Approve</button>
      </form>
      <div dangerouslySetInnerHTML={{ __html: submission.msg }} />
    </div>
  );
}
