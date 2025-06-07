import os
import sqlite3
import bcrypt
from flask import Flask, request, jsonify, send_from_directory, session, redirect, url_for, render_template
from flask_login import LoginManager, UserMixin, login_user, login_required, logout_user, current_user
from flask_cors import CORS, cross_origin
from datetime import timedelta
from functools import wraps
from bot import run_bot

DB_PATH = '/data/app.db'
FLAG = 'CTF{Stars-Bars-And-Headers}'

# --- Flask App Factory ---
def create_app():
    
    app = Flask(__name__, static_folder='static', template_folder='templates')
    app.secret_key = os.environ.get('SECRET_KEY', 'supersecretkey')
    app.permanent_session_lifetime = timedelta(hours=6)
    app.config.update(
    SESSION_COOKIE_SAMESITE="None",
    SESSION_COOKIE_SECURE=True
    )

    @app.route('/api/request-validation/<int:cid>', methods=['POST'])
    def request_validation(cid):
        # Find the conversion and its reference_url
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('SELECT id, reference_url FROM conversions WHERE id=?', (cid,))
        row = cur.fetchone()
        if not row:
            con.close()
            return jsonify({'error': 'Conversion not found'}), 404
        _, ref_url = row
        # Check if already verified
        cur.execute('SELECT verified FROM conversions WHERE id=?', (cid,))
        if cur.fetchone()[0]:
            con.close()
            return jsonify({'success': True, 'message': 'Already validated.'})
        # Try to access the reference URL using the bot
        try:
            bot_result = run_bot(cid)
            if bot_result and isinstance(bot_result, str) and bot_result.startswith('Error:'):
                # Validation failed, return error to frontend
                con.close()
                return jsonify({'error': f'Validation failed: {bot_result}'}), 400
        except Exception as e:
            con.close()
            return jsonify({'error': f'Connection error: {e}'}), 502
        # Mark as verified
        cur.execute('UPDATE conversions SET verified=1 WHERE id=?', (cid,))
        con.commit()
        con.close()
        return jsonify({'success': True, 'message': 'Conversion validated and marked as verified.'})

    # --- DB Init ---
    os.makedirs('/data', exist_ok=True)
    if not os.path.exists(DB_PATH):
        init_db()
    # else: 
    #     os.remove(DB_PATH)
    #     init_db()

    # --- Login Manager ---
    login_manager = LoginManager()
    login_manager.init_app(app)

    class User(UserMixin):
        def __init__(self, id, username, password_hash, is_admin, flag):
            self.id = id
            self.username = username
            self.password_hash = password_hash
            self.is_admin = is_admin
            self.flag = flag

    @login_manager.user_loader
    def load_user(user_id):
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('SELECT id, username, password_hash, is_admin, flag FROM users WHERE id=?', (user_id,))
        row = cur.fetchone()
        con.close()
        if row:
            return User(*row)
        return None

    # --- CORS ---
    def cors_headers(resp, origin=None):
        resp.headers['Access-Control-Allow-Origin'] = origin
        resp.headers['Access-Control-Allow-Credentials'] = 'true'
        return resp

    # --- API ---
    @app.route('/api/signup', methods=['POST'])
    def signup():
        data = request.get_json()
        username = data.get('username')
        password = data.get('password')
        if not username or not password:
            return jsonify({'error': 'Missing fields'}), 400
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('SELECT id FROM users WHERE username=?', (username,))
        if cur.fetchone():
            con.close()
            return jsonify({'error': 'Username exists'}), 400
        pw_hash = bcrypt.hashpw(password.encode(), bcrypt.gensalt(12)).decode()
        cur.execute('INSERT INTO users (username, password_hash, is_admin) VALUES (?, ?, 0)', (username, pw_hash))
        con.commit()
        user_id = cur.lastrowid
        con.close()
        user = load_user(user_id)
        login_user(user)
        resp = jsonify({'success': True})
        resp = cors_headers(resp)
        return resp

    @app.route('/api/login', methods=['POST'])
    def login():
        data = request.get_json()
        username = data.get('username')
        password = data.get('password')
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('SELECT id, username, password_hash, is_admin, flag FROM users WHERE username=?', (username,))
        row = cur.fetchone()
        con.close()
        if not row:
            return jsonify({'error': 'Invalid credentials'}), 401
        user = User(*row)
        if not bcrypt.checkpw(password.encode(), user.password_hash.encode()):
            return jsonify({'error': 'Invalid credentials'}), 401
        login_user(user)
        resp = jsonify({'success': True})
        resp = cors_headers(resp)
        return resp

    @app.route('/api/logout', methods=['POST'])
    @login_required
    def logout():
        logout_user()
        resp = jsonify({'success': True})
        resp = cors_headers(resp)
        return resp

    @app.route('/api/change-password', methods=['PUT', 'OPTIONS'])
    @login_required
    def change_password():
        # CORS flaw: reflect Origin
        origin = request.headers.get('Origin')
        if request.method == 'OPTIONS':
            resp = app.make_response('')
            resp.headers['Access-Control-Allow-Origin'] = origin
            resp.headers['Access-Control-Allow-Credentials'] = 'true'
            resp.headers['Access-Control-Allow-Methods'] = 'PUT,OPTIONS'
            resp.headers['Access-Control-Allow-Headers'] = 'Content-Type'
            return resp
        data = request.get_json()
        new = data.get('new_password')
        if not new:
            resp = jsonify({'error': 'Missing new_password'})
            return cors_headers(resp, origin), 400
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        new_hash = bcrypt.hashpw(new.encode(), bcrypt.gensalt(12)).decode()
        cur.execute('UPDATE users SET password_hash=? WHERE id=?', (new_hash, current_user.id))
        con.commit()
        con.close()
        resp = jsonify({'success': True})
        return cors_headers(resp, origin)

    @app.route('/api/conversions', methods=['GET'])
    def get_conversions():
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('SELECT id, metric_base, imperial_base, image_url, verified, reference_url, notes FROM conversions')
        rows = cur.fetchall()
        con.close()
        conversions = [dict(zip(['id','metric_base','imperial_base','image_url','verified','reference_url','notes'], r)) for r in rows]
        resp = jsonify(conversions)
        return cors_headers(resp)

    @app.route('/api/conversions', methods=['POST'])
    @login_required
    def add_conversion():
        data = request.get_json()
        metric = data.get('metric_base')
        imperial = data.get('imperial_base')
        image_url = data.get('image_url')
        reference_url = data.get('reference_url')
        notes = data.get('notes')
        if not (metric and imperial and image_url and reference_url):
            return jsonify({'error': 'Missing fields'}), 400
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('INSERT INTO conversions (metric_base, imperial_base, image_url, verified, reference_url, notes) VALUES (?, ?, ?, 0, ?, ?)',
            (metric, imperial, image_url, reference_url, notes or ''))
        conv_id = cur.lastrowid
        cur.execute('INSERT INTO reviews (conversion_id, submitted_by, reference_url, reviewed) VALUES (?, ?, ?, 0)',
            (conv_id, current_user.id, reference_url))
        con.commit()
        con.close()
        resp = jsonify({'success': True})
        return cors_headers(resp)

    @app.route('/api/conversions/<int:cid>', methods=['GET'])
    def get_conversion(cid):
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('SELECT id, metric_base, imperial_base, image_url, verified, reference_url, notes FROM conversions WHERE id=?', (cid,))
        row = cur.fetchone()
        con.close()
        if not row:
            return jsonify({'error': 'Not found'}), 404
        resp = jsonify(dict(zip(['id','metric_base','imperial_base','image_url','verified','reference_url','notes'], row)))
        return cors_headers(resp)

    def admin_required(f):
        @wraps(f)
        def decorated(*args, **kwargs):
            if not current_user.is_authenticated or not current_user.is_admin:
                return jsonify({'error': 'Admin only'}), 403
            return f(*args, **kwargs)
        return decorated

    @app.route('/api/verification/<int:review_id>', methods=['GET'])
    @login_required
    @admin_required
    def verify_review(review_id):
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('UPDATE reviews SET reviewed=1, reviewed_by=? WHERE id=?', (current_user.id, review_id))
        con.commit()
        con.close()
        resp = jsonify({'success': True})
        return cors_headers(resp)

    @app.route('/api/profile', methods=['GET'])
    @login_required
    def api_profile():
        data = {'username': current_user.username, 'is_admin': bool(current_user.is_admin)}
        if current_user.is_admin:
            data['flag'] = FLAG
        resp = jsonify(data)
        return cors_headers(resp)

    @app.route('/profile', methods=['GET'])
    @login_required
    def profile():
        flag = FLAG if current_user.is_admin else None
        return render_template('profile.html', username=current_user.username, is_admin=current_user.is_admin, flag=flag)

    @app.route('/api/scoreboard', methods=['GET'])
    def scoreboard():
        con = sqlite3.connect(DB_PATH)
        cur = con.cursor()
        cur.execute('SELECT username FROM users WHERE is_admin=0')
        users = [r[0] for r in cur.fetchall()]
        con.close()
        resp = jsonify({'users': users})
        return cors_headers(resp)

    @app.route('/', methods=['GET'])
    def index():
        return send_from_directory(app.static_folder, 'index.html')

    @app.route('/static/<path:path>')
    def static_files(path):
        return send_from_directory(app.static_folder, path)

    # --- Secure CORS for all other routes ---
    @app.after_request
    def set_cors_headers(response):
        if request.path == '/api/change-password':
            return response
        return cors_headers(response)

    return app

