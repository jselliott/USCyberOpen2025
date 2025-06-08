import React, { useState } from "react";
import AuthPage from "./AuthPage";
import Editor from "./Editor";

const App = () => {
  const [token, setToken] = useState(localStorage.getItem("token"));

  const handleAuth = (newToken) => {
    localStorage.setItem("token", newToken);
    setToken(newToken);
  };

  const handleLogout = () => {
    localStorage.removeItem("token");
    setToken(null);
  };

  return (
    <div className="min-h-screen bg-gray-100">
      {token ? (
        <Editor token={token} onLogout={handleLogout} />
      ) : (
        <AuthPage onAuth={handleAuth} />
      )}
    </div>
  );
};

export default App;