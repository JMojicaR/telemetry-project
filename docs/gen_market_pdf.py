#!/usr/bin/env python3
"""Generate market_analysis_car.pdf from the car rear-view market analysis."""
from reportlab.lib.pagesizes import A4
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import mm
from reportlab.platypus import (SimpleDocTemplate, Paragraph, Spacer, Table,
                                TableStyle, HRFlowable)

OUT = "docs/market_analysis_car.pdf"

styles = getSampleStyleSheet()
title = ParagraphStyle("TitleX", parent=styles["Title"], fontSize=20, spaceAfter=6)
sub = ParagraphStyle("SubX", parent=styles["Normal"], fontSize=10, textColor=colors.HexColor("#555555"), alignment=1, spaceAfter=14)
h1 = ParagraphStyle("H1X", parent=styles["Heading1"], fontSize=13, spaceBefore=14, spaceAfter=6, textColor=colors.HexColor("#1a1a1a"))
h2 = ParagraphStyle("H2X", parent=styles["Heading2"], fontSize=11, spaceBefore=8, spaceAfter=4)
body = ParagraphStyle("BodyX", parent=styles["Normal"], fontSize=9.5, leading=13, spaceAfter=6)
bullet = ParagraphStyle("BulletX", parent=body, leftIndent=14, bulletIndent=4, spaceAfter=3)
cell = ParagraphStyle("CellX", parent=styles["Normal"], fontSize=8.5, leading=11)
cellb = ParagraphStyle("CellBX", parent=cell, fontName="Helvetica-Bold")
small = ParagraphStyle("SmallX", parent=styles["Normal"], fontSize=8, textColor=colors.HexColor("#666666"), spaceAfter=4)

def P(t, s=body): return Paragraph(t, s)
def C(t, bold=False): return Paragraph(t, cellb if bold else cell)

