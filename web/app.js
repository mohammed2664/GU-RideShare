/* app.js — GU RideShare Frontend Application Logic (C11 Engine Bridge)
 *
 * Manages:
 *  - Campus Authentication (@unigoa.ac.in Student/Faculty/Staff) with 1-click team demo logins
 *  - Ride & Booking CRUD operations via Python/C11 bridge
 *  - Driver in-place Ride Editing (C11 Binary Search)
 *  - Multi-Criteria Linear Route/Date/MinSeats Filtering
 *  - Digital Campus Transit Pass modal with dynamic QR Code generator and Boarding OTP
 *  - Formal C Audit Report export (campus_summary.txt)
 *  - Leaflet GIS Campus Map and 5x5 Hub Matrix
 *  - UPI QR Code settlement
 */

let offerSeatsCount = 3;
let bookSeatsCount = 1;
let allRides = [];
let allBookings = [];
let authMode = 'login';
let currentUser = null;
let authToken = null;
let currentPayingBooking = null;
let leafletMap = null;
let mapRouteLine = null;
let currentNotifications = [];

/* ── XSS Neutralizer ──────────────────────────────────────────── */
function escapeHtml(str) {
  if (str === null || str === undefined) return '';
  return String(str)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#039;');
}

/* ── Leaflet Map Coordinates ───────────────────────────────────── */
const LOCATION_COORDS = {
  0: { name: "Goa University", lat: 15.4593, lng: 73.8340, desc: "Taleigão Plateau Main Campus" },
  1: { name: "Panjim", lat: 15.4909, lng: 73.8278, desc: "KTC Bus Stand / Miramar Circle" },
  2: { name: "Margao", lat: 15.2736, lng: 73.9580, desc: "KTC Margao / Station Hub" },
  3: { name: "Vasco", lat: 15.3982, lng: 73.8113, desc: "Vasco City / Airport Junction" },
  4: { name: "Ponda", lat: 15.4026, lng: 74.0086, desc: "Ponda Bus Stand / Tisk" }
};

const CAMPUS_PINS = [
  { name: "Main Administrative Gate", lat: 15.4620, lng: 73.8345 },
  { name: "Faculty of Science & Tech", lat: 15.4570, lng: 73.8328 },
  { name: "University Central Library", lat: 15.4582, lng: 73.8336 },
  { name: "Silver Jubilee Hall", lat: 15.4595, lng: 73.8312 },
  { name: "Hostels & Sports Complex", lat: 15.4545, lng: 73.8355 }
];

/* ── Map Initialisation ───────────────────────────────────────── */
function initMapIfNeeded() {
  if (leafletMap) {
    setTimeout(() => leafletMap.invalidateSize(), 200);
    return;
  }
  const mapEl = document.getElementById('leaflet-map');
  if (!mapEl) return;

  leafletMap = L.map('leaflet-map').setView([15.42, 73.89], 11);
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 18,
    attribution: '&copy; OpenStreetMap contributors'
  }).addTo(leafletMap);

  // Hub Markers
  Object.keys(LOCATION_COORDS).forEach(id => {
    const hub = LOCATION_COORDS[id];
    const isGU = id === "0";
    const iconHtml = isGU
      ? `<div style="background:#193b70;color:#fff;border:2px solid #fff;border-radius:50%;width:30px;height:30px;display:flex;align-items:center;justify-content:center;box-shadow:0 2px 6px rgba(0,0,0,0.3);"><span class="material-symbols-outlined" style="font-size:18px;">school</span></div>`
      : `<div style="background:#1c796c;color:#fff;border:2px solid #fff;border-radius:50%;width:26px;height:26px;display:flex;align-items:center;justify-content:center;box-shadow:0 2px 5px rgba(0,0,0,0.25);"><span class="material-symbols-outlined" style="font-size:15px;">location_on</span></div>`;
    const customIcon = L.divIcon({ html: iconHtml, className: '', iconSize: [30, 30], iconAnchor: [15, 15] });
    L.marker([hub.lat, hub.lng], { icon: customIcon }).addTo(leafletMap)
      .bindPopup(`<b>${escapeHtml(hub.name)}</b><br><span style="font-size:11px;color:#666;">${escapeHtml(hub.desc)}</span>`);
  });

  // Campus Micro-markers
  CAMPUS_PINS.forEach(p => {
    const miniIcon = L.divIcon({
      html: `<div style="background:#294f8b;width:10px;height:10px;border-radius:50%;border:1.5px solid #fff;box-shadow:0 1px 3px rgba(0,0,0,0.3);"></div>`,
      className: '', iconSize: [10, 10], iconAnchor: [5, 5]
    });
    L.marker([p.lat, p.lng], { icon: miniIcon }).addTo(leafletMap)
      .bindPopup(`<b>Campus Landmark:</b><br>${escapeHtml(p.name)}`);
  });
}

function resetMapView() {
  if (!leafletMap) return;
  leafletMap.setView([15.42, 73.89], 11);
  if (mapRouteLine) { leafletMap.removeLayer(mapRouteLine); mapRouteLine = null; }
}

function viewRideOnMap(originId, destId, routeTitle) {
  switchTab('map');
  initMapIfNeeded();
  const orig = LOCATION_COORDS[originId];
  const dest = LOCATION_COORDS[destId];
  if (!orig || !dest || !leafletMap) return;

  if (mapRouteLine) { leafletMap.removeLayer(mapRouteLine); }

  const latlngs = [
    [orig.lat, orig.lng],
    [(orig.lat + dest.lat) / 2 + 0.015, (orig.lng + dest.lng) / 2],
    [dest.lat, dest.lng]
  ];

  mapRouteLine = L.polyline(latlngs, {
    color: '#193b70',
    weight: 5,
    opacity: 0.85,
    dashArray: '8, 8'
  }).addTo(leafletMap);

  leafletMap.fitBounds(mapRouteLine.getBounds(), { padding: [50, 50] });
  showToast(`Showing route on map: ${routeTitle}`);
}

/* ── Authenticated Fetch ──────────────────────────────────────── */
function apiFetch(url, options = {}) {
  const headers = { ...(options.headers || {}) };
  if (authToken) headers.Authorization = `Bearer ${authToken}`;
  return fetch(url, { ...options, headers });
}

/* ── Authentication ───────────────────────────────────────────── */
function setAuthMode(mode) {
  authMode = mode;
  const registering = mode === 'register';
  const resetting = mode === 'reset';

  const nameWrap = document.getElementById('auth-name-wrap');
  const roleWrap = document.getElementById('auth-role-wrap');
  const phoneWrap = document.getElementById('auth-phone-wrap');
  const forgotLink = document.getElementById('auth-forgot-link');
  const passLabel = document.getElementById('auth-pass-label');

  if (nameWrap) {
    nameWrap.classList.toggle('hidden', !registering);
    nameWrap.classList.toggle('flex', registering);
    document.getElementById('auth-name').required = registering;
  }
  if (roleWrap) {
    roleWrap.classList.toggle('hidden', !registering);
    roleWrap.classList.toggle('flex', registering);
  }
  if (phoneWrap) {
    phoneWrap.classList.toggle('hidden', !registering && !resetting);
    phoneWrap.classList.toggle('flex', registering || resetting);
    const phoneInput = document.getElementById('auth-phone');
    if (phoneInput) phoneInput.required = registering || resetting;
  }
  if (forgotLink) {
    forgotLink.classList.toggle('hidden', resetting);
  }
  if (passLabel) {
    passLabel.innerText = resetting ? 'New Account PIN / Password' : 'Account PIN / Password';
  }

  const submitBtn = document.getElementById('auth-submit');
  if (submitBtn) {
    if (resetting) submitBtn.innerText = 'Reset Security PIN';
    else if (registering) submitBtn.innerText = 'Register with Goa Univ';
    else submitBtn.innerText = 'Sign in';
  }

  document.getElementById('auth-login-tab').className = (mode === 'login')
    ? 'flex-1 rounded-lg bg-surface-container-lowest py-2 text-sm font-bold text-primary shadow-sm'
    : 'flex-1 rounded-lg py-2 text-sm font-semibold text-on-surface-variant';
  document.getElementById('auth-register-tab').className = registering
    ? 'flex-1 rounded-lg bg-surface-container-lowest py-2 text-sm font-bold text-primary shadow-sm'
    : 'flex-1 rounded-lg py-2 text-sm font-semibold text-on-surface-variant';
}

function toggleAuthModal() {
  const overlay = document.getElementById('auth-overlay');
  overlay.classList.toggle('hidden');
  overlay.classList.toggle('flex');
}

