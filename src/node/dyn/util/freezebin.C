
//// freezebin:  reduce all top-level binary CM velocities, leaving
////             positions unchanged.
////
//// Options:     -c    add a comment to the output snapshot [false]
////              -f    specify freeze factor [0]

//   version 1:  Dec 1992   Piet Hut
//               Oct 2001   Steve McMIllan

#include "dyn.h"

#ifdef TOOLBOX

local void freezebin(dyn * b, real fac)
{
    dyn * bi;

    for_all_daughters(dyn, b, bi)
	if (bi->is_parent()) bi->scale_vel(fac);
}

main(int argc, char ** argv)
{
    bool  c_flag = FALSE;
    char  *comment;
    real fac = 0;

    check_help();

    extern char *poptarg;
    int c;
    char* param_string = "c:f:";

    while ((c = pgetopt(argc, argv, param_string)) != -1)
	switch(c) {

	    case 'c': c_flag = TRUE;
		      comment = poptarg;
		      break;
	    case 'f': fac = atof(poptarg);
	    	      break;
            case '?': params_to_usage(cerr, argv[0], param_string);
	              get_help();
                      exit(1);
        }            

    dyn *b;

    while (b = get_dyn(cin)) {
        if (c_flag == TRUE)
            b->log_comment(comment);
        b->log_history(argc, argv);

        freezebin(b, fac);
	put_dyn(cout, *b);	
	rmtree(b);
    }
}

#endif

// endof: freeze.C