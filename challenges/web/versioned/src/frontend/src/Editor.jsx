import React, { useEffect, useState } from "react";
import axios from "axios";

const Editor = ({ token, onLogout }) => {
  const [content, setContent] = useState("");
  const [originalContent, setOriginalContent] = useState("");
  const [revisions, setRevisions] = useState([]);
  const [status, setStatus] = useState("");
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const fetchFile = async () => {
      try {
        const res = await axios.get("/api/file", {
          headers: { Authorization: `Bearer ${token}` },
        });
        setContent(res.data);
        setOriginalContent(res.data);
      } catch {
        setStatus("Failed to load file.");
      } finally {
        setLoading(false);
      }
    };
    fetchFile();
  }, [token]);

  useEffect(() => {
    const fetchRevisions = async () => {
      try {
        const res = await axios.get("/api/revisions", {
          headers: { Authorization: `Bearer ${token}` },
        });
        setRevisions(res.data);
      } catch {}
    };
    fetchRevisions();
  }, [token]);

  useEffect(() => {
    const confirmUnload = (e) => {
      if (content !== originalContent) {
        e.preventDefault();
        e.returnValue = "";
      }
    };
    window.addEventListener("beforeunload", confirmUnload);
    return () => window.removeEventListener("beforeunload", confirmUnload);
  }, [content, originalContent]);

  const handleSave = async () => {
    try {
      await axios.post(
        "/api/save",
        { content },
        { headers: { Authorization: `Bearer ${token}` } }
      );
      setStatus("Saved!");
      setOriginalContent(content);
    } catch {
      setStatus("Failed to save.");
    }
  };

  const loadRevision = async (hash) => {
    if (content !== originalContent) {
      const proceed = window.confirm("You have unsaved changes. Load anyway?");
      if (!proceed) return;
    }
    try {
      const res = await axios.get(`/api/revision/${hash}`, {
        headers: { Authorization: `Bearer ${token}` },
      });
      setContent(res.data.content);
      setStatus(`Loaded revision ${hash.slice(0, 7)}`);
    } catch {
      setStatus("Failed to load revision.");
    }
  };

  return (
    <div className="max-w-4xl mx-auto mt-10 p-6 bg-white shadow rounded">
      <div className="flex justify-between items-center mb-4">
        <h1 className="text-2xl font-semibold">Scratchpad</h1>
        <button
          onClick={onLogout}
          className="text-sm text-blue-600 hover:underline"
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
              className="bg-blue-600 text-white px-4 py-2 rounded hover:bg-blue-700"
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
