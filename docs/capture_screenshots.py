import os
import subprocess
import time
import sys
from playwright.sync_api import sync_playwright

os.makedirs('docs/images', exist_ok=True)
server_proc = subprocess.Popen([sys.executable, 'web/server.py'])
time.sleep(2)

try:
    with sync_playwright() as p:
        browser = p.chromium.launch()
        page = browser.new_page(viewport={'width': 1200, 'height': 800})
        page.goto('http://127.0.0.1:8080')
        page.wait_for_timeout(1500)

        # 1. Login Modal
        auth_overlay = page.locator('#auth-overlay')
        if not auth_overlay.is_visible():
            page.evaluate("toggleAuthModal(true)")
            page.wait_for_timeout(500)
        page.screenshot(path='docs/images/screenshot_login.png')
        print('Saved screenshot_login.png')

        # Sign in as Faaris
        page.evaluate("quickLoginTeam('faaris@unigoa.ac.in', 'pass123')")
        page.wait_for_timeout(1500)

        # 2. Dashboard Overview (Hero + stats + search bar)
        page.evaluate("window.scrollTo(0, 0);")
        page.wait_for_timeout(500)
        page.screenshot(path='docs/images/screenshot_dashboard.png')
        print('Saved screenshot_dashboard.png')

        # 3. Active Rides Grid
        page.evaluate("switchTab('rides');")
        page.wait_for_timeout(500)
        page.locator('#rides-grid').scroll_into_view_if_needed()
        page.wait_for_timeout(500)
        page.screenshot(path='docs/images/screenshot_rides.png')
        print('Saved screenshot_rides.png')

        # 4. Interactive Leaflet Map
        page.evaluate("switchTab('map');")
        page.wait_for_timeout(2500)
        page.locator('#leaflet-map').scroll_into_view_if_needed()
        page.wait_for_timeout(1000)
        page.locator('#leaflet-map').screenshot(path='docs/images/screenshot_map.png')
        print('Saved screenshot_map.png')

        # 5. Bookings Tab & Digital Transit Pass Modal
        page.evaluate("switchTab('bookings');")
        page.wait_for_timeout(1000)
        # Call openDigitalPass directly using bare allBookings or booking ID 5008
        page.evaluate("""() => {
            if (typeof allBookings !== 'undefined' && allBookings.length > 0) {
                const b = allBookings[0];
                openDigitalPass(b.bookingId, b.rideId);
            } else {
                openDigitalPass(5008, 101);
            }
        }""")
        page.wait_for_timeout(1200)
        pass_modal = page.locator('#digital-pass-modal')
        if pass_modal.is_visible():
            pass_card = page.locator('#digital-pass-modal > div')
            pass_card.screenshot(path='docs/images/screenshot_pass.png')
            print('Saved screenshot_pass.png (modal card)')
            page.evaluate("closeDigitalPassModal();")
        else:
            print('Warning: pass modal was not visible')

        # 6. 2D Matrix & C Audit Report
        page.evaluate("switchTab('matrix'); fetchReportPreview();")
        page.wait_for_timeout(1500)
        page.locator('#tab-content-matrix').scroll_into_view_if_needed()
        page.wait_for_timeout(800)
        page.screenshot(path='docs/images/screenshot_matrix.png')
        print('Saved screenshot_matrix.png')

        browser.close()
        print('All 6 screenshots captured successfully!')
finally:
    server_proc.terminate()
