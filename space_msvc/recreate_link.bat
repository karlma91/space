echo off
echo NB! Hvis du har gjort lokale endringer vil disse slettes!
rd /S src
mklink /J src ..\src
pause