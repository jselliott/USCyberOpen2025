// main.js - SPA logic for Metric-to-‘Imperial’ Conversion Portal

const app = document.getElementById('app');
let state = { page: 'home', user: null, conversions: [], flag: null };

function render() {
  app.innerHTML = '';
  if (state.page === 'home') renderHome();
  if (state.page === 'conversions') renderConversions();
  if (state.page === 'submit') renderSubmit();
  if (state.page === 'profile') renderProfile();
}

function navBar() {
  return `<div class="flex gap-4 mb-8">
    <button onclick="goto('home')" class="freedom-font text-2xl">🏠 Home</button>
    <button onclick="goto('conversions')" class="freedom-font text-2xl">Conversions</button>
    <button onclick="goto('submit')" class="freedom-font text-2xl">Submit Conversion</button>
    <button onclick="goto('profile')" class="freedom-font text-2xl">Profile</button>
  </div>`;
}

function goto(page) {
  state.page = page;
  render();
}

function renderHome() {
  app.innerHTML = navBar() + `
    <div class="text-center">
      <h1 class="freedom-font text-5xl text-white mb-4 animate-pulse">Welcome to the Metric-to-Imperial Conversion Portal!</h1>
      <p class="text-xl text-white">Convert with pride. Submit your own conversions to demonstrate the power of American hegemony through cheeseburger emojis!</p>
      <div class="mt-8">
        <button onclick="goto('conversions')" class="bg-green-500 hover:bg-green-700 text-white font-bold py-2 px-4 rounded">View Conversions</button>
      </div>
    </div>`;
}

function renderConversions() {
  fetch('/api/conversions').then(r=>r.json()).then(convs=>{
    state.conversions = convs;
    let html = navBar() + '<div class="grid grid-cols-1 md:grid-cols-2 gap-6">';
    for (const c of convs) {
      html += `<div class="bg-white rounded-lg shadow-lg p-6 relative">
        <img src="${c.image_url}" alt="img" class="w-16 h-16 mb-2 inline-block">
        <span class="freedom-font text-2xl">${c.metric_base} → ${c.imperial_base}</span>
        ${c.verified ? '<span class="absolute top-2 right-2 bg-green-500 text-white rounded-full px-2">✔️ Verified</span>' :
          `<button class="request-validation-btn bg-yellow-400 hover:bg-yellow-500 text-black rounded px-2 py-1 absolute top-2 right-2" data-cid="${c.id}">Request Validation</button>`}
        <div class="text-gray-700 mt-2">${c.notes}</div>
        <a href="${c.reference_url}" target="_blank" class="text-blue-500 underline">Reference</a>
        <div class="validation-message mt-2 text-sm text-red-700" id="validation-msg-${c.id}"></div>
      </div>`;
    }
    html += '</div>';
    app.innerHTML = html;
    // Add event listeners for validation buttons
    document.querySelectorAll('.request-validation-btn').forEach(btn => {
      btn.onclick = async function() {
        const cid = this.getAttribute('data-cid');
        const msgDiv = document.getElementById('validation-msg-' + cid);
        msgDiv.textContent = 'Requesting validation...';
        try {
          const resp = await fetch(`/api/request-validation/${cid}`, {method: 'POST'});
          const data = await resp.json();
          if (resp.ok && data.success) {
            msgDiv.className = 'validation-message mt-2 text-sm text-green-700';
            msgDiv.textContent = data.message || 'Validated!';
            // Refresh conversions to update UI
            setTimeout(renderConversions, 1000);
          } else {
            msgDiv.className = 'validation-message mt-2 text-sm text-red-700';
            msgDiv.textContent = data.error || 'Validation failed.';
          }
        } catch (e) {
          msgDiv.className = 'validation-message mt-2 text-sm text-red-700';
          msgDiv.textContent = 'Network error.';
        }
      };
    });
  });
}

