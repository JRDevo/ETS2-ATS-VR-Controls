import sys
from PIL import Image, ImageDraw, ImageFont
# banner_languages.py IN OUT (v0.3.0): the "All 43 game languages" badge on docs/images/banner.png and social-preview.jpg,
# in the dark cab roof above "ETS2 · ATS" (top right), in the title's orange. Run once on the v0.2.0 pictures.
src, out = sys.argv[1], sys.argv[2]
im = Image.open(src).convert('RGB')
px = im.load()
# the orange of "ETS2 · ATS"
cs = [px[x, y] for x in range(1050, 1225) for y in range(75, 100) if px[x, y][0] > 200 and px[x, y][0] - px[x, y][2] > 150]
orange = tuple(sorted(c[i] for c in cs)[len(cs) // 2] for i in range(3))   # the median of the letters' full-colour pixels
S = 4                                                   # drawn 4x and shrunk: smooth edges
font = ImageFont.truetype('C:/Windows/Fonts/seguisb.ttf', 19 * S)
text = 'All 43 game languages'
pad_x, h = 14 * S, 32 * S
tw = ImageDraw.Draw(im).textlength(text, font=font)
w = int(tw + 2 * pad_x)
right, top = 1222, 22
layer = Image.new('RGBA', (w, h), (0, 0, 0, 0))
d = ImageDraw.Draw(layer)
d.rounded_rectangle((0, 0, w - 1, h - 1), radius=h // 2, fill=(12, 14, 18, 215), outline=orange + (255,), width=2 * S)
d.text((w / 2, h / 2 + S), text, font=font, fill=orange + (255,), anchor='mm')
layer = layer.resize((w // S, h // S), Image.LANCZOS)
im.paste(layer, (right - layer.width, top), layer)
if out.lower().endswith('.jpg'):
    im.save(out, quality=92)
else:
    im.save(out)
print('orange', orange)
