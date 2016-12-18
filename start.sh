Xephyr :1 -terminate -screen 1024x768 & xephyr_pid=$!
sleep 2
DISPLAY=:1
$HOME/tinywm/main & main_pid=$!
sleep 2
xsetroot -solid "#aaaaaa"
urxvt & urxvt_pid=$!
chromium & chrome_pid=$!

read _

kill $chrome_pid
kill $urxvt_pid
kill $main_pid
kill $xephyr_pid