async function submitAuth(event) {
  if (event && event.preventDefault) event.preventDefault();
  const email = document.getElementById('auth-email').value.trim();
  const password = document.getElementById('auth-password').value;

  if (authMode === 'reset') {
    const phone = document.getElementById('auth-phone').value.trim();
    const res = await fetch('/api/auth/reset-pin', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ email, phone, newPin: password }),
    });
    const data = await res.json();
    if (!res.ok) {
      showToast(data.error || 'PIN reset failed.', true);
      return;
    }
    showToast(data.message || 'PIN reset successfully. Please sign in.');
    setAuthMode('login');
    document.getElementById('auth-password').value = '';
    return;
  }

  const payload = { email, pin: password, password };

  if (authMode === 'register') {
    payload.name = document.getElementById('auth-name').value.trim();
    payload.role = document.getElementById('auth-role').value;
    payload.phone = document.getElementById('auth-phone').value.trim() || '9876543210';
  }

  const endpoint = authMode === 'register' ? '/api/register' : '/api/login';
  const res = await fetch(endpoint, {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload),
  });
  const data = await res.json();
  if (!res.ok) {
    showToast(data.error || 'Authentication failed.', true);
    return;
  }
  authToken = data.token;
  currentUser = data.user;
  localStorage.setItem('guRideShareSession', JSON.stringify({ token: authToken, user: currentUser }));
  applyCurrentUser();
  await loadDashboard('id');
  showToast(data.message || 'Authenticated successfully.');
}

function quickLoginTeam(email, pin) {
  setAuthMode('login');
  document.getElementById('auth-email').value = email;
  document.getElementById('auth-password').value = pin;
  submitAuth(new Event('submit'));
}

function applyCurrentUser() {
  const signedIn = Boolean(currentUser && authToken);
  const overlay = document.getElementById('auth-overlay');
  overlay.classList.toggle('hidden', signedIn);
  overlay.classList.toggle('flex', !signedIn);
  document.getElementById('sign-out-btn').classList.toggle('hidden', !signedIn);

  if (!signedIn) {
    const badge = document.getElementById('notif-badge');
    if (badge) badge.classList.add('hidden');
    return;
  }
  document.getElementById('profile-name').innerText = currentUser.name;
  document.getElementById('profile-role').innerText = `${currentUser.role || 'Member'} • ${currentUser.email}`;
  document.getElementById('profile-initials').innerText = initials(currentUser.name);
  document.getElementById('offer-name').value = currentUser.name;
  document.getElementById('book-passenger').value = currentUser.name;
  document.getElementById('primary-action-label').innerText = 'Offer a ride or reserve a seat';

  const today = new Date().toISOString().split('T')[0];
  const dateEl = document.getElementById('offer-date');
  if (dateEl) {
    dateEl.value = today;
    dateEl.min = today;
  }
  fetchNotifications();
}

async function signOut() {
  if (authToken) {
    await apiFetch('/api/auth/logout', { method: 'POST' });
  }
  localStorage.removeItem('guRideShareSession');
  authToken = null;
  currentUser = null;
  document.getElementById('profile-name').innerText = 'Guest';
  document.getElementById('profile-role').innerText = 'Sign in to continue';
  document.getElementById('profile-initials').innerText = 'GU';
  applyCurrentUser();
  showToast('You have been signed out.');
}

function initials(name) {
  return name.split(/\s+/).filter(Boolean).slice(0, 2).map(w => w[0].toUpperCase()).join('') || 'GU';
}

/* ── Rendering ────────────────────────────────────────────────── */
function renderSuggestion(suggestion) {
  const banner = document.getElementById('hub-suggestion-banner');
  if (!suggestion) {
    banner.classList.add('hidden');
    banner.classList.remove('flex');
    return;
  }
  document.getElementById('hub-suggestion-text').innerText =
    `No direct carpool found. Recommended 1-Stop Transfer: ${suggestion.route} (${suggestion.distanceKm} km total).`;
  banner.classList.remove('hidden');
  banner.classList.add('flex');
}

function renderRides(rides) {
  const grid = document.getElementById('rides-grid');
  const label = document.getElementById('rides-count-label');
  label.innerText = `Showing ${rides.length} live ${rides.length === 1 ? 'route' : 'routes'}`;

  if (!rides.length) {
    grid.innerHTML = `<div class="col-span-full bg-surface-container-lowest rounded-2xl p-6 text-center text-sm text-on-surface-variant">No matching rides found. Try another origin, destination, date, or seat requirement.</div>`;
    return;
  }

  const userEmail = (currentUser && currentUser.email || '').toLowerCase();
  const userName = (currentUser && currentUser.name || '').toLowerCase();

  grid.innerHTML = rides.map(r => {
    const isMyRide = (r.driverEmail && r.driverEmail.toLowerCase() === userEmail) || (r.driverName && r.driverName.toLowerCase() === userName);
    const seatBadgeClass = r.availableSeats <= 1
      ? 'bg-error-container text-on-error-container'
      : 'bg-secondary-container/60 text-on-secondary-container';
    const parts = r.routeDescription.split(' -> ');
    const viaText = parts.length === 3 ? `Via ${parts[1]}` : 'Direct Route';

    return `
      <div class="bg-surface-container-lowest rounded-2xl p-4 shadow-sm flex flex-col gap-3 border ${isMyRide ? 'border-primary/50 ring-1 ring-primary/20' : 'border-surface-container'}">
        <div class="flex items-center justify-between">
          <div class="flex items-center gap-1.5 flex-wrap">
            <span class="font-mono text-xs bg-surface-container-high text-primary font-bold px-2 py-0.5 rounded">#${r.rideId}</span>
            <span class="font-label-sm text-xs ${seatBadgeClass} font-semibold px-2 py-0.5 rounded-full flex items-center gap-1">
              <span class="material-symbols-outlined text-[13px]">airline_seat_recline_normal</span> ${r.availableSeats}/${r.totalSeats} Avail
            </span>
            ${isMyRide ? `<span class="font-label-sm text-[11px] bg-primary text-on-primary font-bold px-2 py-0.5 rounded-full">Your Ride</span>` : ''}
          </div>
          <span class="text-lg text-primary font-bold">&#8377;${r.farePerSeat}</span>
        </div>

        <div class="flex items-center gap-2.5">
          <div class="w-10 h-10 rounded-full bg-surface-variant flex items-center justify-center text-primary font-bold text-xs shrink-0">${initials(r.driverName)}</div>
          <div class="flex flex-col min-w-0">
            <div class="flex items-center gap-1.5 flex-wrap">
              <span class="text-sm font-bold text-on-surface truncate">${escapeHtml(r.driverName)}</span>
              ${r.isDriverVerified ? `
                <span class="inline-flex items-center gap-0.5 px-2 py-0.5 rounded-full text-[10px] font-bold bg-teal-100 text-teal-800 border border-teal-200" title="Goa University Dean & Security Checked">
                  <span class="material-symbols-outlined text-[13px] text-teal-700">verified_user</span>
                  Verified (${r.driverTrustScore || 95}%)
                </span>
              ` : `
                <span class="inline-flex items-center gap-0.5 px-2 py-0.5 rounded-full text-[10px] font-medium bg-amber-50 text-amber-700 border border-amber-200" title="Campus Verification In Progress">
                  <span class="material-symbols-outlined text-[13px] text-amber-600">hourglass_top</span>
                  Pending KYC (${r.driverTrustScore || 55}%)
                </span>
              `}
            </div>
            <div class="flex items-center gap-1.5 text-xs text-on-surface-variant flex-wrap mt-0.5">
              <span class="font-mono font-bold text-primary bg-primary/5 px-1.5 py-0.5 rounded text-[11px] border border-primary/10">${escapeHtml(r.vehicleNumber)}</span>
              ${r.driverPhone ? `<span>&bull; &#128222; ${escapeHtml(r.driverPhone)}</span>` : ''}
            </div>
            ${r.vehicleDesc ? `
              <div class="mt-1 flex items-center gap-1 text-[11px] bg-surface-container-low px-2 py-0.5 rounded-md border border-surface-container/60 truncate" title="Visual vehicle recognition guide for campus pickup">
                <span class="material-symbols-outlined text-[13px] text-secondary shrink-0">visibility</span>
                <span class="truncate font-medium text-on-surface"><span class="text-[10px] uppercase font-semibold text-on-surface-variant mr-0.5">Vehicle:</span>${escapeHtml(r.vehicleDesc)}</span>
              </div>
            ` : ''}
          </div>
        </div>

        <div class="bg-surface-container-low rounded-xl p-2.5 flex flex-col gap-1">
          <div class="flex items-center gap-2">
            <span class="w-2.5 h-2.5 rounded-full bg-primary shrink-0"></span>
            <span class="text-sm font-medium text-on-surface truncate">${escapeHtml(r.originName)}</span>
            <span class="font-mono text-xs text-on-surface-variant ml-auto font-semibold">${escapeHtml(r.formattedTime)}</span>
          </div>
          <div class="ml-1 pl-3 border-l-2 border-dashed border-outline-variant py-0.5">
            <span class="text-xs text-on-surface-variant flex items-center gap-1 truncate">
              <span class="material-symbols-outlined text-[14px]">alt_route</span> ${escapeHtml(viaText)}
            </span>
          </div>
          <div class="flex items-center gap-2">
            <span class="w-2.5 h-2.5 rounded-sm bg-secondary shrink-0"></span>
            <span class="text-sm font-bold text-on-surface truncate">${escapeHtml(r.destinationName)}</span>
            <span class="font-mono text-xs text-secondary font-semibold ml-auto">${r.distanceKm} km</span>
          </div>
          <div class="pt-1 border-t border-surface-container/60 flex items-center justify-between text-[11px] text-on-surface-variant">
            <span>&#128205; ${escapeHtml(r.campusPoint || 'Main Gate')}</span>
            <span>&#128197; ${escapeHtml(r.rideDate || '2026-11-23')}</span>
          </div>
        </div>

        <div class="flex items-center justify-between pt-1 flex-wrap gap-2">
          <div class="flex items-center gap-1.5">
            <button class="text-xs text-primary font-semibold hover:underline flex items-center gap-0.5" onclick="viewRideOnMap(${r.originId}, ${r.destinationId}, '${escapeHtml(r.originName)} to ${escapeHtml(r.destinationName)}')">
              <span class="material-symbols-outlined text-[15px]">map</span> View on Map
            </button>
            <button class="text-xs text-secondary font-bold hover:underline flex items-center gap-0.5 ml-2" onclick="openLiveRideTracker(null, ${r.rideId})">
              <span class="material-symbols-outlined text-[15px]">radar</span> Live GPS
            </button>
          </div>

          <div class="flex items-center gap-1.5 ml-auto">
            ${isMyRide ? `
              <button class="h-8 px-2.5 bg-primary/10 text-primary hover:bg-primary hover:text-white rounded-lg text-xs font-semibold flex items-center gap-1 transition-all" onclick="openEditRideModal(${r.rideId}, '${escapeHtml(r.departureTime)}', ${r.totalSeats}, '${escapeHtml(r.originName)}', '${escapeHtml(r.destinationName)}', '${escapeHtml(r.vehicleDesc || '')}')">
                <span class="material-symbols-outlined text-[14px]">edit</span> Edit
              </button>
              <button class="h-8 px-2.5 bg-error/10 text-error hover:bg-error hover:text-white rounded-lg text-xs font-semibold flex items-center gap-1 transition-all" onclick="cancelMyRide(${r.rideId})">
                <span class="material-symbols-outlined text-[14px]">cancel</span> Cancel
              </button>
              <button class="h-8 px-2.5 bg-secondary/10 text-secondary hover:bg-secondary hover:text-white rounded-lg text-xs font-semibold flex items-center gap-1 transition-all" onclick="completeMyRide(${r.rideId})">
                <span class="material-symbols-outlined text-[14px]">check</span> Complete
              </button>
            ` : `
              <button class="h-9 px-3.5 bg-primary hover:bg-primary-container text-on-primary rounded-lg font-label-md text-xs font-semibold flex items-center gap-1 shadow-sm disabled:opacity-40"
                ${r.availableSeats === 0 ? 'disabled' : ''}
                onclick="prefillBooking(${r.rideId})">
                <span>${r.availableSeats === 0 ? 'Full' : 'Reserve Seat'}</span>
                <span class="material-symbols-outlined text-[16px]">chevron_right</span>
              </button>
            `}
          </div>
        </div>
      </div>
    `;
  }).join('');
}

function populateBookingSelect(rides) {
  const sel = document.getElementById('book-ride-select');
  const userEmail = (currentUser && currentUser.email ? currentUser.email.toLowerCase() : '');
  const userName = (currentUser && currentUser.name ? currentUser.name.toLowerCase() : '');
  // Filter out full rides, expired/completed rides, and driver's own rides
  const bookableRides = (rides || []).filter(r => {
    if (r.availableSeats <= 0) return false;
    if (r.status === 'EXPIRED' || r.status === 'CANCELLED' || r.status === 'COMPLETED') return false;
    const isDriver = (r.driverEmail && r.driverEmail.toLowerCase() === userEmail) || (r.driverName && r.driverName.toLowerCase() === userName);
    return !isDriver;
  });

  if (!bookableRides.length) {
    sel.innerHTML = '<option value="" disabled selected>No active rides available from other drivers</option>';
  } else {
    sel.innerHTML = bookableRides.map(r =>
      `<option value="${r.rideId}" data-fare="${r.farePerSeat}">#${r.rideId}: ${escapeHtml(r.driverName)} — ${escapeHtml(r.originName)} → ${escapeHtml(r.destinationName)} (₹${r.farePerSeat})</option>`
    ).join('');
  }
  updateBookingEstimate();
}

