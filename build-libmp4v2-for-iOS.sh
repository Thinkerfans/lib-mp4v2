#!/bin/sh

#IOS_BASE_SDK=10.2
SOURCE="mp4v2-2.0.0"

ROOT=`pwd`
FAT="$ROOT/fat"
THIN="$ROOT/thin"

ARCHS="i386 x86_64 armv7 armv7s arm64 "
#ARCHS="i386"
#ARCHS="i386 x86_64"
CONFIGURE_FLAGS="--disable-gch --disable-debug --disable-util \
                  --enable-shared=no"

clean()
{
  rm -rf $THIN
  rm -rf $FAT
}

clean

for ARCH in $ARCHS
do
  echo "building $ARCH .."

  if [ "$ARCH" = "i386" -o "$ARCH" = "x86_64" ]
      then
      PLATFORM="iPhoneSimulator"
      CPU=
      if [ "$ARCH" = "x86_64" ]
          then
          SIMULATOR="-mios-simulator-version-min=7.0"
          HOST=
      else
          SIMULATOR="-mios-simulator-version-min=5.0"
          HOST="--host=i386-apple-darwin"
      fi
  else
      PLATFORM="iPhoneOS"
      if [ $ARCH = "armv7s" ]
          then
          CPU="--cpu=swift"
      else
          CPU=
      fi
      SIMULATOR=
      HOST="--host=arm-apple-darwin"
  fi

  DEVROOT=`xcode-select -p`/"Platforms/$PLATFORM.platform/Developer"
  XCRUN_SDK=`echo $PLATFORM | tr '[:upper:]' '[:lower:]'`
  #SDKROOT=$DEVROOT/SDKs/$PLATFORM$IOS_BASE_SDK.sdk
  SDKROOT=`(xcrun --sdk $XCRUN_SDK --show-sdk-path)`
  
  
  CFLAGS="-arch $ARCH $SIMULATOR -pipe -no-cpp-precomp -isysroot $SDKROOT -I$SDKROOT/usr/include/"

  export CFLAGS="$CFLAGS"

  export CXX="llvm-g++"
  export CC="llvm-gcc"

  if [ "$ARCH" = "i386" -o "$ARCH" = "x86_64" ]
     then
     export LD=$DEVROOT/usr/bin/ld
     export LDFLAGS="-L$SDKROOT/usr/lib/"
 else
     export LD=$DEVROOT/usr/bin/ld
     export AS=$DEVROOT/usr/bin/as
     export NM=$DEVROOT/usr/bin/nm
     export LDFLAGS="-L$SDKROOT/usr/lib/"
     export LIBTOOL=$DEVROOT/usr/bin/libtool
     export LIPO=$DEVROOT/usr/bin/lipo
     export OTOOL=$DEVROOT/usr/bin/otool
     export NMEDIT=$DEVROOT/usr/bin/nmedit
     export DSYMUTIL=$DEVROOT/usr/bin/dsymutil
     export STRIP=$DEVROOT/usr/bin/strip
 fi


  export CPPFLAGS=$CFLAGS
  export CXXFLAGS=$CFLAGS

  make distclean
  $ROOT/$SOURCE/configure $CONFIGURE_FLAGS $HOST --prefix="$THIN/$ARCH"
  make
  make install

  echo "build $ARCH done.."
done

echo "building fat .."

mkdir -p "${FAT}/lib"

set - $ARCHS
CWD=`pwd`

cd ${THIN}/$1/lib
for LIB in `ls *.a`
do
    echo $LIB
    cd $CWD
    xcrun -sdk iphoneos lipo -create `find $THIN -name $LIB` -output $FAT/lib/$LIB
done

cd $CWD
cp -rf $THIN/$1/include $FAT

echo "build fat done.."
