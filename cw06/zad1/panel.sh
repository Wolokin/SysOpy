#!/bin/sh
ipcrm --all=msg
tmux kill-server
sleep 1
NAME=chat
TMUX= tmux new-session -d -s $NAME
tmux send-keys './server.out' C-m
tmux split-window -h
tmux send-keys 'watch -n 0.1 ipcs -q' C-m
sleep 1
tmux split-window -v
tmux send-keys './client.out' C-m
tmux select-pane -t 0
tmux split-window -v
tmux send-keys './client.out' C-m
tmux -2 attach-session -d
