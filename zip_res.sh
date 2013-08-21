rm game_data.zip

cd game_data
zip -rX0 ../game_data.zip . -x "*/.*" -x ".*"

cd ..
