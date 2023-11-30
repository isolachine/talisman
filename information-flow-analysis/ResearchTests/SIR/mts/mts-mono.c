/*----------------------------------------------------------------------*/
/* mts-mono.c
   This is a modified version of mts-old.c (which was based upon
   the original make_test_script.c) to handle the legacy universe
   files of SIR and make them compatible with things like Java and
   Mono runtime-engines.  All this script does is add another
   option to the command line (which is required) that will indicate
   the command to issue on the command line prior to the executable name
   specified (supplied on the command line).  The usage statement has
   been modified also to show where this new command line flag/switch
   and its value (the runtime-engine to use, e.g. mono or java) are
   and what are permitted values at this time.  The following is the
   original readme text of make_test_script.c

   make_test_script.c
   This program turns a universe or testplan file into a test 
   script -- possibly a selective one.  It is an ugly program
   with an ugly interface, but it works.

   You give it the name of a directory that is organized
   in our format, with subdirectories for inputs, testplans,
   and so forth.  You also give it the name of the executable
   that must be run, the plan file, a flag R, D, T, or S or N 
   to indicate what type of test script you want generated, 
   and the name of the script to be created.

   IF you are asking for a tracing (T) script, you also
   provide the name of the source file, without suffix,
   from which the executable was built; that name is used
   to locate the trace file in the Aristotle database.
   If you aren't asking for a tracing script, you type NULL
   for that argument.

   IF you want a selective (S or N) script, you type the name of the
   testlist file; else type NULL for that argument.  
   S writes a script to run selected tests, N writes one
   to run non-selected tests.

   The program reads the given universe or plan file, and creates a
   script of the type you asked for, that runs only the tests listed
   in the testlist file if you supplied one.

 
   EXAMPLE:

   Given a plan file that contains just two lines:

7 1 9 < input/inp.58
2 3 5 < input/inp.46
   
   here's what this program generates.   This is for the "schedule"
   Siemens program, which I had in a test directory of the
   usual structure called "/nfs/mica/u3/grother/siemens/schedule".
   And for which I'd made an executable called "schedule".
   And for which I had the above plan file, called "plan1".

To get an "R" script I typed:

    a.out /nfs/mica/u3/grother/siemens/schedule /nfs/mica/u3/grother/siements/source/schedule /nfs/mica/u3/grother/siemens/schedule/testplans/plan1 R script NULL NULL

And I got:

echo ">>>>>>>>running test 1"
/nfs/mica/u3/grother/siemens/schedule/source/schedule 7 1 9  < /nfs/mica/u3/grother/siemens/schedule/inputs/input/inp.58 > /nfs/mica/u3/grother/siemens/schedule/outputs/t1
echo ">>>>>>>>running test 2"
/nfs/mica/u3/grother/siemens/schedule/source/schedule 2 3 5  < /nfs/mica/u3/grother/siemens/schedule/inputs/input/inp.46 > /nfs/mica/u3/grother/siemens/schedule/outputs/t2

For each test, it prints that it's running the test, then runs it.
It puts output into the outputs subdirectory.
Output file "tn" for test "n".

To get a "D" script I typed:

    a.out /nfs/mica/u3/grother/siemens/schedule /nfs/mica/u3/grother/siements/source/schedule /nfs/mica/u3/grother/siemens/schedule/testplans/plan1 D script NULL NULL

And I got:

echo ">>>>>>>>running test 1"
/nfs/mica/u3/grother/siemens/schedule/source/schedule 7 1 9  < /nfs/mica/u3/grother/siemens/schedule/inputs/input/inp.58 > /nfs/mica/u3/grother/siemens/schedule/newoutputs/t1
cmp -s /nfs/mica/u3/grother/siemens/schedule/outputs/t1 /nfs/mica/u3/grother/siemens/schedule/newoutputs/t1 || echo different results
echo ">>>>>>>>running test 2"
/nfs/mica/u3/grother/siemens/schedule/source/schedule 2 3 5  < /nfs/mica/u3/grother/siemens/schedule/inputs/input/inp.46 > /nfs/mica/u3/grother/siemens/schedule/newoutputs/t2
cmp -s /nfs/mica/u3/grother/siemens/schedule/outputs/t2 /nfs/mica/u3/grother/siemens/schedule/newoutputs/t2 || echo different results

For each test n, it prints that it's running the test, then
runs it, putting output file "tn" into the newoutputs
directory.  It then diffs this output file with the "tn"
in the outputs directory, and if there's a diff prints a message. 


Assume I had an instrumented executable called "schedule" in my source dir.
To get a "T" script I typed:

    a.out /nfs/mica/u3/grother/siemens/schedule /nfs/mica/u3/grother/siements/source/schedule /nfs/mica/u3/grother/siemens/schedule/testplans/plan1 T script schedule NULL

And I got:

echo ">>>>>>>>running test 1"
/nfs/mica/u3/grother/siemens/schedule/source/schedule 7 1 9  < /nfs/mica/u3/grother/siemens/schedule/inputs/input/inp.58 > /nfs/mica/u3/grother/siemens/schedule/newoutputs/t1
cp $ARISTOTLE_DB_DIR/schedule.c.tr /nfs/mica/u3/grother/siemens/schedule/traces/0.tr
echo ">>>>>>>>running test 2"
/nfs/mica/u3/grother/siemens/schedule/source/schedule 2 3 5  < /nfs/mica/u3/grother/siemens/schedule/inputs/input/inp.46 > /nfs/mica/u3/grother/siemens/schedule/newoutputs/t2
cp $ARISTOTLE_DB_DIR/schedule.c.tr /nfs/mica/u3/grother/siemens/schedule/traces/1.tr

For each test n, it runs the instrumented schedule, which causes schedule.c.tr
to be put into the aristotle database directory.  The script then copies that
over into the traces directory.  Notice that test number 1 results in
trace file 0.tr, 2 produces 1.tr, and so on.


Assume I had a list of selected tests named "testlist" in my testplans dir.
To get an "S" script I typed:

    a.out /nfs/mica/u3/grother/siemens/schedule /nfs/mica/u3/grother/siements/source/schedule /nfs/mica/u3/grother/siemens/schedule/testplans/plan1 S script NULL /nfs/mica/u3/grother/siemens/schedule/testplans/testlist

I'll get something just like the "D" result, but for only the
tests that were listed in testlist. 



Assume I had a list of selected tests named "testlist" in my testplans dir.
To get an "N" script I typed:

    a.out /nfs/mica/u3/grother/siemens/schedule /nfs/mica/u3/grother/siements/source/schedule /nfs/mica/u3/grother/siemens/schedule/testplans/plan1 N script NULL /nfs/mica/u3/grother/siemens/schedule/testplans/testlist

I'll get something just like the "D" result, but for only the
tests that were NOT listed in testlist. 

*/
/*---------------------------------------------------------------------*/
 