function renderBookings(bookings) {
  const grid = document.getElementById('bookings-grid');
  document.getElementById('bookings-count-label').innerText = `${bookings.length} Entries`;
  document.getElementById('tab-count-bookings').innerText = bookings.length;

  if (!bookings.length) {
    grid.innerHTML = `<div class="col-span-full rounded-2xl bg-surface-container-lowest p-6 text-center text-sm text-on-surface-variant">No bookings found for this account. Reserve a seat from available rides!</div>`;
    return;
  }

  grid.innerHTML = bookings.map(b => {
    const isConfirmed = b.bookingStatus === 'CONFIRMED';
    const isPaid = b.paymentStatus === 'PAID';
    const statusBadge = isConfirmed
      ? 'bg-secondary-container/70 text-on-secondary-container'
      : 'bg-error-container text-on-error-container';

    const matchedRide = allRides.find(r => r.rideId === b.rideId);
    const driverPhone = matchedRide && matchedRide.driverPhone ? matchedRide.driverPhone.replace(/\D/g, '') : '';

    return `
      <div class="bg-surface-container-lowest rounded-2xl p-4 shadow-sm flex flex-col gap-2.5 border border-surface-container">
        <div class="flex items-center justify-between">
          <div class="flex items-center gap-2">
            <span class="font-mono text-sm text-primary font-bold">#BK-${b.bookingId}</span>
            <span class="font-mono text-xs bg-surface-container-high text-on-surface-variant px-2 py-0.5 rounded font-medium">Ride #${b.rideId}</span>
          </div>
          <span class="inline-flex items-center gap-1 px-2.5 py-0.5 rounded-full ${statusBadge} font-label-sm text-xs font-bold">
            <span class="material-symbols-outlined text-[13px]">${isConfirmed ? 'check_circle' : 'cancel'}</span> ${escapeHtml(b.bookingStatus)}
          </span>
        </div>

        ${isConfirmed && b.otp ? `
          <div class="bg-primary/5 border border-primary/20 rounded-xl p-2.5 flex items-center justify-between">
            <div class="flex items-center gap-1.5">
              <span class="material-symbols-outlined text-primary text-[18px]">pin</span>
              <span class="text-xs font-semibold text-primary">Boarding OTP:</span>
            </div>
            <span class="font-mono text-sm font-extrabold text-primary tracking-widest bg-surface-container-lowest px-2 py-0.5 rounded shadow-sm border border-primary/10">${escapeHtml(b.otp)}</span>
          </div>
        ` : ''}

        <div class="flex items-center justify-between pt-1">
          <div class="flex flex-col">
            <span class="text-sm font-bold text-on-surface">${escapeHtml(b.passengerName)}</span>
            <span class="text-xs text-on-surface-variant">${b.seatsBooked} Reserved ${b.seatsBooked === 1 ? 'Seat' : 'Seats'}</span>
          </div>
          <div class="text-right">
            <span class="font-label-sm text-[11px] text-on-surface-variant">Total Fare</span>
            <p class="text-base font-bold text-primary">&#8377;${b.totalFare}</p>
            <span class="inline-block text-[10px] px-2 py-0.2 rounded-full font-semibold ${isPaid ? 'bg-secondary-container/80 text-on-secondary-container' : 'bg-amber-100 text-amber-800'}">
              ${isPaid ? 'Payment Paid' : 'Payment Due'}
            </span>
          </div>
        </div>

        ${isConfirmed ? `
          <div class="pt-2 border-t border-surface-container flex items-center justify-between flex-wrap gap-2">
            <div class="flex items-center gap-2 flex-wrap">
              <button class="text-xs font-label-sm font-bold text-white bg-primary hover:bg-primary/90 flex items-center gap-1 px-2.5 py-1 rounded-lg shadow-sm" onclick="openLiveRideTracker(${b.bookingId}, ${b.rideId})">
                <span class="material-symbols-outlined text-[16px]">radar</span> Track Ride
              </button>
              <button class="text-xs font-label-sm font-bold text-primary hover:underline flex items-center gap-1 bg-primary/5 px-2.5 py-1 rounded-lg" onclick="openDigitalPass(${b.bookingId}, ${b.rideId})">
                <span class="material-symbols-outlined text-[16px]">badge</span> Digital Pass
              </button>
              ${driverPhone ? `
                <a href="https://wa.me/91${driverPhone}?text=Hi%2C%20I%20have%20booked%20seat(s)%20on%20Ride%20%23${b.rideId}%20on%20GU%20RideShare." target="_blank" class="text-xs font-label-sm font-semibold text-secondary hover:underline flex items-center gap-1">
                  <span class="material-symbols-outlined text-[15px]">chat</span> WhatsApp
                </a>
              ` : ''}
              <button class="text-xs font-label-sm font-semibold text-primary hover:underline flex items-center gap-1" onclick="openUpiPayment(${b.bookingId}, ${b.rideId}, ${b.totalFare}, '${escapeHtml(b.passengerName)}')">
                <span class="material-symbols-outlined text-[15px]">qr_code_2</span> Pay UPI
              </button>
            </div>
            <button class="text-xs font-label-sm font-semibold text-error hover:underline flex items-center gap-1" onclick="handleCancelBooking(${b.bookingId})">
              <span class="material-symbols-outlined text-[15px]">undo</span> Cancel Seat
            </button>
          </div>` : ''}
      </div>
    `;
  }).join('');
}

