Xephyr :1 -terminate -screen 1024x768 &
sleep 2
DISPLAY=:1
$HOME/tinywm/main &
sleep 2
xsetroot -solid "#aaaaaa"
urxvt &
urxvt &
urxvt
