import subprocess, sys, os
Import("env")

def merge_firmware(source, target, env):
    build_dir  = env.subst("$BUILD_DIR")
    flash_size = "8MB"

    bootloader = os.path.join(build_dir, "bootloader.bin")
    partitions = os.path.join(build_dir, "partitions.bin")
    app        = os.path.join(build_dir, "firmware.bin")
    merged     = os.path.join(build_dir, "merged.bin")

    esptool = os.path.join(
        env.subst("$PROJECT_PACKAGES_DIR"),
        "tool-esptoolpy", "esptool.py"
    )

    cmd = [
        sys.executable, esptool,
        "--chip", "esp32s3",
        "merge_bin",
        "--flash_mode", "dio",
        "--flash_freq", "80m",
        "--flash_size", flash_size,
        "0x0",     bootloader,
        "0x8000",  partitions,
        "0x10000", app,
        "-o", merged,
    ]

    print("Merging firmware for Wokwi...")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode == 0:
        print(f"Merged binary: {merged}")
    else:
        print("Merge failed:", result.stderr)

env.AddPostAction("$BUILD_DIR/firmware.bin", merge_firmware)
