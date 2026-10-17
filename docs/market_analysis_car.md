# Market Analysis & ROI — 3-Camera Rear-View Selector (DIY kit)

> Dated **October 2026**. Prices in MXN. Competitors sourced from MercadoLibre
> México, Amazon.com.mx, and Steren. This follows the same DIY-kit framing used
> for the lights_control project: **sell as a DIY kit first** so no NOM-001-SCFI
> certification is required at launch.

---

## 1. Product & differentiation

The product is a **3-camera rear-view system** that *automatically* switches the
7" monitor to the correct camera:

- **Reverse** → CENTER camera
- **Left turn** → LEFT blind-spot camera
- **Right turn** → RIGHT blind-spot camera
- **Hazards / idle** → CENTER (default)

**What makes it different from what's on the shelf:** every cheap backup camera
and dash cam on MercadoLibre is either reverse-only or a fixed front/rear
recorder. None of them switch on **turn signals** to show the *blind spot you're
actually turning into*. That blind-spot-aware switching — powered by the ESP32
reading the tail-light signals through optocouplers — is the whole value.

The electronics are a **DIY kit** (build-it-yourself), which sidesteps NOM-001
certification at launch and keeps fixed costs low.

---

## 2. Market & competitors

| Segment | Price range | Notes |
|---------|-------------|-------|
| Single backup camera + monitor | $500–1,500 | Reverse-only, commodity |
| Dash cam 2–3 channels (records) | $1,000–3,700 | Front/rear recording, no blind-spot switching |
| 3-in-1 surveillance camera | $640–1,520 | Reverse/front/cabin, fixed views |
| 360° 4-camera systems | $2,500–6,000 | Bird's-eye, needs calibration/install |
| **This product (3 rear + auto-switch)** | **$1,600–3,200** | Blind-spot-aware, no 360 calibration |

Examples found (Oct 2026):
- "Pantalla con 3 cámaras" dash cam, 4" — **$989** (CiberModa)
- Dash cam 3 canales 1080p — **$1,500–3,700** (Amazon MX)
- Cámara 3-en-1 reversa/frente/dentro — **$639–1,522** (Sears)
- 360° systems — **$2,500–6,000** (ML / Amazon MX)

**Target customer:** drivers of cars/vans/trucks with poor rear visibility
(pickups with caps, cargo vans, small delivery fleets), DIY enthusiasts, and
fleet owners who want blind-spot coverage without a $4,000+ 360° install.

---

## 3. BOM & unit cost

Two product variants:

### A. Complete kit (cameras + monitor included)

| Item | Small batch | Mass |
|------|-------------|------|
| 3× CVBS/AHD 170° IP68 cameras | $900 | $270 |
| 7" CVBS monitor (2 AV) | $700 | $350 |
| Controller (ESP32 + mux + optos + power + LEDs + wiring) | $750 | $305 |
| **Unit cost** | **$2,350** | **$925** |

### B. Core kit (controller only — buyer supplies cameras/monitor)

| Item | Small batch | Mass |
|------|-------------|------|
| ESP32 DevKit | $180 | $60 |
| CD4052 + socket | $30 | $8 |
| PC817 optocouplers ×3 + resistors | $33 | $6 |
| Power (fuse, SS54, TVS, LM2596, caps) | $159 | $50 |
| LEDs + button + resistors | $22 | $10 |
| Enclosure + RCA pigtails + wiring + misc | $315 | $111 |
| Custom PCB | — | $60 |
| **Unit cost** | **$739** | **$305** |

The **core kit** is the recommended lead product: the switcher is the
differentiated part, and cameras/monitors are commodities the buyer already
understands how to buy. Full BOM in `docs/BOM_car.md`.

---

## 4. Sell price & margin

| Variant | Sell price | Margin (small batch) | Margin (mass) |
|---------|-----------|----------------------|---------------|
| Core kit (controller) | $1,599 | $860 | $1,294 |
| Complete kit (all-in) | $3,199 | $849 | $2,274 |

Sell prices are set **below** 360° systems ($2,500–6,000) but **above** basic
backup cameras ($500–1,500), justified by the unique turn-signal switching.

---

## 5. Fixed costs

| Item | DIY kit (no cert) | Certified product |
|------|-------------------|-------------------|
| Prototyping, packaging, photography | $15,000 | $25,000 |
| Initial inventory (≈20 core kits) | $15,000 | $30,000 |
| Marketing / listings / ads | $10,000 | $20,000 |
| NOM-001-SCFI testing + compliance | — | $60,000 |
| **Total fixed** | **$40,000** | **$135,000** |

---

## 6. Break-even

| Scenario | Margin/unit | Break-even |
|----------|-------------|------------|
| Core kit, small batch | $860 | **~47 units** |
| Complete kit, small batch | $849 | **~47 units** |
| Complete kit, mass production | $2,274 | **~18 units** |

---

## 7. ROI (Year 1)

Assumes **20 units/month** (240/year) — conservative for a DIY niche with a
MercadoLibre + Tindie listing.

**Core kit, small batch:**
- Revenue: 240 × $1,599 = $383,760
- COGS: 240 × $739 = $177,360
- Gross profit: **$206,400**
- Net (after $40K fixed): **~$166,400**

**Complete kit, small batch:**
- Revenue: 240 × $3,199 = $767,760
- COGS: 240 × $2,350 = $564,000
- Gross profit: **$203,760**
- Net: **~$163,760**

**Payback period:** ~2–3 months at 20 units/month (≈$17K margin/month against
$40K fixed). At mass-production margins this drops to ~1 month.

---

## 8. Sell plan

1. **Tindie** — international DIY audience, zero listing friction, "maker"
   credibility (launch the core kit here first).
2. **MercadoLibre México** — the highest-traffic channel for the Mexican
   market; list both variants with Spanish listings.
3. **Amazon.com.mx** — once volume justifies FBA/ads; search traffic for
   "camara de reversa" is high.
4. **Direct / WhatsApp + fleet referrals** — cargo vans and delivery fleets are
   the highest-value repeat customers (multi-unit orders).
5. **Certified upgrade** — only if demand proves out; then pursue NOM-001 and
   sell as an installable product at $4,000+ (the 360° price band).

---

## 9. Risks & mitigations

| Risk | Mitigation |
|------|-----------|
| CVBS perceived as "old tech" vs 4K dash cams | Lead with the *blind-spot switching* benefit, not resolution; AHD upgrade path exists |
| Cameras/monitor are commodity (thin margin on complete kit) | Push the **core kit** where the switcher is the value |
| Automotive install scares some buyers | Include the optocoupler harness + step-by-step `assembly_guide.md`; emphasize "no soldering the car, just tap 3 wires" |
| Returns from mis-install | Video install guide + pre-soldered controller board |
| Price pressure from Chinese dash cams | Differentiate on turn-signal switching + local Spanish support |

---

## 10. Recommendation

Launch the **core kit at $1,599 MXN** first (highest margin, lowest inventory
risk, the differentiated part). Offer the **complete kit at $3,199** as an
upsell. Target **~47 units to break even** and **~$166K net at 20/month** with
no certification cost. Keep the AHD/DVR quad-view as the premium upgrade once
volume justifies it.