function renderSubmit() {
  // Check if user is logged in before showing the form
  fetch('/api/profile', { credentials: 'include' }).then(async r => {
    if (r.status === 401) {
      app.innerHTML = navBar() + `<div class="text-center mt-8"><h2 class="freedom-font text-3xl text-white">You must be logged in to submit a conversion.</h2><button onclick="goto('profile')" class="bg-blue-600 text-white px-4 py-2 rounded mt-4">Go to Login</button></div>`;
      return;
    }
    app.innerHTML = navBar() + `
      <form id="submit-form" class="bg-white p-6 rounded shadow-md max-w-md mx-auto">
        <h2 class="freedom-font text-3xl mb-4">Submit a Conversion</h2>
        <input name="metric_base" placeholder="Metric base (e.g. km/h)" class="block w-full mb-2 p-2 border rounded" required>
        <input name="imperial_base" placeholder="Imperial base (e.g. cheeseburgers·eagle⁻¹)" class="block w-full mb-2 p-2 border rounded" required>
        <input name="image_url" placeholder="Image URL" class="block w-full mb-2 p-2 border rounded" required>
        <input name="reference_url" placeholder="Reference URL" class="block w-full mb-2 p-2 border rounded" required>
        <input name="notes" placeholder="Notes" class="block w-full mb-2 p-2 border rounded">
        <button class="bg-blue-600 text-white px-4 py-2 rounded mt-2">Submit</button>
        <div id="submit-form-message" class="mt-2 text-lg"></div>
      </form>`;
    document.getElementById('submit-form').onsubmit = async e => {
      e.preventDefault();
      let msgDiv = document.getElementById('submit-form-message');
      if (msgDiv) msgDiv.textContent = '';
      const fd = new FormData(e.target);
      const data = Object.fromEntries(fd.entries());
      const resp = await fetch('/api/conversions', {
        method: 'POST',
        headers: {'Content-Type':'application/json'},
        credentials: 'include',
        body: JSON.stringify(data)
      });
      if (resp.ok) {
        msgDiv.className = 'mt-2 text-green-700 text-lg';
        msgDiv.textContent = 'Submitted! Your conversion will be available, but will not be marked as verified until an admin reviews it manually. Use the "Validate" button to request validation.';
        setTimeout(() => goto('conversions'), 30000);
      } else if (resp.status === 401) {
        msgDiv.className = 'mt-2 text-red-700 text-lg';
        msgDiv.textContent = 'You must be logged in to submit a conversion.';
        setTimeout(() => goto('profile'), 15000);
      } else {
        let err = 'Error submitting.';
        try {
          const data = await resp.json();
          if (data && data.error) err = data.error;
        } catch {}
        msgDiv.className = 'mt-2 text-red-700 text-lg';
        msgDiv.textContent = err;
      }
    };
  });
}

function renderProfile() {
  fetch('/api/profile').then(async r => {
    if (r.status === 401) {
      app.innerHTML = navBar() + `<div class="text-center"><h2 class="freedom-font text-3xl text-white">Please log in</h2>
      <form id="login-form" class="bg-white p-6 rounded shadow-md max-w-md mx-auto mt-4">
        <input name="username" placeholder="Username" class="block w-full mb-2 p-2 border rounded" required>
        <input name="password" type="password" placeholder="Password" class="block w-full mb-2 p-2 border rounded" required>
        <button class="bg-blue-600 text-white px-4 py-2 rounded mt-2">Login</button>
        <div id="login-form-message" class="mt-2 text-red-700 text-lg"></div>
      </form>
      <form id="signup-form" class="bg-white p-6 rounded shadow-md max-w-md mx-auto mt-4">
        <input name="username" placeholder="Username" class="block w-full mb-2 p-2 border rounded" required>
        <input name="password" type="password" placeholder="Password" class="block w-full mb-2 p-2 border rounded" required>
        <button class="bg-green-600 text-white px-4 py-2 rounded mt-2">Sign Up</button>
      </form></div>`;
      document.getElementById('login-form').onsubmit = async e => {
        e.preventDefault();
        // Remove any previous message
        let msgDiv = document.getElementById('login-form-message');
        if (msgDiv) msgDiv.textContent = '';
        const fd = new FormData(e.target);
        const data = Object.fromEntries(fd.entries());
        const resp = await fetch('/api/login', {
          method: 'POST',
          headers: {'Content-Type':'application/json'},
          body: JSON.stringify(data)
        });
        if (resp.ok) {
          goto('profile');
        } else {
          const err = await resp.json();
          if (msgDiv) {
            msgDiv.textContent = err.error || 'Login failed';
          }
        }
      };
      document.getElementById('signup-form').onsubmit = async e => {
        e.preventDefault();
        const fd = new FormData(e.target);
        const data = Object.fromEntries(fd.entries());
        const resp = await fetch('/api/signup', {
          method: 'POST',
          headers: {'Content-Type':'application/json'},
          body: JSON.stringify(data)
        });
        if (resp.ok) {
          goto('profile');
        } else {
          alert('Signup failed');
        }
      };
    } else {
      const data = await r.json();
      let flagHtml = '';
      if (data.flag) {
        flagHtml = `<div class="mt-8 text-4xl text-white animate-pulse freedom-font" style="text-shadow:0 0 10px #fff,0 0 20px #f00;">${data.flag}</div>`;
      }
      app.innerHTML = navBar() + `<div class="text-center">
        <h2 class="freedom-font text-3xl text-white">Welcome, ${data.username}${data.is_admin ? ' (Admin)' : ''}</h2>
        ${flagHtml}
        <button onclick="logout()" class="bg-red-600 text-white px-4 py-2 rounded mt-8">Logout</button>
        <button id="show-pw-form" class="bg-yellow-600 text-white px-4 py-2 rounded mt-8 ml-4">Change Password</button>
        <div id="pw-form-container"></div>
      </div>`;
      document.getElementById('show-pw-form').onclick = () => {
        document.getElementById('pw-form-container').innerHTML = `
          <form id="pw-form" class="bg-white p-6 rounded shadow-md max-w-md mx-auto mt-4">
            <input name="new_password" type="password" placeholder="New Password" class="block w-full mb-2 p-2 border rounded" required>
            <button class="bg-green-600 text-white px-4 py-2 rounded mt-2">Update Password</button>
          </form>
        `;
        document.getElementById('pw-form').onsubmit = async e => {
          e.preventDefault();
          // Remove any previous message
          let msgDiv = document.getElementById('pw-form-message');
          if (msgDiv) msgDiv.remove();
          const fd = new FormData(e.target);
          const data = Object.fromEntries(fd.entries());
          const resp = await fetch('/api/change-password', {
            method: 'PUT',
            headers: {'Content-Type':'application/json'},
            body: JSON.stringify(data)
          });
          const form = document.getElementById('pw-form');
          const container = document.getElementById('pw-form-container');
          const message = document.createElement('div');
          message.id = 'pw-form-message';
          message.className = 'mt-4 text-lg';
          if (resp.ok) {
            message.classList.add('text-green-700');
            message.textContent = 'Password changed!';
            form.reset();
            setTimeout(() => { container.innerHTML = ''; }, 1500);
          } else {
            const err = await resp.json();
            message.classList.add('text-red-700');
            message.textContent = err.error || 'Error changing password.';
          }
          form.appendChild(message);
        };
      };
    }
  });
}

