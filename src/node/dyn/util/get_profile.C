
       //=======================================================//    _\|/_
      //  __  _____           ___                    ___       //      /|\ ~
     //  /      |      ^     |   \  |         ^     |   \     //          _\|/_
    //   \__    |     / \    |___/  |        / \    |___/    //            /|\ ~
   //       \   |    /___\   |  \   |       /___\   |   \   // _\|/_
  //     ___/   |   /     \  |   \  |____  /     \  |___/  //   /|\ ~
 //                                                       //            _\|/_
//=======================================================//              /|\ ~

// profile: print out a cumulative mass profile for a system,
//	    relative to the tandard system center.
//.............................................................................
//    version 1:  June 1997, Steve McMillan
//.............................................................................

#include "dyn.h"

typedef struct {real radius;
		real mass;} rm;

int comp(rm *a, rm* b)
{
    if (a->radius < b->radius)
        return -1;
    else if (a->radius > b->radius)
        return 1;
    else
        return 0;
}

main(int argc, char ** argv)
{
    extern char *poptarg;
    int c;
    const char *param_string = "c";

    while ((c = pgetopt(argc, argv, "param_string",
		    "$Revision$", _SRC_)) != -1)
	switch(c) {

	    case 'c': break;
            case '?': params_to_usage(cerr, argv[0], param_string);
		      exit(1);
	}            

    dyn *b;

    while (b = get_dyn()) {

        real msum = 0;
	int i = 0, n = 0;

	vec pos, vel;
        get_std_center(b, pos, vel);

	pos -= b->get_pos();		// center includes pos of b...

	for_all_daughters(dyn, b, bb) n++;

	rm *sys = new rm[n];

	for_all_daughters(dyn, b, bb) {

	    sys[i].radius = square(bb->get_pos() - pos);
	    sys[i].mass = bb->get_mass();
	    
	    i++;
	}

	qsort(sys, (size_t) n, (size_t) sizeof(rm), comp);

	for (i = 0; i < n; i++) {
	    msum += sys[i].mass;
	    cerr << i << " " << sqrt(sys[i].radius) << " " << msum << endl;
	}

	rmtree(b);
	delete [] sys;
    }
}

