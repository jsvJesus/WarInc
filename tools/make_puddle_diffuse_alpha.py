import os
import shutil
import subprocess
from pathlib import Path

from PIL import Image


ROOT = Path(__file__).resolve().parents[1]

MASK_DDS = ROOT / "bin" / "Data" / "Water" / "Puddles" / "puddles_perlin.dds"
OUT_DIR = ROOT / "bin" / "Data" / "Water"

TEMP_DIR = ROOT / "tools" / "_puddle_temp"
TEMP_MASK_PNG = TEMP_DIR / "puddles_perlin.png"
TEMP_RGBA_PNG = TEMP_DIR / "puddles_diffuse_alpha.png"

OUT_DDS = OUT_DIR / "puddles_diffuse_alpha.dds"

WATER_RGB = (21, 25, 21)


def find_texconv():
	local = ROOT / "tools" / "texconv.exe"

	if local.exists():
		return str(local)

	found = shutil.which("texconv.exe")

	if found:
		return found

	found = shutil.which("texconv")

	if found:
		return found

	raise RuntimeError(
		"texconv.exe не найден. Установи: winget install Microsoft.DirectXTex.Texconv "
		"или положи texconv.exe в папку WarInc\\tools\\"
	)


def run(cmd):
	print(" ".join(str(x) for x in cmd))
	subprocess.check_call(cmd)


def main():
	if not MASK_DDS.exists():
		raise RuntimeError(f"Не найден файл: {MASK_DDS}")

	TEMP_DIR.mkdir(parents=True, exist_ok=True)
	OUT_DIR.mkdir(parents=True, exist_ok=True)

	texconv = find_texconv()

	run([
		texconv,
		"-y",
		"-ft", "png",
		"-m", "1",
		"-o", str(TEMP_DIR),
		str(MASK_DDS)
	])

	if not TEMP_MASK_PNG.exists():
		raise RuntimeError(f"texconv не создал PNG: {TEMP_MASK_PNG}")

	mask = Image.open(TEMP_MASK_PNG).convert("L")

	rgba = Image.new("RGBA", mask.size, WATER_RGB + (255,))
	rgba.putalpha(mask)
	rgba.save(TEMP_RGBA_PNG)

	run([
		texconv,
		"-y",
		"-ft", "dds",
		"-f", "DXT5",
		"-m", "0",
		"-dx9",
		"-sepalpha",
		"--keep-coverage", "0.033",
		"-o", str(OUT_DIR),
		str(TEMP_RGBA_PNG)
	])

	if not OUT_DDS.exists():
		raise RuntimeError(f"DDS не создан: {OUT_DDS}")

	print("")
	print("OK:")
	print(OUT_DDS)


if __name__ == "__main__":
	main()