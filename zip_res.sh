make -C bin/data
make -C particles
make -C textures
make -C tilemaps

mkdir -p .temporary_res
rm -rf .temporary_res/game_data
mkdir .temporary_res/game_data

cd .temporary_res/game_data

unzip -quo -d . ../data.zip
unzip -quo -d particles ../particles.zip
unzip -quo -d textures ../textures.zip
unzip -quo -d tilemaps ../tilemaps.zip

rm ../../game_data.zip
zip -rX0 ../../game_data.zip . -x "*/.*" -x ".*"

cd ..
