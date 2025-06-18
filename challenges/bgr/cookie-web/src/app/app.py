import os
from flask import Flask, render_template, request, make_response, send_from_directory, abort
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__)
COOKIE_NAME = 'cookie'
COOKIE_SECRET = os.getenv('SECRET_COOKIE')
COOKIE_DEFAULT = 'QWZ0ZXIgaW5zcGVjdGluZyB0aGUgY29udGVudHMsIGhlJ2xsIGhvcCBvbiB0aGUgUk9CT1QgdmFjY3V1bSBwaWNraW5nIHVwIHRoZSBjcnVtYnMgaGUgbWFkZS4KQ3J1bWIgMTogZFY5Q1FHc3paRjloVA=='
FLAG = os.getenv('FLAG')

KITCHEN_ROOT = os.path.join(os.path.dirname(__file__), 'kitchen')

@app.route('/')
def index():
    user_cookie = request.cookies.get(COOKIE_NAME)
    if user_cookie == COOKIE_SECRET:
        response = make_response(render_template('index.html', show_flag=True, flag=FLAG))
    else:
        response = make_response(render_template('index.html', show_flag=False))
        response.set_cookie(COOKIE_NAME, COOKIE_DEFAULT, httponly=True, secure=False, samesite='Lax')
    return response

@app.route('/admin')
def admin_login():
    return render_template('admin_login.html')

@app.route('/kitchen', defaults={'req_path': ''})
@app.route('/kitchen/<path:req_path>')
def kitchen(req_path):
    from datetime import datetime
    abs_path = os.path.join(KITCHEN_ROOT, req_path)
    if not os.path.exists(abs_path):
        return abort(404)
    if os.path.isfile(abs_path):
        return send_from_directory(KITCHEN_ROOT, req_path)
    entries = []
    for e in sorted(os.listdir(abs_path)):
        full_path = os.path.join(abs_path, e)
        stat = os.stat(full_path)
        entries.append({
            'name': e + ('/' if os.path.isdir(full_path) else ''),
            'path': os.path.join(req_path, e).lstrip('/'),
            'mtime': datetime.fromtimestamp(stat.st_mtime).strftime('%Y-%m-%d %H:%M'),
            'size': '-' if os.path.isdir(full_path) else stat.st_size
        })
    return render_template('kitchen.html', current_path=req_path, entries=entries)

@app.route('/robots.txt')
def robots_txt():
    return app.send_static_file('robots.txt')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)