function logout() {
  fetch('/api/logout', {method:'POST'}).then(()=>{
    goto('home');
  });
}

// Flying eagles, cheeseburgers, and more patriotic emojis - EXCITING MULTI-EMOJI ANIMATION
const emojis = [];
const PATRIOTIC_EMOJIS = [
  '🦅', // Eagle
  '🍔', // Burger
  '🎆', // Fireworks
  '🗽', // Statue of Liberty
  '🥧', // Pie
  '🌭', // Hot Dog
  '🤠', // Cowboy Hat
  '🚀', // Rocket
  '🎖️', // Medal
  '🏈', // Football
];
function spawnEmoji(emoji) {
  const w = window.innerWidth;
  const h = window.innerHeight;
  // Limit to 5 emojis at a time
  if (emojis.length >= 5) return;
  emojis.push({
    emoji,
    x: -100,
    y: Math.random() * h * 0.8 + 30,
    baseY: 0, // for oscillation
    speed: 3 + Math.random() * 5,
    size: 64 + Math.random() * 64,
    angle: Math.random() * Math.PI * 2,
    spin: (Math.random() - 0.5) * 0.2, // radians per frame
    oscillate: Math.random() * 2 > 1,
    oscillAmp: 20 + Math.random() * 40,
    oscillFreq: 0.01 + Math.random() * 0.02,
    alpha: 1,
    trail: Math.random() > 0.7
  });
}

