
//// snap2speck:  dump out an N-body system in a dumb format suitable
////         	  for digestion by partiview.
////
//// Options:     -p    specify precision of output [6 sig. fig.]

#include "dyn.h"

#ifdef TOOLBOX

main(int argc, char** argv)
{
    dyn *root, *ni;

    check_help();

    extern char *poptarg;
    int c;
    char* param_string = "p:";

    int p = STD_PRECISION;

    while ((c = pgetopt(argc, argv, param_string)) != -1)
	switch(c)
	    {
	    case 'p': p = atoi(poptarg);
		      break;
            case '?': params_to_usage(cerr, argv[0], param_string);
	              get_help();
                      exit(1);
	    }

    cout.precision(p);

    while (root = get_dyn(cin)) {

	real n = 0;
	for_all_daughters(dyn, root, ni) n += 1;

	cout << "datavar 0 colorb_v" << endl;
	cout << "datavar 1 lum" << endl;

	for_all_daughters(dyn, root, ni)

	    cout << ni->get_pos() << " "
		 << ni->get_index()/n << " "	// colors 0 to 1
		 << ni->get_mass()
		 << endl;

	rmtree(root);
    }
}

#endif