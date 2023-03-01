# DSPOLED
Interfaces a 256x64 pixel OLED display to CamillaDSP running on a Raspberry Pi 4

# Building & Use
DSPOLED expects the CamillaDSP websocket server to be listening at address **ws://127.0.0.1:1234**
```
sudo apt-get install -y fonts-open-sans
make
sudo ./dspoled
```

# Automatically Run at Startup
You can optionally edit the oled.service file and change both instances of **\<Full-Path-To-DSPOLED-Directory\>** to be the full directory path where you have the dspoled executable and bitmaps. Once it's been updated install the service.
```
sudo cp oled.service /lib/systemd/system/oled.service
sudo systemctl enable oled
sudo systemctl start oled
```
