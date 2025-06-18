# The built-in jinja2 functionality is really helpful!
from flask import Flask, request, jsonify, render_template_string
from functools import wraps
from uuid import uuid4
import sqlite3

app = Flask(__name__)

db = sqlite3.connect("notes.db")
cursor = db.cursor()

def check_user_agent(f):
    @wraps(f)
    def decorator(*args, **kwargs):
        user_agent = request.headers.get('User-Agent')
        if user_agent.startswith("NinjaNote"):
            return f(*args, **kwargs)
        else:
            return jsonify({"error", "Invalid User Agent"}), 401
    return decorator

@app.route("/")
def index():
    return "Welcone to NinjaNote! Please use the provided CLI to continue."

@app.route("/api/submit", methods=["POST"])
@check_user_agent
def submit_note():
    try:
        data = request.json
        note_id = str(uuid4())
        title = data['title']
        content = data['content']

        cursor.execute("INSERT INTO notes (note_id, title, content) VALUES (?, ?, ?)", (note_id, title, content))
        db.commit()

        return jsonify({"note_id": note_id})
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route("/api/notes/<note_id>", methods=["GET"])
@check_user_agent
def read_note(note_id):
    try:
        cursor.execute("SELECT title, content FROM notes WHERE note_id = ?", (note_id,))
        note = cursor.fetchone()
        
        if note:
            title = note[0]
            content = note[1]
            template = "Note ID: {{note_id}}\nTitle: {{title}}\nNote: " + content
            return render_template_string(template, title = title, note_id = note_id)
        else:
            return jsonify({"error": "Note does not exist"}), 404
    except Exception as e:
        return jsonify({"error": str(e)}), 500




if __name__ == "__main__":
    app.run(host = '0.0.0.0', port = 8080)