function renderMatrix(cells) {
  const hubs = ['Goa University', 'Panjim', 'Margao', 'Vasco', 'Ponda'];
  const map = {};
  cells.forEach(c => { map[`${c.originId}-${c.destId}`] = c; });

  let html = `<thead><tr class="bg-surface-container-high">
    <th class="p-2.5 font-label-sm text-xs text-primary font-bold text-left rounded-tl-lg">Origin \\ Dest</th>`;
  hubs.forEach(h => { html += `<th class="p-2.5 font-label-sm text-xs text-primary font-bold">${escapeHtml(h)}</th>`; });
  html += `</tr></thead><tbody class="text-xs divide-y divide-surface-container">`;

  hubs.forEach((h, i) => {
    html += `<tr><td class="p-2.5 font-bold text-primary text-left bg-surface-container-low">${i}: ${escapeHtml(h)}</td>`;
    hubs.forEach((_, j) => {
      const cell = map[`${i}-${j}`];
      if (i === j || !cell) {
        html += `<td class="p-2.5 bg-surface-container text-on-surface-variant font-label-sm">&mdash;</td>`;
      } else {
        html += `<td class="p-2.5"><span class="block font-bold text-on-surface">&#8377;${cell.fare}</span><span class="text-[10px] text-on-surface-variant">${cell.distanceKm} km</span></td>`;
      }
    });
    html += `</tr>`;
  });
  html += `</tbody>`;
  document.getElementById('matrix-table').innerHTML = html;
}

/* ── Dashboard Data Loading ───────────────────────────────────── */
async function loadDashboard(sortMode = 'id') {
  const [ridesRes, bookingsRes, statsRes, matrixRes] = await Promise.all([
    fetch(`/api/rides?sort=${sortMode}`).then(r => r.json()),
    apiFetch('/api/bookings?mine=1').then(r => r.json()),
    fetch('/api/stats').then(r => r.json()),
    fetch('/api/matrix').then(r => r.json()),
  ]);

  renderSuggestion(null);
  allRides = ridesRes.rides || [];
  allBookings = bookingsRes.bookings || [];

  renderRides(allRides);
  populateBookingSelect(allRides);
  renderBookings(allBookings);
  renderMatrix(matrixRes.matrix || []);

  const s = statsRes.stats || {};
  document.getElementById('kpi-rides').innerText = s.TotalRides || allRides.length;
  document.getElementById('kpi-seats').innerText = s.AvailableSeats || 0;
  document.getElementById('kpi-pool').innerText = `\u20B9${s.OpenFarePool || 0}`;
  document.getElementById('kpi-revenue').innerText = `\u20B9${s.BookedRevenue || 0}`;
  document.getElementById('tab-count-rides').innerText = s.TotalRides || allRides.length;

  if (currentUser && authToken) {
    fetchNotifications();
  }
}

/* ── Search, Multi-field Linear Filter, Sort ───────────────────── */
async function applyFilters() {
  const origin = document.getElementById('filter-origin').value;
  const dest = document.getElementById('filter-destination').value;
  const keyword = encodeURIComponent(document.getElementById('keyword-search').value.trim() || 'ALL');

  const dateInput = document.getElementById('filter-date');
  const dateVal = dateInput && dateInput.value ? dateInput.value : 'ALL';

  const minSeatsInput = document.getElementById('filter-min-seats');
  const minSeatsVal = minSeatsInput ? minSeatsInput.value : '0';

  const url = `/api/search?origin=${origin}&dest=${dest}&keyword=${keyword}&date=${encodeURIComponent(dateVal)}&minSeats=${minSeatsVal}`;
  const res = await fetch(url).then(r => r.json());
  renderRides(res.rides || []);
  renderSuggestion(res.suggestion || null);
  switchTab('rides');
}

function resetFilters() {
  document.getElementById('filter-origin').value = '-1';
  document.getElementById('filter-destination').value = '-1';
  document.getElementById('keyword-search').value = '';
  document.getElementById('ride-id-search').value = '';
  document.getElementById('filter-date').value = '';
  document.getElementById('filter-min-seats').value = '0';
  loadDashboard('id');
  showToast('Filters reset to default.');
}

async function lookupRideId() {
  const raw = document.getElementById('ride-id-search').value.trim().replace(/^#?GU-/i, '');
  const res = await fetch(`/api/find?id=${encodeURIComponent(raw)}`);
  const data = await res.json();
  renderSuggestion(null);
  if (!res.ok || data.message) {
    showToast(data.error || data.message || 'Ride ID not found.', true);
    renderRides([]);
    return;
  }
  renderRides(data.rides || []);
  switchTab('rides');
  showToast(`Ride #${raw} found via Recursive Binary Search.`);
}

async function toggleSort(type) {
  const timeBtn = document.getElementById('sort-time-btn');
  const fareBtn = document.getElementById('sort-fare-btn');
  if (type === 'time') {
    timeBtn.classList.replace('bg-surface-container-low', 'bg-surface-container');
    fareBtn.classList.replace('bg-surface-container', 'bg-surface-container-low');
    showToast('Sorted by departure time (Selection Sort - Lab 5).');
  } else {
    fareBtn.classList.replace('bg-surface-container-low', 'bg-surface-container');
    timeBtn.classList.replace('bg-surface-container', 'bg-surface-container-low');
    showToast('Sorted by lowest fare (Bubble Sort - Lab 6).');
  }
  const res = await fetch(`/api/rides?sort=${type}`).then(r => r.json());
  renderSuggestion(null);
  renderRides(res.rides || []);
}

function quickSelectPlateau() {
  document.getElementById('filter-destination').value = '0';
  applyFilters();
  showToast('Filtered for arrivals to Goa University (0).');
}

/* ── Driver Edit Ride (C11 Binary Search) ──────────────────────── */
function openEditRideModal(rideId, time, seats, originName, destName, vehicleDesc) {
  document.getElementById('edit-ride-id').value = rideId;
  document.getElementById('edit-ride-time').value = time;
  document.getElementById('edit-ride-seats').value = seats;
  document.getElementById('edit-ride-route-label').innerText = `Ride #${rideId}`;
  document.getElementById('edit-ride-desc-label').innerText = `${originName} → ${destName}`;
  const vDescEl = document.getElementById('edit-ride-vehicle-desc');
  if (vDescEl) vDescEl.value = vehicleDesc || '';
  const modal = document.getElementById('edit-ride-modal');
  modal.classList.remove('hidden');
  modal.classList.add('flex');
}

function closeEditRideModal() {
  const modal = document.getElementById('edit-ride-modal');
  modal.classList.add('hidden');
  modal.classList.remove('flex');
}

async function submitEditRide(event) {
  event.preventDefault();
  const rideId = document.getElementById('edit-ride-id').value;
  const time = document.getElementById('edit-ride-time').value.trim();
  const seats = document.getElementById('edit-ride-seats').value.trim();
  const vDescEl = document.getElementById('edit-ride-vehicle-desc');
  const vehicleDesc = vDescEl ? vDescEl.value.trim() : '';

  const res = await apiFetch('/api/edit-ride', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ rideId, departureTime: time, totalSeats: seats, vehicleDesc }),
  });
  const data = await res.json();
  if (!res.ok) {
    showToast(data.error || 'Edit failed.', true);
    return;
  }
  showToast(data.message || 'Ride updated successfully.');
  closeEditRideModal();
  await loadDashboard('id');
}

