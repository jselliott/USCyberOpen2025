from flask import Flask, jsonify

app = Flask(__name__)

@app.route('/list', methods=['GET'])
def list_files():
    return jsonify([
        {
            "name": "example.txt",
            "mime": "text/plain",
            "size": 123,
            "modified": "Fri May 24 12:34:56 UTC 2025"
        },
        {
            "name": "malicious.class",
            "mime": "application/java-vm",
            "size": 456,
            "modified": "Fri May 24 12:35:00 UTC 2025"
        }
    ])

@app.route('/get/<string:filename>', methods=['GET'])
def get_file(filename):
    return "this is a test"

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