/* Include files */

#include <stdio.h>
#include <strings.h>

/*---------------------------------------------------------------------*/
 
/* global variables */
 
#define INPUTMAX 4096
#define FILENAMEMAX 256
#define STOP 999999999

/*----------------------------------------------------------------------*/
/* Description:  main
 
     Does all the work.

   Parameters:  Takes command line args.  See the printf near
                the top of the code, below.
 
   Return value:  none; prints out a test script.
 
   Revision History: 11-8-1996  Gregg Rothermel      -    created
 
*/
/*---------------------------------------------------------------------*/

main(int argc, char *argv[])

{

    FILE *fp, *nfp, *tfp;
    char input[INPUTMAX];
    char tinput[INPUTMAX];
    char fname[INPUTMAX];
    char params[INPUTMAX];
    char univfile[FILENAMEMAX];
    char outfile[FILENAMEMAX];
    char testlistfile[FILENAMEMAX];
    char outname[FILENAMEMAX];
    char progname[FILENAMEMAX];     
    char virtmach[FILENAMEMAX];  /* added for mts-mono support */
    char test_db_dir[FILENAMEMAX];  
    char inpfile[FILENAMEMAX];
    char outpfile[FILENAMEMAX];
    char newoutpfile[FILENAMEMAX];
    char exename[FILENAMEMAX];     
    char bracket[2];
    char scripttypechar[2];

    int i,j,k;
    int testnum;
    int nextlistedtest;
    int langle;
    int useinputfile;
    int scripttype;
    int selecttype;
    int printit;

    if (argc != 9)
    {
       printf("USAGE: %s <dir> <exe> <plan> R|D|T <outfile> <program> <testlist> <virt-machine>\n",argv[0]);
       printf("\n");
       printf("  <dir>      : name of test database dir \n");
       printf("  <exe>      : name of exe \n");
       printf("  <plan>     : name of plan \n");
       printf("  R|D|T|S|N  : script type = Runall, runall-and-Diff,runall-Trace\n");
       printf("                             run-Selected, run-Nonselected \n");
       printf("  <outfile>  : name of output script \n");
       printf("  <program>  : suffix of program name or NULL -- only for script type T.\n");
       printf("  <testlist> : testlist or NULL -- only for script type S or N.\n");
       printf("  <virt-machine>: the virtual machine needed to execute the <program>\n");
       printf("                  currently only java and mono are valid values\n");
       printf("\n");
       exit(-1);
    }

    /* argv[1] holds the name of the test database directory for some source file */
    strcpy(test_db_dir,argv[1]);

    /* argv[2] holds the name of an executable to be run by the script */
    strcpy(exename,argv[2]);

    /* argv[3] holds name of plan file */
    strcpy(univfile,argv[3]);
    fp = fopen(univfile,"rt");
    if (fp == NULL)
    {
       printf("Cant find universe or plan file %s",univfile);
       exit(-1);
    }

    selecttype = 0;
    /* see what type of script is required; set scripttype accordingly;
       no real error-checking on what user provided as a flag */
    if (strstr(argv[4],"R")!=NULL || strstr(argv[4],"r")!=NULL)
    {
       scripttype=1;
       strcpy(scripttypechar,"R");
    }
    else if (strstr(argv[4],"D")!=NULL || strstr(argv[4],"d")!=NULL)
    {
       scripttype=2;
       strcpy(scripttypechar,"D");
    }
    else if (strstr(argv[4],"T")!=NULL || strstr(argv[4],"t")!=NULL)
    {
       scripttype=3;
       strcpy(scripttypechar,"T");
    }
    else if (strstr(argv[4],"S")!=NULL || strstr(argv[4],"s")!=NULL)
    {
       selecttype=4;
       scripttype=2;
       strcpy(scripttypechar,"S");
    }
    else if (strstr(argv[4],"N")!=NULL || strstr(argv[4],"n")!=NULL)
    {
       selecttype=5;
       scripttype=2;
       strcpy(scripttypechar,"N");
    }
    else
    {
       printf("Improper usage \n",argv[0]);
       exit(-1);
    }

    /* open the output script file */
    strcpy(outfile,argv[5]);
    nfp = fopen(outfile,"wt");
    if (nfp == NULL)
    {
       printf("Cant open script file %s",outfile);
       exit(-1);
    }

    strcpy(progname,argv[6]);

    if (selecttype > 0)
    {
       strcpy(testlistfile,argv[7]);
       tfp = fopen(testlistfile,"rt");
       if (tfp == NULL)
       {
          printf("Can't find testlist file %s",testlistfile);
          exit(-1);
       }
    }
    if (strstr(argv[8],"mono")!=NULL || strstr(argv[8],"java")!=NULL)
        strcpy(virtmach,argv[8]);
    else {
       printf("<virt-machine> must be either java or mono\n");
       exit(-1);
    }


    /* write the type of script file as a comment into top of script */
    fprintf(nfp,"echo script type: %s\n",scripttypechar);

    /* test numbers start at 1 and are gotten from line # in univ file */
    testnum = 1;

    /* clear out strings we'll read a line from the universe file into */
    for (k=0;k<INPUTMAX;k++)
    {
       input[k] = '\0';
       params[k] = '\0';
       fname[k] = '\0';
    }

    /* get first line of input */
    fgets(input,INPUTMAX,fp); 
    if (input == NULL)
    {
       printf("Error on initial read from universe file.\n");
       fclose(fp);
       fclose(nfp);
       exit(-1);
    }

    /* get the first test from the testlist if necessary */
    if ( (selecttype == 4) || (selecttype == 5) )
    {
        fgets(tinput,INPUTMAX,tfp);
        if (feof(tfp))
           nextlistedtest=STOP;
        else
           sscanf(tinput,"%d",&nextlistedtest);
    }

    /* big loop to deal with each line just read from file, until EOF */
    while (!feof(fp))
    {

       /* get the next test from the testlist if necessary */
       if ( (selecttype == 4) || (selecttype == 5) )
       {
          if (testnum < nextlistedtest) 
          {
              if (selecttype == 4)
                 printit=0;
              else
                 printit=1;
          }
          else /* testnum=nextlistedtest */
          {
              if (selecttype == 4)
                 printit=1;
              else
                 printit=0;

              fgets(tinput,INPUTMAX,tfp);
              if (feof(tfp))
                 nextlistedtest=STOP;
              else
                 sscanf(tinput,"%d",&nextlistedtest);
          }
       } 

       /* look for the left angle bracket, from right end of string */
       for (langle=strlen(input)-1;input[langle]!='<'&&langle>=0;langle--);

       /* langle is -1 if there isn't a "<", else it gives the
          index into input for the "<" */

       /* if there's no angle bracket, the line just 
          lists parameters, no input file */
       if (langle<0)
          useinputfile = 0;
       else
          useinputfile = 1;

       if (useinputfile) /* is there is a "<", we have an fname after it */
       {                /* copy from after the "<" onward into fname */
          k=0;
          for (j=langle+1;j<strlen(input)&&input[j]!='\n';j++)
          {
             if (input[j]==' ') continue;
             fname[k] = input[j];
             k++;
          }
          fname[k] = '\0';
       }

       /* now, anything up to the "<", or to the endofline if there
          was no "<", is a param list.  */
       if (langle<0)
       {
          /* no angle bracket; fool the loop into thinking
          there was a "<" at the endofline char, and put
          a space in the "bracket" variable for use in the
          output statements that occur later */

          langle = strlen(input) - 1;
          strcpy(bracket," ");
       }
       else
       {

          /* there was a bracket; put it into "bracket" for use in outputs later */
          strcpy(bracket,"<");
       }

       /* now read out the parameter list */
       k=0;
       for (j=0;j<langle;j++)
       {
          params[k] = input[j];
          k++;
       }
       params[k] = '\0';

       /* make name of inputfile for this testnum, if there is one */
       if (useinputfile)
          sprintf(inpfile,"%s/inputs/%s",test_db_dir,fname);
       else
          strcpy(inpfile," ");

       /* make names of output file and new output file for this testnum */
       sprintf(outpfile,"%s/outputs/t%d",test_db_dir,testnum);
       sprintf(newoutpfile,"%s/newoutputs/t%d",test_db_dir,testnum);

       if (scripttype==1)   /* a runall script */
       {
          fprintf(nfp,"echo \">>>>>>>>running test %d\"\n",testnum);
          fprintf(nfp,"%s %s %s %s %s > %s 2>&1\n",virtmach, exename,params,bracket,inpfile,outpfile);
       }
       else if (scripttype==3) /* a tracing script */
       {
          fprintf(nfp,"echo \">>>>>>>>running test %d\"\n",testnum);
          fprintf(nfp,"%s %s %s %s %s > %s 2>&1\n", virtmach, exename,params,bracket,inpfile,newoutpfile);
          fprintf(nfp,"cp $ARISTOTLE_DB_DIR/%s.c.tr %s/traces/%d.tr\n",
                  progname,test_db_dir,testnum-1); 

          /* when testing the correctness of the tracer, we find it useful to
             check the output of the instrumented exe against the old output put out
             by a non-instrumented exe:  uncomment the next line */
         /*  fprintf(nfp,"cmp -s %s %s || echo different results\n",outpfile,newoutpfile);  */

       }
       else if (scripttype==2) /* a diffing script */
       {
          /* this script type is the type we generate for N or S type */

          if ( (selecttype == 0) || (printit == 1))
          {
             fprintf(nfp,"echo \">>>>>>>>running test %d\"\n",testnum);
             fprintf(nfp,"%s %s %s %s %s > %s 2>&1\n",virtmach, exename,params,bracket,inpfile,newoutpfile);
             fprintf(nfp,"cmp -s %s %s || echo different results\n",outpfile,newoutpfile);
          }
       }

       testnum++;

       /* get next line from plan or universe file */
       for (k=0;k<INPUTMAX;k++)
       {
          input[k] = '\0';
          params[k] = '\0';
          fname[k] = '\0';
       }
       fgets(input,INPUTMAX,fp); 
       if (input == NULL)
       {
          printf("Error on read from universe file.\n");
          fclose(fp);
          fclose(nfp);
          exit(-1);
       }

    }

    fclose(fp);
    fclose(nfp);
    exit(0);

}