/* ── Digital Campus Boarding Pass with QR Code ─────────────────── */
function renderQrCodeSvg(containerId, text) {
  const container = document.getElementById(containerId);
  if (!container) return;

  // Use online QR service with automatic graceful offline SVG pattern fallback
  const imgUrl = `https://api.qrserver.com/v1/create-qr-code/?size=140x140&margin=0&data=${encodeURIComponent(text)}`;
  container.innerHTML = `
    <img src="${imgUrl}" alt="Digital Boarding QR" class="w-32 h-32 rounded-lg"
         onerror="this.onerror=null; renderOfflineQrFallback('${containerId}');"/>
  `;
}

function renderOfflineQrFallback(containerId) {
  const container = document.getElementById(containerId);
  if (!container) return;
  // Geometric SVG QR-like matrix representation when offline
  container.innerHTML = `
    <svg width="128" height="128" viewBox="0 0 21 21" fill="#193b70" class="rounded">
      <rect x="0" y="0" width="7" height="7" fill="none" stroke="#193b70" stroke-width="1"/>
      <rect x="2" y="2" width="3" height="3" fill="#193b70"/>
      <rect x="14" y="0" width="7" height="7" fill="none" stroke="#193b70" stroke-width="1"/>
      <rect x="16" y="2" width="3" height="3" fill="#193b70"/>
      <rect x="0" y="14" width="7" height="7" fill="none" stroke="#193b70" stroke-width="1"/>
      <rect x="2" y="16" width="3" height="3" fill="#193b70"/>
      <rect x="9" y="3" width="1" height="5"/>
      <rect x="11" y="5" width="2" height="1"/>
      <rect x="9" y="9" width="3" height="3"/>
      <rect x="15" y="9" width="2" height="4"/>
      <rect x="9" y="15" width="4" height="2"/>
      <rect x="15" y="15" width="4" height="4"/>
    </svg>
  `;
}

function openDigitalPass(bookingId, rideId) {
  const booking = (allBookings || []).find(b => b.bookingId === bookingId);
  const ride = (allRides || []).find(r => r.rideId === rideId);
  if (!booking) return;

  document.getElementById('pass-booking-id').innerText = `#BK-${booking.bookingId}`;
  document.getElementById('pass-passenger-name').innerText = booking.passengerName;
  document.getElementById('pass-passenger-role').innerText = (currentUser && currentUser.role) || 'Student (MCA)';
  document.getElementById('pass-otp-code').innerText = booking.otp || '----';

  if (ride) {
    document.getElementById('pass-origin').innerText = ride.originName;
    document.getElementById('pass-dest').innerText = ride.destinationName;
    document.getElementById('pass-time').innerText = ride.formattedTime || ride.departureTime;
    document.getElementById('pass-date').innerText = ride.rideDate || '2026-11-23';
    document.getElementById('pass-pickup').innerText = ride.campusPoint || 'Main Administrative Gate';
    document.getElementById('pass-driver-name').innerText = ride.driverName;
    document.getElementById('pass-vehicle-no').innerText = ride.vehicleNumber;
    const passVDescEl = document.getElementById('pass-vehicle-desc');
    if (passVDescEl) {
      passVDescEl.innerText = ride.vehicleDesc || 'Verified Campus Vehicle';
    }
    const trustEl = document.getElementById('pass-trust-score');
    if (trustEl) {
      trustEl.innerText = `${ride.driverTrustScore || 95}% Trust`;
    }
  }

  const qrData = `GU-CARPOOL|BK:${booking.bookingId}|RIDE:${rideId}|OTP:${booking.otp}|PAX:${booking.passengerName}|FARE:${booking.totalFare}`;
  renderQrCodeSvg('pass-qr-container', qrData);

  const modal = document.getElementById('digital-pass-modal');
  modal.classList.remove('hidden');
  modal.classList.add('flex');
}

function closeDigitalPassModal() {
  const modal = document.getElementById('digital-pass-modal');
  modal.classList.add('hidden');
  modal.classList.remove('flex');
}

/* ── Formal C Audit Report Export ──────────────────────────────── */
async function exportAuditReport() {
  showToast('Exporting C11 audit summary report...');
  try {
    const res = await fetch('/api/export-report');
    const data = await res.json();
    if (!res.ok) {
      showToast(data.error || 'Failed to export report.', true);
      return;
    }

    // Trigger file download in browser
    const blob = new Blob([data.report], { type: 'text/plain;charset=utf-8' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = data.filename || 'campus_summary.txt';
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);

    // Update the live preview box if present
    const previewEl = document.getElementById('audit-report-preview');
    if (previewEl) previewEl.innerText = data.report;

    showToast('C Audit Report exported: campus_summary.txt');
  } catch (err) {
    showToast('Failed to export audit report.', true);
  }
}

async function fetchReportPreview() {
  try {
    const res = await fetch('/api/export-report');
    const data = await res.json();
    const previewEl = document.getElementById('audit-report-preview');
    if (previewEl && data.report) {
      previewEl.innerText = data.report;
      showToast('Audit preview updated.');
    }
  } catch (_) {
    showToast('Could not fetch audit preview.', true);
  }
}

/* ── Ride & Booking Actions ───────────────────────────────────── */
async function handleOfferRide(e) {
  e.preventDefault();
  const vDescEl = document.getElementById('offer-vehicle-desc');
  const payload = {
    driverName: document.getElementById('offer-name').value.trim(),
    vehicleNumber: document.getElementById('offer-vehicle').value.trim(),
    vehicleDesc: vDescEl ? vDescEl.value.trim() : '',
    originId: document.getElementById('offer-origin').value,
    destinationId: document.getElementById('offer-destination').value,
    departureTime: document.getElementById('offer-time').value.trim(),
    totalSeats: offerSeatsCount,
    viaStop: document.getElementById('offer-stops').value.trim() || 'Direct',
    rideDate: document.getElementById('offer-date').value,
    driverPhone: document.getElementById('offer-phone').value.trim(),
    campusPoint: document.getElementById('offer-campus-point').value,
  };
  const res = await apiFetch('/api/add-ride', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload),
  });
  const data = await res.json();
  if (!res.ok) {
    showToast(data.error || 'Validation failed.', true);
    return;
  }
  showToast(data.message);
  toggleActionPanel();
  loadDashboard('id');
}

async function handleBookSeat(e) {
  e.preventDefault();
  const payload = {
    rideId: document.getElementById('book-ride-select').value,
    seatsRequested: bookSeatsCount,
  };
  const res = await apiFetch('/api/book', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload),
  });
  const data = await res.json();
  if (!res.ok) {
    showToast(data.error || 'Booking failed.', true);
    return;
  }
  showToast(data.message);
  toggleActionPanel();
  await loadDashboard('id');
  switchTab('bookings');
}

async function handleCancelBooking(bookingId) {
  if (!confirm('Cancel this booking and restore the seats?')) return;
  const res = await apiFetch('/api/cancel-booking', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ bookingId }),
  });
  const data = await res.json();
  if (!res.ok) {
    showToast(data.error || 'Cancellation failed.', true);
    return;
  }
  showToast(data.message);
  await loadDashboard('id');
  switchTab('bookings');
}

async function cancelMyRide(rideId) {
  if (!confirm(`Cancel Ride #${rideId}? All passenger bookings will be cancelled and seats released.`)) return;
  const res = await apiFetch('/api/cancel-ride', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ rideId }),
  });
  const data = await res.json();
  if (!res.ok) {
    showToast(data.error || 'Could not cancel ride.', true);
    return;
  }
  showToast(data.message);
  await loadDashboard('id');
}

async function completeMyRide(rideId) {
  if (!confirm(`Mark Ride #${rideId} as completed?`)) return;
  const res = await apiFetch('/api/complete-ride', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ rideId }),
  });
  const data = await res.json();
  if (!res.ok) {
    showToast(data.error || 'Failed to complete ride.', true);
    return;
  }
  showToast(data.message);
  await loadDashboard('id');
}

/* ── UPI Payment ──────────────────────────────────────────────── */
function openUpiPayment(bookingId, rideId, amount) {
  currentPayingBooking = bookingId;
  document.getElementById('upi-modal-amount').innerText = `₹${amount}`;
  const vpa = `campus.rideshare@upi`;
  document.getElementById('upi-modal-vpa').innerText = `VPA: ${vpa}`;

  const upiUrl = `upi://pay?pa=${vpa}&pn=GU+RideShare&am=${amount}&cu=INR&tn=Carpool+Ride+${rideId}`;
  document.getElementById('upi-intent-link').href = upiUrl;

  const qrApiUrl = `https://api.qrserver.com/v1/create-qr-code/?size=180x180&data=${encodeURIComponent(upiUrl)}`;
  document.getElementById('upi-qr-image').src = qrApiUrl;

  document.getElementById('upi-modal').classList.remove('hidden');
  document.getElementById('upi-modal').classList.add('flex');
}

