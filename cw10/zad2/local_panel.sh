#!/bin/sh
tmux kill-server
sleep 1
NAME=chat
TMUX= tmux new-session -d -s $NAME
tmux send-keys './server.out 42069 ./xo_server.sock' C-m
tmux split-window -h
tmux send-keys './client.out test0 local ./xo_server.sock' C-m
tmux split-window -v
tmux send-keys './client.out test1 local ./xo_server.sock' C-m
tmux select-pane -t 0
tmux split-window -v
tmux send-keys './client.out test2 local ./xo_server.sock' C-m
tmux -2 attach-session -d
