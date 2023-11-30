#!/usr/bin/env bash
# linking example

ANNOTATION=true
CONSTANT=true
FINAL=false

if [ $FINAL = true ] ; then
    TABLE_FILE=../../../../../Mediator-Documentation/Mediator-Analysis-Data/Gaps/Purpose-Final.md
elif [ $ANNOTATION = true ] ; then
    TABLE_FILE=../../../../../Mediator-Documentation/Mediator-Analysis-Data/Gaps/Purpose-Relaxed.md
elif [ $CONSTANT = true ] ; then
    TABLE_FILE=../../../../../Mediator-Documentation/Mediator-Analysis-Data/Gaps/Purpose-Strict.md
else
    TABLE_FILE=../../../../../Mediator-Documentation/Mediator-Analysis-Data/Gaps/Purpose-Field.md
fi

OUT_DIR=results
CUR_DIR=$(pwd)

automation()
{
    cd ./$1/
    pwd
    rm -f config.json
    mkdir -p $OUT_DIR
    for f in ./configs/* ; do 
        FUNCTION=$(echo $f | sed -e 's/.\/configs\/\(.*\).json/\1/')
        echo $FUNCTION
        mkdir -p $OUT_DIR/$FUNCTION

        cp $f ./config.json

        if [ $ANNOTATION = true ] ; then
            sed -i -r "s/\"using_whitelist\": false/\"using_whitelist\": true/g" "config.json"
        else
            sed -i -r "s/\"using_whitelist\": true/\"using_whitelist\": false/g" "config.json"
        fi

        if [ $CONSTANT = true ] ; then
            sed -i -r "s/\"using_constant\": false/\"using_constant\": true/g" "config.json"
        else
            sed -i -r "s/\"using_constant\": true/\"using_constant\": false/g" "config.json"
        fi

        cp $f $OUT_DIR/$FUNCTION/config.json

        ./run.sh $2 $FUNCTION

        mv constraints-*.con $OUT_DIR/$FUNCTION/
        mv tmp-$FUNCTION.dat $OUT_DIR/$FUNCTION/$FUNCTION.dat

        python3 ../../processing_tools/constraint_graph.py $OUT_DIR/$FUNCTION/constraints-explicit.con $OUT_DIR/$FUNCTION/graph-explicit.dot
        python3 ../../processing_tools/constraint_graph.py $OUT_DIR/$FUNCTION/constraints-implicit.con $OUT_DIR/$FUNCTION/graph-implicit.dot

        # echo 'Looking for unlabeled variables'
        # python3 ../../processing_tools/unlabeled_violation.py $OUT_DIR/$FUNCTION/graph-explicit.dot > $OUT_DIR/$FUNCTION/unlabeled.log

        echo -e "[$FUNCTION]" > $OUT_DIR/$FUNCTION/gaps.log

        echo 'Looking for explicit flow gaps'
        echo -e '\nexplicit:' >> $OUT_DIR/$FUNCTION/gaps.log
        python3 ../../processing_tools/shortest_path.py $OUT_DIR/$FUNCTION/graph-explicit.dot >> $OUT_DIR/$FUNCTION/gaps.log lattice.txt

        echo 'Looking for implicit flow gaps'
        echo -e '\nimplicit:' >> $OUT_DIR/$FUNCTION/gaps.log
        python3 ../../processing_tools/shortest_path.py $OUT_DIR/$FUNCTION/graph-implicit.dot >> $OUT_DIR/$FUNCTION/gaps.log lattice.txt

        echo 'Updating the detected gaps table...'
        python3 ../../processing_tools/update_gaps.py $TABLE_FILE $OUT_DIR/$FUNCTION/gaps.log

    done
    rm -f config.json
    cd -
}

main()
{   
    echo "Building for any changes."
    cd ../
    make
    cd ../poolalloc/
    make
    cd $CUR_DIR
    
    echo "Executing automation."
    # automation tomoyo test.bc
    # automation apparmor test.bc
    automation selinux test.bc
    # automation xserver test.bc
}

main