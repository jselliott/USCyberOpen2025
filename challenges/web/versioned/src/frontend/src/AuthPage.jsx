import React, { useState } from "react";
import axios from "axios";

const AuthPage = ({ onAuth }) => {
  const [activeTab, setActiveTab] = useState("register");
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [usernameAvailable, setUsernameAvailable] = useState(true);
  const [error, setError] = useState("");

  const checkUsername = async () => {
    try {
      const res = await axios.get(`/api/username_check?username=${username}`);
      setUsernameAvailable(res.data.good);
    } catch {
      setUsernameAvailable(false);
    }
  };

  const handleRegister = async (e) => {
    e.preventDefault();
    await checkUsername();
    if (!usernameAvailable) {
      setError("Username is already taken.");
      return;
    }
    try {
      const res = await axios.post("/api/register", { username, password });
      onAuth(res.data.token);
    } catch {
      setError("Registration failed.");
    }
  };

  const handleLogin = async (e) => {
    e.preventDefault();
    try {
      const res = await axios.post("/api/login", { username, password });
      onAuth(res.data.token);
    } catch {
      setError("Login failed.");
    }
  };

  return (
    <div className="max-w-md mx-auto mt-16 p-6 bg-white shadow-lg rounded-md">
      <div className="flex justify-center mb-4">
        <button
          className={`px-4 py-2 mx-1 rounded-t ${
            activeTab === "register"
              ? "bg-blue-600 text-white"
              : "bg-gray-200 text-gray-600"
          }`}
          onClick={() => {
            setActiveTab("register");
            setError("");
          }}
        >
          Register
        </button>
        <button
          className={`px-4 py-2 mx-1 rounded-t ${
            activeTab === "login"
              ? "bg-blue-600 text-white"
              : "bg-gray-200 text-gray-600"
          }`}
          onClick={() => {
            setActiveTab("login");
            setError("");
          }}
        >
          Login
        </button>
      </div>
      <form
        onSubmit={activeTab === "register" ? handleRegister : handleLogin}
        className="flex flex-col space-y-3"
      >
        <input
          type="text"
          placeholder="Username"
          value={username}
          onBlur={activeTab === "register" ? checkUsername : undefined}
          onChange={(e) => {
            setUsername(e.target.value.toLowerCase());
            if (activeTab === "register") setUsernameAvailable(true);
          }}
          className="p-2 border border-gray-300 rounded"
        />
        {activeTab === "register" && !usernameAvailable && (
          <p className="text-red-500 text-sm">Username is taken</p>
        )}
        <input
          type="password"
          placeholder="Password"
          value={password}
          onChange={(e) => setPassword(e.target.value)}
          className="p-2 border border-gray-300 rounded"
        />
        <button
          type="submit"
          className="bg-blue-600 text-white py-2 rounded hover:bg-blue-700"
        >
          {activeTab === "register" ? "Register" : "Login"}
        </button>
        {error && <p className="text-red-600 text-sm">{error}</p>}
      </form>
    </div>
  );
};

export default AuthPage;