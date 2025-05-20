// app/page.js
'use client';

import { useEffect, useState, useTransition } from 'react';

export default function Home() {
  const [msg, setMsg] = useState('');
  const [submissions, setSubmissions] = useState([]);
  const [pending, startTransition] = useTransition();

  useEffect(() => {
    fetch('/api/list')
      .then((res) => res.json())
      .then((data) => setSubmissions(data));
  }, []);

  const handleSubmit = (e) => {
    e.preventDefault();
    startTransition(() => {
      fetch('/api/submit', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ beg: msg }),
      })
        .then((res) => res.json())
        .then(() => {
          setMsg('');
          fetch('/api/list')
            .then((res) => res.json())
            .then((data) => setSubmissions(data));
        });
    });
  };

  return (
    <div className="p-8 max-w-xl mx-auto">
      <h1 className="text-2xl font-bold mb-4">Beg-o-Matic 3000 &#x1f916;</h1>
      <p>
      Have you ever tried to beg another team or admin to give you hints on a challenge, or maybe even the flag itself? Well now is your time to shine! Our state-of-the-art robot will view your pleas and, if they're convincing enough, may find you worthy of a flag!
      </p>
      <form onSubmit={handleSubmit}>
        <textarea
          name="beg"
          value={msg}
          onChange={(e) => setMsg(e.target.value)}
          placeholder="Dear Admin..."
          className="w-full h-32 border rounded p-2 mb-4"
        />
        <button type="submit" disabled={pending} className="bg-blue-600 text-white px-4 py-2 rounded">
          {pending ? 'Submitting...' : 'Submit Request'}
        </button>
      </form>

      <h2 className="text-xl font-semibold mt-8 mb-2">Your Submissions</h2>
      <ul className="space-y-4">
      {submissions.map((sub) => (
        <li key={sub.id} className="border rounded p-4 bg-white shadow">
          <p className="mb-1"><strong>Message:</strong> {sub.msg}</p>
          {sub.flag ? (
            <p className="text-green-600 font-mono">✅ Approved! Your flag: {sub.flag}</p>
          ) : (
            <p className="text-yellow-600">⏳ Awaiting review...</p>
          )}
        </li>
      ))}
      </ul>
    </div>
  );
}