# --- DB Init ---
def init_db():
    con = sqlite3.connect(DB_PATH)
    cur = con.cursor()
    cur.execute('''CREATE TABLE IF NOT EXISTS users (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        username TEXT UNIQUE,
        password_hash TEXT,
        is_admin INTEGER,
        flag TEXT,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    )''')
    cur.execute('''CREATE TABLE IF NOT EXISTS conversions (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        metric_base TEXT,
        imperial_base TEXT,
        image_url TEXT,
        verified INTEGER,
        reference_url TEXT,
        notes TEXT
    )''')
    cur.execute('''CREATE TABLE IF NOT EXISTS reviews (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        conversion_id INTEGER,
        submitted_by INTEGER,
        reference_url TEXT,
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        reviewed INTEGER,
        reviewed_by INTEGER,
        review_notes TEXT
    )''')
    # Admin user
    admin_hash = bcrypt.hashpw(b'AmericaRules!123', bcrypt.gensalt(12)).decode()
    cur.execute('INSERT INTO users (id, username, password_hash, is_admin, flag) VALUES (1, "admin", ?, 1, ?)', (admin_hash, FLAG))
    # Seed conversions
    conversions = [
        (1, 'kilometres per hour', 'Cheeseburger-Powered Bald Eagle Velocity', 'https://cdn.example.com/eagle1.png', 1, 'https://en.wikipedia.org/wiki/Hamburger', 'Standard drive-thru Mach number.'),
        (2, 'degrees Celsius', 'Freedom-Furnace Sizzle Units', 'https://cdn.example.com/bbq2.png', 1, 'https://en.wikipedia.org/wiki/Barbecue', 'Zero equals chilly tailgate beer, one-hundred equals molten cheddar apocalypse.'),
        (3, 'litres', 'MegaGulps of Mountain Dew', 'https://cdn.example.com/dew3.png', 0, 'https://en.wikipedia.org/wiki/Soft_drink', 'Hydration protocol for overnight LAN patriotism.'),
        (4, 'kilograms', 'Tactical Baconators', 'https://cdn.example.com/bacon4.png', 0, 'https://en.wikipedia.org/wiki/Bacon', 'Mass of pure smoky independence ready for orbital drop.'),
        (5, 'metres', 'Statues-of-Liberty Vertically Stacked', 'https://cdn.example.com/liberty5.png', 1, 'https://www.nps.gov/stli/', 'Vertical patriotism elevation index.'),
        (6, 'joules', 'George Washington Cherry-Tree Chops', 'https://cdn.example.com/chop6.png', 0, 'https://en.wikipedia.org/wiki/Joule', 'Energy required to not tell a lie while felling timber for freedom.'),
        (7, 'newtons', 'Black Friday Shopping Rushes', 'https://cdn.example.com/apollo7.png', 1, 'https://www.nasa.gov/', 'Force equal to one small step for man, one giant kick for country.'),
        (8, 'pascals', 'Big Mac Happy Meals', 'https://cdn.example.com/pressure8.png', 0, 'https://en.wikipedia.org/wiki/Pressure_cooking', 'Exactly how many chili cook-off atmospheres your ideals can withstand.'),
        (9, 'watts', 'Fourth-of-July Firework Blasts', 'https://cdn.example.com/fireworks9.png', 1, 'https://www.energy.gov/', 'Continuous power of one backyard finale going off every second.'),
        (10, 'seconds', 'Twinkie Bites', 'https://cdn.example.com/drum10.png', 0, 'https://en.wikipedia.org/wiki/The_Star-Spangled_Banner', 'Temporal measure of crowd-sourced chest-thump cadence.'),
        (11, 'square metres', 'Football (without feet) Fields of Freedom', 'https://cdn.example.com/field11.png', 1, 'https://en.wikipedia.org/wiki/American_football_field', 'Surface area required for a single-end-zone BBQ tailgate extravaganza.'),
        (12, 'cubic metres', 'Slurpee Gulps', 'https://cdn.example.com/canyon12.png', 0, 'https://www.nps.gov/grca/', 'Volume of majestic emptiness that echoes with liberty yodels.'),
        (13, 'metres per second squared', 'Mustang GT Burnout Forces', 'https://cdn.example.com/mustang13.png', 1, 'https://en.wikipedia.org/wiki/Ford_Mustang', 'Acceleration felt when redlining freedom at a summer drag strip.'),
        (14, 'kilometres', 'Route-66 Road-Trip Segments', 'https://cdn.example.com/rt66_14.png', 0, 'https://en.wikipedia.org/wiki/U.S._Route_66', 'Distance between obligatory diner milkshake refuelings.'),
        (15, 'grams', 'Hot-Dog Links of Liberty', 'https://cdn.example.com/hotdog15.png', 1, 'https://en.wikipedia.org/wiki/Hot_dog', 'Mass of democracy squeezed into a ballpark bun.'),
        (16, 'millimetres', 'George Washington Hair Widths', 'https://cdn.example.com/hair16.png', 0, 'https://en.wikipedia.org/wiki/George_Washington', 'Finest dimensional standard since the powdered-wig era.'),
        (17, 'kilojoules', 'Bald-Eagle Breakfast-Burrito Bursts', 'https://cdn.example.com/burrito17.png', 1, 'https://en.wikipedia.org/wiki/Burrito', 'Energy reserve released every sunrise over the Rockies.'),
        (18, 'microseconds', 'Oreos', 'https://cdn.example.com/humming18.png', 0, 'https://en.wikipedia.org/wiki/Hummingbird', 'Time between freedom ringing and your phone’s “USA” ringtone.'),
        (19, 'megapascals', 'Space-Shuttle Launch Chest Thumps', 'https://cdn.example.com/shuttle19.png', 1, 'https://en.wikipedia.org/wiki/Space_Shuttle', 'Pressure wave that makes you salute involuntarily at liftoff.'),
        (20, 'kilowatt-hours', 'Texas-Sized Power-Grid Bursts', 'https://cdn.example.com/texas20.png', 1, 'https://en.wikipedia.org/wiki/Electric_power_industry_in_Texas', 'Electrical oomph to keep every porch light on during Friday-night lights.'),
    ]
    for c in conversions:
        cur.execute('INSERT INTO conversions (id, metric_base, imperial_base, image_url, verified, reference_url, notes) VALUES (?, ?, ?, ?, ?, ?, ?)', c)
    con.commit()
    con.close()

if __name__ == '__main__':
    app = create_app()
    app.run(debug=True, host='0.0.0.0', port=8000)