function closeUpiModal() {
  document.getElementById('upi-modal').classList.add('hidden');
  document.getElementById('upi-modal').classList.remove('flex');
  currentPayingBooking = null;
}

async function markCurrentBookingPaid() {
  if (!currentPayingBooking) return;
  const res = await apiFetch('/api/payment-status', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ bookingId: currentPayingBooking, paymentStatus: 'PAID' }),
  });
  const data = await res.json();
  if (res.ok) {
    showToast(data.message || 'Payment marked as PAID.');
    closeUpiModal();
    await loadDashboard('id');
  } else {
    showToast(data.error || 'Could not update payment status.', true);
  }
}

/* ── Live Ride Telemetry & GPS Tracking (Lab 1, 4, 8) ──────────── */
let trackerLeafletMap = null;
let trackerCarMarker = null;
let trackerRouteLine = null;
let trackerWaypointMarkers = [];
let trackerCurrentRideId = null;
let trackerCurrentBookingOtp = '----';
let trackerSimulationTimer = null;
let trackerIsSimulating = false;

function initTrackerMapIfNeeded() {
  const container = document.getElementById('tracker-leaflet-map');
  if (!container) return;

  if (!trackerLeafletMap) {
    trackerLeafletMap = L.map('tracker-leaflet-map', {
      zoomControl: true,
      attributionControl: false
    }).setView([15.4620, 73.8340], 12);

    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
      maxZoom: 18,
    }).addTo(trackerLeafletMap);
  }

  setTimeout(() => {
    if (trackerLeafletMap) trackerLeafletMap.invalidateSize();
  }, 200);
}

async function openLiveRideTracker(bookingId, rideId) {
  trackerCurrentRideId = rideId;
  trackerCurrentBookingOtp = '----';

  if (bookingId && typeof allBookings !== 'undefined') {
    const booking = allBookings.find(b => b.bookingId === bookingId);
    if (booking && booking.otp) {
      trackerCurrentBookingOtp = booking.otp;
    }
  }

  const modal = document.getElementById('live-tracker-modal');
  modal.classList.remove('hidden');
  modal.classList.add('flex');

  initTrackerMapIfNeeded();
  document.getElementById('track-progress-slider').value = 35;
  document.getElementById('track-slider-percent').innerText = '35%';

  await fetchAndRenderTelemetry(rideId, 35);
}

function closeLiveRideTracker() {
  if (trackerSimulationTimer) {
    clearInterval(trackerSimulationTimer);
    trackerSimulationTimer = null;
    trackerIsSimulating = false;
    document.getElementById('track-play-icon').innerText = 'play_arrow';
    document.getElementById('track-play-label').innerText = 'Auto Drive';
  }

  const modal = document.getElementById('live-tracker-modal');
  modal.classList.add('hidden');
  modal.classList.remove('flex');
}

async function fetchAndRenderTelemetry(rideId, progress) {
  try {
    const res = await apiFetch(`/api/track-ride?id=${rideId}&progress=${progress}`);
    if (!res.ok) {
      const err = await res.json();
      showToast(err.error || 'Failed to fetch ride telemetry.', true);
      return;
    }

    const data = await res.json();
    const telem = data.telemetry;
    const waypoints = data.waypoints || [];
    const ride = data.ride || {};

    // Update KPI metrics
    document.getElementById('track-ride-badge').innerText = `#${telem.rideId}`;
    document.getElementById('track-eta-val').innerText = `${telem.etaMinutes} mins`;
    document.getElementById('track-dist-val').innerText = `${telem.distanceRemainingKm} km`;
    document.getElementById('track-otp-val').innerText = trackerCurrentBookingOtp;
    document.getElementById('track-checkpoint-text').innerText = `At: ${telem.currentCheckpoint}`;
    document.getElementById('track-next-checkpoint').innerText = `Next: ${telem.nextCheckpoint}`;
    document.getElementById('track-step-text').innerText = `Step ${telem.statusStep}: ${telem.statusText}`;

    document.getElementById('track-driver-name').innerText = ride.driverName || 'Verified Driver';
    document.getElementById('track-vehicle-val').innerText = ride.vehicleNumber || 'GA-01-AB-1204';
    const trackVDescEl = document.getElementById('track-vehicle-desc');
    if (trackVDescEl) {
      trackVDescEl.innerText = ride.vehicleDesc || 'Verified Campus Vehicle';
    }
    document.getElementById('track-lat-display').innerText = telem.lat.toFixed(4);
    document.getElementById('track-lng-display').innerText = telem.lng.toFixed(4);

    const driverPhone = (ride.driverPhone || '').replace(/\D/g, '');
    const waLink = document.getElementById('track-whatsapp-link');
    if (driverPhone) {
      waLink.href = `https://wa.me/91${driverPhone}?text=Hi%2C%20tracking%20Ride%20%23${rideId}%20on%20GU%20RideShare.%20My%20Boarding%20OTP%20is%20${trackerCurrentBookingOtp}.`;
      waLink.classList.remove('hidden');
    } else {
      waLink.classList.add('hidden');
    }

    // Render 5-Step Stepper
    renderStepper(telem.statusStep);

    // Update Leaflet map
    if (trackerLeafletMap) {
      // Clear old waypoint markers & route
      trackerWaypointMarkers.forEach(m => trackerLeafletMap.removeLayer(m));
      trackerWaypointMarkers = [];

      if (trackerRouteLine) {
        trackerLeafletMap.removeLayer(trackerRouteLine);
        trackerRouteLine = null;
      }

      // Draw route polyline
      if (waypoints.length > 1) {
        const lineCoords = waypoints.map(w => [w.lat, w.lng]);
        trackerRouteLine = L.polyline(lineCoords, {
          color: '#193b70',
          weight: 5,
          opacity: 0.85,
          dashArray: '8, 8'
        }).addTo(trackerLeafletMap);

        // Add waypoint circle markers
        waypoints.forEach((w, idx) => {
          const isOrigin = (idx === 0);
          const isDest = (idx === waypoints.length - 1);
          const color = isOrigin ? '#193b70' : (isDest ? '#10b981' : '#64748b');
          const marker = L.circleMarker([w.lat, w.lng], {
            radius: isOrigin || isDest ? 7 : 5,
            fillColor: color,
            color: '#ffffff',
            weight: 2,
            opacity: 1,
            fillOpacity: 0.9
          }).addTo(trackerLeafletMap).bindPopup(`<b>${escapeHtml(w.checkpointName)}</b><br>${w.percent}% of route`);
          trackerWaypointMarkers.push(marker);
        });
      }

      // Animated car marker
      const carIcon = L.divIcon({
        html: `
          <div style="position: relative; width: 34px; height: 34px; display: flex; align-items: center; justify-content: center; background: #193b70; border: 2px solid #ffffff; border-radius: 50%; box-shadow: 0 4px 10px rgba(0,0,0,0.3); font-size: 17px; transform: translate(-50%, -50%);">
            🚗
            <div style="position: absolute; inset: -4px; border-radius: 50%; border: 2px solid #10b981; animation: ping 1.5s cubic-bezier(0, 0, 0.2, 1) infinite; opacity: 0.75;"></div>
          </div>
        `,
        className: 'custom-car-pin',
        iconSize: [0, 0],
        iconAnchor: [0, 0]
      });

      if (!trackerCarMarker) {
        trackerCarMarker = L.marker([telem.lat, telem.lng], { icon: carIcon }).addTo(trackerLeafletMap);
      } else {
        trackerCarMarker.setLatLng([telem.lat, telem.lng]);
      }

      trackerLeafletMap.panTo([telem.lat, telem.lng]);
    }
  } catch (err) {
    console.error('Error fetching telemetry:', err);
  }
}

function renderStepper(currentStep) {
  const steps = [
    { num: 1, label: 'Booked' },
    { num: 2, label: 'Approaching' },
    { num: 3, label: 'OTP Boarding' },
    { num: 4, label: 'In Transit' },
    { num: 5, label: 'Arrived' }
  ];

  const container = document.getElementById('track-stepper-container');
  if (!container) return;

  container.innerHTML = steps.map(s => {
    const isPast = s.num < currentStep;
    const isCurrent = s.num === currentStep;
    const bgClass = isPast
      ? 'bg-emerald-600 text-white'
      : (isCurrent ? 'bg-primary text-white ring-2 ring-primary/40 animate-pulse' : 'bg-surface-container-high text-on-surface-variant');

    return `
      <div class="flex flex-col items-center gap-1">
        <div class="w-6 h-6 rounded-full flex items-center justify-center font-bold text-[11px] ${bgClass}">
          ${isPast ? '✓' : s.num}
        </div>
        <span class="text-[9px] font-semibold text-on-surface truncate w-full">${s.label}</span>
      </div>
    `;
  }).join('');
}

