to compile and run
build the app cmd B select build release
or
create terminal @ofroot/apps/myapp/{this_app_name} e.g. apps/myapps/Kinect2_1

make RunRelease

now need to copy libs across to bin/{current_proj}.app/Contents/MacOS/ from the openni2 addon directory
copy_ni_libs is in the local makefile for this purpose
make copy_ni_libs

now do a make RunRelease
or move to the executalbe directory
cd bin/Kinect2_1.app/Contents/MacOS
and run ./Kinect2_1 or whatever you have name d your binary executable




