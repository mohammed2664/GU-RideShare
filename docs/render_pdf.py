import os
from playwright.sync_api import sync_playwright

def render():
    html_path = os.path.abspath('docs/report_template.html')
    pdf_path = os.path.abspath('docs/Team 6_Campus RideShare & Carpool Matcher.pdf')

    print(f"Loading HTML: {html_path}")
    print(f"Target PDF: {pdf_path}")

    with sync_playwright() as p:
        browser = p.chromium.launch()
        page = browser.new_page()
        # file:/// URL for local HTML
        url = 'file:///' + html_path.replace('\\', '/')
        page.goto(url, wait_until='networkidle')
        page.wait_for_timeout(1000)

        footer_html = '''
        <div style="font-size: 8pt; font-family: 'Segoe UI', Arial, sans-serif; color: #64748b; width: 100%; display: flex; justify-content: space-between; padding: 0 14mm;">
          <span>Team 6 &bull; Campus RideShare &amp; Carpool Matcher (CSA5003 PSPL SEA)</span>
          <span>Page <span class="pageNumber"></span> of <span class="totalPages"></span></span>
        </div>
        '''

        page.pdf(
            path=pdf_path,
            format='A4',
            print_background=True,
            display_header_footer=True,
            header_template='<div></div>',
            footer_template=footer_html,
            margin={'top': '14mm', 'bottom': '16mm', 'left': '14mm', 'right': '14mm'}
        )
        browser.close()

    if os.path.exists(pdf_path):
        size_kb = os.path.getsize(pdf_path) / 1024
        print(f"SUCCESS: PDF generated ({size_kb:.1f} KB)")
    else:
        print("ERROR: PDF was not created")

if __name__ == '__main__':
    render()