function onTrackerSliderChange(val) {
  document.getElementById('track-slider-percent').innerText = `${val}%`;
  if (trackerCurrentRideId) {
    fetchAndRenderTelemetry(trackerCurrentRideId, val);
  }
}

function toggleTrackerSimulation() {
  if (trackerIsSimulating) {
    clearInterval(trackerSimulationTimer);
    trackerSimulationTimer = null;
    trackerIsSimulating = false;
    document.getElementById('track-play-icon').innerText = 'play_arrow';
    document.getElementById('track-play-label').innerText = 'Auto Drive';
  } else {
    trackerIsSimulating = true;
    document.getElementById('track-play-icon').innerText = 'pause';
    document.getElementById('track-play-label').innerText = 'Pause';

    trackerSimulationTimer = setInterval(() => {
      const slider = document.getElementById('track-progress-slider');
      let val = parseInt(slider.value) + 5;
      if (val > 100) {
        val = 0; // Loop simulation
      }
      slider.value = val;
      onTrackerSliderChange(val);
    }, 1000);
  }
}

/* ── Notification Center Drawer (Lab 4, 8 & Lab #G) ───────────── */
async function fetchNotifications() {
  if (!currentUser || !authToken) return;
  try {
    const res = await apiFetch('/api/notifications');
    if (!res.ok) return;
    const data = await res.json();
    currentNotifications = data.notifications || [];
    const unread = data.unreadCount || 0;

    const badge = document.getElementById('notif-badge');
    if (badge) {
      if (unread > 0) {
        badge.innerText = unread > 99 ? '99+' : unread;
        badge.classList.remove('hidden');
      } else {
        badge.classList.add('hidden');
      }
    }

    const countLabel = document.getElementById('notif-count-label');
    if (countLabel) {
      countLabel.innerText = `${unread} Unread • ${currentNotifications.length} Total`;
    }

    const list = document.getElementById('notification-list');
    if (!list) return;

    if (!currentNotifications.length) {
      list.innerHTML = `
        <div class="p-6 text-center text-xs text-on-surface-variant flex flex-col items-center gap-2">
          <span class="material-symbols-outlined text-[32px] text-on-surface-variant/50">notifications_off</span>
          <span>No notifications yet. Activity updates and ride alerts will appear here.</span>
        </div>
      `;
      return;
    }

    list.innerHTML = currentNotifications.map(n => {
      let icon = 'notifications';
      let iconColor = 'text-primary bg-primary/10';
      if (n.type === 'SECURITY_OTP') {
        icon = 'key';
        iconColor = 'text-amber-700 bg-amber-100';
      } else if (n.type === 'BOOKING') {
        icon = 'airline_seat_recline_normal';
        iconColor = 'text-emerald-700 bg-emerald-100';
      } else if (n.type === 'CANCEL') {
        icon = 'cancel';
        iconColor = 'text-rose-700 bg-rose-100';
      } else if (n.type === 'RIDE_EDIT') {
        icon = 'edit_road';
        iconColor = 'text-blue-700 bg-blue-100';
      }

      const cardBg = n.isRead
        ? 'bg-surface-container-lowest border-surface-container'
        : 'bg-primary/5 border-primary/30 ring-1 ring-primary/20';

      return `
        <div class="rounded-xl p-3 border ${cardBg} shadow-sm flex flex-col gap-1.5 transition-all text-xs">
          <div class="flex items-center justify-between">
            <div class="flex items-center gap-1.5">
              <div class="w-6 h-6 rounded-md flex items-center justify-center shrink-0 ${iconColor}">
                <span class="material-symbols-outlined text-[15px]">${icon}</span>
              </div>
              <span class="font-bold text-on-surface truncate max-w-[190px]">${escapeHtml(n.title)}</span>
            </div>
            ${!n.isRead ? '<span class="w-2 h-2 rounded-full bg-primary shrink-0 animate-pulse" title="Unread"></span>' : ''}
          </div>
          <p class="text-[11px] text-on-surface-variant leading-relaxed">${escapeHtml(n.message)}</p>
          <div class="flex items-center justify-between text-[10px] text-on-surface-variant/80 pt-1 border-t border-surface-container/40">
            <span class="font-mono">${escapeHtml(n.type)}</span>
            <span>${escapeHtml(n.timestamp)}</span>
          </div>
        </div>
      `;
    }).join('');
  } catch (err) {
    console.error('Error fetching notifications:', err);
  }
}

function toggleNotificationDrawer(forceOpen) {
  const overlay = document.getElementById('notification-overlay');
  if (!overlay) return;
  if (typeof forceOpen === 'boolean') {
    overlay.classList.toggle('hidden', !forceOpen);
    overlay.classList.toggle('flex', forceOpen);
  } else {
    overlay.classList.toggle('hidden');
    overlay.classList.toggle('flex');
  }
  if (!overlay.classList.contains('hidden')) {
    fetchNotifications();
  }
}

async function markAllNotificationsRead() {
  if (!currentUser || !authToken) return;
  try {
    const res = await apiFetch('/api/notifications/read', { method: 'POST' });
    if (res.ok) {
      showToast('All notifications marked as read.');
      await fetchNotifications();
    }
  } catch (err) {
    console.error('Error marking notifications read:', err);
  }
}

/* ── Tab & Panel Navigation ───────────────────────────────────── */
function switchTab(tab) {
  ['rides', 'map', 'bookings', 'matrix', 'admin'].forEach(t => {
    const content = document.getElementById('tab-content-' + t);
    const btn = document.getElementById('tab-btn-' + t);
    const navBtn = document.getElementById('nav-btn-' + t);
    if (content) {
      content.classList.toggle('hidden', t !== tab);
      content.classList.toggle('flex', t === tab);
    }
    if (btn) {
      if (t === tab) {
        btn.classList.add('bg-surface-container-lowest', 'text-primary', 'shadow-sm');
        btn.classList.remove('text-on-surface-variant');
      } else {
        btn.classList.remove('bg-surface-container-lowest', 'text-primary', 'shadow-sm');
        btn.classList.add('text-on-surface-variant');
      }
    }
    if (navBtn) {
      if (t === tab) {
        navBtn.classList.add('text-primary');
        navBtn.classList.remove('text-on-surface-variant');
      } else {
        navBtn.classList.remove('text-primary');
        navBtn.classList.add('text-on-surface-variant');
      }
    }
  });

  if (tab === 'map') {
    setTimeout(initMapIfNeeded, 100);
  } else if (tab === 'admin') {
    loadAdminMetrics();
  }
}