def table(data, widths, header=True, align_center_cols=()):
    t = Table(data, colWidths=widths, repeatRows=1 if header else 0)
    st = [
        ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#1f3864") if header else colors.white),
        ("TEXTCOLOR", (0, 0), (-1, 0), colors.white if header else colors.black),
        ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold" if header else "Helvetica"),
        ("GRID", (0, 0), (-1, -1), 0.4, colors.HexColor("#bbbbbb")),
        ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
        ("TOPPADDING", (0, 0), (-1, -1), 3.5),
        ("BOTTOMPADDING", (0, 0), (-1, -1), 3.5),
        ("LEFTPADDING", (0, 0), (-1, -1), 5),
        ("RIGHTPADDING", (0, 0), (-1, -1), 5),
    ]
    if header:
        st.append(("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.white, colors.HexColor("#f2f5fa")]))
    for col in align_center_cols:
        st.append(("ALIGN", (col, 0), (col, -1), "CENTER"))
    t.setStyle(TableStyle(st))
    return t

doc = SimpleDocTemplate(OUT, pagesize=A4, leftMargin=16*mm, rightMargin=16*mm, topMargin=15*mm, bottomMargin=15*mm, title="Market Analysis - 3-Camera Rear-View Selector")
W = A4[0] - 32*mm
story = []

story.append(P("Market Analysis &amp; ROI", title))
story.append(P("3-Camera Rear-View Selector (DIY kit) — October 2026 · prices in MXN", sub))
story.append(HRFlowable(width="100%", thickness=1, color=colors.HexColor("#1f3864"), spaceAfter=10))

story.append(P("1. Product &amp; differentiation", h1))
story.append(P("A 3-camera rear-view system that <b>automatically</b> switches the 7\" monitor to the correct camera: <b>reverse</b> → center, <b>left turn</b> → left blind spot, <b>right turn</b> → right blind spot, <b>hazards/idle</b> → center."))
story.append(P("The differentiation: cheap backup cameras and dash cams on the market are reverse-only or fixed front/rear recorders. <b>None switch on turn signals</b> to show the blind spot you are turning into. That blind-spot-aware switching — an ESP32 reading the tail-light signals through optocouplers — is the entire value. Sold as a DIY kit first, so no NOM-001 certification is required at launch."))

story.append(P("2. Market &amp; competitors", h1))
story.append(table([
    [C("Segment", True), C("Price range", True), C("Notes", True)],
    [C("Single backup camera + monitor"), C("$500–1,500"), C("Reverse-only, commodity")],
    [C("Dash cam 2–3 channels"), C("$1,000–3,700"), C("Recording, no blind-spot switching")],
    [C("3-in-1 surveillance camera"), C("$640–1,520"), C("Reverse/front/cabin, fixed")],
    [C("360° 4-camera systems"), C("$2,500–6,000"), C("Bird's-eye, needs calibration")],
    [C("This product (3 rear + auto-switch)", True), C("$1,600–3,200", True), C("Blind-spot-aware, no calibration", True)],
], [W*0.42, W*0.24, W*0.34], align_center_cols=(1,)))
story.append(Spacer(1, 6))
story.append(P("<b>Target customer:</b> drivers of pickups with caps, cargo vans and small delivery fleets (poor rear visibility), DIY enthusiasts, and fleet owners who want blind-spot coverage without a $4,000+ 360° install."))

story.append(P("3. Unit cost (BOM)", h1))
story.append(P("<b>Complete kit</b> (cameras + monitor included)", h2))
story.append(table([
    [C("Item", True), C("Small batch", True), C("Mass", True)],
    [C("3× CVBS/AHD 170° IP68 cameras"), C("$900"), C("$270")],
    [C("7\" CVBS monitor (2 AV)"), C("$700"), C("$350")],
    [C("Controller (ESP32 + mux + optos + power + LEDs)"), C("$750"), C("$305")],
    [C("Unit cost", True), C("$2,350", True), C("$925", True)],
], [W*0.62, W*0.19, W*0.19], align_center_cols=(1, 2)))
story.append(Spacer(1, 8))
story.append(P("<b>Core kit</b> (controller only — buyer supplies cameras/monitor)", h2))
story.append(table([
    [C("Item", True), C("Small batch", True), C("Mass", True)],
    [C("ESP32 DevKit"), C("$180"), C("$60")],
    [C("CD4052 + socket"), C("$30"), C("$8")],
    [C("PC817 optocouplers ×3 + resistors"), C("$33"), C("$6")],
    [C("Power (fuse, SS54, TVS, LM2596, caps)"), C("$159"), C("$50")],
    [C("LEDs + button + resistors"), C("$22"), C("$10")],
    [C("Enclosure + RCA pigtails + wiring + misc"), C("$315"), C("$111")],
    [C("Custom PCB"), C("—"), C("$60")],
    [C("Unit cost", True), C("$739", True), C("$305", True)],
], [W*0.62, W*0.19, W*0.19], align_center_cols=(1, 2)))
story.append(P("The <b>core kit</b> is the lead product: the switcher is the differentiated part, and cameras/monitors are commodities.", small))

story.append(P("4. Sell price &amp; margin", h1))
story.append(table([
    [C("Variant", True), C("Sell price", True), C("Margin (small)", True), C("Margin (mass)", True)],
    [C("Core kit (controller)"), C("$1,599"), C("$860"), C("$1,294")],
    [C("Complete kit (all-in)"), C("$3,199"), C("$849"), C("$2,274")],
], [W*0.37, W*0.21, W*0.21, W*0.21], align_center_cols=(1, 2, 3)))

story.append(P("5. Fixed costs", h1))
story.append(table([
    [C("Item", True), C("DIY kit (no cert)", True), C("Certified", True)],
    [C("Prototyping, packaging, photography"), C("$15,000"), C("$25,000")],
    [C("Initial inventory (~20 core kits)"), C("$15,000"), C("$30,000")],
    [C("Marketing / listings / ads"), C("$10,000"), C("$20,000")],
    [C("NOM-001-SCFI testing + compliance"), C("—"), C("$60,000")],
    [C("Total fixed", True), C("$40,000", True), C("$135,000", True)],
], [W*0.58, W*0.21, W*0.21], align_center_cols=(1, 2)))

story.append(P("6. Break-even", h1))
story.append(table([
    [C("Scenario", True), C("Margin/unit", True), C("Break-even", True)],
    [C("Core kit, small batch"), C("$860"), C("~47 units")],
    [C("Complete kit, small batch"), C("$849"), C("~47 units")],
    [C("Complete kit, mass production"), C("$2,274"), C("~18 units")],
], [W*0.55, W*0.20, W*0.25], align_center_cols=(1, 2)))

story.append(P("7. ROI (Year 1)", h1))
story.append(P("Assumes <b>20 units/month</b> (240/year) — conservative for a DIY niche with a MercadoLibre + Tindie listing."))
story.append(table([
    [C("", True), C("Core kit", True), C("Complete kit", True)],
    [C("Revenue"), C("$383,760"), C("$767,760")],
    [C("COGS"), C("$177,360"), C("$564,000")],
    [C("Gross profit"), C("$206,400"), C("$203,760")],
    [C("Net (after $40K fixed)"), C("$166,400"), C("$163,760")],
], [W*0.45, W*0.275, W*0.275], align_center_cols=(1, 2)))
story.append(P("<b>Payback period:</b> ~2–3 months at 20/month (~$17K margin/month vs $40K fixed); ~1 month at mass margins."))

story.append(P("8. Sell plan", h1))
for b in [
    "<b>1. Tindie</b> — international DIY audience, zero listing friction, maker credibility (launch the core kit here first).",
    "<b>2. MercadoLibre México</b> — highest-traffic channel for the Mexican market; list both variants in Spanish.",
    "<b>3. Amazon.com.mx</b> — once volume justifies FBA/ads; strong search traffic for \"camara de reversa\".",
    "<b>4. Direct / WhatsApp + fleet referrals</b> — cargo vans and delivery fleets are the highest-value repeat buyers.",
    "<b>5. Certified upgrade</b> — only if demand proves out; then NOM-001 and sell installable at $4,000+.",
]:
    story.append(P(b, bullet))

story.append(P("9. Risks &amp; mitigations", h1))
story.append(table([
    [C("Risk", True), C("Mitigation", True)],
    [C("CVBS seen as \"old\" vs 4K dash cams"), C("Lead with blind-spot switching, not resolution; AHD path exists")],
    [C("Thin margin on complete kit (commodity parts)"), C("Push the core kit where the switcher is the value")],
    [C("Automotive install scares buyers"), C("Optocoupler harness + step-by-step assembly guide; \"tap 3 wires\"")],
    [C("Returns from mis-install"), C("Video guide + pre-soldered controller board")],
    [C("Chinese dash-cam price pressure"), C("Differentiate on turn-signal switching + Spanish support")],
], [W*0.46, W*0.54]))

story.append(P("10. Recommendation", h1))
story.append(P("Launch the <b>core kit at $1,599 MXN</b> first (highest margin, lowest inventory risk, the differentiated part). Offer the <b>complete kit at $3,199</b> as an upsell. Target <b>~47 units to break even</b> and <b>~$166K net at 20/month</b> with no certification cost. Keep the AHD/DVR quad-view as the premium upgrade once volume justifies it."))

story.append(Spacer(1, 10))
story.append(HRFlowable(width="100%", thickness=0.5, color=colors.HexColor("#bbbbbb"), spaceAfter=4))
story.append(P("Sources: MercadoLibre México, Amazon.com.mx, Sears (Oct 2026). Full BOM in docs/BOM_car.md. Prices vary with USD/MXN and stock.", small))

doc.build(story)
print(f"Wrote {OUT}")