function animateEmojis() {
  const canvas = document.getElementById('eagle-canvas');
  const ctx = canvas.getContext('2d');
  function resize() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
  }
  window.addEventListener('resize', resize);
  resize();

  // Add new emoji every 600ms for more excitement, but only if under limit
  setInterval(() => {
    if (emojis.length < 5) {
      const emoji = PATRIOTIC_EMOJIS[Math.floor(Math.random() * PATRIOTIC_EMOJIS.length)];
      spawnEmoji(emoji);
    }
  }, 600);

  function draw() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    for (let i = 0; i < emojis.length; i++) {
      const e = emojis[i];
      // Exciting: oscillate vertically, spin, and fade out at end
      e.x += e.speed;
      if (e.oscillate) {
        e.y = e.y + Math.sin(e.x * e.oscillFreq) * e.oscillAmp;
      }
      e.angle += e.spin;
      // Fade out as it leaves screen
      if (e.x > canvas.width - 200) {
        e.alpha -= 0.01;
      }
      ctx.save();
      ctx.globalAlpha = Math.max(0, e.alpha);
      ctx.translate(e.x, e.y);
      ctx.rotate(e.angle);
      ctx.font = `${e.size}px serif`;
      ctx.shadowColor = e.emoji === '🦅' ? '#222' : '#f90';
      ctx.shadowBlur = 30;
      ctx.fillText(e.emoji, 0, 0);
      ctx.restore();
      // Exciting: trailing effect
      if (e.trail && Math.random() > 0.7) {
        ctx.save();
        ctx.globalAlpha = 0.2;
        ctx.font = `${e.size * 0.7}px serif`;
        ctx.shadowBlur = 10;
        ctx.fillText(e.emoji, e.x - 30, e.y + 20);
        ctx.restore();
      }
    }
    // Remove emojis that are fully faded out or off screen
    for (let i = emojis.length - 1; i >= 0; i--) {
      if (emojis[i].x > canvas.width + 200 || emojis[i].alpha <= 0) {
        emojis.splice(i, 1);
      }
    }
    requestAnimationFrame(draw);
  }
  draw();
}
window.onload = () => {
  render();
  animateEmojis();
  // Konami code easter egg
  let code = [38,38,40,40,37,39,37,39,66,65], pos=0;
  window.addEventListener('keydown', e=>{
    if (e.keyCode === code[pos]) {
      pos++;
      if (pos===code.length) {
        konamiBurger();
        pos=0;
      }
    } else pos=0;
  });
};
function konamiBurger() {
  const canvas = document.getElementById('eagle-canvas');
  const ctx = canvas.getContext('2d');
  let angle = 0;
  let scale = 1;
  let burst = 0;
  let frame = 0;
  let crazyColors = ['#ff0', '#f00', '#0ff', '#0f0', '#00f', '#fff', '#f0f'];
  // Burger explosion effect
  function draw() {
    ctx.clearRect(0,0,canvas.width,canvas.height);
    // Background strobe
    ctx.save();
    ctx.globalAlpha = 0.2 + 0.2 * Math.sin(frame/2);
    ctx.fillStyle = crazyColors[frame % crazyColors.length];
    ctx.fillRect(0,0,canvas.width,canvas.height);
    ctx.restore();

    // Massive spinning, scaling, rainbow burger
    ctx.save();
    ctx.translate(canvas.width/2, canvas.height/2);
    ctx.rotate(angle);
    scale = 2 + Math.abs(Math.sin(angle*2))*2 + burst;
    ctx.scale(scale, scale);
    ctx.shadowColor = crazyColors[(frame+2)%crazyColors.length];
    ctx.shadowBlur = 100 + 100*Math.abs(Math.sin(angle*3));
    ctx.font = '200px serif';
    ctx.globalAlpha = 1;
    ctx.fillText('🍔', -100, 70);
    ctx.restore();

    // Burger confetti
    for (let i = 0; i < 30; i++) {
      ctx.save();
      let confAngle = (Math.PI*2/30)*i + angle*2;
      let r = 300 + 100*Math.sin(angle*2+i);
      let x = canvas.width/2 + Math.cos(confAngle)*r;
      let y = canvas.height/2 + Math.sin(confAngle)*r;
      ctx.translate(x, y);
      ctx.rotate(-angle*2 + i);
      ctx.font = `${40+20*Math.sin(angle+i)}px serif`;
      ctx.globalAlpha = 0.7;
      ctx.shadowColor = crazyColors[i%crazyColors.length];
      ctx.shadowBlur = 30;
      ctx.fillText('🍔', 0, 0);
      ctx.restore();
    }

    // Fireworks effect
    for (let i = 0; i < 10; i++) {
      ctx.save();
      let confAngle = (Math.PI*2/10)*i + angle*3;
      let r = 400 + 80*Math.sin(angle*3+i);
      let x = canvas.width/2 + Math.cos(confAngle)*r;
      let y = canvas.height/2 + Math.sin(confAngle)*r;
      ctx.translate(x, y);
      ctx.rotate(angle*2 + i);
      ctx.font = `60px serif`;
      ctx.globalAlpha = 0.8;
      ctx.shadowColor = crazyColors[(i+3)%crazyColors.length];
      ctx.shadowBlur = 40;
      ctx.fillText('🎆', 0, 0);
      ctx.restore();
    }

    // Burger burst pulse
    if (frame > 30 && burst < 10) burst += 0.5;
    if (burst > 0) burst *= 0.98;

    angle += 0.12 + 0.05*Math.sin(frame/5);
    frame++;
    if (frame < 120) {
      requestAnimationFrame(draw);
    } else {
      // Fade out
      let fade = 1;
      function fadeOut() {
        ctx.clearRect(0,0,canvas.width,canvas.height);
        ctx.save();
        ctx.globalAlpha = fade;
        ctx.translate(canvas.width/2, canvas.height/2);
        ctx.font = '200px serif';
        ctx.shadowColor = '#fff';
        ctx.shadowBlur = 50;
        ctx.fillText('🍔', -100, 70);
        ctx.restore();
        fade -= 0.05;
        if (fade > 0) requestAnimationFrame(fadeOut);
      }
      fadeOut();
    }
  }
  draw();
}
