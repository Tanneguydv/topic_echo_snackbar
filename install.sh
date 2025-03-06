rm -rf build
mkdir build
cd build
cmake ..
make
make install
export IGN_GUI_PLUGIN_PATH=/usr/local/lib/x86_64-linux-gnu/ign-gui-6/custom_plugins/:$IGN_GUI_PLUGIN_PATH