/* ── Master Admin Console & Safety Oversight (Lab 4, 8 & Lab #G) ─ */
async function loadAdminMetrics() {
  try {
    const [heapRes, usersRes] = await Promise.all([
      fetch('/api/admin/heap-stats').then(r => r.json()),
      fetch('/api/users').then(r => r.json()),
    ]);

    const hs = heapRes.heapStats || {};
    const totalBytes = hs.TotalAllocatedBytes || 0;
    const totalKb = (totalBytes / 1024).toFixed(1);

    const totalEl = document.getElementById('admin-total-heap');
    if (totalEl) totalEl.innerText = `${totalBytes.toLocaleString()} Bytes (${totalKb} KB Allocated)`;

    const setHeapBox = (idBytes, idCap, bytes, count, cap) => {
      const elB = document.getElementById(idBytes);
      const elC = document.getElementById(idCap);
      if (elB) elB.innerText = `${bytes.toLocaleString()} B`;
      if (elC) elC.innerText = `Cap: ${cap} | Count: ${count}`;
    };

    setHeapBox('admin-heap-rides', 'admin-cap-rides', hs.RidesBytes || 0, hs.RideCount || 0, hs.RideCap || 0);
    setHeapBox('admin-heap-bookings', 'admin-cap-bookings', hs.BookingsBytes || 0, hs.BookCount || 0, hs.BookCap || 0);
    setHeapBox('admin-heap-users', 'admin-cap-users', hs.UsersBytes || 0, hs.UserCount || 0, hs.UserCap || 0);
    setHeapBox('admin-heap-notifs', 'admin-cap-notifs', hs.NotifsBytes || 0, hs.NotifCount || 0, hs.NotifCap || 0);

    const users = usersRes.users || [];
    const userCountLabel = document.getElementById('admin-user-count-label');
    if (userCountLabel) userCountLabel.innerText = `${users.length} Registered Accounts`;

    const tbody = document.getElementById('admin-users-tbody');
    if (!tbody) return;

    if (!users.length) {
      tbody.innerHTML = `<tr><td colspan="6" class="p-4 text-center text-on-surface-variant">No campus users registered.</td></tr>`;
      return;
    }

    tbody.innerHTML = users.map(u => {
      const isVer = u.isVerified === 1;
      const badgeClass = isVer
        ? 'bg-teal-100 text-teal-800 border-teal-200'
        : 'bg-amber-100 text-amber-800 border-amber-200';
      const badgeIcon = isVer ? 'verified_user' : 'pending';
      const btnClass = isVer
        ? 'bg-error/10 text-error hover:bg-error hover:text-white'
        : 'bg-teal-600 text-white hover:bg-teal-700';

      return `
        <tr class="hover:bg-surface-container-low transition-all">
          <td class="p-2.5 font-bold flex items-center gap-2">
            <div class="w-7 h-7 rounded-full bg-primary/10 text-primary font-bold flex items-center justify-center text-xs shrink-0">${initials(u.fullName)}</div>
            <div class="flex flex-col min-w-0">
              <span class="truncate">${escapeHtml(u.fullName)}</span>
              <span class="text-[10px] text-on-surface-variant font-mono">ID: ${u.userId}</span>
            </div>
          </td>
          <td class="p-2.5 font-mono text-[11px] text-on-surface-variant">${escapeHtml(u.email)}</td>
          <td class="p-2.5">
            <span class="px-2 py-0.5 rounded-full text-[10px] font-semibold bg-surface-container-high text-on-surface">${escapeHtml(u.role)}</span>
          </td>
          <td class="p-2.5">
            <span class="inline-flex items-center gap-1 px-2 py-0.5 rounded-full text-[10px] font-bold border ${badgeClass}">
              <span class="material-symbols-outlined text-[13px]">${badgeIcon}</span> ${u.trustScore}%
            </span>
          </td>
          <td class="p-2.5 font-mono text-[11px] text-on-surface-variant">${escapeHtml(u.dlNumber || 'Pending')}</td>
          <td class="p-2.5 text-right">
            <button class="px-2.5 py-1 rounded-lg text-xs font-semibold shadow-sm transition-all ${btnClass}" onclick="toggleDriverVerification('${escapeHtml(u.email)}', ${isVer})">
              ${isVer ? 'Revoke KYC' : 'Verify Driver'}
            </button>
          </td>
        </tr>
      `;
    }).join('');
  } catch (err) {
    console.error('Error loading admin metrics:', err);
  }
}

async function toggleDriverVerification(email, currentStatus) {
  try {
    const res = await apiFetch('/api/driver/verify', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ email, verified: !currentStatus }),
    });
    const data = await res.json();
    if (!res.ok) {
      showToast(data.error || 'Verification update failed.', true);
      return;
    }
    showToast(data.message || 'Driver verification updated.');
    await loadAdminMetrics();
    await loadDashboard('id');
  } catch (err) {
    showToast('Failed to update driver status.', true);
  }
}

async function purgeCancelledBookings() {
  if (!confirm('Purge cancelled booking records to defragment and reclaim contiguous C heap memory?')) return;
  try {
    const res = await apiFetch('/api/admin/purge-cancelled', { method: 'POST' });
    const data = await res.json();
    if (!res.ok) {
      showToast(data.error || 'Purge failed.', true);
      return;
    }
    showToast(data.message || 'Purged cancelled bookings and defragmented heap.');
    await loadAdminMetrics();
    await loadDashboard('id');
  } catch (err) {
    showToast('Failed to purge cancelled bookings.', true);
  }
}

function toggleActionPanel() {
  const panel = document.getElementById('action-panel');
  const icon = document.getElementById('panel-toggle-icon');
  panel.classList.toggle('hidden');
  icon.style.transform = panel.classList.contains('hidden') ? 'rotate(0deg)' : 'rotate(180deg)';
}

function openRelevantActionPanel() {
  openAction('offer');
}

function openAction(action) {
  if (!currentUser) { applyCurrentUser(); return; }
  const panel = document.getElementById('action-panel');
  if (panel.classList.contains('hidden')) toggleActionPanel();
  switchSubAction(action);
  panel.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
}

function switchSubAction(action) {
  const isOffer = action === 'offer';
  document.getElementById('form-offer-ride').classList.toggle('hidden', !isOffer);
  document.getElementById('form-book-ride').classList.toggle('hidden', isOffer);
  document.getElementById('action-subtab-offer').className = isOffer
    ? 'flex-1 pb-2 text-sm sm:text-base font-bold text-primary border-b-2 border-primary flex items-center justify-center gap-1.5'
    : 'flex-1 pb-2 text-sm sm:text-base font-medium text-on-surface-variant flex items-center justify-center gap-1.5';
  document.getElementById('action-subtab-book').className = !isOffer
    ? 'flex-1 pb-2 text-sm sm:text-base font-bold text-primary border-b-2 border-primary flex items-center justify-center gap-1.5'
    : 'flex-1 pb-2 text-sm sm:text-base font-medium text-on-surface-variant flex items-center justify-center gap-1.5';
}

/* ── Seat Counters ────────────────────────────────────────────── */
function incrementOfferSeats() {
  if (offerSeatsCount < 7) {
    offerSeatsCount++;
    document.getElementById('offer-seats-val').innerText = offerSeatsCount;
  }
}
function decrementOfferSeats() {
  if (offerSeatsCount > 1) {
    offerSeatsCount--;
    document.getElementById('offer-seats-val').innerText = offerSeatsCount;
  }
}
function incrementBookSeats() {
  if (bookSeatsCount < 7) {
    bookSeatsCount++;
    document.getElementById('book-seats-val').innerText = bookSeatsCount;
    updateBookingEstimate();
  }
}
function decrementBookSeats() {
  if (bookSeatsCount > 1) {
    bookSeatsCount--;
    document.getElementById('book-seats-val').innerText = bookSeatsCount;
    updateBookingEstimate();
  }
}
function updateBookingEstimate() {
  const sel = document.getElementById('book-ride-select');
  if (!sel || sel.selectedIndex < 0) return;
  const fare = parseInt(sel.options[sel.selectedIndex].getAttribute('data-fare') || '0', 10);
  document.getElementById('book-fare-preview').innerText = `₹${fare * bookSeatsCount}`;
}
function prefillBooking(rideId) {
  if (!currentUser) {
    showToast('Sign in with @unigoa.ac.in to reserve a seat.', true);
    applyCurrentUser();
    return;
  }
  const panel = document.getElementById('action-panel');
  if (panel.classList.contains('hidden')) toggleActionPanel();
  switchSubAction('book');
  document.getElementById('book-ride-select').value = String(rideId);
  updateBookingEstimate();
  panel.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
}

/* ── Toast Notifications ──────────────────────────────────────── */
let toastTimeout;
function showToast(msg, isError = false) {
  const banner = document.getElementById('toast-banner');
  const box = document.getElementById('toast-box');
  const icon = document.getElementById('toast-icon');
  document.getElementById('toast-message').innerText = msg;
  box.className = isError
    ? 'bg-error text-on-error px-4 py-2.5 rounded-xl shadow-xl flex items-center justify-between gap-2'
    : 'bg-primary text-on-primary px-4 py-2.5 rounded-xl shadow-xl flex items-center justify-between gap-2';
  icon.innerText = isError ? 'error' : 'check_circle';
  banner.classList.remove('opacity-0', '-translate-y-24', 'pointer-events-none');
  banner.classList.add('opacity-100', 'translate-y-0');
  clearTimeout(toastTimeout);
  toastTimeout = setTimeout(dismissToast, 3200);
}
function dismissToast() {
  const banner = document.getElementById('toast-banner');
  banner.classList.add('opacity-0', '-translate-y-24', 'pointer-events-none');
  banner.classList.remove('opacity-100', 'translate-y-0');
}

/* ── Bootstrap ────────────────────────────────────────────────── */
const savedSession = localStorage.getItem('guRideShareSession');
if (savedSession) {
  try {
    const session = JSON.parse(savedSession);
    authToken = session.token;
    currentUser = session.user;
  } catch (_) {
    localStorage.removeItem('guRideShareSession');
  }
}

async function bootstrap() {
  const todayStr = new Date().toISOString().split('T')[0];
  const dateInput = document.getElementById('offer-date');
  if (dateInput) {
    dateInput.min = todayStr;
    if (!dateInput.value) dateInput.value = todayStr;
  }
  const filterDateInput = document.getElementById('filter-date');
  if (filterDateInput) {
    filterDateInput.min = todayStr;
  }

  if (currentUser) {
    const res = await apiFetch('/api/auth/me');
    if (!res.ok) {
      localStorage.removeItem('guRideShareSession');
      authToken = null;
      currentUser = null;
    }
  }
  applyCurrentUser();
  await loadDashboard('id');
}

bootstrap();
