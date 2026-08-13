import PIL
from PIL import Image, ImageDraw, ImageFont
import os

def create_mockup():
    print("Generating Nestingator3000 mockup screenshot...")

    # 1. Create a blank image with Retro Windows color (RGB: 212, 208, 200 / #D4D0C8)
    w, h = 1100, 780
    img = Image.new("RGB", (w, h), (212, 208, 200))
    draw = ImageDraw.Draw(img)

    # Try to load a standard sans-serif font, fallback to default
    try:
        font_regular = ImageFont.load_default()
        font_bold = ImageFont.load_default()
    except Exception:
        font_regular = None
        font_bold = None

    # Let's draw 3D borders for panels and buttons helper
    def draw_3d_rect(x1, y1, x2, y2, raised=True):
        # Outer light border
        c1 = (255, 255, 255) if raised else (64, 64, 64)
        c2 = (64, 64, 64) if raised else (255, 255, 255)

        # Draw borders
        draw.line([(x1, y2), (x1, y1), (x2, y1)], fill=c1, width=1)
        draw.line([(x2, y1), (x2, y2), (x1, y2)], fill=c2, width=1)

    # 2. Draw Title Bar (Classic Windows Blue Gradient / Solid Blue)
    draw.rectangle([0, 0, w, 28], fill=(10, 36, 106))
    # Close/Min/Max buttons
    draw_3d_rect(w - 24, 4, w - 6, 22, True)
    draw.text((w - 18, 6), "X", fill=(0, 0, 0))
    draw_3d_rect(w - 46, 4, w - 28, 22, True)
    draw.text((w - 40, 6), "口", fill=(0, 0, 0))
    draw_3d_rect(w - 68, 4, w - 50, 22, True)
    draw.text((w - 62, 6), "_", fill=(0, 0, 0))

    # Title text
    draw.text((10, 6), "Nestingator3000", fill=(255, 255, 255))

    # 3. Draw Menu Bar
    menu_y = 28
    draw.rectangle([0, menu_y, w, menu_y + 24], fill=(212, 208, 200))
    draw.line([(0, menu_y + 24), (w, menu_y + 24)], fill=(128, 128, 128), width=1)

    menus = ["Plik", "Edycja", "Widok", "Narzędzia", "Biblioteka", "Pomoc"]
    curr_x = 10
    for m in menus:
        draw.text((curr_x, menu_y + 5), m, fill=(0, 0, 0))
        curr_x += len(m) * 8 + 15

    # 4. Draw Toolbar (Retro buttons)
    tool_y = 52
    draw.rectangle([0, tool_y, w, tool_y + 44], fill=(212, 208, 200))
    draw.line([(0, tool_y + 44), (w, tool_y + 44)], fill=(128, 128, 128), width=1)

    tools = ["New", "Open", "Print", "DXF", "Płyty", "Settings", "Delete", "Nesting", "NC", "PDF", "ZIP"]
    curr_x = 10
    for t in tools:
        tw = len(t) * 8 + 20
        draw_3d_rect(curr_x, tool_y + 5, curr_x + tw, tool_y + 38, True)
        draw.text((curr_x + 10, tool_y + 14), t, fill=(0, 0, 0))
        curr_x += tw + 8

    # 5. Left Panel: BAZA DETALI GroupBox
    baza_x1, baza_y1, baza_x2, baza_y2 = 10, 105, 460, 415
    draw_3d_rect(baza_x1, baza_y1, baza_x2, baza_y2, False)
    draw.text((baza_x1 + 15, baza_y1 - 5), " BAZA DETALI ", fill=(0, 0, 0), background=(212, 208, 200))

    # Table inside Baza Detali
    table_x1, table_y1, table_x2, table_y2 = baza_x1 + 10, baza_y1 + 20, baza_x2 - 10, baza_y2 - 50
    draw.rectangle([table_x1, table_y1, table_x2, table_y2], fill=(255, 255, 255))
    draw_3d_rect(table_x1, table_y1, table_x2, table_y2, False)

    folders = ["☑ 📁 Elementy", "☑ 📁 Meble", "☑ 📁 Fronty", "☑ 📁 Własne"]
    fy = table_y1 + 10
    for f in folders:
        draw.text((table_x1 + 10, fy), f, fill=(0, 0, 0))
        fy += 35

    # Search Labeled inputs
    draw.text((baza_x1 + 10, baza_y2 - 35), "Szukaj:", fill=(0, 0, 0))
    draw.rectangle([baza_x1 + 65, baza_y2 - 38, baza_x1 + 280, baza_y2 - 16], fill=(255, 255, 255))
    draw_3d_rect(baza_x1 + 65, baza_y2 - 38, baza_x1 + 280, baza_y2 - 16, False)

    # Action buttons under Baza Detali
    draw_3d_rect(baza_x1 + 290, baza_y2 - 38, baza_x1 + 315, baza_y2 - 16, True)
    draw.text((baza_x1 + 298, baza_y2 - 32), "🖍", fill=(0, 0, 0))

    draw_3d_rect(baza_x1 + 320, baza_y2 - 38, baza_x1 + 345, baza_y2 - 16, True)
    draw.text((baza_x1 + 328, baza_y2 - 32), "▶", fill=(0, 0, 0))

    draw_3d_rect(baza_x1 + 350, baza_y2 - 38, baza_x1 + 395, baza_y2 - 16, True)
    draw.text((baza_x1 + 355, baza_y2 - 32), "[ Reset", fill=(0, 0, 0))

    draw_3d_rect(baza_x1 + 400, baza_y2 - 38, baza_x1 + 435, baza_y2 - 16, True)
    draw.text((baza_x1 + 405, baza_y2 - 32), "+-]", fill=(0, 0, 0))

    # 6. Left Panel: Komponenty GroupBox
    komp_x1, komp_y1, komp_x2, komp_y2 = 10, 425, 460, 740
    draw_3d_rect(komp_x1, komp_y1, komp_x2, komp_y2, False)
    draw.text((komp_x1 + 15, komp_y1 - 5), " Komponenty ", fill=(0, 0, 0), background=(212, 208, 200))

    # List of components
    list_x1, list_y1, list_x2, list_y2 = komp_x1 + 10, komp_y1 + 20, komp_x2 - 10, komp_y2 - 45
    draw.rectangle([list_x1, list_y1, list_x2, list_y2], fill=(255, 255, 255))
    draw_3d_rect(list_x1, list_y1, list_x2, list_y2, False)

    # Table columns
    draw.rectangle([list_x1 + 1, list_y1 + 1, list_x2 - 1, list_y1 + 20], fill=(225, 225, 225))
    draw.text((list_x1 + 10, list_y1 + 4), "Nazwa", fill=(0, 0, 0))
    draw.text((list_x1 + 180, list_y1 + 4), "Szer (X)", fill=(0, 0, 0))
    draw.text((list_x1 + 260, list_y1 + 4), "Wys (Y)", fill=(0, 0, 0))
    draw.text((list_x1 + 340, list_y1 + 4), "Ilość", fill=(0, 0, 0))
    draw.line([(list_x1, list_y1 + 20), (list_x2, list_y1 + 20)], fill=(128, 128, 128), width=1)

    comp_data = [
        ("☑ 📄 Bok.dxf", "600", "300", "x2"),
        ("☑ 📄 Front.dxf", "400", "400", "x8"),
        ("☑ 📄 Plecy.dxf", "800", "500", "x1"),
    ]
    cy = list_y1 + 25
    for row in comp_data:
        draw.text((list_x1 + 10, cy), row[0], fill=(0, 0, 0))
        draw.text((list_x1 + 180, cy), row[1], fill=(0, 0, 0))
        draw.text((list_x1 + 260, cy), row[2], fill=(0, 0, 0))
        draw.text((list_x1 + 340, cy), row[3], fill=(0, 0, 0))
        cy += 25

    # Bottom buttons
    draw_3d_rect(komp_x1 + 10, komp_y2 - 35, komp_x1 + 110, komp_y2 - 10, True)
    draw.text((komp_x1 + 25, komp_y2 - 28), "+ Dodaj]", fill=(0, 0, 0))

    draw_3d_rect(komp_x1 + 120, komp_y2 - 35, komp_x1 + 220, komp_y2 - 10, True)
    draw.text((komp_x1 + 135, komp_y2 - 28), "+ Usuń]", fill=(0, 0, 0))

    draw_3d_rect(komp_x1 + 350, komp_y2 - 35, komp_x1 + 385, komp_y2 - 10, True)
    draw.text((komp_x1 + 360, komp_y2 - 28), "▲", fill=(0, 0, 0))

    draw_3d_rect(komp_x1 + 395, komp_y2 - 35, komp_x1 + 430, komp_y2 - 10, True)
    draw.text((komp_x1 + 405, komp_y2 - 28), "▼", fill=(0, 0, 0))


    # 7. Right Panel: PODGLĄD PŁYTY GroupBox
    pod_x1, pod_y1, pod_x2, pod_y2 = 475, 105, 1085, 510
    draw_3d_rect(pod_x1, pod_y1, pod_x2, pod_y2, False)
    draw.text((pod_x1 + 15, pod_y1 - 5), " PODGLĄD PŁYTY ", fill=(0, 0, 0), background=(212, 208, 200))

    # Canvas area (technical dark slate gray with grid and red/blue/gray shapes nested perfectly)
    can_x1, can_y1, can_x2, can_y2 = pod_x1 + 15, pod_y1 + 20, pod_x2 - 15, pod_y2 - 15
    draw.rectangle([can_x1, can_y1, can_x2, can_y2], fill=(40, 40, 40))
    draw_3d_rect(can_x1, can_y1, can_x2, can_y2, False)

    # Board outer frame
    sh_x1, sh_y1, sh_x2, sh_y2 = can_x1 + 20, can_y1 + 20, can_x2 - 20, can_y2 - 20
    draw.rectangle([sh_x1, sh_y1, sh_x2, sh_y2], fill=(65, 65, 65))
    draw.rectangle([sh_x1, sh_y1, sh_x2, sh_y2], outline=(120, 120, 120), width=1)

    # Grid dotted lines
    for x in range(sh_x1 + 30, sh_x2, 30):
        draw.line([(x, sh_y1), (x, sh_y2)], fill=(80, 80, 80), width=1)
    for y in range(sh_y1 + 30, sh_y2, 30):
        draw.line([(sh_x1, y), (sh_x2, y)], fill=(80, 80, 80), width=1)

    # Draw shapes onto board matching image
    # Standard components are light gray (220, 220, 220), with some red (204, 30, 30) and blue (20, 90, 210)
    # Let's draw several beautiful boxes representing high density nesting!
    shapes = [
        # x, y, width, height, color
        (sh_x1 + 10, sh_y1 + 10, 80, 60, (204, 30, 30)),     # Red box top left
        (sh_x1 + 95, sh_y1 + 10, 120, 70, (220, 220, 220)),
        (sh_x1 + 220, sh_y1 + 10, 100, 70, (220, 220, 220)),
        (sh_x1 + 10, sh_y1 + 75, 140, 65, (220, 220, 220)),
        (sh_x1 + 155, sh_y1 + 85, 90, 80, (220, 220, 220)),
        (sh_x1 + 250, sh_y1 + 85, 110, 80, (20, 90, 210)),    # Blue box middle-right
        (sh_x1 + 365, sh_y1 + 10, 160, 90, (220, 220, 220)),
        (sh_x1 + 10, sh_y1 + 145, 130, 85, (220, 220, 220)),
        (sh_x1 + 145, sh_y1 + 170, 200, 80, (220, 220, 220)),
        (sh_x1 + 350, sh_y1 + 170, 180, 80, (220, 220, 220)),
        (sh_x1 + 370, sh_y1 + 105, 120, 60, (220, 220, 220)),
    ]
    for sx, sy, sw, sh, color in shapes:
        if sx + sw < sh_x2 and sy + sh < sh_y2:
            draw.rectangle([sx, sy, sx + sw, sy + sh], fill=color, outline=(50, 50, 50))
            # Draw internal geometry pathways
            draw.line([(sx + 5, sy + 5), (sx + sw - 5, sy + 5)], fill=(30, 30, 30), width=1)
            draw.line([(sx + sw - 5, sy + 5), (sx + sw - 5, sy + sh - 5)], fill=(30, 30, 30), width=1)


    # 8. Right Panel: Parametry GroupBox
    par_x1, par_y1, par_x2, par_y2 = 475, 520, 1085, 740
    draw_3d_rect(par_x1, par_y1, par_x2, par_y2, False)
    draw.text((par_x1 + 15, par_y1 - 5), " Parametry ", fill=(0, 0, 0), background=(212, 208, 200))

    # Left Column parameters
    draw.text((par_x1 + 20, par_y1 + 25), "Płyta:", fill=(0, 0, 0))
    draw.rectangle([par_x1 + 110, par_y1 + 22, par_x1 + 250, par_y1 + 44], fill=(255, 255, 255))
    draw_3d_rect(par_x1 + 110, par_y1 + 22, par_x1 + 250, par_y1 + 44, False)
    draw.text((par_x1 + 115, par_y1 + 27), "3000 x 1500", fill=(0, 0, 0))

    draw.text((par_x1 + 20, par_y1 + 55), "Margines :", fill=(0, 0, 0))
    draw.rectangle([par_x1 + 110, par_y1 + 52, par_x1 + 250, par_y1 + 74], fill=(255, 255, 255))
    draw_3d_rect(par_x1 + 110, par_y1 + 52, par_x1 + 250, par_y1 + 74, False)
    draw.text((par_x1 + 115, par_y1 + 57), "10 mm", fill=(0, 0, 0))

    draw.text((par_x1 + 20, par_y1 + 85), "Odstęp:", fill=(0, 0, 0))
    draw.rectangle([par_x1 + 110, par_y1 + 82, par_x1 + 250, par_y1 + 104], fill=(255, 255, 255))
    draw_3d_rect(par_x1 + 110, par_y1 + 82, par_x1 + 250, par_y1 + 104, False)
    draw.text((par_x1 + 115, par_y1 + 87), ":5 mm", fill=(0, 0, 0))

    draw.text((par_x1 + 20, par_y1 + 115), "Obrót:", fill=(0, 0, 0))
    # Rotation checkboxes: ☑ 0 ☑ 90 ☐ 180 ☐ 270
    draw.text((par_x1 + 110, par_y1 + 115), "☑ 0  ☑ 90  ☐ 180  ☐ 270", fill=(0, 0, 0))

    # Right Column parameters (NC)
    draw.text((par_x1 + 280, par_y1 + 25), "NC:", fill=(0, 0, 0))

    draw.text((par_x1 + 280, par_y1 + 55), "Narzędzie:", fill=(0, 0, 0))
    draw.rectangle([par_x1 + 370, par_y1 + 52, par_x1 + 520, par_y1 + 74], fill=(255, 255, 255))
    draw_3d_rect(par_x1 + 370, par_y1 + 52, par_x1 + 520, par_y1 + 74, False)
    draw.text((par_x1 + 375, par_y1 + 57), "Oscylacyjne", fill=(0, 0, 0))
    draw.text((par_x1 + 505, par_y1 + 57), "▼", fill=(0, 0, 0)) # ComboBox Arrow

    draw.text((par_x1 + 280, par_y1 + 85), "Posuw:", fill=(0, 0, 0))
    draw.rectangle([par_x1 + 370, par_y1 + 82, par_x1 + 520, par_y1 + 104], fill=(255, 255, 255))
    draw_3d_rect(par_x1 + 370, par_y1 + 82, par_x1 + 520, par_y1 + 104, False)
    draw.text((par_x1 + 375, par_y1 + 87), "1200", fill=(0, 0, 0))

    # OWNER DRAW GENERATE BUTTON (Deep Blue style with bold white text)
    btn_x1, btn_y1, btn_x2, btn_y2 = par_x1 + 280, par_y1 + 130, par_x2 - 20, par_y2 - 20
    draw.rectangle([btn_x1, btn_y1, btn_x2, btn_y2], fill=(51, 102, 187))
    draw_3d_rect(btn_x1, btn_y1, btn_x2, btn_y2, True)
    draw.text((btn_x1 + 65, btn_y1 + 25), "GENERUJ NESTING", fill=(255, 255, 255))


    # 9. Bottom Status Bar (5 sections/panels)
    sb_y = h - 25
    draw.rectangle([0, sb_y, w, h], fill=(212, 208, 200))
    draw.line([(0, sb_y), (w, sb_y)], fill=(128, 128, 128), width=1)

    # Division lines
    draw.line([(220, sb_y + 3), (220, h - 3)], fill=(128, 128, 128), width=1)
    draw.line([(221, sb_y + 3), (221, h - 3)], fill=(255, 255, 255), width=1)

    draw.line([(420, sb_y + 3), (420, h - 3)], fill=(128, 128, 128), width=1)
    draw.line([(421, sb_y + 3), (421, h - 3)], fill=(255, 255, 255), width=1)

    draw.line([(680, sb_y + 3), (680, h - 3)], fill=(128, 128, 128), width=1)
    draw.line([(681, sb_y + 3), (681, h - 3)], fill=(255, 255, 255), width=1)

    draw.line([(880, sb_y + 3), (880, h - 3)], fill=(128, 128, 128), width=1)
    draw.line([(881, sb_y + 3), (881, h - 3)], fill=(255, 255, 255), width=1)

    # Panels texts
    draw.text((10, sb_y + 5), "18 detali", fill=(0, 0, 0))
    draw.text((230, sb_y + 5), "2 płyty", fill=(0, 0, 0))
    draw.text((430, sb_y + 5), "Wykorzystanie 92.41%", fill=(0, 0, 0))
    draw.text((690, sb_y + 5), "Czas 1.82 s", fill=(0, 0, 0))
    draw.text((890, sb_y + 5), "5 wątków", fill=(0, 0, 0))

    # Save output file
    os.makedirs("Nesting2D", exist_ok=True)
    img.save("Nesting2D/zrzut_ekranu.png")
    print("Mockup generated successfully as Nesting2D/zrzut_ekranu.png!")

if __name__ == "__main__":
    create_mockup()
