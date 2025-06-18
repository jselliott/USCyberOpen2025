from flask import Flask, render_template, request, jsonify, send_file
import tempfile
import subprocess
import os
import shutil
import logging
from logging.config import dictConfig

dictConfig({
    'version': 1,
    'formatters': {'default': {
        'format': '[%(asctime)s] %(levelname)s in %(module)s: %(message)s',
    }},
    'handlers': {'wsgi': {
        'class': 'logging.StreamHandler',
        'stream': 'ext://flask.logging.wsgi_errors_stream',
        'formatter': 'default'
    }},
    'root': {
        'level': 'INFO',
        'handlers': ['wsgi']
    }
})

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route("/simple/uscg-leetcode-validator/")
def fake_repo():
    return '''
<html><body>
<a href="/packages/uscg-leetcode-validator-1.0.9.tar.gz">uscg-leetcode-validator-1.0.9</a>
</body></html>
'''

@app.route("/packages/uscg-leetcode-validator-1.0.9.tar.gz")
def fake_package():
    return send_file("/app/files/uscg-leetcode-validator-1.0.9.tar.gz")

@app.route('/api/submit', methods=['POST'])
def submit_code():

    data = request.get_json()

    if not data:
        return jsonify({"output":"No code submitted."})

    code = data.get('code')
    options = data.get("options",{"DEBUG":"false"})

    if not code:
        return jsonify({"output":"No code submitted."})
    
    # prepare environment
    for key in options:
        os.environ[key] = options[key]

    # Create a temp directory for this run
    tmpdir = tempfile.mkdtemp()
    code_path = os.path.join(tmpdir, "user.py")
    test_path = os.path.join(tmpdir, "test_cases.txt")

    try:

        # Save submitted code
        with open(code_path, "w") as f:
            f.write(code)

        # Copy test cases
        shutil.copyfile("files/test_cases.txt", test_path)

        # Check for validator updates
        output = subprocess.run(
            ["pip", "install", "--upgrade", "uscg-leetcode-validator"],
            capture_output=True,
            text=True
        )

        logging.info(output.stdout)

        # Run the validator
        result = subprocess.run(
            ["uscg-leetcode-validator", code_path, test_path],
            capture_output=True,
            text=True,
            timeout=5
        )

        output = result.stdout + result.stderr

        return jsonify({"output":output})

    except subprocess.TimeoutExpired:
        return jsonify({"output":"Execution timed out."})
    except Exception as e:
        return jsonify({"output":str(e)})
    finally:
        shutil.rmtree(tmpdir)

if __name__ == "__main__":
    app.run(host="0.0.0.0",port="5000")
