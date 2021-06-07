#!/bin/sh
tmux kill-server
sleep 1
NAME=chat
TMUX= tmux new-session -d -s $NAME
tmux send-keys './server.out 42069 ./xo_server.sock' C-m
tmux split-window -h
tmux send-keys './client.out test0 net 127.0.0.1:42069' C-m
tmux split-window -v
tmux send-keys './client.out test1 net 127.0.0.1:42069' C-m
tmux select-pane -t 0
tmux split-window -v
tmux send-keys './client.out test2 net 127.0.0.1:42069' C-m
sleep 1
tmux -2 attach-session -d
