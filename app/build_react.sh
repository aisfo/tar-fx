rm -rf src/main/assets
mkdir src/main/assets
../node_modules/.bin/react-native bundle --platform android --dev false --entry-file app/src/main/react/index.android.js --bundle-output src/main/assets/index.android.bundle --assets-dest src/main/res/