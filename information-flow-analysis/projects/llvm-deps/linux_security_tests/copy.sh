#!/usr/bin/env bash
# linking example

CUR_DIR=$(pwd)
PARENT_DIR=../../../../Mediator-Documentation/Mediator-Analysis-Data/Gaps/Raw
SETTING=$(echo $(pwd) | sed 's/^.*_\(.*\)/\1/')

rsync -azhv $CUR_DIR/apparmor/results/ $PARENT_DIR/$SETTING/apparmor/
rsync -azhv $CUR_DIR/tomoyo/results/ $PARENT_DIR/$SETTING/tomoyo/
rsync -azhv $CUR_DIR/selinux/results/ $PARENT_DIR/$SETTING/selinux/
