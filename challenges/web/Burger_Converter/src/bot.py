import os
import time
import sqlite3
import requests
from selenium import webdriver
from selenium.webdriver.chrome.options import Options

DB_PATH = "/data/app.db"
ADMIN_USER = "admin"
ADMIN_PASS = "AmericaRules!123"
BASE_DOMAIN = os.environ.get("FQDN", "localhost")
BASE_URL = os.environ.get("BASE_URL", f"https://{BASE_DOMAIN}")
USER_AGENT = f"PatriotAdminBot/1.0 (+https://{BASE_DOMAIN})"


def get_admin_cookie():
    s = requests.Session()
    s.post(
        f"{BASE_URL}/api/login",
        json={"username": ADMIN_USER, "password": ADMIN_PASS},
        verify=False,
    )
    for c in s.cookies:
        if c.name == "session":
            return c.value
    return None


def run_bot(conversion):
    con = sqlite3.connect(DB_PATH)
    cur = con.cursor()
    cur.execute("SELECT id, reference_url FROM conversions WHERE id=?", (conversion,))
    row = cur.fetchone()
    if not row:
        con.close()
        return "Error: Unable to access conversion"
    review_id, url = row
    # Get admin cookie
    admin_cookie = get_admin_cookie()
    if not admin_cookie:
        print("[BOT] Could not get admin cookie!")
        return "Error: Unable to get admin loaded, this is a problem with the bot, please report it to the CTF organizers."
    # Setup simple headless Chrome
    options = Options()
    options.add_argument("--headless=new")
    options.add_argument("--no-sandbox")
    options.set_capability("acceptInsecureCerts", True)
    options.add_argument("--ignore-certificate-errors")
    options.add_argument("--disable-dev-shm-usage")
    options.add_argument("--disable-gpu")
    options.add_argument(
        "--disable-features=BlockThirdPartyCookies,ThirdPartyStoragePartitioning"
    )
    options.add_experimental_option(
        "prefs",
        {
            "profile.cookie_controls_mode": 0,  # allow all
            "network.cookie.cookieBehavior": 0,  # Firefox compat
        },
    )
    options.add_argument("--disable-software-rasterizer")
    options.add_argument(f"user-agent={USER_AGENT}")
    driver = webdriver.Chrome(options=options)
    # driver.get(BASE_URL)  # Set domain
    # driver.add_cookie({'name': 'session', 'value': admin_cookie, 'path': '/', 'httpOnly': True, 'secure': True, 'domain': BASE_DOMAIN, 'sameSite': 'None' })
    # print(f'[BOT] Visiting {url}')
    # driver.get(url)

    driver.execute_cdp_cmd(
        "Network.setCookie",
        {
            "name": "session",
            "value": admin_cookie,
            "url": f"{BASE_URL}/",
            "path": "/",
            "secure": True,  # keep Secure
            "sameSite": "None",  # cross-site allowed
        },
    )

    driver.get(f"{BASE_URL}/api/profile")
    # Hit the real login endpoint **from inside** the browser
    # driver.execute_script("""
    # await fetch('/api/login', {
    #     method:  'POST',
    #     credentials: 'include',
    #     headers: { 'Content-Type': 'application/json' },
    #     body: JSON.stringify({username: 'admin', password: 'AmericaRules!123'})
    # });
    # """)

    # Now move to the attacker-controlled URL
    print(driver.get_cookies())
    driver.get(url)

    # Wait for DOM to load
    # Optionally, you can interact with the DOM here using driver.page_source or driver.find_element
    # For now, just check if page loaded by checking title or body
    page_title = driver.title
    page_source = driver.page_source
    # If page_source is empty, treat as error
    if not page_source or "<body" not in page_source:
        driver.quit()
        print(f"[BOT] Error: Could not load DOM for {url}")
        con.close()
        return f"Error: Could not load DOM for {url}"
    time.sleep(2)
    driver.quit()
    # Mark as reviewed
    cur.execute("UPDATE reviews SET reviewed=1, reviewed_by=1 WHERE id=?", (review_id,))
    con.commit()
    con.close()
    print(f"[BOT] Reviewed {url}")


if __name__ == "__main__":
    run_bot()
