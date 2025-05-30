from flask import Flask, send_file, jsonify, abort
import os
from mimetypes import guess_type
from datetime import datetime
import time

app = Flask(__name__)
FILE_DIR = "./files"

@app.route("/")
def list_files():
    files = []
    for fname in os.listdir(FILE_DIR):
        fpath = os.path.join(FILE_DIR, fname)
        if os.path.isfile(fpath):
            stat = os.stat(fpath)
            files.append({
                "name": fname,
                "size": stat.st_size,
                "modified": datetime.utcfromtimestamp(stat.st_mtime).strftime("%a %b %d %H:%M:%S UTC %Y"),
                "mime": guess_type(fname)[0] or "application/octet-stream"
            })
    return jsonify(files)

@app.route("/delayme")
def delay_me():
    time.sleep(10)
    return "DELAYED!"

@app.route("/<path:filename>")
def get_file(filename):
    fpath = os.path.join(FILE_DIR, "EvilStorage.jar")
    if not os.path.isfile(fpath):
        return abort(404, description="File not found")
    return send_file(fpath, mimetype=guess_type(filename)[0] or "application/octet-stream")

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)
