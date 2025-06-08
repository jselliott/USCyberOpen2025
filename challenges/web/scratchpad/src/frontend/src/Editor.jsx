import React, { useEffect, useState } from "react";
import axios from "axios";

const Editor = ({ token, onLogout }) => {
  const [content, setContent] = useState("");
  const [originalContent, setOriginalContent] = useState("");
  const [revisions, setRevisions] = useState([]);
  const [unsavedRevision, setUnsavedRevision] = useState(null);
  const [status, setStatus] = useState("");
  const [loading, setLoading] = useState(true);

  const fetchRevisions = async () => {
    try {
      const res = await axios.get("/api/revisions", {
        headers: { Authorization: `Bearer ${token}` },
      });
      setRevisions(res.data);
    } catch {
      setStatus("Failed to load revisions.");
    }
  };

  useEffect(() => {
    if (!token) {
      onLogout();
      return;
    }

    const fetchFile = async () => {
      try {
        const res = await axios.get("/api/file", {
          headers: { Authorization: `Bearer ${token}` },
        });
        setContent(res.data);
        setOriginalContent(res.data);
      } catch (err) {
        setStatus("Unauthorized or session expired.");
        setTimeout(() => window.location.reload(), 1000);
      } finally {
        setLoading(false);
      }
    };

    fetchFile();
    fetchRevisions();
  }, [token]);

  useEffect(() => {
    const confirmUnload = (e) => {
      if (unsavedRevision && content !== originalContent) {
        e.preventDefault();
        e.returnValue = "";
      }
    };
    window.addEventListener("beforeunload", confirmUnload);
    return () => window.removeEventListener("beforeunload", confirmUnload);
  }, [unsavedRevision, content, originalContent]);

  const handleSave = async () => {
    try {
      await axios.post(
        "/api/save",
        { content },
        {
          headers: { Authorization: `Bearer ${token}` },
        }
      );
      setStatus("Saved!");
      setOriginalContent(content);
      setUnsavedRevision(null);
      fetchRevisions();
    } catch (e) {
      console.log(e);
      setStatus("Failed to save.");
    }
  };

  const loadRevision = async (hash) => {
    if (content !== originalContent) {
      const confirm = window.confirm("You have unsaved changes. Load anyway?");
      if (!confirm) return;
    }

    try {
      const res = await axios.post('/api/revision/', {action: "show", hash}, {headers: { Authorization: `Bearer ${token}` }});
      setContent(res.data);
      setUnsavedRevision(hash);
      setStatus(`Loaded revision ${hash.slice(0, 7)}`);
    } catch {
      setStatus("Failed to load revision.");
    }
  };

  return (
    <div className="max-w-4xl mx-auto mt-10 p-6 bg-white shadow rounded">
      <div className="flex justify-between items-center mb-4">
        <h1 className="text-2xl font-semibold">USCG Personal Scratchpad</h1>
        <button
          className="text-red-500 hover:underline"
          onClick={onLogout}
        >
          Logout
        </button>
      </div>

      {loading ? (
        <p className="text-gray-600">Loading...</p>
      ) : (
        <>
          <textarea
            className="w-full h-64 border border-gray-300 rounded p-2 font-mono"
            value={content}
            onChange={(e) => setContent(e.target.value)}
          />
          <div className="flex justify-between items-center mt-4">
            <button
            disabled={content === originalContent}
            className={`px-4 py-2 rounded text-white ${
              content === originalContent
                ? 'bg-gray-400 cursor-not-allowed'
                : 'bg-blue-600 hover:bg-blue-700'
            }`}
              onClick={handleSave}
            >
              Save
            </button>
            {status && <span className="text-sm text-gray-600">{status}</span>}
          </div>

          <div className="mt-6">
            <h2 className="text-lg font-semibold mb-2">Revision History</h2>
            <ul className="space-y-1 max-h-48 overflow-y-auto border p-2 rounded">
              {revisions.map((rev) => (
                <li key={rev.hash} className="flex justify-between items-center">
                  <span className="text-sm font-mono">{rev.date}</span>
                  <button
                    onClick={() => loadRevision(rev.hash)}
                    className="text-blue-600 hover:underline text-sm"
                  >
                    Load
                  </button>
                </li>
              ))}
            </ul>
          </div>
        </>
      )}
    </div>
  );
};

export default Editor;
