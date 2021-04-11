idf_component_register(SRCS src/canvas.cpp src/collisiondetector.cpp src/displaycontroller.cpp src/fabfonts.cpp src/fabui.cpp src/fabutils.cpp src/scene.cpp src/terminal.cpp src/terminfo.cpp src/ulp_macro_ex.cpp src/comdrivers/ps2controller.cpp src/comdrivers/ps2device.cpp src/comdrivers/tsi2c.cpp src/devdrivers/DS3231.cpp src/devdrivers/keyboard.cpp src/devdrivers/mouse.cpp src/devdrivers/soundgen.cpp src/devdrivers/swgenerator.cpp src/devdrivers/usbkb.cpp src/dispdrivers/SSD1306Controller.cpp src/dispdrivers/TFTControllerGeneric.cpp src/dispdrivers/TFTControllerSpecif.cpp src/dispdrivers/vgacontroller.cpp
                       INCLUDE_DIRS src                       # Edit following two lines to set component requirements (see docs)
                       REQUIRES ulp fatfs spiffs arduino
                       PRIV_REQUIRES )

