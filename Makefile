# Attempt to load a config.make file.
# If none is found, project defaults in config.project.make will be used.
ifneq ($(wildcard config.make),)
	include config.make
endif

# make sure the the OF_ROOT location is defined
ifndef OF_ROOT
	OF_ROOT=$(realpath ../../..)
endif

# call the project makefile!
include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk

TARGET_BUILD_DIR = bin
#PRODUCT_NAME

PRODUCT_NAME = $(APPNAME)

copy_ni_libs:
 
# make -j -s 2>&1 && make Release
#	include $(OF_ROOT)/libs/openFrameworksCompiled/project/makefileCommon/compile.project.mk
	@echo "target is " $(TARGET_BUILD_DIR)
	@echo "product name is" $(PRODUCT_NAME)
	@cp -R ../../../addons/ofxNI2/libs/OpenNI2/lib/osx/ $(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/MacOS/

	@cp -R ../../../addons/ofxNI2/libs/NiTE2/lib/osx/libNiTE2.dylib $(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/MacOS/libNiTE2.dylib

	@cp -R ../../../addons/ofxNI2/libs/NiTE2/lib/osx/NiTE.ini $(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/Resources/NiTE.ini

	@cp -R ../../../addons/ofxNI2/libs/NITE2-data/NiTE2 $(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/Resources/
	
	#install_name_tool -change libOpenNI2.dylib $(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/MacOS/libOpenNI2.dylib "$(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/MacOS/$(PRODUCT_NAME)"

	#install_name_tool -change libOpenNI2.dylib $(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/MacOS/../MacOS/libOpenNI2.dylib "$(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/MacOS/$(PRODUCT_NAME)"
	#cp ../../../addons/ofxNI2/libs/OpenNI2/lib/osx/OpenNI2/Drivers/* $(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/MacOS/

	#cp -R ../../../addons/ofxNI2/libs/NiTE2/lib/osx/libNiTE2.dylib $(TARGET_BUILD_DIR)/$(PRODUCT_NAME).app/Contents/MacOS/libNiTE2.dylib

# build copy nilibs and run app
copyrun:
	make -j -s 2>&1 && make Release && make copy_ni_libs && cd bin/$(PRODUCT_NAME).app/Contents/MacOS/ && ./$(PRODUCT_NAME)

runonly: 
	cd bin/$(PRODUCT_NAME).app/Contents/MacOS/ && ./$(PRODUCT_NAME)

	

