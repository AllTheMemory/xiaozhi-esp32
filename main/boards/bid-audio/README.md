# BidAudio Board

ESP32-S3R8 based audio-only board using ES8311 codec, NS4150B amplifier and ZTS6216 microphone.

## Features
- ES8311 codec via I2S
- NS4150B speaker amplifier controlled by `PA_EN`
- SK6816 (WS2812 compatible) addressable LED on GPIO48
- Three buttons: BOOT/Trigger (GPIO0), Volume Up (GPIO39), Volume Down (GPIO40)
- Supports acoustic Wi-Fi provisioning
- No display

## Pin Mapping
| Function | GPIO |
|----------|------|
| I2S MCLK | 2 |
| I2S BCLK | 17 |
| I2S LRCK | 45 |
| I2S DIN  | 15 |
| I2S DOUT | 16 |
| I2C SDA  | 8 |
| I2C SCL  | 18 |
| PA Enable | 46 |
| LED (SK6816) | 48 |
