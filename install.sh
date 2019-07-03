
if "$( which apt-get )" 2> /dev/null; then
   echo "detected Debian-based"
   sudo apt install     cmake  libuv1-dev

elif "$( which yum )" 2> /dev/null; then
   echo "detected Modern Red Hat-based"
   sudo yum install     cmake  libuv-devel

elif "$( which portage )" 2> /dev/null; then
   echo "detected Gentoo-based"
   echo "please make sure you have packages: cmake , libuv deveopment"

elif VERB="$( which pacman )" 2> /dev/null; then
   echo "detected Arch-based"
   echo "please make sure you have packages: cmake , libuv deveopment"
else
   echo "can't detect os type";
   echo "please make sure you have packages: cmake , libuv deveopment"
fi

npm install -g node-gyp

git clone https://github.com/transceptor-technology/libqpack.git
cd libqpack/Release
make
sudo make install
cd ../../

git clone https://github.com/SiriDB/libsiridb.git
cd libsiridb/Release
make all
sudo make install
cd ../../

git clone https://github.com/SiriDB/libsuv.git
cd libsuv/Release
make
sudo make install
cd ../../

node-gyp configure
node-gyp build
